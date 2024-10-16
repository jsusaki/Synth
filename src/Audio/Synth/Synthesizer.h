/*
	Sound Synthesizer
		Waveforms
			Sine Wave
			Square Wave
			Triangle Wave
			Sawtooth Wave

		Oscillator

		Envelope ADSR

		Low Frequency Oscillator (LFO)

		Frequency Modulator

		Amplitude Modulator

		Arpeggiator

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

		Instrument

		Note

		Sequencer

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
		sdl: https://github.com/libsdl-org/SDL_mixer
		winmm

	Audio DSP Lib
		https://github.com/cycfi/q
		https://github.com/libAudioFlux/audioFlux
*/
#include <vector>
#include <unordered_map>

#include "../../Core/Common.h"
#include "Oscillator.h"

// BUG: Sine Wave Clipping in Release to Attack
// DONE: Envelope Control
// DONE: Polyphony

// TODO: Modular Synthesizer
// TODO: Sequencer
// TODO: Scope Improvement
// TODO: Interface Improvement

// TODO: Spectrogram (FFT)
// TODO: Playback

struct WaveData
{
	Wave wave;
	Oscillator::Type waveform;
	std::vector<f32> times;
	std::vector<f32> samples;
};

class Synthesizer
{
public:
	Synthesizer();

public:
	void Update(f64 time);
	void PlayToggle();
	bool IsPlaying();
	void SetMasterVolume(f32 volume);
	f32 GetMasterVolume();

	WaveData& const GetWaveData();
	Oscillator& GetOscillator(std::string id);
	std::unordered_map<std::string, Oscillator>& GetOscillators();
	std::vector<note>& GetNotes();

public:
	f32 m_master_volume = 0.5f;
	f32 max_frequency = 20000.0f;
	bool m_playing = true;
	WaveData wave_data;
	std::unordered_map<std::string, Oscillator> oscillators;
	std::vector<note> notes;

};
