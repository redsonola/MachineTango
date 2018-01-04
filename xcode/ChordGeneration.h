//
//  ChordGeneration.h
//  MagneticGardel
//
//  Created by courtney on 12/21/17.
//
//

#ifndef ChordGeneration_h
#define ChordGeneration_h

//for now just always follow the same progressions... -- this is hacky -- TODO: FIX AND USE DATABASE
namespace InteractiveTango {
    
class ChordGeneration
{
protected:
    std::vector<std::vector<int>> possibleProgressions;
    std::vector<std::vector<MidiNote>> chords;
    std::vector<std::vector<MidiNote>> bass;
    std::vector<std::vector<MidiNote>> top;

    int chordIndex;
    int progressionIndex;
    
public:
    
    ChordGeneration()
    {
        //read all the chords -- xposed to Am from tango fragments section #1 -- six chords ordered by their appearance in that work
        buildPossibleProgressions();
    }
    
    //TODO -- put in database -- ALSO note -- could generate melody ahead of time then choose chord based on melody.... !
    virtual void buildPossibleProgressions()
    {
        std::vector<int> pp1 = {1, 2, 3, 4, 5, 1};
        std::vector<int> pp2 = {1, 1, 2, 2, 1};
        std::vector<int> pp3 = {1, 1, 3, 2, 2, 4, 2, 5, 1};
        
        possibleProgressions.push_back(pp1);
        possibleProgressions.push_back(pp2);
        possibleProgressions.push_back(pp3);
        
        loadMidi();
        
        chordIndex = 0;
        progressionIndex = 0;
    }
    
    //should be in database?
    virtual void loadMidi()
    {
        std::string enclosingFolder = "/Users/courtney/Documents/Interactive Tango Milonga/emtango chord patterns/";
        std::string enclosingOneHand = "one_hand_chords/";
        std::string enclosingWalkingBass = "walking_bass/";
        std::string enclosingtop = "top_voice_accomp/";
        std::string enclosingpizz = "pizzolla_inspired/";

        std::string bass_prefix = "bass_";

        
        std::vector<std::string> filenames =  {"tonic_1.mid", "dominant_2.mid", "two_3.mid", "subDom_4.mid", "six_five_5.mid"};
        
        for(int i=0; i<filenames.size(); i++)
        {
            MidiFileUtility midi;
            MidiFileUtility midibass;
            MidiFileUtility miditop;

            
            midi.readMidiFile(enclosingFolder + enclosingOneHand + filenames[i]);
            chords.push_back(midi.getMelody(1));
            
            midibass.readMidiFile(enclosingFolder + enclosingWalkingBass + bass_prefix + filenames[i]);
            bass.push_back(midibass.getMelody(1));
            
            miditop.readMidiFile(enclosingFolder + enclosingtop + filenames[i]);
            top.push_back(miditop.getMelody(1));

        }
    }
    
    virtual void chooseNextProgressionIndex()
    {
        double choose =((double) std::rand()) / ((double) RAND_MAX);
        
        double i =0;
        while ( choose < (1.0+i)/possibleProgressions.size() && i < possibleProgressions.size()-1   )
        {
            i++;
        }
        
        progressionIndex = i;
        chordIndex = 0;
    }
    
    std::vector<MidiNote> getNextChord()
    {
        if( atProgressionEnd() )
        {
            chooseNextProgressionIndex();
        }
        chordIndex++;
        return chords[possibleProgressions[progressionIndex][chordIndex-1]-1];
    }
    
    //returns relevant bass notes
    std::vector<MidiNote> getBass()
    {
        return bass[possibleProgressions[progressionIndex][chordIndex-1]-1];
    }
    
    //returns relevant top decorative accomp. voice
    std::vector<MidiNote> getTop()
    {
        return top[possibleProgressions[progressionIndex][chordIndex-1]-1];
    }
    
    bool atProgressionEnd()
    {
        return chordIndex >= possibleProgressions[progressionIndex].size();
    }
    
    int getCurHarmony()
    {
        return possibleProgressions[progressionIndex][chordIndex-1];
    }
    
};


class ChordGenerationPizzInspired : public ChordGeneration
{
public:
    ChordGenerationPizzInspired() : ChordGeneration()
    {
        possibleProgressions.clear();
        chords.clear();
        buildPossibleProgressions2();
    }
    
    //should be in database?
    virtual void loadMidi2()
    {
        std::string enclosingFolder = "/Users/courtney/Documents/Interactive Tango Milonga/emtango chord patterns/";
        std::string enclosingpizz = "piazzolla_inspired/";
        
        std::vector<std::string> pizzfilenames =  {"tonic_1.mid", "dominant_2.mid", "subdom_3.mid", "two_4.mid", "predom5.mid", "dom6.mid"};
        
        for(int i=0; i<pizzfilenames.size(); i++)
        {
            MidiFileUtility midi;

            midi.readMidiFile(enclosingFolder + enclosingpizz + pizzfilenames[i]);
            chords.push_back(midi.getMelody(1));
        }
        
        //rearrange other parts to make sense kind of
        bass = { bass[0], bass[1] , bass[3], bass[2], bass[3], bass[1]};
        top = { top[0], top[1] , top[3], top[2], top[3], top[1]};
    }
    
    virtual void buildPossibleProgressions2()
    {
        std::vector<int> pp1 = {1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6};
        std::vector<int> pp2 = {1, 1, 2, 2, 1, 1};
            
        possibleProgressions.push_back(pp1);
        possibleProgressions.push_back(pp2);
            
        loadMidi2();
            
        chordIndex = 0;
        progressionIndex = 0;
    }
    

};

};


#endif /* ChordGeneration_h */
