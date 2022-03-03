# ESP32 BT DSP

Codebase for an ESP32 Bluetooth Speaker with DSP functionality.

## Hardware

This project requires an external DAC.
I've been using a TI PCM5102A (because ofcourse i am)

The PCM5102 outputs a nice line level signal, however an amplifier is required to drive speakers.
I swear I'm not a fanboy but I'd recommend the TPA3116.

## Software

This is based on the esp-idf a2dp sink example.
Also inspired by [YetAnotherElectronicsChannel's Project(s)](https://github.com/YetAnotherElectronicsChannel/ESP32_Bluetooth_Audio_Receiver).

### DSP

DSP functionality has been moved to a [seperate repository](https://github.com/playduck/dsp-playground).

# TODO

- different NLD
- hardware volume control (potentiometer ICs)
- PCB (with assembly?)
- filter/parameter tweaking at runtime (roatry encoder and rgb led?)
- ADC integration and automatic source switching
