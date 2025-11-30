#include "ProjectTemplateStructDTO.h"

#define PRINT_RECV_DTO_DATA
#define SEND_DTO_DATA

void updateDTO(CommandIdEnum commandId, const RxDTO &rxDTO) {
    printDTO(commandId, rxDTO);
}

void delDTO(CommandIdEnum commandId, const RxDTO &rxDTO) {
    printDTO(commandId, rxDTO);
}

void printDTO(CommandIdEnum commandId, const RxDTO &rxDTO) {
#ifdef PRINT_RECV_DTO_DATA
    Serial.print("consumeLine ");
    Serial.print(", commandId: ");
    Serial.print(commandIdToStr(commandId));
    Serial.print(", sendTimeMs: ");
    Serial.print(rxDTO.sendTimeMs);
    Serial.print(", b: ");
    Serial.print(rxDTO.b ? "true" : "false");
    Serial.print(", i: ");
    Serial.print(rxDTO.i);
    Serial.print(", l: ");
    Serial.print(rxDTO.l);
    Serial.print(", f: ");
    Serial.print(rxDTO.f);
    Serial.print(", d: ");
    Serial.print(rxDTO.d);
    Serial.print(", u: ");
    Serial.print(rxDTO.u);
    Serial.println();
#endif
}

CommandUart<DTOFrameWrapper> commandUart([]() {
    Serial1.begin(115200, SERIAL_8N1, MP_D4, MP_D3);
    delay(200);
    return &Serial1;
});

void setup() {
#ifdef LOG_DEBUG
    Serial.begin(115200);
#endif
    commandUart.start(0, 3, -1, true);
}

void sendData() {
    static unsigned long lastSendTimeMs = millis();
    if (millis() - lastSendTimeMs > 1000) {
        lastSendTimeMs = millis();
        const RxDTO rxDTO = {
            .commandId = static_cast<CommandIdEnum>(lastSendTimeMs % 3),
            .sendTimeMs = lastSendTimeMs,
            .b = true,
            .i = 1,
            .l = 2,
            .f = 3.0,
            .d = 4.0,
            .u = 5,
        };
        const auto p = TypeArrayUtils<RxDTO>::obj2BytesPoint(rxDTO);
        commandUart.sendData(p.first, p.second);
        Serial.print("Sending data:");
        Serial.println(lastSendTimeMs);
    }
}

void loop() {
    RTOSLooperMessageBuffer::instance().loop();
#ifdef SEND_DTO_DATA
    sendData();
#endif
}
