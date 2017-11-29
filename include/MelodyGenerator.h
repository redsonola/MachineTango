//
//  MelodyGenerator.h
//  MagneticGardel
//
//  Created by courtney on 11/14/17.
//
//

#ifndef MelodyGenerator_h
#define MelodyGenerator_h



namespace InteractiveTango {
    
    //note -- will need to refactor or recreate another music player class
    //this ugen will send melody notes out via midi -- different than previous
    //BeatTiming *timer, FootOnset *onset
    
    //A faster of implementing this could be just to respond to the OSC messages that the melody player creates.
    //It does create a structure already. -- could add functionally to current interactive tango music player to send the message
    //programmaticlly -- or an inherited music player... the downside is that will have to create a database structure....
    //however, can have some dummy values.
    
    //For now melody generators will be free of orchestral context maybe but it seems like a good idea.
    //Instead -- could respond via OMAX in max/msp -- instead of creating my own melody generator (as I have)
    //hmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
    
    //OK,

    class MelodyGenerator : public UGEN
    {
    private:
        std::vector<MelodyGeneratorAlgorithm *> generators; //a collection of factor oracles <-- refactor into part of a super melodic algorithm class
//        float fo_probability;
        
        std::vector<MappingSchema *> *schemas;
        std::vector<MidiNote> melodyFragment;
        
        bool oneToOneMode; //whether we produce one note per call or else create more notes if busy vs sparse
        
        MelodySection *melodySection; //provides organizatiion
        
        float notesPerUpdate;
        float note_rhythm_ratio_mod;
        float bsMin;
        float bsMax;
        int maxNotesGenerated;

        
    public:
                                                        //TODO set from here, yup
        MelodyGenerator( MelodySection *section=NULL, std::vector<MappingSchema *> *_schemas=NULL, int _maxNotesGenerated = 7 )
        {
            schemas = _schemas;
            oneToOneMode = false; //if yes, ignore profile
//            fo_probability = fp;
            melodySection = section;
            
            //load the fo files -- todo have this in a database
            
            if(_schemas!=NULL)
            {
                int i =0;
                bool found = false;
                while (!found && i<schemas->size())
                {
                    found =   schemas->at(i)->getMappingType() == MappingSchema::MappingSchemaType::EVENT;
                    i++;
                }
                if(found)
                {
                    setMinMaxBusySparse(((PerceptualEvent *)schemas->at(i))->getMinMood(), ((PerceptualEvent *)schemas->at(i))->getMaxMood());
                }
                else oneToOneMode = true;
            }
            
            maxNotesGenerated = _maxNotesGenerated;
        }
        
        void setMinMaxBusySparse(float min, float max)
        {
             bsMin = min;
             bsMax = max;
        }
        
        
        void addGeneratorAlgorithm(MelodyGeneratorAlgorithm *alg)
        {
            if(alg->isTrained())
                generators.push_back(alg);
            else
                std::cerr << "MelodyGenerator: Cannot use algorithm! It has not been trained!";
        }
        
        virtual std::vector<ci::osc::Message> getOSC()
        {
            //probably do nothing here... we'll see
            std::vector<ci::osc::Message> msgs;
            return msgs;
        }
        
        virtual bool oneToOne()
        {
            return oneToOneMode;
        }
        
        virtual void update(float bs, float seconds)
        {
            if(bs == 1)
                notesPerUpdate = 1;
            else
            {
                notesPerUpdate = (int) std::round((((double) std::rand()) / ((double) RAND_MAX) ) * (maxNotesGenerated * (double)bs/(double)bsMax * 0.25 )) +
                std::round(maxNotesGenerated * (double)bs/(double)bsMax * 0.75);
            }
            
            //okay so this maps into a 1-3 thing -- 2, 1, 0.5 -- basically makes notes shorter or longer based on busy sparse
            float which = (double)bs/(double)bsMax ;
            if(which <= 1.0/3.0) note_rhythm_ratio_mod = 2;
            else if(which <= 2.0/3.0) note_rhythm_ratio_mod = 1;
            else note_rhythm_ratio_mod = 0.5;
            
            for(int i=0; i<notesPerUpdate; i++)
            {
                MidiNote note = generators[0]->generateNext();
                if(i==0) note.tick = 0;
                else note.tick *= note_rhythm_ratio_mod;
                melodyFragment.push_back(note);
            }

        }
        
        virtual void update(float seconds=0)
        {
            if(generators.size() <= 0)
            {
                std::cerr << "MelodyGenerator can't generate! No algorithm!\n";
            }
            
            if(oneToOneMode)
                melodyFragment.push_back(generators[0]->generateNext());
            else
            {
                //react to busy/sparse
                std::cout << "This will not react to busy sparse since this generator is being used in one to one mode\n";
            }
            
        }
        
        //also empties out the note buffer.
        virtual std::vector<MidiNote> getCurNotes()
        {
            std::vector<MidiNote> notes(melodyFragment);
            melodyFragment.clear();
            return notes;
        };

    };

}

#endif /* MelodyGenerator_h */
