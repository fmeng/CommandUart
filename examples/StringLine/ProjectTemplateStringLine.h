#pragma once

#define COMMAND_ID_LIST \
X(COMMAND_UART_STRING_LINE)         /*串口行数据*/  \
X(MAX_HOLDER_COUNT)

#include <Arduino.h>
#include "CommandIdEnum.h"
#include "EspMapping.h"
#include "rtos/RTOSLooperMessageBuffer.h"
#include "TypeArrayUtils.h"
#include "CommandUart.h"

void consumeLine(CommandIdEnum commandId, const String &line);

inline void consumeCommand(CommandIdEnum commandId, const uint8_t *p_commandValue, size_t valueSize) {
    const String line = TypeArrayUtils<String>::bytes2ObjCopy(p_commandValue, valueSize);
    consumeLine(commandId, line);
}

REGISTER_COMMAND_HANDLER(CommandIdEnum::COMMAND_UART_STRING_LINE, consumeCommand);

/************************** 内部实现 **************************/

class StringLineFrameWrapper : public FrameWrapper<String, StringLineFrameWrapper> {
public:
    /**
     * 查找帧头
     *
     * @param p_buff
     * @param size
     * @return 数据帧头索引(第一个填充frame对象的有效索引数据、包含), 小于0未找到索引
     */
    int findFrameHeadIncludeIndex(const uint8_t *const p_buff, const size_t size) const {
        int i = 0;
        for (; i < size; i++) {
            if (p_buff[i] != '\n' && p_buff[i] != '\r') {
                break;
            }
        }
        // 全部都是 \n 或 \r
        if (i == size) {
            return -1;
        }
        return i;
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
        for (size_t i = skipIndex; i < size; i++) {
            if (p_buff[i] == '\n' || p_buff[i] == '\r') {
                return i - 1;
            }
        }
        return -1;
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
        int len = static_cast<int>(tailIncludeIndex - headIncludeIndex + 1);
        commandId = COMMAND_UART_STRING_LINE;
        frame = String(reinterpret_cast<const char *>(p_buff + headIncludeIndex), len);
        return len;
    }

    /**
     * frame数据变换结构传送给loop的message buffer
     *
     * @return @return <数组指针, 数组大小>
     */
    std::pair<const uint8_t * const, size_t> toLoopMessageBufferData() const {
        static std::vector<uint8_t> resBuff;
        resBuff.clear();

        resBuff.push_back(static_cast<uint8_t>(commandId));
        resBuff.insert(resBuff.end(), frame.c_str(), frame.c_str() + frame.length());
        resBuff.push_back('\0');

        return std::make_pair(resBuff.data(), resBuff.size());
    }
};
