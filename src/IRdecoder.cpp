#include <IRdecoder.h>

/* Interprets an IR remote with NEC encoding. See IRdecoder.h for more explanation. */

void handleIRsensor(void)
{
  decoder.handleIRsensor();
}

void IRDecoder::init(void)
{
  pinMode(irPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(irPin), ::handleIRsensor, CHANGE);
}

void IRDecoder::handleIRsensor(void)
{
  uint32_t currUS = micros();

  if(!digitalRead(irPin)) // FALLING edge
  {
    fallingEdge = currUS; 
  }

  else // RISING edge
  {
    risingEdge = currUS;

    //and process
    uint32_t delta = risingEdge - fallingEdge; //length of pulse, in us
    uint32_t codeLength = risingEdge - lastRisingEdge;
    lastRisingEdge = risingEdge;
    
    if(delta > 8500 && delta < 9500) // received a start pulse
    {
      index = 0;
      state = IR_PREAMBLE;
      return;
    }
    
    //a pulse is supposed to be 562.5 us, but I found that it averaged 620us or so
    //with the sensor that we're using, which is NOT optimized for IR remotes --
    //it's actually optimized for sensitivity. So I set the maximum accepted pulse
    //length to 700us. On the ESP32 (others?), the pulse is sometimes coming in at 51n us,
    //so lowering the bottom to 500.

    else if(delta < 500 || delta > 700) // pulse wasn't right length => error
    {
      state = IR_ERROR;
      return;
    }

    else if(state == IR_PREAMBLE)
    {
      if(codeLength < 5300 && codeLength > 4800) //preamble
      {
        currCode = 0;
        state = IR_ACTIVE;
      }

      else if(codeLength < 3300 && codeLength > 2800) //repeat code
      {
        state = IR_REPEAT;
        if(((currCode ^ (currCode >> 8)) & 0x00ff0000) != 0x00ff0000) {state = IR_ERROR;} //but recheck code!
        else lastReceiveTime = millis(); //not really used
      }
    }

    else if(state == IR_ACTIVE)
    {
      if(codeLength < 1300 && codeLength > 900) //short = 0
      {
        index++;
      }
      
      else if(codeLength < 2500 && codeLength > 2000) //long = 1
      {
        currCode += ((uint32_t)1 << index);
        index++;
      }
      
      else //error
      {
        state = IR_ERROR;
      }

      if(index == 32) //full set of bits
      {
        //first, check for errors (kinda' ugly, but it works)
        if(((currCode ^ (currCode >> 8)) & 0x00ff0000) != 0x00ff0000) {state = IR_ERROR;}

        else //we're good to go
        {        
          state = IR_COMPLETE;
          lastReceiveTime = millis();
        }
      }
    }
  }
}