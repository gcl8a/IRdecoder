#include <IRdecoder.h>
#include <PCint.h>

/* ISR */
void handleIRsensor(void)
{
  decoder.handleIRsensor();
}

/**
 * Initializes the decoder for a specific pin.
 * 
 * @param p The pin to use for decoding. Can be any external or pin change interrupt.
 * */
void IRDecoder::init(void)
{
  pinMode(pin, INPUT);

  if(digitalPinToInterrupt(pin) != NOT_AN_INTERRUPT)
  {
    Serial.println(F("Attaching ISR"));
    attachInterrupt(digitalPinToInterrupt(pin), ::handleIRsensor, CHANGE);
  }
  else if(digitalPinToPCInterrupt(pin) != NOT_AN_INTERRUPT)
  {
    Serial.print(F("Attaching PC_ISR to PCINT"));
    Serial.println(digitalPinToPCInterrupt(pin));
    attachPCInt(digitalPinToPCInterrupt(pin), ::handleIRsensor);
  }
  else
  {
    Serial.println(F("Not an interrupt pin!"));
  }
}

/**
 * Returns the most recent key code; returns -1 on error 
 * (can't use 0, since many remotes send 0 for a button.)
 * */
int16_t IRDecoder::getKeyCode(bool acceptRepeat) 
{
  if (state == IR_COMPLETE || (acceptRepeat == true && state == IR_REPEAT))
  {
    //check for errors
    if(((currCode ^ (currCode >> 8)) & 0x00ff0000) != 0x00ff0000) 
    {
      state = IR_ERROR;
      return -1;
    }

    else //no errors, so we're good to go
    {        
      state = IR_READY;
      return (currCode >> 16) & 0x0ff;
    }
  }
  else
    return -1;
}

/**
 * Returns the most recent key code; returns -1 on error 
 * (can't use 0, since many remotes send 0 for a button.)
 * */
bool IRDecoder::get32BitCode(uint32_t& code) 
{
  if (state == IR_COMPLETE)
  {
    //check for errors by XOR'ing the first three bytes and then comparing to a checksum
    if((uint8_t)(currCode >> 24) ^ (uint8_t)(currCode >> 16) ^ (uint8_t)(currCode >> 8) ^ (uint8_t)(currCode >> 0))
    {
      state = IR_ERROR;
      return false;
    }

    else //no errors, so we're good to go
    {        
      state = IR_READY;
      code = currCode;
      return true;
    }
  }
  else
    return false;
}

/**
 * Called from the ISR to interpret codes.
 * */
void IRDecoder::handleIRsensor(void)
{
  uint32_t currUS = micros();

  if(!digitalRead(pin)) // FALLING edge
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

    // bits[index] = delta; //used for debugging; obsolete
    
    if(delta > 8500 && delta < 9500) // received a start pulse
    {
      index = 0;
      state = IR_PREAMBLE;
      return;
    }
    
    //a pulse is supposed to be 562.5 us, but I found that it averaged 620us or so
    //with the sensor that we're using, which is NOT optimized for IR remotes --
    //it's actually optimized for sensitivity. So I set the maximum accepted pulse
    //length to 700us

    else if(delta < 500 || delta > 700) // if the pulse isn't the right width -> set error
    {
      state = IR_ERROR;
      currCode = -1;

      return;
    }

    else if(state == IR_PREAMBLE)
    {
      if(codeLength < 5300 && codeLength > 4800) //preamble
      {
        currCode = 0;
        state = IR_ACTIVE;
      }

      else if(codeLength < 3300 && codeLength > 2700) //repeat code
      {
        state = IR_REPEAT;
        if(((currCode ^ (currCode >> 8)) & 0x00ff0000) != 0x00ff0000) {state = IR_ERROR;} 
        lastReceiveTime = millis(); //not really used
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
          state = IR_COMPLETE;
          lastReceiveTime = millis(); //not actually used
      }
    }
  }
}