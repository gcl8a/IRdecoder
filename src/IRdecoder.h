#pragma once 

#include <Arduino.h>

/** \class IRDecoder
 * A class to interpret IR remotes with NEC encoding.
 * 
 * IRdecoder decoder is declared as an extern. The user only need call init(pin) in setup().
 * 
 * NEC encoding sends four bytes in little endian:
 *  device ID
 *  ~device ID
 *  key code
 *  ~key code
 * 
 * using pulse distance coding, but note that many remotes (inkl. the one from Pololu)
 * use a 16-bit device code, so only the key code is inversed.
 * 
 * Sending the inverse allow for easy error checking (and reduces saturation in the receiver).
 * 
 * Codes are sent little endian; this library fills uint32 currCode from the LSB upon reception, 
 * That means that the key code is found in currCode[23..16] and the device ID in currCode[7..0].
 * 
 * This library currently ignores the device ID and does not check it against ~device ID for errors
 * since many remotes don't follow that particular protocol.
 * 
 * For an overview of NEC coding, see: 
 * https://techdocs.altium.com/display/FPGA/NEC+Infrared+Transmission+Protocol
 * 
 * The user can choose whether or not to ignore repeat signals.
 * 
 * The user can also just fetch the 32-bit code and do whatever they want with it.
 * 
 * This does not interpret the codes as a particular key press. That needs to be 
 * mapped on a remote by remote basis. See, for example, ir_codes.h
 * 
 * Note that the code only returns a code once it has received one. If you call a check function
 * twice in rapid succession, the second call will return -1, since no new code is available.
 */

class IRDecoder
{
private:
  int8_t pin = -1;

  enum IR_STATE
  {
    IR_READY,    //idle, returns to this state after you request a code
    IR_PREAMBLE, //received the start burst, waiting for first bit
    IR_REPEAT,   //received repeat code (part of NEC protocol); last code will be returned
    IR_ACTIVE,   //have some bits, but not yet complete
    IR_COMPLETE, //a valid code has been received
    IR_ERROR     //an error occurred; won't return a valid code
  }; 

  volatile IR_STATE state = IR_READY; //a simple state machine for managing reception

  // volatile uint32_t lastReceiveTime = 0; //not actually used -- could be used to sunset codes

  volatile uint32_t currCode = 0; //the most recently received valid code
  volatile uint8_t index = 0;     //for tracking which bit we're on

  volatile uint32_t fallingEdge = 0;
  volatile uint32_t risingEdge = 0;

  //used for tracking spacing between rising edges, i.e., bit value
  volatile uint32_t lastRisingEdge = 0;

public:
#ifdef __DEBUG_IR__
  volatile uint16_t bits[32];  //I used this for debugging; obsolete
#endif

public:
  IRDecoder(void) {}
  bool init(const uint8_t p);      // Must be called in setup()
  void handleIRsensor(void);      // Called by the ISR

  bool getKeyCode(uint8_t& keyCode, bool acceptRepeat = false); 
  bool get32BitCode(uint32_t& code, bool acceptRepeat = false);
};

extern IRDecoder decoder;