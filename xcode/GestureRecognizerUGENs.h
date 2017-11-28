//
//  GestureRecognizerUGENs.h
//  InteractiveTangoReadFromAndroid
//
//  Created by courtney on 4/4/16.
//
//

#ifndef InteractiveTangoReadFromAndroid_GestureRecognizerUGENs_h
#define InteractiveTangoReadFromAndroid_GestureRecognizerUGENs_h


#endif

namespace InteractiveTango {
    
enum TangoGestures { STILLNESS=0, CIRCLING=1, STEPS=2, OCHOS=3, BOLEO=4 };


//TODO: add snappeaks for kicks...
class RecognizedGestures : public UGEN, public MotionDataOutput
{
protected:
    std::vector<TangoGestures> buffer;
    std::vector<double> times;
    std::vector<double> stepOnsetTimes;
    
    int bufferSize;
    TangoGestures curGesture, prevGesture;
    float secondsToSmoothCircling;
    bool testMode;
    
    //using to make more intelligent guesses -- eg., if a foot onset peak is detected then not circling and vice versa
    //could also use probabilities as to whether to use the info or not...
    SnapPeaks *peaks;
    BeatTiming *mTimer;
    
public:
    RecognizedGestures(SnapPeaks *p, BeatTiming *timer, int buffersz=500, float secsCirclingSmoothing=4.5, bool test = false)
    {
        bufferSize = buffersz;
        motionData.push_back(new MotionAnalysisEvent(MotionAnalysisDataType::IntEvent, 0));
        motionData[0]->setName("Recognized Tango Gesture");
        curGesture = TangoGestures::STILLNESS;
        secondsToSmoothCircling = secsCirclingSmoothing;
        testMode = test;
        mTimer = timer; 
        
        peaks = p;
    };
    
    virtual void trimBuffer()
    {
        while( buffer.size() > bufferSize )
        {
            buffer.erase(buffer.begin());
            times.erase(times.begin());
        }
        while(stepOnsetTimes.size() > bufferSize )
        {
            stepOnsetTimes.erase(stepOnsetTimes.begin());
        }
    };
    
    virtual void updateMotionData()
    {
        motionData[0]->setValue(  int(curGesture)  );
    };
    
    virtual void update(float seconds=0)
    {
        trimBuffer();
        prevGesture = curGesture;
        if (peaks->combinedPeak) {
            stepOnsetTimes.push_back(seconds);
//            std::cout << "PEAK!\n";
        }
        
        if (buffer.size() <= 0)
        {
            return;
        }
        else if (buffer.size() == 1 )
        {
            curGesture = buffer[0];
        }
        else if( ( times[times.size()-1] - times[times.size()-2] <= secondsToSmoothCircling ) || testMode )
        {
            curGesture = buffer[buffer.size()-1];
        }
        else
        {
            curGesture = buffer[buffer.size()-1];
            if( curGesture != TangoGestures::BOLEO ) //do not smooth if boleo, recognize instantly!!
            {
                curGesture = smoothedGesture( seconds );
//                if( curGesture == TangoGestures::STEPS || curGesture == TangoGestures::CIRCLING )
//                {
//                    curGesture = CirclingOrStep();
//                }
            }
        }
        
        if ( curGesture != prevGesture )
        {
            std::cout << curGestureToString() << "\n";
        }
        updateMotionData();
    };
    
    std::string curGestureToString()
    {
        return gestureToStr(curGesture);
    }
    
    std::string gestureToStr(TangoGestures gesture)
    {
        std::string strgesture;
        switch (gesture) {
            case TangoGestures::STILLNESS:
                strgesture = "STILLNESS";
                break;
            case TangoGestures::CIRCLING:
                strgesture = "CIRCLING";
                break;
            case TangoGestures::BOLEO:
                strgesture = "BOLEO";
                break;
            case TangoGestures::STEPS:
                strgesture = "STEPS";
                break;
            case TangoGestures::OCHOS:
                strgesture = "OCHOS";
                break;
            default:
                break;
        }
        
        return strgesture;
    };
    
    //counts how many onsets in a window defined by beats
    int beatsInWindow(float beatWindow)
    {
        float timeSum =0;
        int index = stepOnsetTimes.size()-2 ;
        while( index > -1 && ( stepOnsetTimes[ stepOnsetTimes.size()-1 ] - stepOnsetTimes[index] < beatWindow ) )
        {
            timeSum++;
            index--;
        }
        return timeSum;
    };
    
    //use the step detection to make a more informed decision
    virtual TangoGestures CirclingOrStep()
    {
        //how long a beat & 1/2 is during current tempo
        float beatWindow = mTimer->getPulse8() * 8;
        
        if( beatsInWindow(beatWindow) > 1 )
        {
//            std::cout << "it is a step! from beats\n";
            return TangoGestures::STEPS;
        }
        else
        {
            beatWindow = mTimer->getPulse8() * 16; //if a whole 2 bars with no onsets... then probably circling
            if( beatsInWindow(beatWindow) <= 0 )
            {
//                std::cout << "it is a circle! from beats\n";
               return TangoGestures::CIRCLING;
            }
            else return curGesture; //nothing conclusive so go with what has been found
        }
    };
    
    int getIndexInTimeWindow(float seconds = 4)
    {
        int index = 0;
        while( seconds < times[times.size()-1] - times[index] && index < times.size() )
            index++;
        return index;
    };
    
    virtual TangoGestures smoothedGesture(float seconds)
    {
        const int GESTURE_NUM = 5;
        int maxTimeIndex = getIndexInTimeWindow( secondsToSmoothCircling );
        double weightingByTimeAndIncident[GESTURE_NUM]; //stillness circling steps ochos
        
        //initialize weighting array
        for(int i=0; i < GESTURE_NUM; i++)
        {
            weightingByTimeAndIncident[i]=0;
        }
        
        int index = buffer.size() - 1;
        float recencyWeight = 1;
        float recencyDecrement = 1.0/( ( buffer.size() - maxTimeIndex) * 100);
        while ( index >= maxTimeIndex )
        {
            weightingByTimeAndIncident[buffer[index]] += ( times[index] - times[index-1] ) * recencyWeight;
            recencyWeight -= recencyDecrement;
            index--;
        }
        
        //add current time to recency weight (not just past time)
        weightingByTimeAndIncident[ buffer[buffer.size()-1] ] = seconds - times[times.size()-1];
        
        //TODO -- handle equal weights by favoring most recent...
//        std::cout <<"Gestures\n" << "maxTimeIndex: " << maxTimeIndex << std::endl;
        int maxIndex = 0;
        for(int i=0; i < GESTURE_NUM; i++)
        {
//            std::cout << gestureToStr(TangoGestures(i)) << ": " << weightingByTimeAndIncident[i] << std::endl;
            if( weightingByTimeAndIncident[maxIndex] <= weightingByTimeAndIncident[i]  )
                maxIndex = i;
        }
//        std::cout << "------------------------------------------\n";
        
        if( weightingByTimeAndIncident[maxIndex] == 0 ) return curGesture; //if nothing in buffer, return curGesture
        return TangoGestures(maxIndex);
    };
    
    virtual void addRecognizedGesture(TangoGestures gesture, float seconds)
    {
        buffer.push_back(gesture);
        times.push_back(seconds); 
    };
    
    //mostly for testing
    virtual void addCircling( float seconds )
    {
        addRecognizedGesture(TangoGestures::CIRCLING, seconds);
    };
    virtual void addBoleo( float seconds )
    {
        addRecognizedGesture(TangoGestures::BOLEO, seconds);
    };
    virtual void addStillness( float seconds )
    {
        addRecognizedGesture(TangoGestures::STILLNESS, seconds);

    };

    
    
//#define WEKINATOR_STILLNESS "/stillness"
//#define WEKINATOR_CIRCLING "/circling"
//#define WEKINATOR_BOLEO "/boleo"
//#define WEKINATOR_STEPS "/steps"
//#define WEKINATOR_OCHOS "/ochos"
//#define INTERACTIVE_TANGO_PREFIX "/InteractiveTango"
    
    std::string createOSCAddress()
    {
        std::string addr = INTERACTIVE_TANGO_PREFIX;
        switch (curGesture) {
            case TangoGestures::STILLNESS:
                addr += WEKINATOR_STILLNESS;
                break;
            case TangoGestures::CIRCLING:
                addr += WEKINATOR_CIRCLING;
                break;
            case TangoGestures::BOLEO:
                addr += WEKINATOR_BOLEO;
                break;
            case TangoGestures::STEPS:
                addr += WEKINATOR_STEPS;
                break;
            case TangoGestures::OCHOS:
                addr += WEKINATOR_OCHOS;
                break;
            default:
                break;
        }
        
        return addr;
    };
    
    virtual TangoGestures getCurGesture()
    {
        return curGesture;
    };
    
    virtual TangoGestures getPrevGesture()
    {
        return prevGesture;
    };
    
    virtual std::vector<ci::osc::Message> getOSC()
    {
        std::vector<ci::osc::Message> msgs;
        if( testMode )
        {
            ci::osc::Message msg;
            msg.setAddress(createOSCAddress());
            msgs.push_back(msg);
        }
        return msgs;
    };

};
    
//-------------------------------------------
//-------------------------------------------


//ornaments will send their own OSC -- unlike melody or accompaniment
class GestureToMusic :  public MappingSchema
{
protected:
    RecognizedGestures *curGesture;
    bool sendPlay;
    bool stillPlay;
public:
    GestureToMusic(RecognizedGestures *grecognition, BeatTiming *timer) : MappingSchema(timer)
    {
        curGesture = grecognition;
        sendPlay = false;
        stillPlay = false;
    };
    
    virtual bool shouldSendPlay()
    {
        return sendPlay;
    };
    
    virtual bool stillPlaying() //circling and such
    {
        return stillPlay;
    }
    
};
    
//this will send for a repeating figure which grows louder with how long you circle
// is busy or sparse depending on that dancer's movement -- just the actual leg?
// is pointy or sparse based on that leg only
// need to get measures for -- how long to circle? (what is a maximum?) -- and how... busy
    

class CirclingGestures : public GestureToMusic
{
protected:
    float timeSinceStartedCircling;
    float timeStartedCircling;
    
    //
    float maxTimeCircling;
    float minTimeCircling;
    
    bool changedToCircling;
    bool stoppedCircling;
    
    SignalAnalysis *filteredInput;
    SignalVector *filteredInputVector;
    
public:

    CirclingGestures(RecognizedGestures *grecognition, BeatTiming *timer, SignalAnalysis *fiv) : GestureToMusic(grecognition, timer)
    {
        timeSinceStartedCircling = 0;
        timeStartedCircling = -1;
        filteredInputVector = (SignalVector * ) fiv;
        
        //create new motion data
        motionData.push_back(new MotionAnalysisEvent(MotionAnalysisDataType::DoubleEvent, 0));
        motionData[0]->setName("TimeCircling");
        
        //guesssessssss.......
        minTimeCircling = 0;
        maxTimeCircling = timer->getPulse8()*16 * 1; //default 2 bars
        
        motionData[0]->setMinMax(minTimeCircling, maxTimeCircling); //default half a bar....
        
    };
    
    virtual void update(float seconds = 0)
    {
//        std::cout << curGesture->gestureToStr(curGesture->getCurGesture()) << std::endl;
        
        if( curGesture->getCurGesture() == TangoGestures::CIRCLING )
        {
            if(timeStartedCircling == -1)
            {
                timeStartedCircling = seconds;
            }
            timeSinceStartedCircling = seconds - timeStartedCircling;
            changedToCircling = curGesture->getPrevGesture() != TangoGestures::CIRCLING;
            stoppedCircling = false;
            
            if ( changedToCircling )
            {
                std::cout << "CIRCLING!\n";
            }
            
            stillPlay = true;
        }
        else
        {
            if( timeSinceStartedCircling > 0 ) timeSinceStartedCircling -= 0.3;
            timeStartedCircling = -1;
            changedToCircling = false;
            stoppedCircling = curGesture->getPrevGesture() == TangoGestures::CIRCLING;
            
            if ( stoppedCircling )
            {
                std::cout << "STOPPED CIRCLING!\n";
            }
        }
        sendPlay = changedToCircling;
        stillPlay = (timeSinceStartedCircling > 0 );
        
        updateMotionData();
    };
    
    virtual void updateMotionData()
    {
        motionData[0]->setValue(timeSinceStartedCircling);
    };
    
    //CIRCLING_VOLUME  "/InteractiveTango/Circling/Volume"
    std::vector<ci::osc::Message> getOSC()
    {
        std::vector<ci::osc::Message> msgs;
        osc::Message msg;
        osc::Message gMsg;
        
        if( stillPlay )
        {
            msg.setAddress(CIRCLING_VOLUME);
            msg.addFloatArg( motionData[0]->scaledValue() );
            
//            std::ofstream outfile;
//            outfile.open("CIRCLING_GYRO_VECTOR_LENGTH.txt", std::ios::app);
//            outfile << filteredInputVector->getMotionData().at(0)->asFloat() << std::endl;
//            outfile.close(); 
            
//            gMsg.setAddress(CIRCLING_GYRO_VECTOR_LENGTH);
//            gMsg.addFloatArg( filteredInputVector->getMotionData().at(0)->asFloat() );
            
            msgs.push_back(msg);
        }
        
        return msgs;
    };
    
};

    
//-------------------------------------------
//-------------------------------------------
    class BoleoGestures : public GestureToMusic
    {
    protected:
        bool changedToBoleo;
        SnapPeaks *peaks;
    public:
        BoleoGestures(SnapPeaks *p, RecognizedGestures *grecognition, BeatTiming *timer) : GestureToMusic(grecognition, timer)
        {
            //create new motion data
            motionData.push_back(new MotionAnalysisEvent(MotionAnalysisDataType::IntEvent, 0));
            motionData[0]->setName("BoleoTrigger");
            
            changedToBoleo = 0;
            peaks = p;
        };
        
        virtual void updateMotionData()
        {
            motionData[0]->setValue(changedToBoleo);
        };
        
        //for now only do
        virtual void update(float seconds = 0)
        {
            //add snap peaks info, if needed.....
            changedToBoleo = ( curGesture->getCurGesture() == TangoGestures::BOLEO ) && ( curGesture->getPrevGesture() != TangoGestures::BOLEO );
            sendPlay = changedToBoleo;
            
            updateMotionData();
        };
        
        std::vector<ci::osc::Message> getOSC()
        {
            std::vector<ci::osc::Message> msgs;
            return msgs;
        };

    };
    
};
