# IRdecoder
 
A simple IR decoder for NEC-encoded IR transmitters. It was tested with the Vishay
TSSP58038, which is not optimized for IR remotes, which may explain why the pulses are a
little longer than spec. Change if needed.

It should work on any microcontroller with an external interrupt, more or less.
