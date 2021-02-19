# ESP32 BT DSP

Codebase for an ESP32 Bluetooth Speaker with DSP functionality.

## Hardware

This project requires an external DAC.
I've been using a TI PCM5102 (because ofcourse i am)

Also to connect speakers an amplifier is nessaccery.
I swear I'm not a fanboy but I'd recommend the TPA3116.

## Software

This is based on the esp-idf a2dp sink example.
Also inspired by YetAnotherElectronicsChannel's Project.

### DSP

DSP functionality includes:

- Support for different Sample rates
- IIR Filters
  - different parameters can be recalculated "on the fly"
  - support for different types of filters (lowpass, highpass, notch, etc.)
  - cascded filters
- Delay line for time alignment
- Virtual Bass enhancement
  - using the psychoaccustic missing fundamental effect
  - implemented through iir filters and an Non-Linear-Device (NLD) for harmonic distortion
- Volume Control and linear transformation

# TODO

- different NLD
- hardware volume control (potentiometer ICs)
- compressor / soft clipping
- PCB
- filter/parameter tweaking at runtime (roatry encoder and rgb led?)
- testing if FIR filters are feasable
- figure out why ESP32 brownouts with extternal power supplies

