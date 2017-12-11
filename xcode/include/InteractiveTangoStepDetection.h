//
//  InteractiveTangoStepDetection.h
//  InteractiveTangoReadFromAndroid
//
//  Created by courtney on 10/15/14.

#ifndef InteractiveTangoReadFromAndroid_InteractiveTangoStepDetection_h
#define InteractiveTangoReadFromAndroid_InteractiveTangoStepDetection_h

/*
 
 Implementing real-time step detection algorithm in EyesWeb environment
 
 Jonas STANDAERT
 Wouter SPEYBROUCK

 */

#include <algorithm>

namespace InteractiveTango {
    
    
    // Define window sizes, indexes and bounds
    // A: Small window ( 0.1 sec )
    // B: Medium window ( 1 sec )
    // C: Large window ( 60 sec )
#define windowSizeA SR/10.0
#define windowSizeB SR
#define windowSizeC 3000 //SR*60.0 TEST
#define windowSizePrevSwing windowSizeA*2
#define waitBetweenPeaks SR/3 //for testing
    
#define PEAK_THRESH 3.0
    
class Stationary : public SignalAnalysis
{
public:
    Stationary( int id1, std::string portz1, SignalAnalysis *s  ) : SignalAnalysis( s, windowSizeC )
    {
        stationary = false;
        ID1 = id1;
        port = portz1;
    };
    
    virtual std::vector<ci::osc::Message> getOSC()
    {
        std::vector<ci::osc::Message> msgs;
        
        ci::osc::Message msg;
        
        msg.setAddress(SHIMMER_STATIONARY);
        
        msg.addIntArg(ID1);
        msg.addIntArg(atoi(port.c_str()));
        
        msg.addIntArg( stationary ); // true if previous A block  was ’ peak ’ ( false = non peak )
        
        msgs.push_back(msg);
        
        return msgs;
    }
    
    void update(float seconds) //try a low, then high pass filter
    {
        SignalAnalysis::update(seconds);
        if( data1.size()<windowSizeA ) return ; // don't bother until there is enough
        
        //find magnitude
        float mag;
        float statSum = 0;
        float G_FORCE2 = 9.8066 * 2.5;
        for( int i=(data1.size() - 3); i<data1.size(); i++ )
        {
            float x = data1[i]->getData(2)/G_FORCE2 ;
            float y = data1[i]->getData(3)/G_FORCE2 ;
            //float z = data1[i]->getData(4)/G_FORCE2 ;
            
            mag = sqrt( x*x + y*y );//+ z*z );
//            std::cout << "   x  " << x << "   y " << y << "    mag " << mag << std::endl;
            statSum += (  mag <= 0.1 );
        }
        
        stationary = (statSum / 3) > 0.3;
    }
    
private:
    bool stationary;
    int ID1;
    std::string port;
};
    
class StepDetection: public SignalAnalysis
{
public:
    StepDetection( int id1, std::string portz1, SignalAnalysis *s );
    ~StepDetection() ;
    
    //int timestamp;
    
//    // std. dev. of 1 and 0.1 sec windows (B and A windows resp . )
//    double stdDevB[numberOfBwindows];
//    double stdDevA[numberOfAwindows*numberOfBwindows];
    
    // ;total as std. dev.  ( last samples up to 1 minute )
//    double stdDevTotX ;
//    double stdDevTotY ;
//    double stdDevTotZ ;
    
    bool xMove ; // true if previous B block was ’moving’ phase ( false = stop )
    bool xSwing ; // true if previous A block was was ’ swing ’ phase ( false = stance )
    bool xPeak ; // true if previous A block  was ’ peak ’ ( false = non peak )
    
    bool yMove ; // true if previous B block was ’moving’ phase ( false = stop )
    bool ySwing ; // true if previous A block was was ’ swing ’ phase ( false = stance )
    bool yPeak ; // true if previous A block  was ’ peak ’ ( false = non peak )

    bool xdMove ; // true if previous B block was ’moving’ phase ( false = stop )
    bool xdSwing ; // true if previous A block was was ’ swing ’ phase ( false = stance )
    bool xdPeak ; // true if previous A block  was ’ peak ’ ( false = non peak )
    
    bool ydMove ; // true if previous B block was ’moving’ phase ( false = stop )
    bool ydSwing ; // true if previous A block was was ’ swing ’ phase ( false = stance )
    bool ydPeak ; // true if previous A block  was ’ peak ’ ( false = non peak )
    
    bool xd2Move ; // true if previous B block was ’moving’ phase ( false = stop )
    bool xd2Swing ; // true if previous A block was was ’ swing ’ phase ( false = stance )
    bool xd2Peak ; // true if previous A block  was ’ peak ’ ( false = non peak )
    
    bool yd2Move ; // true if previous B block was ’moving’ phase ( false = stop )
    bool yd2Swing ; // true if previous A block was was ’ swing ’ phase ( false = stance )
    bool yd2Peak ; // true if previous A block  was ’ peak ’ ( false = non peak )
    
    bool zMove ; // true if previous B block was ’moving’ phase ( false = stop )
    bool zSwing ; // true if previous A block was was ’ swing ’ phase ( false = stance )
    bool zPeak ; // true if previous A block  was ’ peak ’ ( false = non peak )

    std::vector<int> previousSwingPhase;
//    double previousZSwingPhase[windowSizeA];

    
    bool isXMoving(){ return xMove; };
    bool isXSwing(){ return xSwing; };
    bool isXPeak(){ return xPeak; };
    
    bool isYMoving(){ return yMove; };
    bool isYSwing(){ return ySwing; };
    bool isYPeak(){ return yPeak; };
    
    bool combinedPeak;
    int sinceLastPeak;
    bool lastFramePeak;
    
    bool isZMoving(){ return zMove; };
    bool isZSwing(){ return zSwing; };
    bool isZPeak(){ return zPeak; };
    

//    static const int  numberOfAwindows = windowSizeB / windowSizeA ;
//    static const int  numberOfBwindows = windowSizeC / windowSizeB ;
    
    // Data output to file
    std::ofstream fout ;
    std::string FileName ;
    
    int ID1;
	std::string port1;
    
    virtual std::vector<ci::osc::Message> getOSC();
    virtual void update(float seconds);
    

protected:
    //bool execute( bool &move, bool &swing, bool &peak, int bufferS, double inputValues[]);
    double findStdDev( double *buffer, int startIndex, int endIndex );
    double findStdDev( std::vector<double>buffer, int startIndex, int endIndex );

    bool execute( bool &move, bool &swing, bool &peak,int bufferS, double inputValues[], bool isY);
    bool execute(std::vector<double> input);
    bool findSwing(std::vector<double> input);


    void derivative(double input[], double *output, int sz);
    void derivative(double input[], std::vector<double> &output, int sz);

    void findMag(double inputX[], double inputY[], double *output, int sz);
    std::vector<double> findMag(std::vector<double> inputX, std::vector<double> inputY);
    void findpeaks(double input[], std::vector<double> &peaks, std::vector<int> &indices, int sz );
};
    
void StepDetection::findpeaks(double input[], std::vector<double> &peaks, std::vector<int> &indices, int sz )
{
/* from matlab
    y = y(:)';
    
    switch length(y)
case 0
    ind = [];
case 1
    ind = 1;
otherwise
    dy = diff(y);
    not_plateau_ind = find(dy~=0);
    ind = find( ([dy(not_plateau_ind) 0]<0) & ([0 dy(not_plateau_ind)]>0) );
    ind = not_plateau_ind(ind);
    if y(1)>y(2)
        ind = [1 ind];
    end
    if y(end-1)<y(end)
        ind = [ind length(y)];
    end
    end
    
    if nargout > 1
        peaks = y(ind);
    end
*/
    std::vector<double> dy;
    derivative(input, dy, sz);
    
    //find non-zero
    std::vector<int> not_plateau_ind;
    for(int i=0; i<sz; i++)
    {
        if( dy[i]!=0 ) not_plateau_ind.push_back(i);
    }
    
    if(not_plateau_ind.empty()) return; 
    for( int i=1; i<not_plateau_ind.size()-1; i++ )
    {
        if( ( dy[not_plateau_ind[i+1]] < 0 )  &  ( dy[not_plateau_ind[i]] > 0 )  )
        {
            indices.push_back( not_plateau_ind[i]  );
        }
    }
    
//later, deal with
//    if( input[0]<input[1] )
//        indices.insert(indices.begin(), 0);
//    
//    if( input[sz-2]<input[sz-1] )
//        indices.push_back(sz-1);
    
    for( int i=0; i<indices.size(); i++)
        peaks.push_back(input[indices[i]]);
}

StepDetection::StepDetection( int id1, std::string portz1, SignalAnalysis *s  ) : SignalAnalysis( s, windowSizeC )
{

    xMove = false;
    xSwing = false;
    xPeak = false;
    
    yMove = false;
    ySwing = false;
    yPeak = false;

    xdMove = false;
    xdSwing = false;
    xdPeak = false;
    
    yd2Move = false;
    yd2Swing = false;
    yd2Peak = false;
    
    xd2Move = false;
    xd2Swing = false;
    xd2Peak = false;
    
    ydMove = false;
    ydSwing = false;
    ydPeak = false;
    
    zMove = false;
    zSwing = false;
    zPeak = false;
    
    combinedPeak = false;
    lastFramePeak = false;
    
    ID1 = id1;
    port1 = portz1;
    
    sinceLastPeak = waitBetweenPeaks + 1;
    
}
    
StepDetection::~StepDetection(){}
    
//void StepDetection::derivative(double input[], double *output, int sz)
//{
//    assert(sz > 2); //absolute min.
//    
//    output[0] = 0;
//    for(int i=1; i<sz; i++)
//    {
//        output[i] = input[i-1] - input[i];
//    }
//
//}
    

void StepDetection::findMag(double inputX[], double inputY[], double *output, int sz)
{
    for( int i=0; i<sz; i++ )
    {
        output[i] = sqrt( inputX[i]*inputX[i] + inputY[i]*inputY[i] );
    }
}
    
std::vector<double> StepDetection::findMag(std::vector<double> inputX, std::vector<double> inputY)
{
    std::vector<double> output;
    for( int i=0; i<inputX.size(); i++ )
    {
        output.push_back( sqrt( inputX[i]*inputX[i] + inputY[i]*inputY[i] ) );
    }
    return output;
}

    
void StepDetection::derivative(double *input, double *output, int sz)
{
    assert(sz > 2); //absolute min.
        
    output[0] = 0;
    for(int i=1; i<sz; i++){
        output[i] = ( input[i] - input[i-1] );
    }
    
}
    
void StepDetection::derivative(double input[], std::vector<double> &output, int sz)
{
    assert(sz > 2); //absolute min.
    
    output.push_back(0);
    for(int i=1; i<sz; i++)
    {
        output.push_back( ( input[i] - input[i-1] ) );
    }
}


    
double StepDetection::findStdDev( double *buffer, int startIndex, int endIndex )
{
    double sum = 0 ;
    double dev = 0 ;
    double stddev;
    
    int N = endIndex - startIndex;
    for (int i = startIndex; i < endIndex; i ++)
    {
        sum += buffer[i] ;
    }
    for ( int i = startIndex; i < endIndex ; i ++)
    {
        dev += pow( buffer [i]- (sum/N), 2.0 ) ;
    }
    stddev = sqrt( dev / N ) ;
    
    return stddev; 
}
    
double StepDetection::findStdDev( std::vector<double>buffer, int startIndex, int endIndex )
{
    double sum = 0 ;
    double dev = 0 ;
    double stddev;
        
    int N = endIndex - startIndex;
    for (int i = startIndex; i < endIndex; i ++)
    {
        sum += buffer[i] ;
    }
    for ( int i = startIndex; i < endIndex ; i ++)
    {
        dev += pow( buffer [i]- (sum/N), 2.0 ) ;
    }
    stddev = sqrt( dev / N ) ;
        
    return stddev;
}
    
void StepDetection::update(float seconds)
{
    SignalAnalysis::update(seconds);
    if( data1.size() < windowSizeA ) return ;
    
    //fill double with buffer values from x
    double inputValuesX[data1.size()] ;
    double inputValuesY[data1.size()] ;
//    double inputValuesZ[data1.size()] ;
    
    for(int i=0; i < data1.size(); i++)
    {
        inputValuesX[i] = data1[i]->getData(2); //get x accel
    }
    double *diff = new double[data1.size()];
    double *diffBefore = new double[data1.size()];
    derivative(inputValuesX, diffBefore, data1.size());
    derivative(diffBefore, diff, data1.size());
    
    execute(xdMove, xSwing, xPeak, data1.size(), inputValuesX, false );
    execute(xdMove, xdSwing, xdPeak, data1.size(), diffBefore, false );
    execute(xdMove, xd2Swing, xd2Peak, data1.size(), diff, false );
    
    //fill double with buffer values from y
    for(int i=0; i < data1.size(); i++)
    {
        inputValuesY[i] = data1[i]->getData(3); //get y accel
    }
    derivative(inputValuesY, diffBefore, data1.size());
    derivative(diffBefore, diff, data1.size());
    execute(yMove, ySwing, yPeak, data1.size(), inputValuesY, true );
    execute(ydMove, ydSwing, ydPeak, data1.size(), diffBefore, true );
    execute(yd2Move, yd2Swing, yd2Peak, data1.size(), diff, true );
    
    /********NEW*********/
    
    int window = getNewSampleCount()+2;
    
    double mag[int(data1.size())], magDiff[data1.size()], magDiff2[data1.size()];
    findMag(inputValuesX, inputValuesY, mag, data1.size());
    derivative(mag, magDiff, data1.size());
    derivative(magDiff, magDiff2, data1.size());
    
    bool midSwing = false;
    int index = data1.size()-window;
    while (index < data1.size() && !midSwing)
    {
        if(  ( magDiff[index] == 0 ) ||  index > 0  )
        {
            if(  ( magDiff[index] == 0 ) || ( magDiff[index] > 0  && magDiff[index-1] <= 0 ) ||  ( magDiff[index] < 0  && magDiff[index-1] >= 0 ) )
            {
//                zeroCrossingCount++;
                //                    std::cout << "zeroCrossingCount  " << zeroCrossingCount << "   diff2:  " << diff2[index] << std::endl;
                midSwing = magDiff2[index] < 0 ;//|| magDiff2[index] == 0;
                //midSwing = true;
            }
        }
        index++;
    }
    
    midSwing =  (midSwing) && ( (sinceLastPeak > waitBetweenPeaks ) );
    if( midSwing || lastFramePeak ) sinceLastPeak = 0;
    else sinceLastPeak += getNewSampleCount(); //get about 2-3 samples each cycle?
    lastFramePeak = midSwing;
    
      combinedPeak = midSwing; //OK FOR NOW -- TODO: seperate this into STDDEV signal & step detection
    //    index--;
//    if ( midSwing ){
//        std::cout << data1[index]->getData(0) << std::endl;
//    }
    
//    int sz=data1.size();
//    int newS = std::min(getNewSampleCount(), sz);
//    for(int i= sz - newS; i<sz; i++)
//    {
//      std::cout << data1[i]->getData(0) << "," << mag[i] << "," << magDiff[i] <<"," << magDiff2[i] << "\n";
//    }

    /********NEW*********/

    
//    // yPeak && xPeak &&
//    combinedPeak =  (xd2Peak) && ( (sinceLastPeak > waitBetweenPeaks ) );
//    if( combinedPeak || lastFramePeak ) sinceLastPeak = 0;
//    else sinceLastPeak += getNewSampleCount(); //get about 2-3 samples each cycle?
//    
//    lastFramePeak = combinedPeak;
    
//    if( combinedPeak ){
//        int newS = getNewSampleCount();
//        int get = (data1.size()-std::ceil((newS)/2));
//        if(newS==0) get--; 
//        std::cout << data1[get]->getData(0) << "\n";
//    }

//TODO: recheck this after further refining... may want to get rid of this (waiting btw peaks)
//    if((!combinedPeak) &&  (xPeak && yPeak ))
//    {
//        std::cout << "disregarded\n";
//    }
    
    //x - dimension less useful, don't calculate for now
//    //fill double with buffer values from x
//    for(int i=0; i < data1.size(); i++)
//    {
//        inputValuesZ[i] = data1[i]->getData(4); //get x accel
//    }
//    execute(zMove, zSwing, zPeak, data1.size(), inputValuesZ);
}
    
bool StepDetection::execute(std::vector<double> input)
{
    double stdDevTot = 0;
    double stdDevA ;
    double sumOrigSigA = 0;
    bool peak = false;
    bool swing = false;
    bool move = false;
    
    int bufferSize = input.size();
    int bufferSizeB = (windowSizeB < bufferSize) ? windowSizeB : bufferSize;
    
    stdDevTot = findStdDev(input, 0, bufferSize); //std dev of all window
    double stdDevB =  findStdDev(input, bufferSize-bufferSizeB, bufferSize); //std dev of 1 sec.
    
    // Define 1 sec block as move / stop
    if ( stdDevB > 0.2*stdDevTot ){
        move = true;
    }
    else{
        move = false ;
    }
    
    // Calculate STDDEV for 0.1 sec window (A) that has been filled
    if( move )
    {
        int bufferSizeA =  (windowSizeA*2 < bufferSize) ? windowSizeA*2 : bufferSize;
        stdDevA = findStdDev(input, bufferSize-bufferSizeA, bufferSize);
        
        for(int i=bufferSize-bufferSizeA; i<bufferSize; i++)
        {
            sumOrigSigA += input[i];
        }
        // Define 0.1 sec window (A) as stance / swing
        if ( stdDevA > 0.2 * stdDevTot ){ //was 2.0 factor... checking...
            swing = true;
        }
        else{
            swing = false;
        }
    }
    else{
        swing = false;
    }
    
    if( move )
    {
        // Define 0.1 sec window (A) as peak / nonpeak
        if ( stdDevA > stdDevTot ){ //&& ( sumOrigSigA < 0 )){//|| !isY ) ){
            peak = true;
        }
        else{
            peak = false;
        }
    }
    else{
        peak = false ;
    }
    
    return peak;

}

    
bool StepDetection::findSwing(std::vector<double> input)
{
    double stdDevTot = 0;
    double stdDevA ;
    double sumOrigSigA = 0;
    bool swing = false;
    bool move = false;
        
    int bufferSize = input.size();
    int bufferSizeB = (windowSizeB < bufferSize) ? windowSizeB : bufferSize;
        
    stdDevTot = findStdDev(input, 0, bufferSize); //std dev of all window
    double stdDevB =  findStdDev(input, bufferSize-bufferSizeB, bufferSize); //std dev of 1 sec.
        
        // Define 1 sec block as move / stop
    if ( stdDevB > 0.2*stdDevTot ){
        move = true;
    }
    else{
        move = false ;
    }
    
    // Calculate STDDEV for 0.1 sec window (A) that has been filled
    if( move )
    {
        int bufferSizeA =  (windowSizeA < bufferSize) ? windowSizeA : bufferSize;
        stdDevA = findStdDev(input, bufferSize-bufferSizeA, bufferSize);
        
        for(int i=bufferSize-bufferSizeA; i<bufferSize; i++)
        {
            sumOrigSigA += input[i];
        }
        // Define 0.1 sec window (A) as stance / swing
        if ( stdDevA > 0.2*stdDevTot ){ //was 2.0 factor... checking...
            swing = true;
        }
        else{
            swing = false;
        }
    }
    else{
        swing = false;
    }
    return swing;
};

    bool StepDetection::execute( bool &move, bool &swing, bool &peak,int bufferS, double inputValues[], bool isY )
    {
        double stdDevTot = 0;
        double stdDevA ;
        double sumOrigSigA = 0;
        
        int bufferSize = bufferS;
        int bufferSizeB = (windowSizeB < bufferSize) ? windowSizeB : bufferSize;
        
        //find derivative of the signal
        double *diff;// = new double[bufferSize];
        double *diffBefore = new double[bufferSize];
        
        diff = inputValues;
        
        //derivative(inputValues, diff, bufferSize);

//        derivative(inputValues, diffBefore, bufferSize);
//        derivative(diffBefore, diff, bufferSize);
        
//        int newS = std::min(getNewSampleCount(), bufferSize);
//        for(int i= bufferSize - newS; i<bufferSize; i++)
//        {
//            std::cout << data1[i]->getData(0) << "," << inputValues[i] << "," << diffBefore[i] <<"," << diff[i] << "\n";
//        }
        
        stdDevTot = findStdDev(diff, 0, bufferSize); //std dev of all window
        double stdDevB =  findStdDev(diff, bufferSize-bufferSizeB, bufferSize); //std dev of 1 sec.
        
        // Define 1 sec block as move / stop
        if ( stdDevB > 0.2*stdDevTot ){
            move = true;
        }
        else{
            move = false ;
        }
        
        // Calculate STDDEV for 0.1 sec window (A) that has been filled
        if( move )
        {
            int bufferSizeA =  (windowSizeA < bufferSize) ? windowSizeA : bufferSize;
            stdDevA = findStdDev(diff, bufferSize-bufferSizeA, bufferSize);
            
            for(int i=bufferSize-bufferSizeA; i<bufferSize; i++)
            {
                sumOrigSigA += inputValues[i];
            }
            // Define 0.1 sec window (A) as stance / swing
            if ( stdDevA > 0.2 * stdDevTot ){ //was 2.0 factor... checking...
                swing = true;
            }
            else{
                swing = false;
            }
        }
        else{
            swing = false;
        }
        
        if( move )
        {
            // Define 0.1 sec window (A) as peak / nonpeak
            if ( stdDevA > stdDevTot ){ //&& ( sumOrigSigA < 0 )){//|| !isY ) ){
                peak = true;
            }
            else{
                peak = false;
            }
        }
        else{
            peak = false ;
        }
        
        //TODO: peak removal -- consider lower peaks when x & y trajectory the same
        
        float prevPeak = 0;
        for( int i=0; i<previousSwingPhase.size(); i++ )
        {
            prevPeak += previousSwingPhase[i];
        }
        prevPeak /= previousSwingPhase.size();
//        peak = peak && ( prevPeak >=0.7 );  //FOR TESTING!!!
        
        previousSwingPhase.push_back(swing);
        if( previousSwingPhase.size() > windowSizePrevSwing )
        {
            previousSwingPhase.erase( previousSwingPhase.begin() );
        }
        
//        std::cout << "totStdDev    " << stdDevTot << " stddevA   " << stdDevA << "   stdDevB  " << stdDevB << std::endl;
        
        //delete [] diff;
        delete [] diffBefore;
        return true;
    };
    

    
    
std::vector<ci::osc::Message> StepDetection::getOSC()
{

    std::vector<ci::osc::Message> msgs;
        
    ci::osc::Message msg;
    
    msg.setAddress(SHIMMER_STEP_DETECTION2);
    
    msg.addIntArg(ID1);
    msg.addIntArg(atoi(port1.c_str()));
    
    msg.addIntArg( xMove );
    msg.addIntArg( xSwing ); // true if previous A block was was ’ swing ’ phase ( false = stance )
    msg.addIntArg( xPeak ); // true if previous A block  was ’ peak ’ ( false = non peak )
    
    msg.addIntArg( yMove ); // true if previous B block was ’moving’ phase ( false = stop )
    msg.addIntArg( ySwing ); // true if previous A block was was ’ swing ’ phase ( false = stance )
    msg.addIntArg( yPeak ); // true if previous A block  was ’ peak ’ ( false = non peak )
    
    msg.addIntArg( zMove ); // true if previous B block was ’moving’ phase ( false = stop )
    msg.addIntArg( zSwing ); // true if previous A block was was ’ swing ’ phase ( false = stance )
    msg.addIntArg( zPeak ); // true if previous A block  was ’ peak ’ ( false = non peak )

    msg.addIntArg( combinedPeak ); // true if previous A block  was ’ peak ’ ( false = non peak )
        
    return msgs;
}
    
// Actions
    
class PeakTimer
{
protected:
    double waitTime;
    double peakTime;
    double curTime;
public:
    PeakTimer()
    {
        waitTime = 0;
        peakTime = 0;
        curTime = 0;
    };
    
    PeakTimer(double w)
    {
        waitTime = w;
    };
    
    void setWaitTime(double wt)
    {
        waitTime = wt;
    };
    
    void peakHappened()
    {
        peakTime = curTime;
    };
    
    void update(double seconds)
    {
        curTime = seconds;
    };
    
    bool readyForNextPeak()
    {
        return ( (peakTime + waitTime) < curTime );
    };
};
    
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
    
class SnapPeaks: public StepDetection
{
protected:
    double prevPeak;
    bool peakFound;
    BeatTiming *beatTimer;
    PeakTimer peakTimer;
    double curPeakHeight;
    
    double xThresh, yThresh, zThresh;

    
public:
    SnapPeaks( int id1, std::string portz1, SignalAnalysis *s, BeatTiming *bt=NULL) : StepDetection( id1, portz1, s )
    {

        xPeak = false;
        yPeak = false;
        zPeak = false;
        
        combinedPeak = false;
        lastFramePeak = false;
        
        ID1 = id1;
        port1 = portz1;
        
        sinceLastPeak = waitBetweenPeaks + 1;
        
        prevPeak = 0;
        peakFound = false;
        
        lastZPeak = false;
        
        beatTimer = bt;
        peakTimer.setWaitTime( beatTimer->getPulse8() ); //for space of a 16th note don't recognize another
        //tODO: must change when bpm changes
        
        curPeakHeight = 0; //current peak, of all axes
        
        xThresh = 0.75;
        yThresh = 1.0;
        zThresh = 0.4;

    };
    
    void setThreshes(double x, double y, double z)
    {
        xThresh = x;
        yThresh = y;
        zThresh = z;
    };
    
    bool getCombinedPeak()
    {
        return combinedPeak;
    };
    
    double getCurrentPeak()
    {
        return curPeakHeight;
    };
    
    //TODO: refactor eek!!!!
    std::vector<double> derivative(std::vector<double> input)
    {
        assert(input.size() > 2); //absolute min.
        
        std::vector<double> output;
        output.push_back(0);
        for(int i=1; i<input.size(); i++)
        {
            output.push_back( input[i-1] - input[i] );
        }
        return output;
    };
    
//    std::vector<double> derivative(double input[], int sz)
//    {
//        assert(sz > 2); //absolute min.
//        
//        std::vector<double> output;
//        output.push_back(0);
//        for(int i=1; i<sz; i++)
//        {
//            output.push_back( input[i-1] - input[i] );
//        }
//        return output;
//    };
    
//    //using 2nd derivative method, returns whether there is a peak in the sample
//    bool findpeaksDerivative(std::vector<double> input, int start, bool findMax=true)
//    {
//        std::vector<double> diff1, diff2;
//        
//        diff1 = derivative(input);
//        diff2 = derivative(diff1);
//        
//        bool peakz = false;
//        int index = start;
//        while (index < input.size() && !peakz)
//        {
//            
//            if( (  ( diff1[index] == 0 ) ||  (index > 0)  ) )// &&  input[index] > PEAK_THRESH  )
//            {
//                if( ( input[index] > 12 ) && findMax && (( diff1[index] == 0 ) || ( diff1[index] > 0  && diff1[index-1] <= 0 ) ||  ( diff1[index] < 0  && diff1[index-1] >= 0 ) ))
//                {
//                    peakz = ( diff2[index] < 0 ) ;
//                }
//                else if( !findMax && (( diff1[index] == 0 ) || ( diff1[index] < 0  && diff1[index-1] >= 0 ) ||  ( diff1[index] > 0  && diff1[index-1] <= 0 )) )
//                {
//                    peakz = ( diff2[index] > 0 ) ;
//                }
//            }
//            index++;
//        }
//        
////        if( peakz )
////            std::cout << data1[index-1]->getData(0) << "  "  << input[index-1] << std::endl;
//        
//        return peakz;
//    };
    
//    //OH GOD I need to just move everything to vectors what is wrong with me.
//    std::vector<double> findSnapPeaksSignal(std::vector<double> input, int sz)
//    {
//        double y[sz];
//        for(int i=0; i<sz; i++){
//            y[i] = input[i];
//        }
//        return findSnapPeaksSignal(y, sz);
//    
//    };
//    
//    
//    std::vector<double> findSnapPeaksSignal(double input[], int sz)
//    {
//        std::vector<double> peaks;
//        std::vector<int> indices;
//        std::vector<double> peaksSignal;
//        findpeaks(input, peaks, indices, sz, 0);
//        
//        if ( peaks.size()==0 )
//        {
//            //init with zeros
//            for( int i=0; i<sz; i++ )
//            {
//                peaksSignal.push_back(0);
//            }
//            return peaksSignal;
//        }
//        
//        //add prevPeak
//        peaksSignal.push_back( prevPeak );
//        indices.insert(indices.begin(), 0);
//        peaks.insert(peaks.begin(), prevPeak);
//        
//        for(int i=1; i<indices.size(); i++ )
//        {
//            for(int j = indices[i-1]; j<indices[i]; j++)
//            {
//                double f = double( (j - indices[i-1])+1 ) / double( indices[i]-indices[i-1] );
//                double newVale =  lerp( peaks[i-1], peaks[i], f ) ;
//                peaksSignal.push_back(newVale);
//            }
//        }
//        int end = indices.size()-1;
//        for( int i=( indices[end]+1 ); i<sz; i++ )
//        {
//            peaksSignal.push_back(peaks[end]);
//        }
//        prevPeak = peaks[end];
//        
//        int check = peaksSignal.size();
//        assert( check == sz );
//        
//        return peaksSignal;
//    };
    
    void findpeaks(double input[], std::vector<double> &peaks, std::vector<int> &indices, int sz, int start )
    {
        /* from matlab
         y = y(:)';
         
         switch length(y)
         case 0
         ind = [];
         case 1
         ind = 1;
         otherwise
         dy = diff(y);
         not_plateau_ind = find(dy~=0);
         ind = find( ([dy(not_plateau_ind) 0]<0) & ([0 dy(not_plateau_ind)]>0) );
         ind = not_plateau_ind(ind);
         if y(1)>y(2)
         ind = [1 ind];
         end
         if y(end-1)<y(end)
         ind = [ind length(y)];
         end
         end
         
         if nargout > 1
         peaks = y(ind);
         end
         */
        std::vector<double> dy;
        StepDetection::derivative(input, dy, sz);
        
        //find non-zero
        std::vector<int> not_plateau_ind;
        for(int i=start; i<sz; i++)
        {
            if( dy[i]!=0 ) not_plateau_ind.push_back(i);
        }
        
        if(not_plateau_ind.empty()) return;
        for( int i=1; i<not_plateau_ind.size()-1; i++ )
        {
            if( ( dy[not_plateau_ind[i+1]] < 0 )  &  ( dy[not_plateau_ind[i]] > 0 )  )
            {
                indices.push_back( not_plateau_ind[i]  );
            }
        }
        
        /* expanding the buffer input to this function takes care of edge cases (rather than the matlab version) */
        for( int i=0; i<indices.size(); i++)
            peaks.push_back(input[indices[i]]);
    }
    
    bool findIC(int dataIndex, double &curpeak, double thresh=0.75, int takeDerivative=1 )
    {
        //        find z peaks
        std::vector<double> inputX;
        //        double inputZ[];
        double dX[data1.size()];
        std::vector<double> xmin;
        std::vector<int> xminIndex;
        
        for(int i=0; i<data1.size(); i++)
        {
            inputX.push_back( data1[i]->getData(dataIndex) );
        }
        
        for(int i=0; i<takeDerivative; i++){
            inputX = derivative(inputX);
        }
        
        for(int i=0; i<data1.size(); i++)
        {
            dX[i] = inputX[i];
        }
        
//        int zi =0;
        int snapSample = data1.size() - std::min(getNewSampleCount(), int(SR/4)) - 5;
        bool FoundPeakX = false;
        findpeaks( dX, xmin, xminIndex, data1.size(), snapSample );

//      TODO: test new method, as this one works
//        while (zi < xminIndex.size() && !FoundPeakX )
//        {
//            FoundPeakX = (xminIndex[zi] >= snapSample+4) && (xmin[zi] > thresh );
//            zi++;
//        }
        
        //this method also finds the curPeakHeight
        curpeak = 0.0;
        for( int i=0; i<xminIndex.size(); i++ )
        {
            if(!FoundPeakX) FoundPeakX = (xminIndex[i] >= snapSample+4) && (xmin[i] > thresh );
            if((xminIndex[i] >= snapSample+4))
                curpeak = std::max( curpeak, xmin[i]);
        }
        
        
        return FoundPeakX;
    };
    
    void update(float seconds)
    {
        SignalAnalysis::update(seconds);
        combinedPeak = false; //CHECK -- but should solve problem of multiple beeps
        if( data1.size() < windowSizeB ) return ;
        if( getNewSampleCount() <=0 ) return ; 
        peakTimer.update(seconds);
        curPeakHeight = NO_DATA;
        
//        combinedPeak = findIC(2, 0.75)  && beatTimer->isOnBeat();;
//        zPeak = findIC(4, 1.0)  && beatTimer->isOnBeat();
//        yPeak = findIC(3, 0.4)  && beatTimer->isOnBeat();
        
        double peakx, peaky, peakz;
        combinedPeak = ( findIC(2, peakx, xThresh ) || findIC(4, peaky, yThresh ) || findIC(3, peakz, zThresh ) ) && beatTimer->isOnBeat();
        combinedPeak = ( combinedPeak ) && ( peakTimer.readyForNextPeak() );
        if( combinedPeak )
        {
            std::cout << "PEAK" << std::endl;
        }
        if(combinedPeak) peakTimer.peakHappened();
        curPeakHeight = std::sqrt(peakx*peakx + peaky*peaky + peakz*peakz);

        
//        if( FoundPeakZ )
//        {
//            snapSample = data1.size() - std::min(getNewSampleCount(), 5);
//            if( snapSample > 0 )
//            {
//                for(int i=snapSample; i<data1.size(); i++)
//                {
//                    std::cout << data1[i]->getData(0) /* <<",1" */ << "\n";
//                }
//            }
//        }
//        std::cout << data1[zminIndex[zi]]->getData(0) << " , " << zmin[zi] <<"\n";
    };
    
    virtual std::vector<ci::osc::Message> getOSC()
    {
        
        std::vector<ci::osc::Message> msgs;
        
        ci::osc::Message msg;
        
        msg.setAddress(SHIMMER_STEP_DETECTION2);
        
        msg.addIntArg(ID1);
        msg.addIntArg(atoi(port1.c_str()));
        
        msg.addIntArg( xMove );
        msg.addIntArg( xSwing ); // true if previous A block was was ’ swing ’ phase ( false = stance )
        msg.addIntArg( xPeak ); // true if previous A block
//        was ’ peak ’ ( false = non peak )
        
        msg.addIntArg( yMove ); // true if previous B block was ’moving’ phase ( false = stop )
        msg.addIntArg( ySwing ); // true if previous A block was was ’ swing ’ phase ( false = stance )
        msg.addIntArg( yPeak ); // true if previous A block  was ’ peak ’ ( false = non peak )
        
        msg.addIntArg( zMove ); // true if previous B block was ’moving’ phase ( false = stop )
        msg.addIntArg( zSwing ); // true if previous A block was was ’ swing ’ phase ( false = stance )
        msg.addIntArg( zPeak ); // true if previous A block  was ’ peak ’ ( false = non peak )
        
        msg.addIntArg( combinedPeak ); // true if previous A block  was ’ peak ’ ( false = non peak )
        
        msgs.push_back(msg);
        
        return msgs;
    }
    

    
protected:
    double lerp(double a, double b, double f)
    {
        return a + f * (b - a);
    };
    
    bool lastZPeak;
};

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------

    //TODO: add measure of how syncopated something is...
class FootOnset : public SignalAnalysisEventOutput
{
protected:
    SnapPeaks *leftFoot, *rightFoot;
    bool leftOnset, rightOnset;
    float stepsPerSampleSize;
    double curPeak, avgPeak;
    std::vector<float> stepTimes; // in seconds
    std::vector<float> stepPeaks; // in seconds
//    std::vector<float> timesSinceLastSteps; // in seconds

    float window;
    int stepCount;
    int rightID, leftID;
    double timeSinceLastStep;

    void updateSteps(float curTime)
    {
        bool inWindow = false;
        while( stepTimes.size()>0 && !inWindow )
        {
            inWindow = stepTimes[0] >= ( curTime - window );
            if(!inWindow)
            {
                stepTimes.erase(stepTimes.begin());
                stepPeaks.erase(stepPeaks.begin());
            }
        }
    };
    
    booleaan fakeStep;
    
public:
    enum MotionDataIndices { STEP_COUNT=0, AVG_PEAK=1, TIME_SINCE_LAST_STEP=2 };
    
    FootOnset(SnapPeaks *left, SnapPeaks *right, int id1, int id2, float sampleSize = 4) :  SignalAnalysisEventOutput(NULL, 0, NULL)
    {
        leftFoot = left;
        rightFoot = right;
        leftOnset = false;
        rightOnset = false;
        window = sampleSize;
        stepsPerSampleSize = 0; //includes kicks... might be a problem with swinging legs
        stepCount = 0;
        avgPeak = 0;
        leftID = id1;
        rightID = id2;
        timeSinceLastStep = 0;
        fakeStep = false;
        
        motionData.push_back(new MotionAnalysisEvent(MotionAnalysisDataType::DoubleEvent, MotionDataIndices::STEP_COUNT));
        motionData.push_back(new MotionAnalysisEvent(MotionAnalysisDataType::DoubleEvent, MotionDataIndices::AVG_PEAK));
        motionData.push_back(new MotionAnalysisEvent(MotionAnalysisDataType::DoubleEvent, MotionDataIndices::TIME_SINCE_LAST_STEP));
        
        setMotionAnalysisMinMax(MotionDataIndices::STEP_COUNT, 0, 5 );
        setMotionAnalysisMinMax(MotionDataIndices::AVG_PEAK, 0, 3);
        setMotionAnalysisMinMax(MotionDataIndices::TIME_SINCE_LAST_STEP, 0, 11); //?
    };
    
    virtual void updateMotionData()
    {
        ( (MotionAnalysisEvent *)motionData[MotionDataIndices::STEP_COUNT])->setValue(stepCount);
        ( (MotionAnalysisEvent *)motionData[MotionDataIndices::AVG_PEAK])->setValue(avgPeak);
        ( (MotionAnalysisEvent *)motionData[MotionDataIndices::TIME_SINCE_LAST_STEP])->setValue(timeSinceLastStep);
    };
    
    void update(float seconds)
    {
        updateSteps(seconds);
        
        bool lastLeftOnset = leftOnset;
//        bool lastRightOnset = rightOnset;
        
        leftOnset = leftFoot->getCombinedPeak();
        rightOnset = rightFoot->getCombinedPeak();
        
        //for testing via keyboard
        if(fakeStep)
        {
            leftOnset = rightOnset = true;
            fakeStep = false;
            
            //currently peak will be 0
        }
        
        if( leftOnset && rightOnset )
        {
            if ( !lastLeftOnset )
            {
                rightOnset = false;
            }
            else leftOnset = false;
        }
        
        if(leftOnset || rightOnset)
        {
            stepTimes.push_back(seconds);
            if( leftOnset )
                curPeak = leftFoot->getCurrentPeak();
            else curPeak = rightFoot->getCurrentPeak();
            stepPeaks.push_back(curPeak);
        }
        if( stepTimes.size() > 0 )
        {
            timeSinceLastStep = stepTimes[stepTimes.size()-1] - seconds;
        }
        
        stepCount = stepTimes.size();
        avgPeak = 0;
        if(stepPeaks.size()>0)
        {
            for( int i=0; i<stepPeaks.size(); i++ )
            {
                avgPeak+=stepPeaks[i];
            }
            avgPeak /= stepPeaks.size();
        }
        updateMotionData();
    };
    
    inline int getStepCount()
    {
        return stepCount;
    };
    
    inline bool isRightOnset()
    {
        return rightOnset;
    };

    inline bool isLeftOnset()
    {
        return leftOnset;
    };
    
    inline void createFakeStep()
    {
        fakeStep = true; 
    }

    inline bool isStepping()
    {
        return ( rightOnset || leftOnset );
    };
    
    virtual std::vector<ci::osc::Message> getOSC()
    {
        std::vector<ci::osc::Message> msgs;
        
        ci::osc::Message msg;
        
        msg.setAddress(SHIMMER_STEP_DETECTION);
        
        msg.addIntArg(leftID);
        msg.addIntArg(rightID);
        
        msg.addIntArg( getStepCount() );
        msg.addIntArg( isLeftOnset() );
        msg.addIntArg( isRightOnset() );
        msg.addIntArg( isRightOnset() || isLeftOnset() );
        msg.addFloatArg(curPeak);
        msg.addFloatArg(timeSinceLastStep);


        
        msgs.push_back(msg);
    
        return msgs;
    }
    
};
    
};

#endif
