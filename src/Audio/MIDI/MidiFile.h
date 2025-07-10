/*
	MIDI File Parser

	#include "MidiFile.h"
	int main()
	{
		MidiFile midi("midi.mid");
	}

	References
		Programming MIDI: https://youtu.be/040BKtnDdg0
			https://github.com/OneLoneCoder/Javidx9/blob/master/PixelGameEngine/SmallerProjects/OneLoneCoder_PGE_MIDI.cpp
		(.mid) Standard MIDI File Format: https://faydoc.tripod.com/formats/mid.htm
		Standard MIDI-File Format Spec. 1.1, updated: https://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html
*/

#include <iostream>
#include <string>
#include <cstdint>
#include <fstream>
#include <vector>
#include <list>

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;

struct MidiEvent
{
	enum class Type
	{
		NOTE_OFF,
		NOTE_ON,
		OTHER
	} e;

	u8 key = 0;
	u8 vel = 0;
	u32 delta_tick = 0;
};

struct MidiNote
{
	u8 key = 0;
	u8 vel = 0;
	u32 start_time = 0;
	u32 duration = 0;
};

struct MidiHeader
{
	u32 file_id;
	u32 length;
	u16 format;
	u16 track_chunks;
	u16 division;
};

struct MidiTrack
{
	std::string name;
	std::string instrument;
	std::vector<MidiEvent> events;
	std::vector<MidiNote> notes;
	u8 max_note = 64;
	u8 min_note = 64;
};

enum Event : u8
{
	NOTE_OFF         = 0x80,
	NOTE_ON          = 0x90,
	AFTER_TOUCH      = 0xA0,
	CONTROL_CHANGE   = 0xB0,
	PROGRAM_CHANGE   = 0xC0,
	CHANNEL_PRESSURE = 0xD0,
	PITCH_BEND       = 0xE0,
	SYSTEM_EXCLUSIVE = 0xF0,
};

enum MetaEvent : u8
{
	SEQUENCE          = 0x00,
	TEXT              = 0x01,
	COPYRIGHT         = 0x02,
	TRACK_NAME        = 0x03,
	INSTRUMENT_NAME   = 0x04,
	LYRICS			  = 0x05,
	MARKER			  = 0x06,
	CUE_POINT         = 0x07,
	CHANNEL_PREFIX    = 0x20,
	END_OF_TRACK      = 0x2F,
	SET_TEMPO         = 0x51,
	SMPTE_OFFSET      = 0x54,
	TIME_SIGNATURE    = 0x58,
	KEY_SIGNATURE     = 0x59,
	SEQUENCE_SPECIFIC = 0x7F,
};

static const u32 MThd = 0x4D546864; // Header Chunk
static const u32 MTrk = 0x4D54726B; // Track Chunk

struct MidiFile
{
	MidiFile() {}

	MidiFile(const std::string& filename)
	{
		Parse(filename);
	}

	bool Parse(const std::string& filename)
	{
		// Load file
		std::ifstream f(filename, std::fstream::in | std::ios::binary);
		if (!f.is_open())
		{
			std::cerr << "ERROR: Could not open the file: " << filename << "\n";
			return false;
		}

		std::cout << "INFO: Reading Midi File: " << filename << "\n";

		// Parse Header Chunk
		MidiHeader h = {
			.file_id = read32(f),
			.length = read32(f),
			.format = read16(f),
			.track_chunks = read16(f),
			.division = read16(f),
		};

		if (h.file_id != MThd)
		{
			std::cerr << "ERROR: Invalid MIDI Header\n";
			return false;
		}

		std::cout << "INFO: File ID: " << std::hex << h.file_id << std::dec << "\n";
		std::cout << "INFO: Header Length: " << h.length << "\n";
		std::cout << "INFO: Format: " << h.format << "\n";
		std::cout << "INFO: Number of Tracks: " << h.track_chunks << "\n";
		std::cout << "INFO: Division: " << h.division << "\n";

		// Parse Track Chunk
		for (u16 chunk = 0; chunk < h.track_chunks; chunk++)
		{
			u32 track_id = read32(f);
			if (track_id != MTrk)
			{
				std::cerr << "ERROR: Invalid Track\n";
				return false;
			}

			u32 track_length = read32(f);

			u32 wall_time = 0;
			prev_status = 0;
			tracks.push_back(MidiTrack());
			end_of_track = false;
			while (!f.eof() && !end_of_track)
			{
				status_time_delta = read_value(f);
				status = f.get();

				if (status < 0x80)
				{
					status = prev_status;
					f.seekg(-1, std::ios_base::cur);
				}

				switch ((status & 0xF0))
				{
				case Event::NOTE_OFF:         NOTE_OFF(f, chunk);         break;
				case Event::NOTE_ON:          NOTE_ON(f, chunk);          break;
				case Event::AFTER_TOUCH:      AFTER_TOUCH(f, chunk);      break;
				case Event::CONTROL_CHANGE:   CONTROL_CHANGE(f, chunk);   break;
				case Event::PROGRAM_CHANGE:   PROGRAM_CHANGE(f, chunk);   break;
				case Event::CHANNEL_PRESSURE: CHANNEL_PRESSURE(f, chunk); break;
				case Event::PITCH_BEND:       PITCH_BEND(f, chunk);       break;
				case Event::SYSTEM_EXCLUSIVE: SYSTEM_EXCLUSIVE(f, chunk); break;
				default:
					std::cout << "ERROR: Unrecognised Status Byte: " << status << std::endl;
				}
			}
		}

		ConvertEventToNote();

		f.close();

		return true;
	}

public: // Helper functions
	// Swap endianness of 32-bit and 16-bit integer
	u32 swap_endian32(u32 n) { return ((n & 0xFF000000) >> 24) | ((n & 0x00FF0000) >> 8) | ((n & 0x0000FF00) << 8) | ((n & 0x000000FF) << 24); }
	u16 swap_endian16(u16 n) { return (n >> 8) | (n << 8); }

	// Read 32 bits
	u32 read32(std::ifstream& f)
	{
		u32 value = 0;
		f.read(reinterpret_cast<char*>(&value), sizeof(u32));
		return swap_endian32(value);
	}

	// Read 16 bits
	u16 read16(std::ifstream& f)
	{
		u16 value = 0;
		f.read(reinterpret_cast<char*>(&value), sizeof(u16));
		return swap_endian16(value);
	}

	// Read bytes from stream as a string
	std::string read_str(std::ifstream& f, u32 length)
	{
		std::string s;
		for (u32 i = 0; i < length; i++) s += f.get();
		return s;
	}

	// Read compressed MIDI value up to 32 bits
	u32 read_value(std::ifstream& f)
	{
		u32 value = 0;
		u8 byte = 0;
		// Read byte
		value = f.get();
		// [1][0][0][0] [0][0][0][0] If most significant bit is set to 1, continue reading
		if (value & 0x80)
		{
			// Extract bottom 7 bits
			value &= 0x7F;
			do
			{
				// Read next byte
				byte = f.get();
				// Construct value by setting bottom 7 bits, then shifting 7 bits
				value = (value << 7) | (byte & 0x7F);
			} while (byte & 0x80); // while most significant bit is 1
		}

		return value;
	}

public: // MIDI Events
	void NOTE_OFF(std::ifstream& f, u32 chunk)
	{
		prev_status = status;
		u8 channel = status & 0x0F;
		u8 note_id = f.get();
		u8 note_vel = f.get();
		tracks[chunk].events.push_back({ MidiEvent::Type::NOTE_OFF, note_id, note_vel, status_time_delta });
	}

	void NOTE_ON(std::ifstream& f, u32 chunk)
	{
		prev_status = status;
		u8 channel = status & 0x0F;
		u8 note_id = f.get();
		u8 note_vel = f.get();
		if (note_vel == 0)
			tracks[chunk].events.push_back({ MidiEvent::Type::NOTE_OFF, note_id, note_vel, status_time_delta });
		else
			tracks[chunk].events.push_back({ MidiEvent::Type::NOTE_ON,  note_id, note_vel, status_time_delta });
	}

	void AFTER_TOUCH(std::ifstream& f, u32 chunk)
	{
		prev_status = status;
		u8 channel = status & 0x0F;
		u8 note_id = f.get();
		u8 note_vel = f.get();
		tracks[chunk].events.push_back({ MidiEvent::Type::OTHER });
	}

	void CONTROL_CHANGE(std::ifstream& f, u32 chunk)
	{
		prev_status = status;
		u8 channel = status & 0x0F;
		u8 control_id = f.get();
		u8 control_value = f.get();
		tracks[chunk].events.push_back({ MidiEvent::Type::OTHER });
	}

	void PROGRAM_CHANGE(std::ifstream& f, u32 chunk)
	{
		prev_status = status;
		u8 channel = status & 0x0F;
		u8 program_id = f.get();
		tracks[chunk].events.push_back({ MidiEvent::Type::OTHER });
	}

	void CHANNEL_PRESSURE(std::ifstream& f, u32 chunk)
	{
		prev_status = status;
		u8 channel = status & 0x0F;
		u8 channel_pressure = f.get();
		tracks[chunk].events.push_back({ MidiEvent::Type::OTHER });
	}

	void PITCH_BEND(std::ifstream& f, u32 chunk)
	{
		prev_status = status;
		u8 channel = status & 0x0F;
		u8 LS7B = f.get();
		u8 MS7B = f.get();
		tracks[chunk].events.push_back({ MidiEvent::Type::OTHER });
	}

	void SYSTEM_EXCLUSIVE(std::ifstream& f, u32 chunk)
	{
		prev_status = 0;
		if (status == 0xFF)
		{
			// Meta Events
			u8 type = f.get();
			u8 length = read_value(f);
			switch (type)
			{
			case SEQUENCE:
				std::cout << "INFO: Sequence Number: " << f.get() << f.get() << std::endl;
				break;
			case TEXT:
				std::cout << "INFO: Text: " << read_str(f, length) << std::endl;
				break;
			case COPYRIGHT:
				std::cout << "INFO: Copyright: " << read_str(f, length) << std::endl;
				break;
			case TRACK_NAME:
				tracks[chunk].name = read_str(f, length);
				std::cout << "INFO: Track Name: " << tracks[chunk].name << std::endl;
				break;
			case INSTRUMENT_NAME:
				tracks[chunk].instrument = read_str(f, length);
				std::cout << "INFO: Instrument Name: " << tracks[chunk].instrument << std::endl;
				break;
			case LYRICS:
				std::cout << "INFO: Lyrics: " << read_str(f, length) << std::endl;
				break;
			case MARKER:
				std::cout << "INFO: Marker: " << read_str(f, length) << std::endl;
				break;
			case CUE_POINT:
				std::cout << "INFO: Cue: " << read_str(f, length) << std::endl;
				break;
			case CHANNEL_PREFIX:
				std::cout << "INFO: Prefix: " << f.get() << std::endl;
				break;
			case END_OF_TRACK:
				end_of_track = true;
				break;
			case SET_TEMPO:
				// Tempo is in microseconds per quarter note
				if (tempo == 0)
				{
					(tempo |= (f.get() << 16));
					(tempo |= (f.get() << 8));
					(tempo |= (f.get() << 0));
					BPM = (60000000 / tempo);
					std::cout << "INFO: Tempo: " << tempo << " (" << BPM << "bpm)" << std::endl;
				}
				break;
			case SMPTE_OFFSET:
				std::cout << "INFO: SMPTE: H:" << f.get() << " M:" << f.get() << " S:" << f.get() << " FR:" << f.get() << " FF:" << f.get() << std::endl;
				break;
			case TIME_SIGNATURE:
				std::cout << "INFO: Time Signature: " << f.get() << "/" << (2 << f.get()) << std::endl;
				std::cout << "INFO: ClocksPerTick: " << f.get() << std::endl;
				// A MIDI "Beat" is 24 ticks, so specify how many 32nd notes constitute a beat
				std::cout << "INFO: 32 per 24Clocks: " << f.get() << std::endl;
				break;
			case KEY_SIGNATURE:
				std::cout << "INFO: Key Signature: " << f.get() << std::endl;
				std::cout << "INFO: Minor Key: " << f.get() << std::endl;
				break;
			case SEQUENCE_SPECIFIC:
				std::cout << "INFO: Sequencer Specific: " << read_str(f, length) << std::endl;
				break;
			case 0xF0: std::cout << "INFO: System Exclusive Begin: " << read_str(f, read_value(f)) << std::endl;
				break;
			case 0xF7: std::cout << "INFO: System Exclusive End: " << read_str(f, read_value(f)) << std::endl;
				break;

			default:
				std::cout << "ERROR: Unrecognised MetaEvent: " << type << std::endl;
			}
		}
	}

	void ConvertEventToNote()
	{
		// Convert Time Events to Notes
		for (auto& track : tracks)
		{
			u32 wall_time = 0;
			std::list<MidiNote> notes;
			for (auto& event : track.events)
			{
				wall_time += event.delta_tick;

				// New Note
				if (event.e == MidiEvent::Type::NOTE_ON)
				{
					notes.push_back({ event.key, event.vel, wall_time, 0 });
				}

				if (event.e == MidiEvent::Type::NOTE_OFF)
				{
					auto note = std::find_if(notes.begin(), notes.end(), [&](const MidiNote& n) { return n.key == event.key; });
					if (note != notes.end())
					{
						note->duration = wall_time - note->start_time;
						track.notes.push_back(*note);
						track.min_note = std::min(track.min_note, note->key);
						track.max_note = std::max(track.max_note, note->key);
						notes.erase(note);
					}
				}
			}
		}

		for (auto& track : tracks)
		{
			for (auto& note : track.notes)
			{
				std::cout << "  Key: " << note.key << "\n";
				std::cout << "  Velocity: " << note.vel << "\n";
				std::cout << "  Start Time: " << note.start_time << "\n";
				std::cout << "  Duration: " << note.duration << "\n";
			}
		}
	}

public:
	std::vector<MidiTrack> tracks;
	u32 tempo = 0;
	u32 BPM = 0;

	u8 status = 0;
	u8 prev_status = 0;
	u32 status_time_delta = 0;
	bool end_of_track = false;
};