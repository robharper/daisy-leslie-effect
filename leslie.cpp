#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

// Declare a local daisy_petal for hardware access
DaisyPetal hw;

bool bypass = true;
bool is_fast = false;

Led led1, led2;

Parameter  speed_slow_ctrl, speed_fast_ctrl, drift_ctrl, vibe_ctrl, intensity_ctrl, top_boost_ctrl;
Svf        svf;
Oscillator horn_lfo, drum_lfo;

static constexpr int32_t kDelayLength = 2400; // 50 ms at 48kHz = .05 * 48000
DelayLine<float, kDelayLength> low_vibe, high_vibe;

float samplerate;
float current_speed = 0;
float brk_freq = 800.0f; // Leslie speakers have a 800Hz crossover
float q = 0.7f;          // Typical crossover resonance is 0.6 - ~1
float acceleration = 0.0003f;

// This runs at a fixed rate, to prepare audio samples
void callback(float **in, float **out, size_t size)
{
    hw.ProcessAllControls();
    float vibe = vibe_ctrl.Process();
    float drift = drift_ctrl.Process();
    float intensity = intensity_ctrl.Process();
    float top_boost = top_boost_ctrl.Process();

    // (De-)Activate bypass and toggle LED when left footswitch is pressed
    if(hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
    {
        bypass = !bypass;
        led1.Set(bypass ? 0.0f : 1.0f);
    }

    if(hw.switches[Terrarium::FOOTSWITCH_2].RisingEdge())
    {
        is_fast = !is_fast;
    }

    float new_speed = is_fast ? speed_fast_ctrl.Process() : speed_slow_ctrl.Process();

    float horn_lfo_val = 0;
    float drum_lfo_val = 0;

    fonepole(current_speed, new_speed, acceleration);
    horn_lfo.SetFreq(current_speed);
    drum_lfo.SetFreq(current_speed * drift);

    for(size_t i = 0; i < size; i++)
    {
        horn_lfo_val = horn_lfo.Process();
        drum_lfo_val = drum_lfo.Process();

        if (bypass) {
            out[0][i] = out[1][i] = in[0][i];
        } else {
            svf.Process(in[0][i]);

            float horn_delay_in_samples = vibe * (1 + horn_lfo_val) * .001f * samplerate + 1;
            float drum_delay_in_samples = vibe * (1 + drum_lfo_val) * .001f * samplerate + 1;

            float low = svf.Low();
            low_vibe.Write(low);
            low = low_vibe.ReadHermite(drum_delay_in_samples);

            float high = svf.High();
            high_vibe.Write(high);
            high = high_vibe.ReadHermite(horn_delay_in_samples);

            float horn_amp = (1.f + intensity * horn_lfo_val) / (1.f + intensity);
            float drum_amp = (1.f + intensity * drum_lfo_val) / (1.f + intensity);

            // Trem at intensity level
            low = low * drum_amp;
            high = high * horn_amp;

            // Mix high and low back together with unequal mixing with topboost
            out[0][i] = out[1][i] = low * (1-top_boost) + high * top_boost;
        }
    }

    led2.Set((horn_lfo_val + 1)/2);

    led1.Update();
    led2.Update();
}

int main(void)
{
    hw.Init();
    samplerate = hw.AudioSampleRate();
    hw.SetAudioBlockSize(12);

    // lfo
    horn_lfo.Init(samplerate);
    horn_lfo.SetWaveform(Oscillator::WAVE_SIN);
    horn_lfo.SetAmp(1);
    horn_lfo.SetFreq(.4);

    drum_lfo.Init(samplerate);
    drum_lfo.SetWaveform(Oscillator::WAVE_SIN);
    drum_lfo.SetAmp(1);
    drum_lfo.SetFreq(.4);

    low_vibe.Init();
    high_vibe.Init();

    svf.Init(samplerate);
    svf.SetFreq(brk_freq);
    svf.SetRes(q);

    // Initialize your knobs here like so:
    speed_slow_ctrl.Init(hw.knob[Terrarium::KNOB_1], 0.0f, 4.0f, Parameter::LINEAR);
    speed_fast_ctrl.Init(hw.knob[Terrarium::KNOB_2], 4.0f, 10.0f, Parameter::LINEAR);
    drift_ctrl.Init(hw.knob[Terrarium::KNOB_3], 0.0f, 1.0f, Parameter::LINEAR);
    top_boost_ctrl.Init(hw.knob[Terrarium::KNOB_4], 0.3f, 0.7f, Parameter::LINEAR);
    intensity_ctrl.Init(hw.knob[Terrarium::KNOB_5], 0.0f, 1.0f, Parameter::LINEAR);
    vibe_ctrl.Init(hw.knob[Terrarium::KNOB_6], .3f, .7f, Parameter::LINEAR);

    // Init the LEDs and set activate bypass
    led1.Init(hw.seed.GetPin(Terrarium::LED_1), false);
    led2.Init(hw.seed.GetPin(Terrarium::LED_2), false, 10000.0f);
    led1.Update();
    bypass = true;

    hw.StartAdc();
    hw.StartAudio(callback);
}