#pragma once

#define COMMAND_ID_LIST \
X(COMMAND_UART_LIGHT)     /*light*/  \
X(COMMAND_UART_CAR)         /*car*/  \
X(MAX_HOLDER_COUNT)

#include <Arduino.h>
#include <utility>
#include <vector>
#include "CommandIdEnum.h"
#include "EspMapping.h"
#include "rtos/RTOSLooperMessageBuffer.h"
#include "TypeArrayUtils.h"
#include "CommandUart.h"

struct MessageWrapper;

#pragma pack(push, 1)  //// 确保结构体不会有额外填充, 开始 1 字节对齐

enum MessageIdEnum : uint8_t {
    LIGHT_COMMAND_ID = 1,
    CAR_COMMAND_ID = 2,
};

struct LightCommand {
    // bool|char(1byte), long(4byte), double(8byte)
    bool open;
};

struct CarCommand {
    // bool|char(1byte), long(4byte), double(8byte)
    long direction;
    double speed;
};

#pragma pack(pop)  //// 确保结构体不会有额外填充, 恢复默认对齐

void consumeLight(CommandIdEnum commandId, const LightCommand &command);

void consumeCar(CommandIdEnum commandId, const CarCommand &command);

std::map<uint8_t, MessageWrapper *> messageIdToWrapperMap;

struct MessageWrapper {
    virtual ~MessageWrapper() = default;

    uint8_t messageId;
    CommandIdEnum commandId;

    /**
     * no header AA 55, no tail 55 AA
     * AA 55 01 02 00 00 00 55 AA
     */
    void *p_frame;

    size_t frameSize;

    MessageWrapper() : messageId(0), commandId(static_cast<CommandIdEnum>(0)), p_frame(nullptr), frameSize(0) {
    }

    MessageWrapper(const uint8_t messageId, const CommandIdEnum commandId, void *p_frame, size_t frameSize)
        : messageId(messageId), commandId(commandId), p_frame(p_frame), frameSize(frameSize) {
        messageIdToWrapperMap[messageId] = this;
    }
};

struct LightCommandWrapper : MessageWrapper {
    LightCommandWrapper()
        : MessageWrapper(LIGHT_COMMAND_ID, COMMAND_UART_LIGHT, new LightCommand(), sizeof(LightCommand)) {
    }
} lightCommandWrapper;

struct CarCommandWrapper : MessageWrapper {
    CarCommandWrapper()
        : MessageWrapper(CAR_COMMAND_ID, COMMAND_UART_CAR, new CarCommand(), sizeof(CarCommand)) {
    }
} carCommandWrapper;

REGISTER_COMMAND_HANDLER(CommandIdEnum::COMMAND_UART_LIGHT,
                         [](CommandIdEnum commandId, uint8_t *p_commandValue, size_t valueSize) {
                         const LightCommand *p = TypeArrayUtils<LightCommand>::bytes2ObjPoint(p_commandValue,valueSize);
                         consumeLight(commandId, *p);
                         });

REGISTER_COMMAND_HANDLER(CommandIdEnum::COMMAND_UART_CAR,
                         [](CommandIdEnum commandId, uint8_t *p_commandValue, size_t valueSize) {
                         const CarCommand *p = TypeArrayUtils<CarCommand>::bytes2ObjPoint(p_commandValue,valueSize);
                         consumeCar(commandId, *p);
                         });

/************************** 内部实现 **************************/

// 调试使用
inline void printHex(const uint8_t *p_buff, size_t size) {
    for (size_t i = 0; i < size; i++) {
        if (p_buff[i] < 0x10) Serial.print('0'); // 补 0
        Serial.print(p_buff[i], HEX);
        Serial.print(' ');
    }
    Serial.println();
}

class Su03TFrameWrapper final : public FrameWrapper<MessageWrapper, Su03TFrameWrapper> {
public:
    /**
     * 查找帧头
     *
     * @param p_buff
     * @param size
     * @return 数据帧头索引(第一个填充frame对象的有效索引数据、包含), 小于0未找到索引
     */
    int findFrameHeadIncludeIndex(const uint8_t *const p_buff, const size_t size) const {
        if (size <= 4) {
            // head AA 55, tail 55 AA
            return -1;
        }
        int startIndex = -1;
        int i = 0;
        int j = 1;
        while (j < size) {
            if (p_buff[i] == 0xAA && p_buff[j] == 0x55) {
                startIndex = j + 1;
                break;
            }
            i++;
            j++;
        }
        if (startIndex < -1 || startIndex >= size) {
            return -1;
        }
        return startIndex;
    }

    /**
     * 查找帧尾
     *
     * @param p_buff
     * @param size
     * @param skipIndex
     * @return 数据帧头索引(最后一个填充frame对象的有效索引数据、包含), 小于0未找到索引
     */
    int findFrameTailIncludeIndex(const uint8_t *const p_buff, const size_t size, const size_t skipIndex) const {
        // head AA 55, tail 55 AA
        int endIndex = -1;
        int i = static_cast<int>(skipIndex);
        int j = static_cast<int>(skipIndex) + 1;
        while (j < size) {
            if (p_buff[i] == 0x55 && p_buff[j] == 0xAA) {
                endIndex = j;
                break;
            }
            i++;
            j++;
        }
        return endIndex;
    }

    /**
     * 使用字节数组构建frame对象
     *
     * @param p_buff
     * @param size
     * @param headIncludeIndex
     * @param tailIncludeIndex
     * @return 返回填充对象消耗的字节数量，小于0未消耗任何字节
     */
    int fillFrameData(const uint8_t *const p_buff, const size_t size, const size_t headIncludeIndex,
                      const size_t tailIncludeIndex) {
        const uint8_t messageId = p_buff[headIncludeIndex];
        auto *wrapper = messageIdToWrapperMap[messageId];
        memcpy(wrapper->p_frame, p_buff + headIncludeIndex + sizeof(messageId), wrapper->frameSize);
        frame.commandId = wrapper->commandId;
        frame.messageId = wrapper->messageId;
        frame.p_frame = wrapper->p_frame;
        frame.frameSize = wrapper->frameSize;
        return frame.frameSize;
    }

    /**
     * frame数据变换结构传送给loop的message buffer
     *
     * @return @return <数组指针, 数组大小>
     */
    std::pair<const uint8_t * const, size_t> toLoopMessageBufferData() const {
        static std::vector<uint8_t> buff;
        buff.clear();
        buff.resize(frame.frameSize + 1);
        buff[0] = frame.commandId;
        memcpy(buff.data() + 1, frame.p_frame, frame.frameSize);
        return std::make_pair(buff.data(), buff.size());
    }
};
