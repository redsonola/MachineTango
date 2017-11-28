//
//  InteractiveTangoSignalAnalysis.h
//  InteractiveTangoReadFromAndroid
//
//  Created by courtney on 10/12/14.
//
//

#ifndef InteractiveTangoReadFromAndroid_InteractiveTangoSignalAnalysis_h
#define InteractiveTangoReadFromAndroid_InteractiveTangoSignalAnalysis_h

//#include "ext.h"
#include <complex>
#include <math.h>
#include <algorithm>
#include "fftw3.h"

//#include "itbase.h"
//#include "itsignal.h"

#include "itpp/itbase.h"
#include "itpp/itsignal.h"
#include "itpp/itstat.h"


# define BUFFER_SIZE 4096
# define MAX_OUT 2049 //must be BUFFER SIZE/2+1

#define USE_ACCEL 0
#define USE_GYRO 1
#define USE_MAG 2

const int SAMPLE_SZ = 4096;

namespace InteractiveTango {
    
    static const double SR = 51.2; // Sample rate

class SignalAnalysis
{
protected:
    std::vector<ShimmerData *> data1;
    std::vector<ShimmerData *> data2;
    
    SignalAnalysis *ugen, *ugen2  ;
    int buffersize;
    
    double findAvg(std::vector<double> input, int start, int end)
    {
        double N = end - start;
        double sum = 0;
        for(int i=start; i<end; i++)
        {
            sum += input[i];
        }
        return (sum/N);
    };
    
public:
	SignalAnalysis(SignalAnalysis *s1 = NULL, int bufsize=1024, SignalAnalysis *s2 = NULL)
	{
        ugen = s1;
        ugen2 = s2;
        setBufferSize(bufsize);
        
//OMFG CHANGE THIS
        
//        if(s1!=NULL) s1->setBufferSize(buffersize);
//        if(s2!=NULL) s2->setBufferSize(buffersize);
	};
    
//    enum SignalAnalysisType
//    {
//        Generic,
//        InputSignal,
//        Filter,
//        StepDetection,
//        SpatialSimilarity,
//        WindowedVariance,
//        WindowedVarianceDerivative
//    };
    
    inline virtual int getNewSampleCount()
    {
        if(ugen != NULL && ugen2==NULL)
            return ugen->getNewSampleCount();
        else if(ugen != NULL && ugen2!=NULL)
            return std::max(ugen->getNewSampleCount(), ugen2->getNewSampleCount());
        else return 0;
    }
    inline virtual void setBufferSize(int sz)
    {
        buffersize = sz;
    };
    
    virtual inline std::vector<ShimmerData *> getBuffer(){
        return data1;
    };
    
    virtual inline std::vector<ShimmerData *> getBuffer2(){
        return data2;
    };
    
    int getBufferSize(){return buffersize;};
    
	virtual void update(float seconds=0)
    {
        if( ugen != NULL ) data1 = ugen->getBuffer();
        if( ugen2 != NULL ) data2 = ugen2->getBuffer();
    };
    
	virtual std::vector<ci::osc::Message> getOSC() = 0;
    std::vector<itpp::vec> toITPPVector(std::vector<ShimmerData *> sdata)
    {
        std::vector<itpp::vec> newVec;
        for(int i=0; i<20; i++)
        {
            itpp::vec v(sdata.size());
            for(int j=0; j<sdata.size(); j++)
            {
                v.set( j, sdata[j]->getData(i));
            }
            newVec.push_back(v);
        }
        return newVec;
    };
    std::vector<float> toFloatVector(itpp::vec input)
    {
        std::vector<float> output;
        for(int i=0; i<input.length(); i++)
        {
            output.push_back(input[i]);
        }
        return output;
    };
};

    
//has output signals that it modifies  & forwards on to others
//only handles one stream of data... don't have any for two at the moment...
class OutputSignalAnalysis : public SignalAnalysis
{
protected:
    std::vector<ShimmerData *> outdata1;
    std::vector<ShimmerData *> outdata2;
    
    void eraseData()
    {
        for( int i=0; i<outdata1.size(); i++ )
            delete outdata1[i];
        
        for( int i=0; i<outdata2.size(); i++ )
            delete outdata2[i];
        
        outdata1.clear();
        outdata2.clear();
    };
public:
    
    OutputSignalAnalysis(SignalAnalysis *s1, int bufsize, SignalAnalysis *s2 = NULL) : SignalAnalysis(s1, bufsize, s2)
    {
    
    }

    //puts in accel data slots -- all other data left alone -- ALSO only
    void toShimmerOutputVector( std::vector<float> inputX, std::vector<float> inputY, std::vector<float> inputZ )
    {
        for(int i=0; i<inputX.size(); i++){
            ShimmerData *data = new ShimmerData();
            data->setData(0, data1[i]->getData(0));
            data->setData(1, data1[i]->getData(1));
            data->setData(2, inputX[i]);
            data->setData(3, inputY[i]);
            data->setData(4, inputZ[i]);
            outdata1.push_back(data);
        }
    };
    
    virtual void update(float seconds = 0){
        eraseData();
        SignalAnalysis::update(seconds);
    };
    
    virtual std::vector<ShimmerData *> getBuffer(){
        return outdata1;
    };
    
    virtual std::vector<ShimmerData *> getBuffer2(){
        return outdata2;
    };
};
    
    
//only gets inputs so sends nothing -- not an output bc it doesn't modify its inputs
    //TODO: this filters for acc data ONLY, so turn on options, etc.
class InputSignal : public SignalAnalysis
{
protected:
//    ShimmerData * lastValue;  //ok, TODO: kill when done
    int ID1;
    bool isAndroid;
public:
    InputSignal(int idz, bool android, SignalAnalysis *s1 = NULL, int bufsize=1024, SignalAnalysis *s2 = NULL) : SignalAnalysis(s1, bufsize, s2)
    {
//        lastValue = NULL;
        ID1= idz;
        isAndroid = android;
    };

    
    virtual std::vector<ci::osc::Message> getOSC()
    {
        std::vector<ci::osc::Message> msgs;
        
        if(getNewSampleCount() > 0)
        {
            int numNew = std::min(int(getNewSampleCount()), int(data1.size()));
            int sample = data1.size() - numNew;
            for(int i=sample; i<data1.size(); i++)
            {
                ci::osc::Message msg;
                msg.setAddress(OSC_SHIMMERDATA_TO_SC);
                msg.addIntArg(ID1);
                
                double getRidOfG = (isAndroid) ? 1 : (G_FORCE * 2.5);
                for( int j=2; j<11; j++ )
                {
                    
                    msg.addFloatArg( data1[i]->getData(j) / getRidOfG ); //get ride of G_FORCE in the acceleration
                }
                for( int j=11; j<20; j++ )
                {
                    
                    msg.addFloatArg( data1[i]->getData(j) );
                }
                msgs.push_back( msg );
            }
        }
//        std::cout << msgs.size() << std::endl; 
        return msgs;
    };
    SensorData *sensor  ;

    void setInput( SensorData *s1 )
    {
        sensor = s1;
    };
    
    virtual int getNewSampleCount()
    {
        return sensor->getNewSampleCount();
    };
    
	virtual void update(float seconds=0)  //TODO... maybe just change bufsize & check when filtering.
    {
        data1.clear(); 
        if( sensor != NULL )
        {
            std::vector<ShimmerData *> data = sensor->getBuffer(buffersize);
            for(int i =0; i<data.size(); i++) //FILTERING OUT UNWANTED DATA>>> NOTE!!!
            {
                if( data[i]->getData(2) != NO_DATA )
                    data1.push_back(data[i]);
            }
        }
    };

};
    
class Filter : public OutputSignalAnalysis
{
protected:
    itpp::vec b;
    itpp::vec a;
    
public:
    Filter(SignalAnalysis *s1, std::string  _b, std::string  _a, int sz=3060 ) : OutputSignalAnalysis(s1, sz)
    {
        b = _b.c_str();
        a = _a.c_str();
    };
    
    void CoreDoubleN(double *X, int MX, int NX, double *a, double *b,
                     int order, double *Z, double *Y)
    {
        // Direct form II transposed method for general filter length.
        // Implemented as time domain difference equations.
        // INPUT:
        //   X:  Double array. Operation happens of 1st dimension.
        //   MX: Number of elements in the 1st dimension
        //   NX: Number of columns, considers mutliple dimensions.
        //   a, b: Double vector of filter parameters. Both have nParam elements.
        //       The first element of a is 1.
        //   Z:  DOUBLE array, initial conditions.
        //   order: Number of filter parameters, order of filter + 1.
        // OUTPUT:
        //   Z:  DOUBLE array, final conditions.
        //   Y:  Double array, allocated by the caller.
        
        double Xi, Yi;
        int i, j, R;
        
        i = 0;
        while (NX--) {                         // Next slice
            R = i + MX;                         // End of the column
            while (i < R) {
                Xi = X[i];                       // Get signal
                Yi = b[0] * Xi + Z[0];           // Filtered value
                for (j = 1; j < order; j++) {    // Update conditions
                    Z[j - 1] = b[j] * Xi + Z[j] - a[j] * Yi;
                }
                Z[order - 1] = b[order] * Xi - a[order] * Yi;
                
                Y[i++] = Yi;                      // Write to output 
            } 
            Z += order;                          // Next condition vector 
        }
    }
    
    std::vector<float> filterFunc(std::vector<float> input)
    {
        
        double Y[ input.size() ], X[ input.size() ];
        int asize = a.size();
        int bsize = b.size();

        double a1[asize], b1[bsize];
        double order = b.size()-1;
        int MX = input.size();
        
        for(int i=0; i<input.size(); i++)
        {
            X[i] = input[i];
        }
        
        for(int i=0; i<b.size(); i++){
            b1[i] = b[i];
        }
        
        for(int i=0; i<a.size(); i++)
            a1[i] = a[i];

//        
//        CoreDoubleN( X,  MX,  NX,  a1,  b1, order,  Z,  &Y[0]);
        filter(order, a1, b1, input.size(), X, &Y[0]);
        
        std::vector<float> output;
        for(int i=0; i<MX; i++)
        {
            output.push_back(float(Y[i]));
        }
        
        return output;

    };
    
    std::vector<double> filterFunc(std::vector<double> input)
    {
        int MX = input.size();
        double Y[ MX ], X[ MX ];
        double a1[a.size()], b1[b.size()];
        double order = b.size()-1;
        
//        std::cout << MX << "\n";
        
        for(int i=0; i<input.size(); i++)
        {
            X[i] = input[i];
        }
        
        for(int i=0; i<b.size(); i++){
            b1[i] = b[i];
        }
        
        for(int i=0; i<a.size(); i++)
            a1[i] = a[i];
        
//        for(int i=0; i<MX; i++)
//        {
//            std::cout << i << "," << X[i] << "," << input[i] << "\n";
//            
//        }
        
        //
        //        CoreDoubleN( X,  MX,  NX,  a1,  b1, order,  Z,  &Y[0]);
        filter(order, a1, b1, input.size(), X, Y);
        
        std::vector<double> output;
        for(int i=0; i<MX; i++)
        {
            output.push_back((Y[i]));
//            std::cout << i << "," << Y[i] << "\n";

        }
        
//        std::cout << "Y: " << sizeof(Y)/sizeof(Y[0]) << std::endl; 
        assert(MX == sizeof(Y)/sizeof(Y[0]));
        
        return output;
        
    };

    std::vector<float> shimmerToFloat(int index)
    {
        std::vector<float> output;
        for(int i=0; i < data1.size() ; i++)
        {
            output.push_back( data1[i]->getData(index) );
        }
        return output;
    };
    
    void filter(int ord, double *a, double *b, int np, double *x, double *y)
    {
        int i,j;
        y[0]=b[0]*x[0];
        for (i=1;i<ord+1;i++)
        {
            y[i]=0.0;
            for (j=0;j<i+1;j++)
                y[i]=y[i]+b[j]*x[i-j];
            for (j=0;j<i;j++)
                y[i]=y[i]-a[j+1]*y[i-j-1];
        }
        /* end of initial part */
        for (i=ord+1;i<np+1;i++)
        {
            y[i]=0.0;
            for (j=0;j<ord+1;j++)
                y[i]=y[i]+b[j]*x[i-j];
            for (j=0;j<ord;j++)
                y[i]=y[i]-a[j+1]*y[i-j-1];
        }
        return;
    } /* end of filter */
    
    
    //TODO: oh god will have to clean this up
    std::vector<double> update(std::vector<double> inSig)
    {
        return filterFunc( inSig );
    }

    //only so far on accel values
    virtual void update(float seconds=0)
    {
        OutputSignalAnalysis::update(seconds);
        if( data1.size() < b.size() + 5 ) return ;
        
        
        std::vector<float> resX = filterFunc( shimmerToFloat(2) );
        std::vector<float> resY = filterFunc( shimmerToFloat(3) );

//        int sz=data1.size();
//        int newS = std::min(getNewSampleCount(), sz);
//        for(int i= sz - newS; i<sz; i++)
//        {
//            std::cout << data1[i]->getData(0) << "," << data1[i]->getData(3) << "," << resY[i] << "\n";
//        }
        
        std::vector<float> resZ = filterFunc( shimmerToFloat(4) );
        
        toShimmerOutputVector( resX, resY, resZ ); //always puts results in accel place...
        
        
//        std::vector<itpp::vec> sdata1 = toITPPVector( data1 );
//
//            itpp::vec resX = itpp::filter( b, a, sdata1[2] );
//            itpp::vec resY = itpp::filter( b, a, sdata1[3] );
//            itpp::vec resZ = itpp::filter( b, a, sdata1[4] );
//
//            toShimmerOutputVector( toFloatVector(resX), toFloatVector(resY), toFloatVector(resZ) ); //always puts results in accel place...
    };
    
    //dummy for OSC for now.
    virtual std::vector<ci::osc::Message> getOSC()
    {
        std::vector<ci::osc::Message> msgs;
        return msgs;
    };
    
};
    
class LPFilter5Hz : public Filter
{
public:
    // low-pass filter is 6 degrees, 5Hz - cut-off
    //co-efficients from matlab, from butterworth
    LPFilter5Hz(SignalAnalysis *s1, int sz=3060) : Filter(s1, "0.2413 0.2413", "1.0000 -0.5173", sz)
    {
    };
    
};
    
    class HPFilter1xHz : public Filter
    {
    public:
        // low-pass filter is 6 degrees, 5Hz - cut-off
        //co-efficients from matlab, from butterworth
        HPFilter1xHz(SignalAnalysis *s1, int sz=3060) : Filter(s1, "0.9999 -0.9999", "1.0000 -0.9999", sz)
        {
        };
        
    };
    
    class LPFilter1Hz2d : public Filter
    {
    public:
        // low-pass filter is 6 degrees, 5Hz - cut-off
        //co-efficients from matlab, from butterworth
        LPFilter1Hz2d(SignalAnalysis *s1, int sz=3060) : Filter(s1, "0.00001213 0.00004854 0.00007281 0.00004854 0.00001213", "1.0000 -3.6794 5.0886 -3.1345 0.7255", sz)
        {
        };
        
    };
    
    class LPFilter8Hz : public Filter
    {
    public:
        // low-pass filter is 3 degrees, 8Hz - cut-off
        //co-efficients from matlab, from butterworth
        LPFilter8Hz(SignalAnalysis *s1, int sz=3060) : Filter(s1,"0.0552 0.1656 0.1656 0.0552", "1.0000 -1.0810 0.6465 -0.1240", sz)
        {
        };
        
    };
    

    class LPFilter5Hz5d : public Filter
    {
    public:
        //co-efficients from matlab, from butterworth
        LPFilter5Hz5d(SignalAnalysis *s1, int sz=3060) : Filter(s1,"0.0012 0.0058 0.0116 0.0116 0.0058 0.0012", "1.0000 -3.0225 3.9098 -2.6389 0.9207 -0.1319", sz)
        {
//            b =
//            
//            0.0012    0.0058    0.0116    0.0116    0.0058    0.0012
//            
//            a =
//            
//            1.0000   -3.0225    3.9098   -2.6389    0.9207   -0.1319
            
            
            
//            b =
//            
//            0.0021    0.0082    0.0123    0.0082    0.0021
//            
//            
//            a =
//            
//            1.0000   -2.7230    2.9236   -1.4408    0.2730
            
//            b = 0.00001213 0.00004854 0.00007281 0.00004854 0.00001213
//            
//            
//            a = 1.0000 -3.6794 5.0886 -3.1345 0.7255
            
            
//            b =
//            
//            0.0007 0.0030 0.0044 0.0030 0.0007
//            
//            
//            a =
//            
//            1.0000 -3.0405 3.5556 -1.8829 0.3797
            
            
//            b =  0.0044    0.0178    0.0267    0.0178    0.0044
//            
//            
//            a =  1.0000   -2.4073    2.3745   -1.0913    0.1952
            
//            b = 0.0002 0.0007 0.0010 0.0007 0.0002
//            a = 1.0000 -3.3594 4.2755 -2.4390 0.5256
            
            
//            b = 0.0004 0.0022 0.0044 0.0044 0.0022 0.0004
//            a = 1.0000 -3.4158 4.8475 -3.5353 1.3179 -0.2002
            
//            b =
//            
//            1.0e-03 *
//            
//            0.0000192 0.0000958 0.0001916 0.0001916 0.0000958 0.0000192
//            
//            a =
//            
//            1.0000 -4.2063 7.1312 -6.0853 2.6118 -0.4508
            
//            b =
//            
//            0.0021 0.0082 0.0123 0.0082 0.0021
//            
//            
//            a =
//            
//            1.0000 -2.7230 2.9236 -1.4408 0.2730
            
        };
        
    };
    
    class LPFilter10Hz : public Filter
    {
    public:
        // low-pass filter is 3 degrees, 10Hz - cut-off
        //co-efficients from matlab, from butterworth
        LPFilter10Hz(SignalAnalysis *s1, int sz=3060) : Filter(s1, "0.0940 0.2821 0.2821 0.0940", "1.0000 -0.6227 0.4363 -0.0614", sz)
        {
        };
        
    };
    
    class LPFilter20Hz : public Filter
    {
    public:
        // low-pass filter is 3 degrees, 10Hz - cut-off
        //co-efficients from matlab, from butterworth
        LPFilter20Hz(SignalAnalysis *s1, int sz=3060) : Filter(s1,
        "0.1640 1.3120 4.5920 9.1841 11.4801 9.1841 4.5920 1.3120 0.1640",
        "1.0000 4.4841 9.2881 11.4399 9.0940 4.7529 1.5889 0.3097 0.0269", sz)
        {
            
            //  b= 0.1640 1.3120 4.5920 9.1841 11.4801 9.1841 4.5920 1.3120 0.1640
            
            //  a= 1.0000    4.4841    9.2881   11.4399    9.0940    4.7529 1.5889    0.3097    0.0269
        
        };
        
    };
    
    

    

    
    
    class LPFilter15Hz : public Filter
    {
    public:
        // low-pass filter is 1 degrees, 15Hz - cut-off
        //co-efficients from matlab, from butterworth
        LPFilter15Hz(SignalAnalysis *s1, int sz=3060) : Filter(s1,"0.5697 0.5697","1.0000 0.1395", sz)
        {
        };
        
    };
    
    class LPFilter1Hz : public Filter
    {
    public:
        // low-pass filter is 3 degrees, 1Hz - cut-off
        //co-efficients from matlab, from butterworth
        LPFilter1Hz(SignalAnalysis *s1, int sz=3060) : Filter(s1,
        "0.0002051 0.0006153 0.0006153 0.0002051",
        "1.0000 -2.7547 2.5386 -0.7822", sz)
        {
//            b = 0.0009 0.0018 0.0009
//            a = 1.0000 -1.9133 0.9169
            
//     for 1 hz
//            0.0002051 0.0006153 0.0006153 0.0002051
//            1.0000 -2.7547 2.5386 -0.7822
            
//   for 2 hz
//            b =
//            
//            0.0015    0.0044    0.0044    0.0015
//            
//            
//            a =
//            
//            1.0000   -2.5103    2.1334   -0.6113
       
//            1 hz, 1st degree
//            b = 0.0579    0.0579
//            a = 1.0000   -0.8842
        };
        
    };


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

//Signal Analysis objects that have outcomes which interface with mapping schemeas, etc. etc. -- need to srsly refactor this, as similar to other things, sharing data, but do NOT have time NOW
class SignalAnalysisEventOutput : public SignalAnalysis
{
protected:
    std::vector<MotionAnalysisData *> motionData;
public:
    SignalAnalysisEventOutput(SignalAnalysis *s1 = NULL, int bufsize=1024, SignalAnalysis *s2 = NULL)  : SignalAnalysis(s1, bufsize, s2)
    {
    };
    
    virtual ~SignalAnalysisEventOutput()
    {
        for(int i=0; i<motionData.size(); i++)
        {
            if(motionData[i] != NULL)
            {
                delete motionData[i];
            }
        }
    };
    
    std::vector<MotionAnalysisData * > getMotionData()
    {
        return motionData;
    };
    virtual void updateMotionData(){}; //overloaded with indices other places, so not abstract
    virtual void setMotionAnalysisMinMax(int i, double minv,  double maxv)
    {
        assert( i<motionData.size() && i>=0);
        ((MotionAnalysisEvent * ) motionData[i])->setMinMax(minv, maxv);
    };
};

    
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

    
class WindowedVariance : public SignalAnalysisEventOutput
{
protected:
    double curVariance;
    int ID1;
    std::string port;
public:
    WindowedVariance(SignalAnalysis *s1, int sz=SR/3, int idz=0, std::string p="" ) : SignalAnalysisEventOutput(s1, sz)
    {
        curVariance = 0;
        ID1 = idz;
        port = p;
        motionData.push_back(new MotionAnalysisEvent(MotionAnalysisDataType::DoubleEvent, 0));
        setMotionAnalysisMinMax(0, 0, 11);
    };
    
    double getVariance()
    {
        return curVariance;
    };
    
    virtual void updateMotionData()
    {
       ( (MotionAnalysisEvent *)motionData[0])->setValue(curVariance);
    };
    
    virtual void update(float seconds=0)
    {
        SignalAnalysis::update(seconds);
        if( data1.size() < buffersize) return ;
        
        std::vector<ShimmerData *>::iterator startIter = data1.end() - buffersize;
        std::vector<ShimmerData *>::iterator endIter = data1.end();
        std::vector<ShimmerData *> buf( startIter, endIter );
        
        std::vector<double>  mag;
        for(int i=0; i<buffersize; i++)
        {
            mag.push_back( std::sqrt(buf[i]->getData(2)*buf[i]->getData(2) + buf[i]->getData(3)*buf[i]->getData(3) + buf[i]->getData(4)*buf[i]->getData(4)) );
        }
        
        double mean = findAvg(mag, 0, buffersize);
        double sum = 0;
        for(int i=0; i<buffersize; i++)
        {
            double diff = (mag[i] - mean);
            sum = sum + (diff*diff);
        }
        
        curVariance = sum / buffersize;
        curVariance = curVariance / (G_FORCE * 2.5);
        updateMotionData();
        
        
        
//        int snapSample = data1.size() - std::min(getNewSampleCount(), buffersize);
//        for(int i=snapSample; i<data1.size(); i++)
//        {
//            std::cout << data1[i]->getData(0) << " curVar , " << curVariance << "\n";
//        }

    };
    
    virtual std::vector<ci::osc::Message> getOSC()
    {
        std::vector<ci::osc::Message> msgs;
        ci::osc::Message msg;
        
        msg.addIntArg(ID1);
        msg.addIntArg(atoi(port.c_str()));
        msg.setAddress(SHIMMER_WINDOWED_VARIATION);
        msg.addFloatArg(curVariance);
        
        msgs.push_back(msg);
        
        return msgs;
    };

};
//------------------------------------------
//------------------------------------------
     

    //-------------------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------------------
    //the CHANGE in variance
    
    class WindowedVarianceDifference : public SignalAnalysisEventOutput
    {
    protected:
        double curVarianceDiff, curVarianceDiffAvg, lastVar;
        WindowedVariance *variance;
        int ID1;
        std::string port;
        std::vector<double> pastVar;
        std::vector<double> pastVarDiff;
        int window;

    public:
        WindowedVarianceDifference(WindowedVariance *v, int idz, std::string p ) : SignalAnalysisEventOutput(NULL, 0)
        {
            lastVar = 0;
            ID1 = idz;
            port = p;
            variance = v;
            motionData.push_back(new MotionAnalysisEvent(MotionAnalysisDataType::DoubleEvent, 0));
            setMotionAnalysisMinMax(0, 0, 2);
            curVarianceDiffAvg = 0;
            window = variance->getBufferSize() * 2 ; //roughly a second buffer
        };
        
        virtual void setWindowSize(int sz)
        {
            window = sz;
        };

        virtual void updateMotionData()
        {
            ((MotionAnalysisEvent *)motionData[0])->setValue(curVarianceDiffAvg);
        };
        
        double getVarianceDiff()
        {
            return curVarianceDiff;
        };
        
        virtual void update(float seconds=0)
        {
//          curVarianceDiff = variance->getVariance() - lastVar;
//          lastVar = variance->getVariance() ;
            
            pastVar.push_back( variance->getVariance() );
            if ( pastVar.size() < variance->getBufferSize()  ) return;
            
            if( pastVar.size() >  variance->getBufferSize() )
            {
                pastVar.erase(pastVar.begin());
            }
            
            if( pastVarDiff.size() >  ( window ) )
            {
                pastVarDiff.erase(pastVarDiff.begin());
            }
            
            LPFilter10Hz lpfilter(NULL);
            std::vector<double> lpVar = lpfilter.update(pastVar);
            curVarianceDiff = std::abs( pastVar[ pastVar.size()-1 ] - pastVar[ pastVar.size()-2 ] );
            pastVarDiff.push_back(curVarianceDiff);
            
            curVarianceDiffAvg = 0;
            if( pastVarDiff.size()>0 )
            {
                for(int i=0; i<pastVarDiff.size(); i++)
                {
                    curVarianceDiffAvg += pastVarDiff[i];
                }
                curVarianceDiffAvg /= pastVarDiff.size();
            }
            updateMotionData();
//            std::cout << curVarianceDiffAvg << std::endl;
        };
        
        virtual std::vector<ci::osc::Message> getOSC()
        {
            std::vector<ci::osc::Message> msgs;
            ci::osc::Message msg;
            
            msg.addIntArg(ID1);
            msg.addIntArg(atoi(port.c_str()));
            msg.setAddress(SHIMMER_WINDOWED_VARIATION_DIFF);
            msg.addFloatArg(curVarianceDiff);
            
            msgs.push_back(msg);
            
            return msgs;
        };
        
    };
    
    
//------------------------------------------
//------------------------------------------
    
class CrossCoVar : public SignalAnalysisEventOutput
{
protected:
        
    double curPeak, curPeakModVariance;
    int ID1, ID2;
    std::string port1, port2;
    WindowedVariance *variance1, *variance2;
    
        
public:
    CrossCoVar(SignalAnalysis *s1, SignalAnalysis *s2, WindowedVariance *v1, WindowedVariance *v2, int id1, int id2, std::string p1, std::string p2 ) : SignalAnalysisEventOutput(s1, 20, s2)
    {
            
        ID1 = id1;
        ID2 = id2;
        port1 = p1;
        port2 = p2;
        variance1 = v1;
        variance2 = v2;
        motionData.push_back(new MotionAnalysisEvent(MotionAnalysisDataType::DoubleEvent, 0));
        setMotionAnalysisMinMax(0, 0, 11);
    };
    
    inline int getID1(){ return ID1; };
    inline int getID2(){ return ID2; };
    
    virtual void updateMotionData()
    {
       ( (MotionAnalysisEvent *)motionData[0])->setValue(curPeakModVariance);
    };
    
    virtual void update(float seconds = 0)
    {
        SignalAnalysis::update(seconds);
        if( data1.size()<buffersize || data2.size()<buffersize ) return ;
        
        std::vector<ShimmerData *>::iterator startIter = data1.end() - buffersize;
        std::vector<ShimmerData *>::iterator endIter = data1.end();
        std::vector<ShimmerData *> buf1( startIter, endIter );
        
        std::vector<ShimmerData *>::iterator startIter2 = data2.end() - buffersize;
        std::vector<ShimmerData *>::iterator endIter2 = data2.end();
        std::vector<ShimmerData *> buf2( startIter2, endIter2 );
            
        std::vector<itpp::vec> sdata1 = toITPPVector( buf1 );
        std::vector<itpp::vec> sdata2 = toITPPVector( buf2 );
            
        std::vector<itpp::vec> lpdata1;
        std::vector<itpp::vec> lpdata2;
        
        //mult. by a ones, to make it exactly!!! how matlab does it.
        itpp::vec onesArray(sdata1[1].length()); onesArray.ones();
            
        sdata1[2] = sdata1[2] / (G_FORCE * 2.5);
        sdata1[3] = sdata1[3] / (G_FORCE * 2.5) ;
        sdata1[4] = sdata1[4] / (G_FORCE * 2.5) ;
            
        sdata2[2] = sdata2[2] / (G_FORCE * 2.5);
        sdata2[3] = sdata2[3] / (G_FORCE * 2.5) ;
        sdata2[4] = sdata2[4] / (G_FORCE * 2.5) ;
            
        //to make equivalent to xcov, must subtract the mean
        itpp::vec x1 = sdata1[2] - (itpp::mean(sdata1[2])*onesArray);
        itpp::vec y1 = sdata1[3] - (itpp::mean(sdata1[3])*onesArray);
        itpp::vec z1 = sdata1[4] - (itpp::mean(sdata1[4])*onesArray);
        
        itpp::vec x2 = sdata2[2] - (itpp::mean(sdata2[2])*onesArray);
        itpp::vec y2 = sdata2[3] - (itpp::mean(sdata2[3])*onesArray);
        itpp::vec z2 = sdata2[4] - (itpp::mean(sdata2[4])*onesArray);
        
        //low-pass filter
        //        lpdata1.push_back( itpp::filter( b, a, x1 ) );
        //        lpdata1.push_back( itpp::filter( b, a, y1 ) );
        //        lpdata1.push_back( itpp::filter( b, a, z1 ) );
        //
        //        lpdata2.push_back( itpp::filter( b, a, x2 ) );
        //        lpdata2.push_back( itpp::filter( b, a, y2 ) );
        //        lpdata2.push_back( itpp::filter( b, a, z2 ) );
        //
        
        itpp::vec xCov, yCov, zCov;
        xCov = itpp::xcorr( x1, x2, -1, "unbiased" );
        yCov = itpp::xcorr( y1, y2, -1, "unbiased" );
        zCov = itpp::xcorr( z1, z2, -1, "unbiased" );
        //        xCov = itpp::xcorr( lpdata1[0], lpdata2[0], -1, "coeff" );
        //        yCov = itpp::xcorr( lpdata1[1], lpdata2[1], -1, "coeff" );
        //        zCov = itpp::xcorr( lpdata1[2], lpdata2[2], -1, "coeff" );
        
        // check with means, too
        std::vector<double> magXcov;
        for( int i=0; i< xCov.length(); i++ )
        {
            magXcov.push_back( std::sqrt( xCov.get(i)*xCov.get(i) + yCov.get(i)*yCov.get(i) + zCov.get(i)*zCov.get(i)) );
        }
            
        curPeak = -10000;
        for(int i=0; i<magXcov.size(); i++)
        {
            if( magXcov[i] > curPeak  ) curPeak = magXcov[i];
        }
        
        double meanVar = (variance1->getVariance() + variance2->getVariance())/3.0;
        curPeakModVariance = curPeak / meanVar ;
        updateMotionData();
        
//        int snapSample = buf1.size() - std::min(getNewSampleCount(), buffersize);
//        for(int i=snapSample; i<buf1.size(); i++)
//        {
//                
//            std::cout << sdata1[0].get(i) << " , " << sdata2[0].get(i) <<  " , " << curPeak << " , " << meanVar << " , " << curPeak / meanVar  <<"\n";
////
////                //            std::cout << sdat1v2va1[0].get(i) << " , " << sdata2[0].get(i) << " , "<< x1[0].get(i) << " , "<< y1[1].get(i) << " , "<< z1[2].get(i) << " , "<< x2[0].get(i) << " , "<< y2[1].get(i) << " , "<< z2[2].get(i) <<"\n";
////                
////                //           std::cout << sdata1[0].get(i) << " , " << sdata2[0].get(i) << " , "<< x1.get(i) << " , "<< y1.get(i) << " , "<< z1.get(i) << " , "<< x2.get(i) << " , "<< y2.get(i) << " , "<< z2.get(i) <<"\n";
////                
////                //            std::cout << sdata1[0].get(i) << " , " << sdata2[0].get(i) << " , "<< sdata1[2].get(i) << " , "<< sdata1[3].get(i) << " , "<< sdata1[4].get(i) << " , "<< sdata1[2].get(i) << " , "<< sdata1[3].get(i) << " , "<< sdata1[4].get(i) <<"\n";
////                
//        }
    };
        
    virtual std::vector<ci::osc::Message> getOSC()
    {
        std::vector<ci::osc::Message> msgs;
        
        ci::osc::Message msg;
        msg.addIntArg(ID1);
        msg.addIntArg(ID2);
        msg.addIntArg(atoi(port1.c_str()));
        msg.addIntArg(atoi(port2.c_str()));
        msg.setAddress(SHIMMER_CROSS_COVARIATION);
        msg.addFloatArg(curPeak);
        msg.addFloatArg(curPeakModVariance);

        
        msgs.push_back(msg);
        
        return msgs;
    };
        
    inline float getPeak()
    {
        return curPeak;
    };
    
};
    
    
    //------------------------------------------
    //------------------------------------------
    
    
    

    
    //average event values across signal analyses,only use for one var things (may modify later)
    //3-1-2015 -- CDB --- Modified so that it averages across multiple motionData, so can use with footsteps, etc.
    class MotionDataAverage : public SignalAnalysisEventOutput
    {
    protected:
        std::vector<SignalAnalysisEventOutput *> signals;
    public:
        MotionDataAverage() : SignalAnalysisEventOutput(NULL, 0, NULL)
        {
            //do nothin -- for the integer version, later
        }
        MotionDataAverage( std::vector<SignalAnalysisEventOutput *> sigz ) : SignalAnalysisEventOutput(NULL, 0, NULL)
        {
            assert(sigz.size() >= 2);
            //also signals should have the same amt of motiondata
            
            signals = sigz;
            
            int N = sigz[0]->getMotionData().size();
            
            for(int i=0; i<N; i++)
            {
                motionData.push_back(new MotionAnalysisEvent(MotionAnalysisDataType::DoubleEvent, 0));
            
                MotionAnalysisEvent *ev = (MotionAnalysisEvent * ) sigz[0]->getMotionData().at(i);
                setMotionAnalysisMinMax(i, ev->getMin(), ev->getMax());
            }
        };
        
        virtual void updateMotionData(int k, double avgMotionData)
        {
            ( (MotionAnalysisEvent *)motionData[k])->setValue(avgMotionData);
        };
        
        virtual void update(float seconds = 0)
        {
            assert(signals.size() > 0 );
            
            int N = signals[0]->getMotionData().size();
            for(int k=0; k<N; k++)
            {
                double avgMotionData = 0;
                for( int i=0; i<signals.size(); i++ )
                {
                    avgMotionData += ((MotionAnalysisEvent *) signals[i]->getMotionData().at(k))->asDouble();
                }
                avgMotionData /= signals.size();
            
                updateMotionData(k, avgMotionData);
            }
        };
        
        //no OSC output
        std::vector< ci::osc::Message > getOSC(){  std::vector< ci::osc::Message > msgs; return msgs; } // erp for now
    };
    
    
    //for int values -- like perceptual schemas
    class MotionIntegerDataAverage : public MotionDataAverage
    {
    public:
        MotionIntegerDataAverage( std::vector<SignalAnalysisEventOutput *> sigz ) : MotionDataAverage()
        {
            signals = sigz;
            
            int N = sigz[0]->getMotionData().size();
            
            for(int i=0; i<N; i++)
            {
                motionData.push_back(new MotionAnalysisEvent(MotionAnalysisDataType::IntEvent, 0));
                
                MotionAnalysisEvent *ev = (MotionAnalysisEvent * ) sigz[0]->getMotionData().at(i);
                setMotionAnalysisMinMax(i, ev->getMin(), ev->getMax());
            }
        };
        
        
        virtual void updateMotionData(int k, double avgMotionData)
        {
            ( (MotionAnalysisEvent *)motionData[k])->setValue(std::round(avgMotionData));
        };

    };

    


};

#endif
