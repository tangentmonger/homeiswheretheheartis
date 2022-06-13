Home Is Where The Heart Is
==========================

Code for an interactive art project by Gabriel.

Notes
-----

Hardware: Arduino Uno with breakbeam sensors on GPIOs 2-4, normally low. Output over USB serial.

The version on the main branch communicates over MIDI, channel 1. Although it's got all the logic for reacting to breakbeam events, it's currently just sending a MIDI message every second for testing.

The version on branch use_osc instead sends an OSC message every second for testing. Had that worked, the next step would have been to swap out the rest of the MIDI sending code for OSC sending code.


