#include "definitions.h"

// config
uint16_t sample_rate = 44100;
bool use_bass_enhancement = true;

// volume
uint8_t current_volume = 0;

// processing
int16_t *samples = 0;
int16_t *outsample = 0;
float insample = 0;
float lowsample = 0;
float highsample = 0;

// delay line
uint8_t delay_ms = 0;
static int16_t delay_line[DELAY_LENGTH] = {0};
uint16_t write_pointer = 0;
int16_t read_offset = 0;

// ndl
float nld_previous = 0.0;

void write_delay_line(int16_t sample)
{
    delay_line[write_pointer] = sample;
    write_pointer = (write_pointer + 1) % (DELAY_LENGTH);
}

int16_t read_delay_line()
{
    return delay_line[write_pointer < read_offset ? (DELAY_LENGTH - 1) + write_pointer - read_offset
                                                  : write_pointer - read_offset];
}

void flush_delay_line()
{
    for (uint16_t i = 0; i < DELAY_LENGTH; i++)
    {
        delay_line[i] = 0;
    }
    write_pointer = 0;
}

void flush_filters()
{
    for (uint8_t i = 0; i < FILTER_AMT; i++)
    {
        filters[i].in_z1 = 0.0;
        filters[i].in_z2 = 0.0;
        filters[i].out_z1 = 0.0;
        filters[i].out_z2 = 0.0;
    }
}

void flush()
{
    flush_filters();
    flush_delay_line();
}

inline float process_iir(float inSampleF, iir_filter_t *config)
{
    // applying coefficients
    float outSampleF =
        (*config).a0 * inSampleF + (*config).a1 * (*config).in_z1 + (*config).a2 * (*config).in_z2 - (*config).b1 * (*config).out_z1 - (*config).b2 * (*config).out_z2;

    // shifting previous samples
    (*config).in_z2 = (*config).in_z1;
    (*config).in_z1 = inSampleF;
    (*config).out_z2 = (*config).out_z1;
    (*config).out_z1 = outSampleF;

    return outSampleF;
}

inline void perform_nld(float* input) {

    // http://cseweb.ucsd.edu/~marora/files/papers/13868.pdf
    // not too sure about the factors here
    // also i'm just using bessel iir filters unlike the paper
    float factor = (*input > nld_previous) ? 0.9  : 0.5;
    //float output = 0.0;

    *input = (*input * factor) + (nld_previous * (1.0 - factor));
    nld_previous = *input;
    // return output;


    // different approch:
    // https://in.mathworks.com/help/audio/ug/psychoacoustic-bass-enhancement-for-band-limited-signals.html;jsessionid=b7b3a27978d1bd350f92192c26c4
}

void process_data(uint8_t *data, size_t item_size)
{
    samples = (int16_t *)data;
    outsample = (int16_t *)data;

    /*
    int8_t delta = s_volume - current_volume;
    if (delta > 0)
    {
        // fade up
        current_volume += (delta > MAX_DELTA) ? MAX_DELTA : delta;
    }
    else if (delta < 0)
    {
        // fade down
        // & 0x7F to remove the sign bit -> abs()
        current_volume -= ((delta & 0x7F) > MAX_DELTA) ? MAX_DELTA : delta & 0x7F;
    }
    */
    // https://www.desmos.com/calculator/m7jptvdycj
    // https://dsp.stackexchange.com/questions/2990/how-to-change-volume-of-a-pcm-16-bit-signed-audio
    float volume = (s_volume / 127.0);
    // float volume = powf(10.0, (-48.0 + ((48.0 + 0)* (s_volume / 127.0))) / 20.0);

    for (int i = 0; i < item_size; i = i + 4)
    {
        // mono mix
        insample = (float)*samples;
        samples++;
        insample += (float)*samples;
        samples++;
        insample /= 2.0;

        // TODO compressor
        // http://c4dm.eecs.qmul.ac.uk/audioengineering/compressors/documents/Reiss-Tutorialondynamicrangecompression.pdf
        // https://www.desmos.com/calculator/r48rpx1mny

        if(use_bass_enhancement)    {
            highsample = process_iir(insample, &filters[0]);
            highsample = process_iir(highsample, &filters[8]);

            // FIL 1
            lowsample = process_iir(insample, &filters[2]);
            lowsample = process_iir(lowsample, &filters[3]);

            // NLD
            perform_nld(&lowsample);

            // FIL 2
            lowsample = process_iir(lowsample, &filters[4]);
            lowsample = process_iir(lowsample, &filters[5]);
            lowsample = process_iir(lowsample, &filters[6]);
            lowsample = process_iir(lowsample, &filters[7]);
            lowsample *= 6;
        }   else    {
            lowsample = insample;
            highsample = insample;
            // highsample = process_iir(insample, &filters[0]);
            // highsample = process_iir(highsample, &filters[8]);
            // lowsample = process_iir(insample, &filters[4]);
            // lowsample = process_iir(lowsample, &filters[5]);
            // lowsample = process_iir(lowsample, &filters[6]);
            // lowsample = process_iir(lowsample, &filters[7]);
        }
        // time alignment
        // write_delay_line(highsample);
        // highsample = read_delay_line();

        // volume
        lowsample *= 0.1;
        lowsample *= volume;
        highsample *= 0.1;
        highsample *= volume;

        // mix = roundf((lowsample + highsample));

        // output restoration
        *outsample = (int16_t)lowsample;
        outsample++;
        *outsample = (int16_t)highsample;
        outsample++;
    }
}

inline void print_filter(iir_filter_t flt)
{
    ESP_LOGI("FILTER", "a0: %f, a1: %f, a2: %f, b1: %f, b2: %f",
             flt.a0, flt.a1, flt.a2, flt.b1, flt.b2);
}

// https://www.earlevel.com/main/2011/01/02/biquad-formulas/
iir_filter_t generate_biquad(filter_type_t type, float Fc, float Fs, float Q, float peakGain)
{

    float a0 = 0, a1 = 0, a2 = 0, b1 = 0, b2 = 0, norm = 0;

    float V = powf(10, fabsf(peakGain) / 20);
    float K = tanf(M_PI * Fc / Fs);

    switch (type)
    {
    case lowpass:
        norm = 1 / (1 + K / Q + K * K);
        a0 = K * K * norm;
        a1 = 2 * a0;
        a2 = a0;
        b1 = 2 * (K * K - 1) * norm;
        b2 = (1 - K / Q + K * K) * norm;
        break;

    case highpass:
        norm = 1 / (1 + K / Q + K * K);
        a0 = 1 * norm;
        a1 = -2 * a0;
        a2 = a0;
        b1 = 2 * (K * K - 1) * norm;
        b2 = (1 - K / Q + K * K) * norm;
        break;

    case bandpass:
        norm = 1 / (1 + K / Q + K * K);
        a0 = K / Q * norm;
        a1 = 0;
        a2 = -a0;
        b1 = 2 * (K * K - 1) * norm;
        b2 = (1 - K / Q + K * K) * norm;
        break;

    case notch:
        norm = 1 / (1 + K / Q + K * K);
        a0 = (1 + K * K) * norm;
        a1 = 2 * (K * K - 1) * norm;
        a2 = a0;
        b1 = a1;
        b2 = (1 - K / Q + K * K) * norm;
        break;

    case peak:
        if (peakGain >= 0)
        { // boost
            norm = 1 / (1 + 1 / Q * K + K * K);
            a0 = (1 + V / Q * K + K * K) * norm;
            a1 = 2 * (K * K - 1) * norm;
            a2 = (1 - V / Q * K + K * K) * norm;
            b1 = a1;
            b2 = (1 - 1 / Q * K + K * K) * norm;
        }
        else
        { // cut
            norm = 1 / (1 + V / Q * K + K * K);
            a0 = (1 + 1 / Q * K + K * K) * norm;
            a1 = 2 * (K * K - 1) * norm;
            a2 = (1 - 1 / Q * K + K * K) * norm;
            b1 = a1;
            b2 = (1 - V / Q * K + K * K) * norm;
        }
        break;
    case lowshelf:
        if (peakGain >= 0)
        { // boost
            norm = 1 / (1 + M_SQRT2 * K + K * K);
            a0 = (1 + sqrtf(2 * V) * K + V * K * K) * norm;
            a1 = 2 * (V * K * K - 1) * norm;
            a2 = (1 - sqrtf(2 * V) * K + V * K * K) * norm;
            b1 = 2 * (K * K - 1) * norm;
            b2 = (1 - M_SQRT2 * K + K * K) * norm;
        }
        else
        { // cut
            norm = 1 / (1 + sqrtf(2 * V) * K + V * K * K);
            a0 = (1 + M_SQRT2 * K + K * K) * norm;
            a1 = 2 * (K * K - 1) * norm;
            a2 = (1 - M_SQRT2 * K + K * K) * norm;
            b1 = 2 * (V * K * K - 1) * norm;
            b2 = (1 - sqrtf(2 * V) * K + V * K * K) * norm;
        }
        break;
    case highshelf:
        if (peakGain >= 0)
        { // boost
            norm = 1 / (1 + M_SQRT2 * K + K * K);
            a0 = (V + sqrtf(2 * V) * K + K * K) * norm;
            a1 = 2 * (K * K - V) * norm;
            a2 = (V - sqrtf(2 * V) * K + K * K) * norm;
            b1 = 2 * (K * K - 1) * norm;
            b2 = (1 - M_SQRT2 * K + K * K) * norm;
        }
        else
        { // cut
            norm = 1 / (V + sqrtf(2 * V) * K + K * K);
            a0 = (1 + M_SQRT2 * K + K * K) * norm;
            a1 = 2 * (K * K - 1) * norm;
            a2 = (1 - M_SQRT2 * K + K * K) * norm;
            b1 = 2 * (K * K - V) * norm;
            b2 = (V - sqrtf(2 * V) * K + K * K) * norm;
        }
        break;
    case none:
    default:
        type = none;
        a0 = 0;
        a1 = 0;
        a2 = 0;
        b1 = 0;
        b2 = 0;
        break;
    }

    iir_filter_t flt = {
        .type = type,
        .a0 = a0,
        .a1 = a1,
        .a2 = a2,
        .b1 = b1,
        .b2 = b2,
        .in_z1 = 0,
        .in_z2 = 0,
        .out_z1 = 0,
        .out_z2 = 0};

    print_filter(flt);

    return flt;
}

void generate_filters()
{
    ESP_LOGI("FILTERS", "Generating Filters");
    // https://www.earlevel.com/main/2016/09/29/cascading-filters/
    // https://www.earlevel.com/main/2013/10/13/biquad-calculator-v2/
    /*
    filters[0] = generate_biquad(highpass, 180, sample_rate, 0.54119610, 6);
    filters[1] = generate_biquad(highpass, 180, sample_rate, 1.3065630, 6);

    filters[2] = generate_biquad(lowpass, 1500, sample_rate, 0.707, -6);
    filters[3] = generate_biquad(lowshelf, 300, sample_rate, 1, -3);

    filters[4] = generate_biquad(highpass, 800, sample_rate, 0.54119610, 6);
    filters[5] = generate_biquad(highpass, 800, sample_rate, 1.3065630, 6);
    */

   // Highfil
   // fourth order lowcut / highpass
    filters[0] = generate_biquad(highpass, 400, sample_rate, 0.707, 6);

    // FIL 1
    // fourth oder lowpass / highcut
    filters[2] = generate_biquad(lowpass, 180, sample_rate, 0.54119610, -6);
    filters[3] = generate_biquad(lowpass, 180, sample_rate, 1.3065630, -6);

    // FIL2 A
    // fourth order lowcut / highpass
    filters[4] = generate_biquad(highpass, 180, sample_rate, 0.54119610, 6);
    filters[5] = generate_biquad(highpass, 180, sample_rate, 1.3065630, 6);
    // FIL2 B
    // fourth order lowpass / highcut
    filters[6] = generate_biquad(lowpass, 400, sample_rate, 0.54119610, 6);
    filters[7] = generate_biquad(lowpass, 400, sample_rate, 1.3065630, 6);

    filters[8] = generate_biquad(highshelf, 2500, sample_rate, 1, -6);

}

void set_delay_distance(uint8_t distance_in_cm)
{
    float distance_one_sample_meters = (1 / sample_rate) * (343.2); //speed of sound in air
    float samples_for_target = distance_in_cm / (distance_one_sample_meters * (100));
    float delay_ms = samples_for_target / sample_rate;
    set_delay_ms((uint8_t)roundf(delay_ms));
}

void set_delay_ms(uint8_t ms)
{
    delay_ms = ms;
    read_offset = round(fmin(delay_ms / (1.0 / sample_rate), DELAY_LENGTH));
}

void set_sample_rate(uint16_t _sample_rate)
{
    sample_rate = _sample_rate;
    generate_filters();
    set_delay_ms(delay_ms);
}
