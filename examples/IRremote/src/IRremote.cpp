/**
 * Example code for how to use the IR decoder library.
 * 
 * Note that decoder is declared in the library, so we don't declare it here.
*/

#include <IRdecoder.h>

void setup(void)
{
    Serial.begin(115200);
    if(!decoder.init(17)) 
    {
        while(1)
        {
            Serial.println("Error in IRDecoder::init().");
            delay(500);
        }
    }
}

void loop(void)
{
    uint8_t keyCode = 0;
    if(decoder.getKeyCode(keyCode)) // add true to accept repeats
    {
        Serial.print(millis());
        Serial.print('\t');
        Serial.print(keyCode);
        Serial.print('\n');
    }
}