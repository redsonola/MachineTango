//
//  ScrambleWindow.h
//  MagneticGardel
//
//  Created by courtney on 8/29/17.
//
//

#ifndef ScrambleWindow_h
#define ScrambleWindow_h

#define INBETWEEN_SCRAMBLE_START_TIMES -1
#define SCRAMBLE_END -2

class ScrambledWindow
{
private:
    std::vector<float> fileStartTimes; //a list of all the actual start times of the fragments
    std::vector<float> postScrambleStartTimes; //after start times are randomized
    float slices; //how many scrambles in each fragment
    float duration; //the length of this window
    float randomDisplacement;
    float sliceDur; //the duration of ea. file/song slice
    
    int index;
public:
    
    ScrambledWindow(float dur, float displace, float sliceCount )
    {
        slices = sliceCount;
        duration = dur;
        randomDisplacement = displace;
        findScrambleTimes();
        
        index = 0;
    };
    
    void findScrambleTimes()
    {
        srand(time(NULL));
        
        //find actual times of slices
        sliceDur = duration / ((float)slices);
        for(int i=0; i<slices; i++)
        {
            fileStartTimes.push_back( sliceDur*i );
        }
        
        //displace them relative to start
        for(int i=0; i<slices; i++)
        {
            //find the new start time
            float displace =((double)std::rand() )/((double)RAND_MAX) * randomDisplacement;
            float plusOrMinus = ((double)std::rand() )/((double)RAND_MAX);
            if (plusOrMinus < 0.5) displace = -displace;
            
            float newStartTime = displace + fileStartTimes[i];
            if(newStartTime < 0.0) newStartTime = 0.0;
            postScrambleStartTimes.push_back(newStartTime);
        }
        
        //ok, now insertion sort the lists according to which ones now start first
        std::vector<float> fileStartTimeTemp;
        std::vector<float> afterScrambleTimeTemp;
        
        fileStartTimeTemp.push_back(fileStartTimes[0]);
        afterScrambleTimeTemp.push_back(postScrambleStartTimes[0]);
        for(int i=1; i<slices; i++)
        {
            std::vector<float>::iterator startsIter = fileStartTimeTemp.begin();
            std::vector<float>::iterator scrambleIter = afterScrambleTimeTemp.begin();

            while ( *scrambleIter < postScrambleStartTimes[i] && scrambleIter != afterScrambleTimeTemp.end()  )
            {
                startsIter++;
                scrambleIter++;
            }
            
            afterScrambleTimeTemp.insert(scrambleIter, postScrambleStartTimes[i]);
            fileStartTimeTemp.insert(startsIter, fileStartTimes[i]);

        }
        postScrambleStartTimes = afterScrambleTimeTemp;
        fileStartTimes = fileStartTimeTemp;
        
    }
    
    void reset()
    {
        index= 0;
    }
    
    //returns the file start time if time to play
    float play(float seconds)
    {
        if( index >= postScrambleStartTimes.size() ) return SCRAMBLE_END;
        if( postScrambleStartTimes[index] <= seconds*1000.0 )
        {
            index++;
//            std::cout << "here - " << fileStartTimes[index-1] <<  "," << sliceDur << std::endl;
            return fileStartTimes[index-1];
        }
        else return INBETWEEN_SCRAMBLE_START_TIMES;
    }
    
    float getSliceDuration()
    {
        return duration;
    }
    
    float sliceDuration()
    {
        return sliceDur;
    }
};

#endif /* ScrambleWindow_h */
