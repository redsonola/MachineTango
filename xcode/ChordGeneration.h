//
//  ChordGeneration.h
//  MagneticGardel
//
//  Created by courtney on 12/21/17.
//
//

#ifndef ChordGeneration_h
#define ChordGeneration_h

//for now just always follow the same progressions...
namespace InteractiveTango {
    
class ChordGeneration
{
    
    std::vector<std::vector<int>> possibleProgressions;
    std::vector<std::vector<MidiNote>> chords;
    int chordIndex;
    int progressionIndex;
    
public:
    
    ChordGeneration()
    {
        //read all the chords -- xposed to Am from tango fragments section #1 -- six chords ordered by their appearance in that work
        buildPossibleProgressions();
    }
    
    //TODO -- put in database -- ALSO note -- could generate melody ahead of time then choose chord based on melody.... !
    void buildPossibleProgressions()
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
    void loadMidi()
    {
        std::string enclosingFolder = "/Users/courtney/Documents/Interactive Tango Milonga/emtango chord patterns/";
        std::vector<std::string> filenames =  {"tonic_1.mid", "dominant_2.mid", "two_3.mid", "subDom_4.mid", "six_five_5.mid"};
        
        for(int i=0; i<filenames.size(); i++)
        {
            MidiFileUtility midi;
            midi.readMidiFile(enclosingFolder + filenames[i]);
            chords.push_back(midi.getMelody(1));
        }
    }
    
    void chooseNextProgressionIndex()
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
        if( chordIndex >= possibleProgressions[progressionIndex].size() )
        {
            chooseNextProgressionIndex();
        }
        chordIndex++;
        return chords[possibleProgressions[progressionIndex][chordIndex-1]-1];
    }
    
};

}

#endif /* ChordGeneration_h */
