#include "ProjectTemplateStringLine.h"

void consumeLine(CommandIdEnum commandId, const String &line) {
    Serial.print("consumeLine ");
    Serial.print(", commandId: ");
    Serial.print(commandIdToStr(commandId));
    Serial.print(", line: ");
    Serial.print(line);
    Serial.println();
}

CommandUart<StringLineFrameWrapper> commandUart([]() {
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

void sendDataLine(const String &data) {
    const String dataLine = data + "\n";
    commandUart.sendData(reinterpret_cast<const uint8_t *>(dataLine.c_str()), dataLine.length());
}

void loop() {
    RTOSLooperMessageBuffer::instance().loop();
    static unsigned long lastSendTimeMs = millis();
    if (millis() - lastSendTimeMs > 1000) {
        String data = String(millis());
        sendDataLine(data);
        lastSendTimeMs = millis();
        Serial.print("Sending data:");
        Serial.println(data);
    }
}
