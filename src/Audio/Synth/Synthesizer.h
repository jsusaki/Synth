/*
	Sound Synthesizer
		Synthesis
			Subtractive
			Additive
			Frequency Modulation
		Waveforms
			Sine Wave
			Square Wave
			Triangle Wave
			Sawtooth Wave
		Noise
			White
			Pink
			Brown
		Oscillator
		Envelope ADSR
		Low Frequency Oscillator (LFO)
		Frequency Modulator
		Amplitude Modulator
		Arpeggiator
		Note
		Instrument
		Sequencer
		Filter
			High pass filter
			Low pass filter
			Band filter
		Effects
			Compression / Expansion
			Flanger / Phaser
			Delay
			Reverb
			Chorus
		Spectrogram
		Oscilloscope

	Modular Synthesizer
		Wiki, Modular Synthesizer: https://en.wikipedia.org/wiki/Modular_synthesizer
		Chiptune: https://en.wikipedia.org/wiki/Chiptune

	Synthesizer
		Synth1: https://daichilab.sakura.ne.jp/softsynth/index.html
		GB SFX Generator: https://patchworkgames.itch.io/gbsfx

	Reference Implementations
		https://github.com/OneLoneCoder/synth
		https://github.com/gsliepen/modsynth/tree/main
		https://github.com/OneLoneCoder/olcSoundWaveEngine
		https://github.com/jan-van-bergen/Synth

	Driver Backend
		miniaudio: https://github.com/mackron/miniaudio
		soloud: https://github.com/jarikomppa/soloud
		sdl: https://github.com/libsdl-org/SDL_mixer

	Audio DSP Lib
		https://github.com/cycfi/q
		https://github.com/libAudioFlux/audioFlux
*/
#pragma once

#include <vector>
#include <unordered_map>

#include "../../Core/Common.h"

#include "Oscillator.h"
#include "Envelope.h"
#include "Filter.h"

// FEATURES
	// TODO: Filters: Low Pass Filter, High Pass Filter

	// TODO: Low-Frequency Oscillator
	// TODO: Effects: Reverb, Chorus, Delay
	// TODO: Modular Synthesizer (module, patch)
	// TODO: Sequencer
	// TODO: Spectrogram (FFT)
	// TODO: .wav support
	// TODO: .midi support
	// TODO: Track
	// TODO: Recording

// IMPROVEMENTS
	// TODO: Oscilloscope improvement
	// TODO: Filter graph improvement

// BUGS
	// KNOWN BUG: Sine Wave Clipping in ADSR: Release to Attack; sound engineering problem: attenuate with filter

// DONE
	// DONE: Audio Driver
	// DONE: Wave
	// DONE: Oscillator	
	// DONE: Waveforms: SINE, SQUARE, TRIANGLE, SAWTOOTH
	// DONE: Oscilloscope
	// DONE: ADSR Envelope
	// DONE: Polyphony
	// DONE: Envelope Control
	// DONE: Volume Control
	// DONE: Midi Keyboard
	// DONE: Envelope ADSR Visualization
	// SOLVED BUG: After some time running the program, it introduces noise (buffer overflow?)
	// DONE: Oscilloscope Interface

struct WaveData
{
	Wave wave;
	Oscillator::Type waveform;
	std::vector<f64> times;
	std::vector<f64> samples;
};

// Modular Synthesizer
class Synthesizer
{
public:
	Synthesizer();

public:
	f64 Synthesize(f64 time_step, note n, bool& note_finished);
	void Update(f64 time);

public:
	void TogglePlay();
	bool IsPlaying();
	void SetMasterVolume(f64 volume);
	f64 GetMasterVolume();

	std::vector<note>& GetNotes();
	const WaveData& GetWaveData();
	void UpdateWaveData(u32 frame, f64 sample);

	Oscillator& GetOscillator(std::string id);
	std::unordered_map<std::string, Oscillator>& GetOscillators();

public:
	f64 m_master_volume;
	f64 m_max_frequency;
	bool m_playing;

	// Notes
	std::vector<note> notes;

	// Modules
	std::unordered_map<std::string, Oscillator> oscillators;
	Envelope m_envelope;
	Filter m_filter;

	// Sample Buffer for visualization
	WaveData wave_data;
};