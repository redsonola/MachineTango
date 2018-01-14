//
//  MotionAnalysisParameterRanges.h
//  InteractiveTangoReadFromAndroid
//
//  Created by courtney on 2/17/16.
//
//

#ifndef InteractiveTangoReadFromAndroid_MotionAnalysisParameterRanges_h
#define InteractiveTangoReadFromAndroid_MotionAnalysisParameterRanges_h


#endif

//replace or fill this with a database file... basically just holds constants

//Couple stepnum polynomial (5th degree):
//1.7080e-05  -6.3520e-04   8.2017e-03  -4.5226e-02   1.7814e-01   9.5468e-02
//
//Cross-covar (5th degree)
//0.010172  -0.182338   1.137096  -2.941093   2.997620   0.021621
//
//Couple windowed var:
//1.9516e+07  -3.5774e+06   2.3904e+05  -7.3003e+03   1.0970e+02   1.8231e-01
//
//Couple windowed var diff (degree 7):
//3.6294e+02  -1.3131e+03   1.8839e+03  -1.3690e+03   5.3592e+02  -1.1268e+02   1.2783e+01   1.2713e-01
//
//Couple onset peaks n=5
//0.0012650  -0.0189436   0.1013621  -0.2672395   0.5485295   0.1243328
//
//leader stepnum n=5
//8.1071e-06  -3.0616e-04   4.1743e-03  -2.8763e-02   1.7231e-01   1.4913e-01
//
//leader onset peaks n=5
//4.4795e-04  -1.0094e-02   8.3281e-02  -3.2165e-01   6.6867e-01   1.8022e-01
//
//leader windowed var
//8.14178  -34.09768   50.64901  -31.98824    8.14549    0.30036
//
//leader windowed var diff
//1.7738e+06  -6.3232e+05   7.9883e+04  -4.3001e+03   9.3799e+01   2.9718e-01
//
//follower onset Peaks
//0.0017706  -0.0274803   0.1534434  -0.3961016   0.6337066   0.1480787
//
//follower step count
//7.4299e-06  -3.4233e-04   5.5351e-03  -3.8355e-02   1.6693e-01   1.1730e-01
//
//follower windowed var diff
//
//5.9183e+06  -1.4412e+06   1.2624e+05  -4.9441e+03   9.1114e+01   2.1183e-01
//
//follower windowed var
//
//4.67319  -20.87216   32.75865  -22.17483    6.57670    0.19809


//class GeneralMotionParams
//{
//public:
//    float footOnsetPeakMin;
//    float footOnsetPeakMax;
//    
//    float windowedVarMin;
//    float windowedVarMax;
//    
//    float footOnsetMin;
//    float footOnsetMax;
//
//    float windowedDiffVarMin;
//    float windowedDiffVarMax;
//    
//    float crossCoVarMin;
//    float crossCoVarMax;
//    
//    float stepNumMin;
//    float stepNumMax;
//
//    
//    GeneralMotionParams()
//    {
//        windowedDiffVarMin = 0;
//        windowedDiffVarMax = 0.05;
//        
//        footOnsetPeakMin = 0;
//        footOnsetPeakMax = 1;
//    };
//};
//
//class SectionASongMinMaxParams : public GeneralMotionParams
//{
//public:
//    SectionASongMinMaxParams()
//    {
//        stepNumMin = 1;
//        stepNumMax = 5;
//    
//        windowedVarMin = 0;
//        windowedVarMax = 0.00508361;
//        
//        crossCoVarMin = 0;
//        crossCoVarMax = 8;
//    };
//
//};
//
//class PorUnaCabezaMinMaxParams : public GeneralMotionParams
//{
//public:
//    PorUnaCabezaMinMaxParams()
//    {
//        stepNumMin = 1;
//        stepNumMax = 8;
//        
//        windowedVarMin = 0;
//        windowedVarMax = 0.0108361;
//        
//        crossCoVarMin = 0;
//        crossCoVarMax = 8;
//    };
//    
//};


//TO DO: METHOD
//Take recorded inputs -- do LARGE moving average filter on them
//now try to create a curve which matches those the resulting data
//OR -- look at average bottom 3rd, middle 3rd, and upper 3rd, create a sep. curves for ea. then connect

class GeneralMotionParams
{
public:

    std::vector<double> windowedVarCoEfficients;
    std::vector<double> footOnsetCoEfficients;
    std::vector<double> windowedDiffVarCoEfficients;
    std::vector<double> crossCoVarCoEfficients;
    std::vector<double> stepNumCoEfficients;
    enum WhichDancer { couple, leader, follower};
    
    GeneralMotionParams()
    {

    };
    WhichDancer getWhichDancer()
    {
        return whichDancer;
    };
protected:
    WhichDancer whichDancer;
};



class SectionASongLeaderParams : public GeneralMotionParams
{
public:
    SectionASongLeaderParams()
    {
        windowedVarCoEfficients = {0, 0.02};
        footOnsetCoEfficients = {0.0, 0.45};
        windowedDiffVarCoEfficients = {0.0, 0.028};
        stepNumCoEfficients = {0, 1.9};
        whichDancer = WhichDancer::leader;

    };
};


//follower onset peak
//min 0.0
//max 4.5
//
//follower windowed var diff
//min 0.0
//max 0.045
//
//follower windowed var
//min 6.5900e-06
//max 1.4794
//
//follower stepnum
//min 0
//max 15
//

//fixed follower -- need to check leader shiz later...
class SectionASongFollowerParams : public GeneralMotionParams
{
public:
    SectionASongFollowerParams()
    {
        windowedVarCoEfficients = {0, 1.0};
        footOnsetCoEfficients = {0.0, 0.4};
        windowedDiffVarCoEfficients = {0.0, 0.05};
        stepNumCoEfficients = {0, 3};
        whichDancer = WhichDancer::follower;
    };
};


//crosscovar
//min 0.0016605
//max 3.4926
class SectionASongCoupleParams : public GeneralMotionParams
{
public:
    SectionASongCoupleParams()
    {
//        windowedVarCoEfficients = {0, 0.05};
//        footOnsetCoEfficients = {0.0, 0.45};
//        windowedDiffVarCoEfficients = {0.0, 0.028};
        crossCoVarCoEfficients = {0.0, 1.5};
//        stepNumCoEfficients = {0, 4};
        windowedVarCoEfficients = {0, 0.1};
        footOnsetCoEfficients = {0.0, 0.4};
        windowedDiffVarCoEfficients = {0.0, 0.05};
        stepNumCoEfficients = {0, 2.25}; //changed for smaller window size
        whichDancer = WhichDancer::couple;

    };
};


class PorUnaCabezaCoupleParams : public GeneralMotionParams
{
    
public:
    PorUnaCabezaCoupleParams()
    {
        windowedVarCoEfficients = {0, 0.045};
        footOnsetCoEfficients = {0.0, 0.45};
        windowedDiffVarCoEfficients = {0.0, 0.038};
        crossCoVarCoEfficients = {0.0016605, 3.5};
        stepNumCoEfficients = {0, 5};
        whichDancer = WhichDancer::couple;

    };
    

};

class PorUnaCabezaLeaderParams : public GeneralMotionParams
{
public:
    // Max Follower Step Num = 4.98
    //couple windowed var = 0.038

    PorUnaCabezaLeaderParams()
    {
        windowedVarCoEfficients = {0, 0.25};
        footOnsetCoEfficients = {0.0, 0.45};
        windowedDiffVarCoEfficients = {0.0, 0.028};
        stepNumCoEfficients = {0, 3};
        whichDancer = WhichDancer::leader;
    };
};



class PorUnaCabezaFollowerParams : public GeneralMotionParams
{
public:
    PorUnaCabezaFollowerParams()
    {
        windowedVarCoEfficients = {0, 0.38};
        footOnsetCoEfficients = {0.0, 0.48};
        windowedDiffVarCoEfficients = {0.0, 0.038};
        stepNumCoEfficients = {0, 4};
        whichDancer = WhichDancer::follower;

    };
};








