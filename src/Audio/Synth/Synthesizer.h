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

	Reference Implementations
		https://github.com/OneLoneCoder/synth
		https://github.com/gsliepen/modsynth/tree/main
		https://github.com/OneLoneCoder/olcSoundWaveEngine
		https://github.com/jan-van-bergen/Synth

	Synthesizer
		Synth1: https://daichilab.sakura.ne.jp/softsynth/index.html
		GB SFX Generator: https://patchworkgames.itch.io/gbsfx

	Driver Backend
		miniaudio: https://github.com/mackron/miniaudio
		soloud: https://github.com/jarikomppa/soloud
		sdl: https://github.com/libsdl-org/SDL_mixer

	Audio DSP Lib
		https://github.com/cycfi/q
		https://github.com/libAudioFlux/audioFlux
*/
#include <vector>
#include <unordered_map>

#include "../../Core/Common.h"
#include "Oscillator.h"


// FEATURES
	// TODO: Envelope ADSR Visualization
	// TODO: Low-Frequency Oscillator
	// TODO: Filters
	// TODO: Effects
	// TODO: Modular Synthesizer
	// TODO: Sequencer
	// TODO: Spectrogram (FFT)
	// TODO: Playback

// IMPROVEMENTS
	// TODO: Oscilloscope Improvement
	// TODO: Graphical User Interface Improvement

// BUGS
	// KNOWN BUG: Sine Wave Clipping in ADSR: Release to Attack
	// KNOWN BUG: After some time running the program, it introduces noise (buffer overflow?)

// DONE
	// DONE: Midi Keyboard
	// DONE: Envelope Control
	// DONE: Polyphony

struct WaveData
{
	Wave wave;
	Oscillator::Type waveform;
	std::vector<f64> times;
	std::vector<f64> samples;
};

//#define ENVELOPE_TEST

// Modular Synthesizer
class Synthesizer
{
public:
	Synthesizer();

public:
	f64 Synthesize(f64 time_step, note n, bool& note_finished);
	void Update(f64 time);

public:
	void PlayToggle();
	bool IsPlaying();
	void SetMasterVolume(f64 volume);
	f64 GetMasterVolume();

	std::vector<note>& GetNotes();
	const WaveData& GetWaveData();
	void UpdateWaveData(u32 frame, f64 sample);

	Oscillator& GetOscillator(std::string id);
	std::unordered_map<std::string, Oscillator>& GetOscillators();

public:
	f64 m_master_volume = 0.5;
	f64 max_frequency = 20000.0;
	bool m_playing = true;

	std::vector<note> notes;

	// Modules
	std::unordered_map<std::string, Oscillator> oscillators;
	Envelope m_envelope;

#ifdef ENVELOPE_TEST
	ADSR m_adsr;
#endif
	// Sample Buffer
	WaveData wave_data;
};

