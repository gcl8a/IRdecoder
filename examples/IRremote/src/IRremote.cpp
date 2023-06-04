/**
 * Example code for how to use the IR decoder library.
 * 
 * Note that decoder is declared in the library, so we don't declare it here.
*/

#include <IRdecoder.h>

void setup(void)
{
    Serial.begin(115200);
    if(!decoder.init(0)) Serial.println("Error in IRDecoder::init().");
}

void loop(void)
{
    int16_t keyCode = decoder.getKeyCode();
    if(keyCode != -1) Serial.println(keyCode);
}