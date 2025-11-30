#pragma once
#include "rtos/RTOSStreamTransport.h"
#include "rtos/RTOSSupplyUtils.h"

template<typename F>
class CommandUart : public RTOSStreamTransport<F> {
    using InitStreamFun = std::function<Stream *()>;

public:
    explicit CommandUart(InitStreamFun initStreamFun,
                           uint16_t const rxBufferOneMsgMaxSize = DEFAULT_RX_BUFFER_ONE_MSG_MAX_SIZE,
                           const uint16_t txBufferOneMsgMaxSize = DEFAULT_TX_BUFFER_ONE_MSG_MAX_SIZE,
                           const uint16_t txBufferAllMsgMaxSize = DEFAULT_TX_BUFFER_ALL_MSG_MAX_SIZE)
        : RTOSStreamTransport<F>("commandUartTask", [initStreamFun, this]() { this->p_stream = initStreamFun(); },
                                 rxBufferOneMsgMaxSize, txBufferOneMsgMaxSize, txBufferAllMsgMaxSize) {
    }

protected:
    Stream *p_stream = nullptr;

    int available() override {
        return p_stream->available();
    }

    size_t readBytes(uint8_t *buffer, size_t length) override {
        return p_stream->readBytes(buffer, length);
    }

    size_t write(const uint8_t *buffer, size_t size) override {
        return p_stream->write(buffer, size);
    }
};
