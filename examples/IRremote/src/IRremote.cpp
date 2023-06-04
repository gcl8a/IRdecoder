#include <IRdecoder.h>

IRDecoder decoder;

void setup(void)
{
    Serial.begin(115200);
    decoder.init(0);
}

void loop(void)
{
    int16_t keyCode = decoder.getKeyCode();
    if(keyCode != -1) Serial.println(keyCode);
}