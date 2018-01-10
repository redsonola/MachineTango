//
//  ExperimentalMusicPlayer.h
//  MagneticGardel
//
//  Created by courtney on 11/17/17.
//
//

#ifndef ExperimentalMusicPlayer_h
#define ExperimentalMusicPlayer_h

#include "sequence_player.h"

#define MAX_DEAD_PLAYERS 200

namespace InteractiveTango
{
    
    class GeneratedMelodySection : public MainMelodySection
    {
    protected:
        MelodyGenerator *generator;
        float lastTimePlayed;
    public:
        
        GeneratedMelodySection (BeatTiming *timer, FootOnset *onset, MelodyGenerator *gen, Instruments *ins=NULL, float perWindowSize=1) : MainMelodySection(timer, onset, ins, perWindowSize)
        {
            generator = gen;
        };
        
        virtual void update(boost::shared_ptr<std::vector<int>> hsprofile, float seconds = 0)
        {
            if( fo->isStepping())
                update(seconds);
        }

        
        virtual void update(float seconds = 0)
        {
            curSeconds = seconds;
            
            //if busy/sparse = 1 then wait a bit (at least an eighth note) before re-genning
            float timeDiff = seconds - lastTimePlayed;
            float quarter = (1.0f / ((float) generator->getBPM() /  60.0f)) ;
            float eight = quarter / 2.0f;
            float sixteenth = eight / 2.0f;

            int bs = findBusySparse(0.01, seconds);
            
            if( timeDiff < sixteenth ) return; // no faster than sixteenth notes but don't place such a hard limit on density... see. 
            
            if(generator->oneToOne() && fo->isStepping() ) //wait for a foot onset to start
            {
                //ok now generate a melody
                generator->update(seconds);
            } //else implement a non-one to one mode solution
            else if( fo->isStepping())
            {
                generator->update(bs, seconds);
                lastTimePlayed = seconds; //beatTimer->getTimeInSeconds();
            }
        };
        
        
        std::vector<MidiNote> getBufferedNotes()
        {
            return generator->getCurNotes();
        }
        
         virtual std::vector<ci::osc::Message> getOSC()
        {
            //probably do nothing here... we'll see
            std::vector<ci::osc::Message> msgs = MainMelodySection::getOSC(); //this should return nothing so far.
            return msgs;
        }
        
        virtual void timesToRepeatSection()
        {
            song_structure.push_back(1); //only one section for now, but a 2nd section may draw upon different melodic material
        }
        
        double getTicksPerBeat()
        {
            return generator->getTicksPerBeat(0);
        }
        
    };
    
    class GeneratedAccompanmentSection : public AccompanimentSection
    {
    protected:
        std::vector<ChordGeneration *> generators;
        std::vector<std::vector<MidiNote>> notes;
        int BEATSPERMEASURE;
        
        //responds to busy sparse by doing the opposite or doing the parallel
        int MEASURES_TO_FLIP_BVS_MIRROR_OR_COUNTER;
        bool bvsMirror;
        bool percBvsMirror;
        bool percBvsChanged;
        int measureBVSCount;
        int bvs;
        int percbvs;  //sends to max for percussion values
        bool pauseForFillOrClose;
        
        bool fiveStopped;
        bool bvsChangedTo2;
        
        int curGen;
        
        std::vector<ci::osc::Message> harmonyMessages;
        
    public:
        
        GeneratedAccompanmentSection(BeatTiming *timer, Instruments *ins) : AccompanimentSection(timer, ins)
        {
            BEATSPERMEASURE = 4;
            generators.push_back(new ChordGeneration());
            generators.push_back(new ChordGenerationPizzInspired());
            curGen = 0;
            
            MEASURES_TO_FLIP_BVS_MIRROR_OR_COUNTER = 8;
            measureBVSCount = 0;
            
            bvsMirror = true;
            percBvsMirror = true;
            percBvsChanged = true; //only send OSC to Max 7 if this is aa different value
            percbvs = -1;
            
            pauseForFillOrClose = false;
            fiveStopped = false;
            bvsChangedTo2 = false;
            
        }
        
        void setBVSMirroring()
        {
            
            if(measureBVSCount >= MEASURES_TO_FLIP_BVS_MIRROR_OR_COUNTER && generators[curGen]->atProgressionEnd() )
            {
                measureBVSCount = 0;
                bvsMirror = chooseRandom() <= 0.5;
                percBvsMirror = chooseRandom() <= 0.5;
            }
        }
        
        double chooseRandom()
        {
            double choose =((double) std::rand()) / ((double) RAND_MAX);
            return choose;
        }
        
        //make busy accompaniments less likely and harder to invoke
        int scaleBVSFrom20to5(int whichBVS)
        {
            //ok this is hard-coded yikes but will fix later...
            if(whichBVS  <= 4 )
            {
                return 1;
            }
            else if(whichBVS <= 9 )
            {
                return 2;
            }
            else if(whichBVS <= 14 )
            {
                return 3;
            }
            else if(whichBVS <= 17 )
            {
                return 4;
            }
            else return 5;
        }
        
        void makeBothFivesLessLikely()
        {
            double chooseRemainFive = chooseRandom();
            if(chooseRemainFive > 0.25) // 75% likely that one will be modified to a 2.
            {
                if(fiveStopped) //five was also stopped previously so suppressed the previously suppreseed bvs value
                {
                    if(bvsChangedTo2) bvs = 2;
                    else percbvs = 2;
                }
                else
                {
                    double chooseBVS = chooseRandom();
                    if(chooseBVS > 0.5)
                    {
                        bvs = 2;
                    }
                    else percbvs = 2;
                }
                bvsChangedTo2 = bvs == 2;
                fiveStopped = true;
            }
            else fiveStopped = false;
        }
        
        void determineBVS(float seconds)
        {
            int lastPBVS = percbvs; //saves last value to check for change.
            int lastBVS = bvs;
            
            float accompwindow = 2;
            int origbvs = findBusySparse(accompwindow, seconds);
            
            //send dancer bvs to max
            ci::osc::Message msg;
            msg.setAddress(BUSY_SPARSE_DANCERS);
            msg.addIntArg(scaleBVSFrom20to5(origbvs));
            harmonyMessages.push_back(msg);
            
            PerceptualEvent *ev = findBusySparseSchema();
//            std::cout << "Couple Busy Sparse: " <<  origbvs << " range:" << ev->getMinMood() << "-" << ev->getMaxMood() << std::endl;
            setBVSMirroring();
            if(!bvsMirror)
            {
                //then do opposite -- TODO: FIX FOR IF I DON'T KNOW MIN MAX --> THIS WAY HACKY VERY FAST TO CHECK!!!!! YIUKES HORRIBLE
                bvs = ev->getMaxMood() + origbvs * -1 + ev->getMinMood(); //max + bvs*-1 + min
            } else bvs = origbvs;
            if(!percBvsMirror)
            {
                percbvs = ev->getMaxMood() + origbvs * -1 + ev->getMinMood(); //max + bvs*-1 + min --
            }  else percbvs = origbvs;
            
            //favors  low and middle values over higher ones.
            bvs = scaleBVSFrom20to5(bvs);
            percbvs = scaleBVSFrom20to5(percbvs);

            //if both bvs are 5, change to something else 75% of the time -- include 4
            if(percbvs >= 4 && bvs >= 4)
                makeBothFivesLessLikely();
            
            percBvsChanged = percbvs != lastPBVS;
        
            //if bvs did go from 0 to 5 or 4, then add a fill
            //addFillsAndSmoothToZeros(lastBVS);
        }
        
        void addFillsAndSmoothToZeros(float lastBVS)
        {
            //if bvs did go to or from 1, then add a fill or close
            //only go to 0 at progression end to decreas awkwardness
            
        
                if( bvs > lastBVS && lastBVS == 1 )
                {
                    if(generators[curGen]->atProgressionEnd())
                        generators[curGen]->resetChordIndex(); //so that when it comes in, it comes in on 1
            
                    ci::osc::Message msg;
                    msg.setAddress(EXPMUSIC_INTROFILL);
                    harmonyMessages.push_back(msg);
                    pauseForFillOrClose = true;
                }
                else if(bvs < lastBVS && bvs == 1)
                {
                    if(generators[curGen]->atProgressionEnd())
                    {
                        ci::osc::Message msg;
                        msg.setAddress(EXPMUSIC_CLOSEFILL);
                        harmonyMessages.push_back(msg);
                        pauseForFillOrClose = true;
                    }
                } else bvs = 2;
        }
        
        void createSampleHarmonyMessages() //now always send -- will filter on max side - since always need these messages for ritard -
        {
//            if(bvs >=3)
//            {
                ci::osc::Message msg;
                msg.setAddress(EXPMUSIC_HARMONY);
                msg.addIntArg(curGen);
                msg.addIntArg(generators[curGen]->getCurHarmony());
                msg.addIntArg(bvs>=3); //should play accord samples?
                harmonyMessages.push_back(msg);
//            }
        }
        
        void getChordGenerationNotes(float seconds)
        {
            if( generators[curGen]->atProgressionEnd() )
            {
                double choose = chooseRandom();
                if(choose < 0.5) curGen = 1; else curGen = 0;
            }
            
            determineBVS(seconds);
            
            std::cout << "bvs :" << bvs << std::endl ;
            
            if( !pauseForFillOrClose )
            {
                //ok... well for now just add lines for each bvs step
                if(bvs >= 2) notes.push_back(generators[curGen]->getNextChord());
                if(bvs >= 3) notes.push_back(generators[curGen]->getBass());
                if(bvs >= 5) notes.push_back(generators[curGen]->getTop());
                createSampleHarmonyMessages();
            }
            else pauseForFillOrClose = false;
        
            //we'll see... for 5, maybe send commands to the percussion parts...
            
        }
    
        //has to be updated with the harmony/section profile
        virtual void update(float seconds = 0 )
        {
            notes.clear();
            
            
            if( beatTimer->isOnBeat(0.0, seconds) ) //exactly on beat
            {
                beatsPlayed++;
            }

            if( beatsPlayed >= BEATSPERMEASURE) //TODO: fix hardcoding
            {
                beatsPlayed = 0;
                shouldStartFile = true;
                measureBVSCount++;
                
                getChordGenerationNotes(seconds);

                //switch the constant instrument
//                if( phraseStart )
//                {
//                    phraseStart = false;
//                    
//                    continuingInstrumentsThroughPhrase.clear();
//                    Orchestra *curOrch = curSoundFile->getOrchestration();
//                    //                    assert( !curOrch->empty() );
//                    for(int i=0; i<maxInstrumentsToHoldConstantThroughPhrase && i<curOrch->size(); i++)
//                    {
//                        continuingInstrumentsThroughPhrase.addInstrument(curOrch->getInstrViaIndex(i));
//                    }
//                }
                
            } else shouldStartFile = false;
        };
        
        std::vector<MidiNote> getBufferedNotes(int i=0)
        {
            return notes[i];
        }
        size_t voiceCount()
        {
            return notes.size();
        }
        
        virtual std::vector<ci::osc::Message> getOSC()
        {
            std::vector<ci::osc::Message> msgs;
            //ok! now we are actually sending osc
            if(percBvsChanged)
            {
                ci::osc::Message msg;
                msg.setAddress(BUSY_SPARSE_PERCUSSION);
                msg.addIntArg(percbvs);
                msgs.push_back(msg);
            }
            
            if(!harmonyMessages.empty())
            {
                for(int i=0; i<harmonyMessages.size(); i++)
                    msgs.push_back(harmonyMessages[i]);
            }
            harmonyMessages.clear();

            
            return msgs; 
        }
    };
    
    class ExperimentalMusicPlayer : public MusicPlayer, public mm::MidiSequencePlayerResponder
    {
    protected:
        MidiOutUtility midiOut; //for now have the player own it... hmmmmmmmm....
        std::vector<mm::MidiSequencePlayer *> players;
        std::vector<int> deletePlayerTag;
        double ticksPerBeat;


    public:
        ExperimentalMusicPlayer() : MusicPlayer()
        {
            main_melody = NULL;
            deletePlayerTag.clear(); //because of the nature of the errors... 
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
            

            for(int i=0; i<c_melodies.size(); i++)
            {
                c_melodies[i]->update(hsprofile, seconds);
                curMelodyOrchestration.addfromOtherOrchestra( c_melodies[i]->getOrchestration() );
            }
                
            for(int i=0; i<accompaniments.size(); i++)
            {
                if( main_melody->isStartOfPhrase() )
                    accompaniments[i]->startPhrase();
                ((GeneratedAccompanmentSection *)accompaniments[i])->update(seconds);
            }
//
//          //TODO: OK FIX SOON!!!!!
//          for(int i=0; i<ornaments.size(); i++)
//          {
//             ornaments[i]->update(hsprofile, &curMelodyOrchestration, seconds);
//          }
//        }
            curHarmonyProfile.reset();
            curHarmonyProfile = hsprofile;
        
            sendMidiMessages();
            deleteDeadPlayers();

    };
        
        
        
        virtual void playerStopped(int tag)
        {
//            std::cout << "putting in queue to delete...\n";
            
            //cannot delete immediately upon this event call as will cause seg fault shit since this would be deleleting the object that is calling that function.
            deletePlayerTag.push_back(tag);

        }
        
        //so, delete these after 10 or 12
        void deleteDeadPlayers()
        {
            while (deletePlayerTag.size() > MAX_DEAD_PLAYERS)
            {
                int tag = deletePlayerTag[0];
                int i=0;
                bool found = false;
                while (!found && i<players.size())
                {
                    found = players[i]->getTag() == tag;
                    i++;
                }

                if(found)
                {
                    delete players[i-1];
                    players.erase(players.begin()+i-1);
                }
                deletePlayerTag.erase(deletePlayerTag.begin());
            }
            
            
        }
    
    
        virtual void sendNoteSeq(std::vector<MidiNote> notes, int channel )
        {
            if(notes.size() <= 0) return;
            
            mm::MidiSequencePlayer *player = new mm::MidiSequencePlayer(*midiOut.getOut());
            
            for(int i=0; i<notes.size(); i++)
            {
//                if(notes[i].tick > 0){
                    //mm::MessageType::NOTE_ON
                    //        MidiMessage(const uint8_t b1, const uint8_t b2, const uint8_t b3, const double ts = 0) : timestamp(ts) { data = {b1, b2, b3}; }

                    std::shared_ptr<mm::MidiMessage> m(mm::MakeNoteOnPtr(channel,notes[i].pitch, notes[i].velocity));
//                    std::cout << "Sending to sequencer: note:" <<notes[i].pitch << "," << notes[i].velocity << std::endl;
                    std::shared_ptr<mm::MidiPlayerEvent> ev(new mm::MidiPlayerEvent(notes[i].tick, m, 1)); //MidiPlayerEvent(double t,std::shared_ptr<MidiMessage> m, int track)
                    
                    ev->channel = channel;
                    ev->tick = notes[i].tick;
                    
                    player->addTimestampedEvent(1, time(NULL), ev);
                    
                    
//                }
//                else midiOut.send(notes[i], channel);
            }
            
//            if(notes.size() <= 1) return ;
            
            player->setBeatTiming(main_melody->getTimer());
            player->setTag(players.size());
            player->setResponder(this);
            player->setTicksPerBeat(ticksPerBeat);
            player->start(); //start it
            players.push_back(player);
        }
        
        
        //OK this is bogus and ridic refactor ASAP
        virtual void sendMidiMessages()
        {
            //collect all of the current midi notes
            std::vector<MidiNote> notes;
            if(main_melody != NULL)
            {
                notes = ((GeneratedMelodySection *) main_melody)->getBufferedNotes();
            }
            ticksPerBeat = ((GeneratedMelodySection *) main_melody)->getTicksPerBeat();
            
            //now send them
            sendNoteSeq(notes, 1);
            
//            std::cout << "Sending Follower notes:" << notes.size();

            
            notes.clear();
            
            for(int i=0; i<c_melodies.size(); i++)
            {
                notes = ((GeneratedMelodySection *) c_melodies[i])->getBufferedNotes();
                
                sendNoteSeq(notes, 2);
//                std::cout << "Sending Leader notes:" << notes.size();
                
                notes.clear();
            }
            
            for(int i=0; i<accompaniments.size(); i++)
            {
                for(int j=0; j<((GeneratedAccompanmentSection *) accompaniments[i])->voiceCount(); j++)
                {
                    notes.clear();
                    notes = ((GeneratedAccompanmentSection *) accompaniments[i])->getBufferedNotes(j);
                    sendNoteSeq(notes, 3+j); //TODO: change for multiple accompaniment sections, altho not relevant
                }
            }
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
