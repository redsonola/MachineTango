//
//  MIDIUtility.h
//  MagneticGardel
//
//  Created by courtney on 11/7/17.
//
//

#ifndef MIDIUtility_h
#define MIDIUtility_h

#include <chrono>
#include <random>
#include <fstream>
#include <string>


//for reference
//inline MidiMessage MakeNoteOn(uint8_t channel, uint8_t note, uint8_t velocity)
//{
//    return MidiMessage(MakeCommand(MessageType::NOTE_ON, channel), note, velocity);
//}

#include "MidiFile.h"

#include "modernmidi.h"
#include "port_manager.h"
#include "midi_input.h"
#include "midi_output.h"
#include "midi_message.h"
#include "midi_event.h"
#include "midi_utils.h"

namespace InteractiveTango {

#define MIDI_MESSAGE_DATA_PITCH_NOTE_ON 1
#define MIDI_MESSAGE_DATA_VEL_NOTE_ON 2

class MidiNote
{
public:
    MidiNote(int p = 60, int vel = 100, double t =0, double d = 0.25)
    {
        pitch = p;
        velocity = vel;
        tick = t;
        duration = d;
    }
    
    double tick;
    double duration;
    int    pitch;
    int velocity;
    
    bool operator==(MidiNote note)
    {
        return (tick == note.tick && pitch == note.pitch);
    }
};
    
    
class MidiFileUtility
{

private:
    

    int track;
    double microsecondsPerBeat;
    double beatsPerMinute;
    double msPerTick;
    double timeSigNumerator;
    double timeSigDenom;
    double ticksPerBeat;
    
    std::vector<std::vector<MidiNote>> melody;
    
public:
    
    void readMidiFile(std::string midifile)
    {
        MidiFile reader;
        reader.read(midifile);
        track=0;
        beatsPerMinute = -1;
        reader.splitTracks(); //just in case

        for(int i=0; i<reader.getTrackCount(); i++)
        {
            track = i;
            melody.push_back(std::vector<MidiNote>());
            convertToMelody(reader);

        }
        fixTicks(reader);
        
        ticksPerBeat = reader.getTicksPerQuarterNote();
//        std::cout << "beatsPerMinute:" << beatsPerMinute << std::endl;
        
    };
    
    double getBPM()
    {
        return beatsPerMinute;
    }
    
    double getTicksPerBeat()
    {
        return ticksPerBeat;
    }
    
    std::vector<MidiNote> getMelody(int track)
    {
        return melody[track];
    }

    void convertToMelody(MidiFile& midifile) {
        midifile.absoluteTicks();
        if (track < 0 || track >= midifile.getNumTracks()) {
            cout << "Invalid track: " << track << " Maximum track is: "
            << midifile.getNumTracks() - 1 << endl;
        }
        int numEvents = midifile.getNumEvents(track);
        
        vector<int> state(128);   // for keeping track of the note states
        
        int i;
        for (i=0; i<128; i++) {
            state[i] = -1;
        }
        
        melody[track].reserve(numEvents);
        melody[track].clear();
        
        MidiNote mtemp;
        int command;
        int pitch;
        int velocity;
        
        
        for (i=0; i<numEvents; i++) {
            command =midifile[track][i][0] & 0xf0;
            MidiEvent event = midifile[track][i];
            
            if(track==0 && beatsPerMinute==-1)
            {
             beatsPerMinute = event.getTempoBPM();
            }
            else if (command == 0x90) {
                pitch = midifile[track][i][1];
                velocity = midifile[track][i][2];
                if (velocity == 0) {
                    // note off
                    goto noteoff;
                } else {
                    // note on
                    state[pitch] = midifile[track][i].tick;
                }
            } else if (command == 0x80) {
                // note off
                pitch = midifile[track][i][1];
            noteoff:
                if (state[pitch] == -1) {
                   continue;
                }
                mtemp.tick = state[pitch];
                mtemp.duration = midifile[track][i].tick - state[pitch];
                mtemp.pitch = pitch;
                mtemp.velocity = velocity;
                melody[track].push_back(mtemp);
                state[pitch] = -1;
            }
        }
    }
    
    void fixTicks(MidiFile& midifile)
    {
        std::vector<std::vector<MidiNote>> tmp;
        for(int track=0; track<midifile.getTrackCount(); track++)
        {
            tmp.push_back(std::vector<MidiNote>());
            if(melody[track].size() > 0){
            tmp[track].push_back(melody[track].at(0));
            for(int i=1; i<melody[track].size(); i++)
            {
                double newTicks = melody[track].at(i).tick - melody[track].at(i-1).tick;
                tmp[track].push_back(melody[track].at(i));
                tmp[track].at(i).tick = newTicks;
            }
            }
        }
        melody = tmp;
    }
    
};
    
class MidiOutUtility
{
protected:
    
    mm::MidiOutput *midiout;
public:
    MidiOutUtility(std::string portname = "IAC Driver IAC Bus 1")
    {
        midiout = new mm::MidiOutput("midiout");
        bool success = midiout->openVirtualPort(portname);
        if(!success)
        {
            std::cout << "Failure opening named port: " << portname << std::endl;
            std::cout << "Attempting to open port 0 instead... ";
            success = midiout->openPort( 0 );
            if(success)
            {
                std::cout << "done\n";
            }
            else
            {
                std::cout << "failed. Ending attempts.\n";
                return;
            }
        }
        else std::cout << "Opened " << portname << std::endl;
    }
    
    ~MidiOutUtility()
    {
        delete midiout;
    }
    
    void send(MidiNote note, int channel=1)
    {
//        std::cout << "note: " << note.pitch << "," << channel << std::endl;
        midiout->send(mm::MakeNoteOn(channel, note.pitch, note.velocity));
    };
    
    
    mm::MidiOutput *getOut(){ return midiout; }
};
    

}

#endif /* MIDIUtility_h */
