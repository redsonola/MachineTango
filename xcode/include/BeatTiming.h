//
//  BeatTiming.h
//  InteractiveTangoReadFromAndroid
//
//  Created by courtney on 11/5/14.
//
//

#ifndef InteractiveTangoReadFromAndroid_BeatTiming_h
#define InteractiveTangoReadFromAndroid_BeatTiming_h

namespace InteractiveTango  {
    
class BeatTimerSaveToFile
{
    
protected:
    std::string filename;
public:
        
BeatTimerSaveToFile(std::string f)
{
    std::stringstream ss;
    ss << f << "_beats.csv";
    filename = ss.str();
};
        
virtual void save(float seconds, int mNum, int beatNum, float s, float curb)
{
    std::ofstream myfile(filename, std::ios::app);
    myfile << seconds << "," << mNum << "," << beatNum << "," << s << "," << curb << std::endl;
    myfile.close();
};

};

    
class BeatTiming
{

public:
    enum class Beats {EIGHTH, SIXTEENTH, QUARTER};
    
    void setBPM(float tempo)
    {
        bpm = tempo;
        bps = tempo / 60.0;
        pulse8ps = bps / 8.0;
        pulse16ps = bps / 16.0;
        updateError();
        
    };
    
    void setWhichPulse(Beats pulse)
    {
        whichPulse = pulse;
        switch( whichPulse )
        {
            case Beats::EIGHTH:
                curPulse = pulse8ps;
                break;
            case Beats::SIXTEENTH:
                curPulse = pulse16ps;
                break;
            case Beats::QUARTER:
                curPulse = 0;
                break;
            default:
                break;
        }
        
    };
    
    float getPulse16()
    {
        return pulse16ps;
    };
    
    float getPulse8()
    {
        return pulse8ps;
    };
    
    void setError(float err) // in percentage of a 16 note (for now)
    {
        percentError = err;
        updateError();
    };
    
    BeatTiming(float tempo = 100.0, float err = 0.25)
    {
        setBPM(tempo);
        setError(err);
        lastBeat = 0;
        setWhichPulse( Beats::EIGHTH );
        saver = NULL;
    };
    
    float getBPM()
    {
        return bpm; 
    };
    
    void setLastBeat(float secs, int mNum, int beatNum, float s, float curb)
    {
        lastBeat = secs;
        
        //save a beat
        if(saver!=NULL && onBeat)
        {
            saver->save(secs, mNum, beatNum, s, curb);
        }
    };
    
    void save(std::string filename)
    {
        saver = new BeatTimerSaveToFile(filename);
    };
    
    void update( float curTime )
    {
//        updateLastBeat(curTime);
        float minTime = lastBeat - marginOfError;
        float maxTime = lastBeat + marginOfError;
        onBeat =  (curTime >= minTime && curTime <= maxTime);
    };
    
    bool isOnBeat(){ return onBeat;  };
    
    bool isOnBeat(float err, float curTime) //for custom error percentages
    {
        float minTime = lastBeat - err;
        float maxTime = lastBeat + err;
        return (curTime >= minTime && curTime <= maxTime);
    };

    
    float getMarginOfError()
    {
        return marginOfError;
    };

protected:
    float bps ;
    float bpm;
    float pulse8ps; //in seconds
    float pulse16ps; //in seconds
    float curPulse;
    float marginOfError;
    float percentError;
    float lastBeat; //(in seconds)
    float onBeat;
    Beats whichPulse;
    BeatTimerSaveToFile *saver;
    
    void updateError()
    {
        marginOfError = pulse16ps * percentError;
    };
    
    void updateLastBeat(float curTime)
    {
        if( lastBeat + curPulse >= curTime  )
        {
            {
                lastBeat += pulse8ps;
            }
        }
    }
    
};


};

#endif
