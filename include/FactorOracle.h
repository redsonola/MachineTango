//
//  FactorOracle.h
//  MagneticGardel
//
//  Created by courtney on 11/5/17.
//
//

#ifndef FactorOracle_h
#define FactorOracle_h

#include <iostream>
#include <string>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <vector>
#include <cstring>
#include <algorithm>
#include <vector>

#include "MIDIUtility.h"

//TODO -- clean this shit up -- 
//modified from https://gist.github.com/ZhanruiLiang/3405709
namespace Liang {

#define For(i, n) for(i = 0; i < n; i++)
#define Forr(i, l, n) for(i = l; i < n; i++)

typedef long long LL;

//not using bounded alphabets or # of elements -- CDB
//const int N = 200005;
//const int E = 26;

class FactorOracle{
    //trans[number of elements][alphabet]
    std::vector<std::vector<int>> trans; //transition
    int _LCS(int p1, int p2){
        if(p2 == sp[p1]) return lrs[p1];
        while(sp[p2] != sp[p1]) p2 = sp[p2];
        return std::min(lrs[p1], lrs[p2]);
    }
    
    std::vector<InteractiveTango::MidiNote> alphabet;
    std::vector<int> sp;
    std::vector<InteractiveTango::MidiNote> midiNotes;

    
public:
    FactorOracle()
    {
        reset(); //init everything
    }
    
    //should be the # of elements -- int lrs[N];
    std::vector<int> lrs; // lengthRepeatedSuffix
    // suffix link;
    // sp[i] = j , means the first ocurrence of the repeated suffix of state i ends in j
    
    //sp[N]
    int n;
    void reset(){
        n = 0;
        sp.clear();
        lrs.clear();
        
        sp.push_back(-1);
        lrs.push_back(0);
    }
// unused for this context
//    void add_sep(){
//        n++;
//        sp.push_back( 0 );
//        lrs. push_back( 0 );
//    }
    void add_letter(InteractiveTango::MidiNote cc){
        // cc: char code
        int j, p1;
        
        int alphaIndex = getAlphaIndex(cc);
        midiNotes.push_back(cc);
        addTransition();
        
        trans[n][alphaIndex] = n + 1;
        j = sp[p1=n];
        ++n;
        while(j != -1 and trans[j][alphaIndex] == -1){
            trans[j][alphaIndex] = n;
            j = sp[p1=j];
        }
        sp[n] = j == -1 ? 0 : trans[j][alphaIndex];
        lrs[n] = sp[n] == 0 ? 0 : _LCS(p1, sp[n]-1) + 1;
    };
    
    void addTransition()
    {
        std::vector<int> alpha;
        for(int i=0; i<alphabet.size(); i++)
            alpha.push_back(0);
        trans.push_back(alpha);
        sp.push_back(0);
        lrs.push_back(0);
    }
    
    int getAlphaIndex(InteractiveTango::MidiNote letter)
    {
        bool found = false;
        int i = 0;
        while(!found && i < alphabet.size())
        {
            found = letter == alphabet[i];
            i++;
        }
        
        //maybe keep in sorted list if need to optimize -- CDB
        if(!found)
        {
            //grow alphabet by one letter
            alphabet.push_back(letter);
            
            //grow the alphabet in trans
            for(int i=0; i<trans.size(); i++)
            {
                trans[i].push_back(0);
            }
            return i;
        }
        else
        {
            return i-1;
        }
    };
    
    InteractiveTango::MidiNote getAlphabet(int index)
    {
        if(index >= 0 && index < alphabet.size())
            return alphabet[index];
        else
        {
            InteractiveTango::MidiNote note;
            note.tick = -1;
            note.pitch = -1;
            return note; //not found
        }
    }
    
    std::vector<int> getTransitions(int tr)
    {
        return trans[tr];
    };
    
    int getSuffixLink(int i)
    {
        return sp[i];
    };
    
    InteractiveTango::MidiNote getMidiNote(int transition)
    {
        return midiNotes[transition];
    }
    
    //returns -1 if can't find note.....
    int getFirstTransitionFromMidiNote(InteractiveTango::MidiNote mid)
    {
        bool found = false;
        int i =0;
        while(i<midiNotes.size() && !found)
        {
            found = midiNotes[i] == mid;
            i++;
        }
        if( found )
            return i;
        else return -1;
    };
    
    int transitionSize()
    {
        return trans.size();
    }
    
};
    
}

//FactorOracle fo;
//
//int main(){
//    string s;
//    int i;
//    int cc;
//    cin >> s;
//    fo.reset();
//    For(i, s.size()){
//        cc = s[i] - 'a';
//        if(cc < E and cc >= 0)
//            fo.add_letter(cc);
//        else
//            fo.add_sep();
//        printf("%2d: %c lrs:%2d sp:%2d\n", i+1, s[i], fo.lrs[i+1], fo.sp[i+1]);
//    }
//}


//TODO: read https://arxiv.org/abs/1411.0698
//on adding supervisory controls to melody -- altho not sure really want for something more experimental...


//TODO: how to label parts of melodies, rhythms, etc. as corresponding to movement, thus can generate based on movement.... ??

//a thought -- harmony can be seen as corresponding to motion -- what if there was more functional/moving harmony when there is more movement... not related to melody generation but perhaps an interesting direction

//a thought --
//label collection of harmony/melody as good for busy/sparse, smooth vs sharp, try heavy/light as well...
//then draw from melodies in that space when movment is in that space

namespace InteractiveTango {
    
class FactorOracle : public MelodyGeneratorAlgorithm
{
private:
    std::string dbfileName;
    Liang::FactorOracle oracle;
    int genIndex;
    std::vector<MidiNote> generatedMidiNotes;
    float choiceBeweenSuffixProb;
    
public:
    FactorOracle() : MelodyGeneratorAlgorithm()
    {
        reset();
        choiceBeweenSuffixProb = 0.3;
    }
    
    void setProbabilityContinueVsSuffixLink(float p)
    {
        choiceBeweenSuffixProb = p;
    }
    
//fix later
    
//    void testTrain()
//    {
//        oracle.reset();
//        int dataset[] = {'A','B','A','B','A','B','A','B','A','A','B','B'};
//        for(int i=0; i<13; i++)
//        {
//            oracle.add_letter(dataset[i]);
//        }
//        
//        //checks mods to factor oracle algorithm using example given in  (Assayag & Dubnov, 2004)
//        // -1  0  0  1  2  3  4  5  6  7  0  0  0
//
//        
//        std::cout << "\n------Test Train-------------------------------------------\n";
//        
//        std::cout << "\n";
//        for(int i=0; i<oracle.transitionSize(); i++)
//        {
//            std::cout << " " << oracle.getSuffixLink(i) << " ";
//        }
//        std::cout << "\n-----------------------------------------------------------\n";
//        
//    }
    
    void reset()
    {
        genIndex = 0;
        oracle.reset();
    }
    
    
    
    //right now expects a midi notes -- need to check with this new schema but checked out before
    virtual void train(std::string _dbfileName, int track=1)
    {
        MelodyGeneratorAlgorithm::train(_dbfileName, track);
        
        dbfileName = _dbfileName;
        
        MidiFileUtility midiFile;
        midiFile.readMidiFile(_dbfileName);
        
        std::vector<MidiNote> notes = midiFile.getMelody(track);
        for(int i=0; i<notes.size(); i++)
        {
            oracle.add_letter(notes.at(i));

        }
        
//        std::cout << "\n";
//        for(int i=0; i<oracle.transitionSize(); i++)
//        {
//            std::cout << " " << oracle.getSuffixLink(i) << " ";
//        }
//        std::cout << "\n-----------------------------------------------------------\n";
    }

    
    //sets the generator index (into the trained factor oracle to a different place in the midi)
    void setGenIndexToMidiNote(InteractiveTango::MidiNote midi)
    {
        genIndex = oracle.getFirstTransitionFromMidiNote(midi);
        
        //if midi doesn't exist here choose at random.
        if(genIndex <= -1)
        {
            genIndex = (int) std::round((((double) std::rand()) / ((double) RAND_MAX)) * oracle.transitionSize());
        }
    }
    
    //returns a midi note, if seed is -1 then it is start
    //from Assayag & Dubnov, 2004
    InteractiveTango::MidiNote generateNext()
    {
        float probOfChoice = choiceBeweenSuffixProb;
        
        //if at the end, take the suffix link
        if (oracle.getSuffixLink(genIndex) == 0)
            probOfChoice   =  1;
        
        double choose =((double) std::rand()) / ((double) RAND_MAX);
        if(choose <= probOfChoice)
        {
            genIndex++; //move forward a transition (as determined from prev)
        }
        else
        {
            //choose a suffix backwards transition from oracle.sp[]
            
            //introduce some probability of choosing a suffix at other than the genIndex
            /*
            An interesting option is to consider not only the suffix link starting at i, but the whole suffix
            chain Sn(i), then choose some element in this chain with regard to some criterion. For
            example, the suffix length lrs can be used : choosing a smaller lrs will result again in more
            variety, with smaller factors duplicated from w. A probability (Assayag & Dubnov, 2004)
            */
            
            //use the suffix to transition backwards in the tree.
            genIndex = oracle.getSuffixLink(genIndex);
            
            
        }
//        std::cout << " g:" << genIndex << " ";
        
        return oracle.getMidiNote(genIndex);
    }
};
}


#endif /* FactorOracle_h */
