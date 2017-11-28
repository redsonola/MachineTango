//
//  SendOSCUGENs.h
//  InteractiveTangoReadFromAndroid
//
//  Created by courtney on 3/17/16.
//
//

#ifndef InteractiveTangoReadFromAndroid_SendOSCUGENs_h
#define InteractiveTangoReadFromAndroid_SendOSCUGENs_h


#endif

using namespace cinder;

namespace InteractiveTango {

//prepares OSC  to wekinator, rather than Ableton, the main output
//since wekinator needs all inputs in one message -- for now only sending feet values
    
class SendOSCToWek : public UGEN
{
protected:
    std::vector<std::vector<ShimmerData *>> data; //expecting the feet data for now.. but can change if add more
    std::vector<SignalAnalysis *> signals;
    std::vector<std::vector<float>> floatsToSend;
    
    //for scaling... hacky but need to do this fast
    std::vector<float> mins, maxes, whichToSend;

    int bufferSize;
    std::string osc_addr;
    
public:
    //OK, now trying something different.
    SendOSCToWek(std::string oscAddr=WEKINATOR_INPUTS, int bufsize=1 ) : UGEN()
    {
        bufferSize = bufsize;
        osc_addr = oscAddr;
        
        maxes = {25,     25,     25,     500,     500,     500,   8.39760,    24.63855,    10.80725 };
        mins = {-25,    -25,    -25,    -500,    -500,    -500, -24.45785,   -24.39760,   -24.38555 };

    };
    
    
    void addSignalToSend(SignalAnalysis * sig)
    {
        signals.push_back(sig);
    };
    
    virtual int fillBuffers()
    {
        bool enoughData = true;
        data.clear(); floatsToSend.clear();
        int newSampleCount = signals[0]->getNewSampleCount(); //find the number of new samples which all signals in the stream have

        for(int i=0; i<1 && enoughData; i++)
        {
            std::vector<ShimmerData *> curBuffer = signals[i]->getBuffer();
            enoughData = curBuffer.size() > 0 && signals[i]->getNewSampleCount() > 0;
            
            if (newSampleCount > signals[i]->getNewSampleCount())
                newSampleCount = signals[i]->getNewSampleCount() ;
            
            if (newSampleCount > curBuffer.size())
                newSampleCount = curBuffer.size();
            
            data.push_back(curBuffer);
        }
        
        if (!enoughData) return 0;
        else return newSampleCount;
    };
    
    float scale(float val, int i)
    {
        float newVal;
        float denom = ( maxes[i] - mins[i] );
        if( denom == 0 )
        {
            newVal = 0;
        }
        else newVal = (val - mins[i]) / denom;
        
        return (newVal * 2) -1;
    }
    
    virtual void update(float seconds = 0)
    {
        //sanity check
        if(signals.size() < 1)
        {
            return;
        }
        
        //update data buffers
        int newSampleCount = fillBuffers();
        if( newSampleCount <= 0 )
        {
           return;
        }
        
        //OK, get the floats to send -- do backwards first, so send most current data -- note, this means sends backwards as well
        //send ACCEL
        
        for ( int i=1; i<=newSampleCount;i++ )
        {
            std::vector<float> toSend, sends;

            std::vector<ShimmerData * > shimmers = data[0];
            ShimmerData *shimmerdata = shimmers[shimmers.size()-i];

            if(shimmerdata != NULL)
            {
                for( int k=2; k<5; k++ ) // add accels
                    toSend.push_back(shimmerdata->getData(k) );
                
                for( int k=11; k<14; k++ ) // add accels
                    toSend.push_back(shimmerdata->getData(k) );
            }
                
            //get signal peaks of jerk
            SignalAnalysisEventOutput *saeo = (SignalAnalysisEventOutput *) signals[1];
            toSend.push_back( saeo->getMotionDataItem(12)->asDouble() );
            toSend.push_back( saeo->getMotionDataItem(13)->asDouble() );
            toSend.push_back( saeo->getMotionDataItem(14)->asDouble() );
        
            for(int sIndex=0; sIndex<maxes.size(); sIndex++)
            {
                sends.push_back( scale(toSend[sIndex], sIndex) * 200);
            }
            floatsToSend.push_back(sends);
        }
    };
    
    virtual std::vector<ci::osc::Message> getOSC()
    {
        std::vector<ci::osc::Message> msgs;
        
        if(signals.size() < 1)
        {
            return msgs;
        }
        
        //record to find scaling for DTW
//        std::ofstream outfile;
//        outfile.open("sendingToWekText2LiveAction.txt", std::ios::app);
        
        //iterate backwards since data was entered in backwards
        for(int i=floatsToSend.size()-1; i>=0; i--)
        {
            osc::Message message;
            message.setAddress( osc_addr );
            for( int j=0; j<floatsToSend[i].size(); j++ )
            {
                message.addFloatArg(floatsToSend[i].at(j));
//                outfile << floatsToSend[i].at(j) << ",";
            }
            msgs.push_back( message );
//            outfile << std::endl;
        }
        return msgs;
    };
};
    
};

