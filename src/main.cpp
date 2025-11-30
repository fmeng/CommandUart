#include <Arduino.h>
#include "ProjectTemplateSu03T.h"

#define PRINT_RECV_DTO_DATA
#define SEND_DTO_DATA

CommandUart<Su03TFrameWrapper> commandUart([]() {
    Serial1.begin(115200, SERIAL_8N1, MP_D4, MP_D3);
    delay(200);
    return &Serial1;
});

// #define LIGHT_COMMAND_ID 1
void consumeLight(CommandIdEnum commandId, const LightCommand &command) {
#ifdef PRINT_RECV_DTO_DATA
    Serial.print("consumeLight, ");
    Serial.print(", commandId: ");
    Serial.print(commandIdToStr(commandId));
    Serial.print(", command.open: ");
    Serial.print(command.open);
    Serial.println();
#endif
}

// #define CAR_COMMAND_ID 2
void consumeCar(CommandIdEnum commandId, const CarCommand &command) {
#ifdef PRINT_RECV_DTO_DATA
    Serial.print("consumeCar, ");
    Serial.print(", commandId: ");
    Serial.print(commandIdToStr(commandId));
    Serial.print(", command.direction: ");
    Serial.print(command.direction);
    Serial.print(", command.speed: ");
    Serial.print(command.speed);
    Serial.println();
#endif
}

void sendCommand(uint8_t messageId, uint8_t *p_content, size_t size) {
    const size_t messageSize = 2/*head*/ + 1/*id*/ + size + 2/*tail*/;
    uint8_t buffer[messageSize];
    buffer[0] = 0xAA;
    buffer[1] = 0x55;
    buffer[2] = messageId;
    memcpy(buffer + 3, p_content, size);
    buffer[messageSize - 2] = 0x55;
    buffer[messageSize - 1] = 0xAA;
    commandUart.sendData(buffer, messageSize);
}

void sendCommand(uint8_t messageId, long value) {
    sendCommand(messageId, reinterpret_cast<uint8_t *>(&value), sizeof(long));
}

void sendCommand(uint8_t messageId, double value) {
    sendCommand(messageId, reinterpret_cast<uint8_t *>(&value), sizeof(double));
}

void setup() {
#ifdef LOG_DEBUG
    Serial.begin(115200);
#endif
    commandUart.start(0, 3, -1, true);
}

void sendData() {
    static unsigned long lastSendTimeMs = millis();

    if (millis() - lastSendTimeMs > 3000) {
        lastSendTimeMs = millis();
        sendCommand(3, static_cast<long>(lastSendTimeMs / 2));
        Serial.print("Sending data long:  ");
        Serial.println(lastSendTimeMs);
    }

    // if (millis() - lastSendTimeMs > 1000) {
    //     lastSendTimeMs = millis();
    //     sendCommand(4, static_cast<double>(lastSendTimeMs) / 2);
    //     Serial.print("Sending data double:");
    //     Serial.println(lastSendTimeMs);
    // }
}

void loop() {
    RTOSLooperMessageBuffer::instance().loop();
#ifdef SEND_DTO_DATA
    sendData();
#endif
}
