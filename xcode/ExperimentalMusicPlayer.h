//
//  ExperimentalMusicPlayer.h
//  MagneticGardel
//
//  Created by courtney on 11/17/17.
//
//

#ifndef ExperimentalMusicPlayer_h
#define ExperimentalMusicPlayer_h

namespace InteractiveTango
{
    
    class GeneratedMelodySection : public MainMelodySection
    {
    protected:
        MelodyGenerator *generator;
    public:
        
        GeneratedMelodySection (BeatTiming *timer, FootOnset *onset, MelodyGenerator *gen, Instruments *ins=NULL, float perWindowSize=1) : MainMelodySection(timer, onset, ins, perWindowSize)
        {
            generator = gen;
        };
        
        virtual void update(boost::shared_ptr<std::vector<int>> hsprofile, float seconds = 0)
        {
            update(seconds);
        }
        
        virtual void update(float seconds = 0)
        {
            //for now this always plays -- phrases determined by dancer
   
//TODO -- implement -- for now, we are just playing in one-to-mode
//            
//            secondsPlayed = beatTimer->getTimeInSeconds() - secondsStarted; //how long in seconds has the file been playing?
//            curSeconds = seconds;
//            
//            if( beatTimer->isOnBeat(0.0, seconds) ) //exactly on beat
//            {
//                beatsPlayed++;
//            }
            
            //lastStepped = fo->isStepping() ;
            
            if(generator->oneToOne() && fo->isStepping() ) //wait for a foot onset to start
            {
                //ok now generate a melody
                generator->update(seconds);
            } //else implement a non-one to one mode solution
            
            
        };
        
        
        std::vector<MidiNote> getBufferedNotes()
        {
            return generator->getCurNotes();
        }
        
         virtual std::vector<ci::osc::Message> getOSC()
        {
            //probably do nothing here... we'll see
            std::vector<ci::osc::Message> msgs = MainMelodySection::getOSC(); //this should return nothing.
            return msgs;
        }
        
        virtual void timesToRepeatSection()
        {
            song_structure.push_back(1); //only one section for now, but a 2nd section may draw upon different melodic material
        }

    };
    
    class ExperimentalMusicPlayer : public MusicPlayer
    {
    protected:
        MidiOutUtility midiOut; //for now have the player own it... hmmmmmmmm....
    public:
        ExperimentalMusicPlayer() : MusicPlayer()
        {
            main_melody = NULL;
        }
        
        void addGeneratedMelodySection(GeneratedMelodySection *section)
        {
            if(main_melody==NULL)
            {
                setMainMelody(section);
            }
            else
            {
                addCounterMelody(section);
            }
        }
        
//        virtual void update(float seconds = 0)
//        {
//            MusicPlayer::update(seconds);
//
//            //ok, send the midi now... may change tho...
//            
//            
//        }
        
        virtual void update(float seconds = 0)
        {
            if(main_melody == NULL) return;
            
            boost::shared_ptr<std::vector<int>> hsprofile(new std::vector<int> ); //empty one for main melody... should fix that setup
            Orchestra curMelodyOrchestration;
            
            main_melody->update(hsprofile, seconds);
            hsprofile.reset();
            hsprofile = main_melody->getHarmonySectionProfile();
            curMelodyOrchestration.addfromOtherOrchestra( main_melody->getOrchestration() );
            
            
            //use last harmony profile for melody that is not playing, etc., if not first one
            if( hsprofile.get() == NULL ) hsprofile = curHarmonyProfile;
            
            //for now
//            if( hsprofile.get() != NULL )
//            {
            
                //                if( !main_melody->shouldPause() )
                //                {
                for(int i=0; i<c_melodies.size(); i++)
                {
                    c_melodies[i]->update(hsprofile, seconds);
                    curMelodyOrchestration.addfromOtherOrchestra( c_melodies[i]->getOrchestration() );
                }
                
//                for(int i=0; i<accompaniments.size(); i++)
//                {
//                    if( main_melody->isStartOfPhrase() )
//                        accompaniments[i]->startPhrase();
//                    accompaniments[i]->update(hsprofile, seconds);
//                }
//            }
//            //TODO: OK FIX SOON!!!!!
//            for(int i=0; i<ornaments.size(); i++)
//            {
//                ornaments[i]->update(hsprofile, &curMelodyOrchestration, seconds);
//            }
//        }
        curHarmonyProfile.reset();
        curHarmonyProfile = hsprofile;
        
        sendMidiMessages();

    };
    
    
        
        
        //OK this is bogus and ridic refactor ASAP
        virtual void sendMidiMessages()
        {
            //collect all of the current midi notes
            std::vector<MidiNote> notes;
            if(main_melody != NULL)
            {
                notes = ((GeneratedMelodySection *) main_melody)->getBufferedNotes();
            }
            
            //now send them
            for(int i=0; i<notes.size(); i++)
            {
                midiOut.send(notes[i]);
            }
            
            notes.clear();
            
            for(int i=0; i<c_melodies.size(); i++)
            {
                notes = ((GeneratedMelodySection *) c_melodies[i])->getBufferedNotes();
                
                for(int j=0; j<notes.size(); j++)
                {
                    midiOut.send(notes[j], i+2);
                }
                
                notes.clear();
            }
            
            
//
            
        }
        

        virtual std::vector<ci::osc::Message> getOSC()
        {
            //probably do nothing here... we'll see
            std::vector<ci::osc::Message> msgs = MusicPlayer::getOSC(); //this should return nothing.
            return msgs;
        }
        
 
    

    };
}

#endif /* ExperimentalMusicPlayer_h */
