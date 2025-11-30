#pragma once

#define COMMAND_ID_LIST \
X(COMMAND_UART_RX_DTO_UPDATE)         /*update*/  \
X(COMMAND_UART_RX_DTO_DEL)         /*del*/  \
X(COMMAND_UART_RX_DTO_PRINT)         /*print*/  \
X(MAX_HOLDER_COUNT)

#include <Arduino.h>
#include "CommandIdEnum.h"
#include "EspMapping.h"
#include "rtos/RTOSLooperMessageBuffer.h"
#include "TypeArrayUtils.h"
#include "CommandUart.h"

#pragma pack(push, 1)  //// 确保结构体不会有额外填充, 开始 1 字节对齐
struct RxDTO {
    CommandIdEnum commandId;
    unsigned long sendTimeMs;
    bool b;
    int i;
    long l;
    float f;
    double d;
    uint8_t u;
};
#pragma pack(pop)  //// 确保结构体不会有额外填充, 恢复默认对齐

void updateDTO(CommandIdEnum commandId, const RxDTO &rxDTO);

void delDTO(CommandIdEnum commandId, const RxDTO &rxDTO);

void printDTO(CommandIdEnum commandId, const RxDTO &rxDTO);


inline void consumeCommand(CommandIdEnum commandId, const uint8_t *p_commandValue, size_t valueSize) {
    const RxDTO rxDTO = TypeArrayUtils<RxDTO>::bytes2ObjCopy(p_commandValue, valueSize);
    switch (rxDTO.commandId) {
        case COMMAND_UART_RX_DTO_UPDATE:
            updateDTO(rxDTO.commandId, rxDTO);
            break;
        case COMMAND_UART_RX_DTO_DEL:
            delDTO(rxDTO.commandId, rxDTO);
            break;
        case COMMAND_UART_RX_DTO_PRINT:
            printDTO(rxDTO.commandId, rxDTO);
            break;
        default:
            printDTO(rxDTO.commandId, rxDTO);
    }
}

REGISTER_COMMAND_HANDLER(CommandIdEnum::COMMAND_UART_RX_DTO_UPDATE, consumeCommand);

REGISTER_COMMAND_HANDLER(CommandIdEnum::COMMAND_UART_RX_DTO_DEL, consumeCommand);

REGISTER_COMMAND_HANDLER(CommandIdEnum::COMMAND_UART_RX_DTO_PRINT, consumeCommand);

/************************** 内部实现 **************************/

class DTOFrameWrapper final : public FrameWrapper<RxDTO, DTOFrameWrapper> {
    using dto_type = RxDTO;
    using dto_frame_type = DTOFrameWrapper;

public:
    /**
     * 查找帧头
     *
     * @param p_buff
     * @param size
     * @return 数据帧头索引(第一个填充frame对象的有效索引数据、包含), 小于0未找到索引
     */
    int findFrameHeadIncludeIndex(const uint8_t *const p_buff, const size_t size) const {
        if (size < sizeof(dto_type)) {
            return -1;
        }
        return 0;
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
        if (size < sizeof(dto_type)) {
            return -1;
        }
        return sizeof(dto_type) - 1;
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
        memcpy(&this->frame, p_buff + headIncludeIndex, sizeof(dto_type));
        this->commandId = this->frame.commandId;
        return sizeof(dto_type);
    }

    /**
     * frame数据变换结构传送给loop的message buffer
     *
     * @return @return <数组指针, 数组大小>
     */
    std::pair<const uint8_t * const, size_t> toLoopMessageBufferData() const {
        static uint8_t buf[1 + sizeof(dto_type)];
        buf[0] = static_cast<uint8_t>(commandId);
        memcpy(buf + 1, &frame, sizeof(dto_type));
        return std::make_pair(buf, sizeof(buf));
    }
};
