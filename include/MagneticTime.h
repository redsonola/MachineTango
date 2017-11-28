//
//  MagneticTime.h
//  MagneticGardel
//
//  Created by courtney on 8/27/17.
//
//

#ifndef MagneticTime_h
#define MagneticTime_h

#include "InteractiveTangoSignalAnalysis.h"
#include "ScrambleWindow.h"


#define PLAY_CLIP_MAGNETIC_SLICE "/InteractiveTango/MagneticTime/playClip"

namespace InteractiveTango {
   
    //WTF MATE? -->TODO: fix
//    class UGEN
//    {
//    public:
//        UGEN(){};
//        virtual std::vector<ci::osc::Message> getOSC() = 0;
//        virtual void update(float seconds=0) = 0;
//    };

class MagneticTime : public UGEN
{
private:
    int totalFrames; //total frames in the file
    int channels;//number of channels -- default is 2
    float totalDuration; // total duration of the song in milliseconds
    
    int numberOfFragmentsPerSong; //how many fragments in the song? -- this will start small-sh, then end when a fragment is the whole song
    int numberOfAllowableFragmentOverlap; //this will start large then get smaller
    
    int fragmentIndex; //where we are in larger sound file
    float fragmentDuration; //how long is the fragment?
    float fragmentStartinMS; //when the current fragment starts
    float fragmentEndinMS; //when the current fragment ends
    
    
    int scrambleIndex; //where we are in scrambled, within fragmentsc
    float scrambleLengthMS; //the total length of the each scramble window
    std::vector<float> actualStartTimes; //a list of all the actual start times of the fragments
    std::vector<float> postScrambleStartTimes; //after start times are randomized
    int howManyScrambles; //how many scrambles in each fragment
    float secondsSinceScrambleWindowStart;
    float scrambleSliceFileStart;
    float scrambleSliceDur;
    
    std::vector<float> *randomDisplacements; //how much to displace ea. scrambled file
    int numberOfScrambleCycles = 15; //this will cycle per fragment not per song
    
    float bpm; //tempo of song
    
    int magneticSectionIndex; //there are 5, based on fragment length
    
    std::vector<ScrambledWindow *> scrambles;
    
    
public:
    MagneticTime() : UGEN()
    {
        totalFrames = 6803748;
        channels = 2;
        totalDuration = 154280;
        
        bpm = 120.0;
        
        
        //first recreate prior magnetic time code
        //do about 1/8 of por una cabeza
        float testDuration = 10*1000;// totalDuration / 80.0f;
        
        //in seconds, will have to xlate to millis
        float d[] = {4.0, 2.5, 1.0, 0.75, 0.5, 0.1, 0.0 };
        randomDisplacements = new std::vector<float>(d, d + sizeof(d) / sizeof(d[0]) );
        numberOfScrambleCycles = randomDisplacements->size();
        
        float testSliceCount = 60;
        
        for (int i=0; i<numberOfScrambleCycles; i++)
        {
            //xlate to millis for this call
            ScrambledWindow *sw = new ScrambledWindow(testDuration, d[i]*1000.0, testSliceCount);
            scrambles.push_back(sw);
//            testSliceCount--;
        }
        
        scrambleIndex = 0;
        secondsSinceScrambleWindowStart = 0;
        scrambleSliceFileStart = INBETWEEN_SCRAMBLE_START_TIMES;
        
    };
    
    //ok now just recreate magnetic time
    
    //find number of fragments based on starting with 1/8, then 1/4, 1/2/ then measure, then 2 measure, then go to the end
    void setFragmentDuration(int sectionIndex)
    {
        fragmentDuration = 8; //fix
    };
    
    
    virtual std::vector<ci::osc::Message> getOSC()
    {
        std::vector<ci::osc::Message> msgs;
        ci::osc::Message msg;
        
//        std::cout << scrambleSliceFileStart << "   " << std::endl;

        
        if(scrambleSliceFileStart!= INBETWEEN_SCRAMBLE_START_TIMES && scrambleSliceFileStart != SCRAMBLE_END)
        {
            msg.setAddress(PLAY_CLIP_MAGNETIC_SLICE);
            msg.addFloatArg(scrambleSliceFileStart);
            msg.addFloatArg(scrambleSliceDur);
        
            msgs.push_back(msg);
        }
        
        return msgs;
    };
    
    virtual void update(float seconds=0)
    {
        if(scrambleIndex >= scrambles.size()) return;
        
        ScrambledWindow *sw = scrambles[scrambleIndex];
        scrambleSliceFileStart = sw->play( secondsSinceScrambleWindowStart );

        secondsSinceScrambleWindowStart+=seconds;
        
        if( scrambleSliceFileStart == SCRAMBLE_END )
        {
            scrambleIndex++;
            std::cout << scrambleIndex << "     ";
            
            if(scrambleIndex < scrambles.size())
            {
                secondsSinceScrambleWindowStart = 0;
            }
            
        }
        else if( scrambleSliceFileStart != INBETWEEN_SCRAMBLE_START_TIMES )
        {
             scrambleSliceDur = sw->sliceDuration();
        }
        

    };
    
    int totalSamples()
    {
        return totalFrames * channels;
    }
    
    float durationInMs()
    {
        return totalDuration;
    }
    
};

}

#endif /* MagneticTime_h */
