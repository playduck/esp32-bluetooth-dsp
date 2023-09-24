# ESP32 BT DSP

Codebase for an ESP32 Bluetooth Speaker with DSP functionality.


## Hardware

This project requires an external DAC.
I've been using a TI PCM5102A (because of course I am)

The PCM5102 outputs a nice line level signal, however an amplifier is required to drive speakers.
I swear I'm not a fanboy, but I'd recommend the TPA3116.

## Software

This version uses esp-idf-v5.1.1.
It was originally written for esp-idf v4.4 and thus uses legacy (pre v8.0) FreeRTOS kernel types and functions.
Make sure to enable:
`COMPONENT CONFIG > FreeRTOS > Kernel > configENABLE_BACKWARD_COMPATIBILITY`


This is based on the esp-idf a2dp sink example.
Also inspired by [YetAnotherElectronicsChannel's Project(s)](https://github.com/YetAnotherElectronicsChannel/ESP32_Bluetooth_Audio_Receiver).

### DSP

DSP functionality has been moved to a [separate repository](https://github.com/playduck/dsp-playground).

# TODO

- different NLD
- hardware volume control (potentiometer ICs)
- PCB (with assembly?)
- filter/parameter tweaking at runtime (rotary encoder and rgb led?)
- ADC integration and automatic source switching
