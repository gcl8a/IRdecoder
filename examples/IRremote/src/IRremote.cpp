#include <IRdecoder.h>

IRDecoder decoder(0);

void setup(void)
{
    Serial.begin(115200);
    decoder.init();
}

void loop(void)
{
    int16_t keyCode = decoder.getKeyCode();
    if(keyCode != -1) Serial.println(keyCode);
}