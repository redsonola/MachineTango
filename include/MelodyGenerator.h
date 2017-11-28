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
        
    public:
        MelodyGenerator( MelodySection *section=NULL, std::vector<MappingSchema *> *_schemas=NULL )
        {
            schemas = _schemas;
            oneToOneMode = true; //if yes, ignore profile
//            fo_probability = fp;
            melodySection = section;
            
            //load the fo files -- todo have this in a database
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
        
        virtual void update(float seconds=0)
        {
            if(generators.size() <= 0)
            {
                std::cerr << "MelodyGenerator can't generate! No algorithm!\n";
            }
            
            melodyFragment.push_back(generators[0]->generateNext());
            
            //TODO: respond to busy/sparse
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
