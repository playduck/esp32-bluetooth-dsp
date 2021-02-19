#pragma once
#include "definitions.h"

#define FILTER_AMT 10
#define MAX_DELTA 20
#define DELAY_LENGTH 512
// #define DELAY_LENGTH 4096

extern bool use_bass_enhancement;

typedef enum
{
   lowpass,
   highpass,
   bandpass,
   notch,
   peak,
   lowshelf,
   highshelf,
   none
} filter_type_t;

typedef struct iir_filter
{
   filter_type_t type;
   float in_z1;
   float in_z2;
   float out_z1;
   float out_z2;
   float a0;
   float a1;
   float a2;
   float b1;
   float b2;
} iir_filter_t;

iir_filter_t filters[FILTER_AMT];

void flush();
float process_iir(float, iir_filter_t *);
void process_data(uint8_t *, size_t);

iir_filter_t generate_biquad(filter_type_t type, float Fc, float Fs, float Q, float peakGain);
void set_delay_distance(uint8_t distance_in_cm);
void set_delay_ms(uint8_t ms);
void set_sample_rate(uint16_t sample_rate);

// for future reference
// https://www.micromodeler.com/dsp/
