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
        
        bool recordOSC;
        std::string whatRecording;
        Timer mTimer; //for recording
        
        
    public:
        //OK, now trying something different.
        SendOSCToWek(std::string oscAddr=WEKINATOR_INPUTS, int bufsize=1 ) : UGEN()
        {
            bufferSize = bufsize;
            osc_addr = oscAddr;
            
            //scale and add weights
            maxes = {2*G_FORCE,     2*G_FORCE,     2*G_FORCE,     500,     500,     500,  1,     1,     1, 1,    G_FORCE/2.0,    G_FORCE/2.0, G_FORCE/2.0,500,     500,     500, 1,    1, 1, 1};
            mins = {-2*G_FORCE,    -2*G_FORCE,    -2*G_FORCE,    -500,    -500,    -500, -1,    -1,   -1, -1,  -G_FORCE/2.0,   -G_FORCE/2.0,   -G_FORCE/2.0,500,     500,     500, -1,   -1,   -1, -1};
            
            recordOSC = false;
            
            mTimer.start();
        };
        
        
        void setRecord(bool rec, std::string whatrec="")
        {
            recordOSC = rec;
            if(rec) whatRecording = rec;
            
            std::ofstream outfile;
            outfile.open("wekinatorOSCRecording.txt", std::ios::app);
            
            if(recordOSC)
            {
                outfile << "GESTURE, "<< whatRecording <<"\n";
            }
            else
            {
                outfile << "END\n";
            }
            outfile.close();
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
            
            for(int i=0; i<signals.size() && enoughData; i++)
            {
                std::vector<ShimmerData *> curBuffer = signals[i]->getBuffer();
                enoughData = curBuffer.size() > 0 && signals[i]->getNewSampleCount() > 0;
                signals[i]->processQuart(true); //also get values for quarternions
                
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
        
        int getSignalCount(){ return signals.size();};
        
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
                
                for (int sigNum=0; sigNum<signals.size(); sigNum++)
                {
                    std::vector<ShimmerData * > shimmers = data[sigNum];
                    ShimmerData *shimmerdata = shimmers[shimmers.size()-i];
                    
                    if(shimmerdata != NULL)
                    {
                        for( int k=2; k<5; k++ ) // add accels
                            toSend.push_back(shimmerdata->getData(k) );
                        
                        for( int k=11; k<14; k++ ) // add gyros
                            toSend.push_back(shimmerdata->getData(k) );
                        
                        //                    std::cout << "quat: ";
                        for( int k=0; k<4; k++ )
                        {
                            float q = shimmerdata->getQuarternion(k);
                            toSend.push_back(q);
                            //                        std::cout << q << ",";
                        }
                        //                    std::cout << std::endl;
                    }
                }
                
                for(int sIndex=0; sIndex<maxes.size(); sIndex++)
                {
                    sends.push_back( scale(toSend[sIndex], sIndex) );
                    //sends.push_back( limitPrecisionToThree(toSend[sIndex]) );
                }
                floatsToSend.push_back(sends);
            }
        };
        
        float limitPrecisionToThree(float num)
        {
            return float(std::round( num * 1000 )) / 1000.0;
        }
        
        virtual std::vector<ci::osc::Message> getOSC()
        {
            std::vector<ci::osc::Message> msgs;
            
            if(signals.size() < 1)
            {
                return msgs;
            }
            
            //record to find scaling for DTW
            std::ofstream outfile;
            if(recordOSC)
            {
                outfile.open("wekinatorOSCRecording.txt", std::ios::app);
            }
            
            //iterate backwards since data was entered in backwards
            for(int i=floatsToSend.size()-1; i>=0; i--)
            {
                
                osc::Message message;
                message.setAddress( osc_addr );
                //            std::cout << osc_addr << ",";
                if(recordOSC) outfile << mTimer.getSeconds() << ",";
                
                //            std::cout << mTimer.getSeconds() << ",";
                
                for( int j=0; j<floatsToSend[i].size(); j++ )
                {
                    message.addFloatArg(limitPrecisionToThree(floatsToSend[i].at(j)));
                    if(recordOSC) outfile << limitPrecisionToThree(floatsToSend[i].at(j)) << ",";
                    //                std::cout << limitPrecisionToThree(floatsToSend[i].at(j)) << ",";
                    
                }
                msgs.push_back( message );
                
                if(recordOSC)
                    outfile << std::endl;
                //            std::cout << std::endl;
                
            }
            if( recordOSC ) outfile.close();
            return msgs;
        };
    };
    
};

