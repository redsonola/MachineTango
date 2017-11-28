//
//  Dancers.h
//  InteractiveTangoReadFromAndroid
//
//  Created by courtney on 12/1/14.
//
//

#ifndef InteractiveTangoReadFromAndroid_Dancers_h
#define InteractiveTangoReadFromAndroid_Dancers_h

namespace InteractiveTango
{
    
    class TangoEntity : public SignalAnalysis //kinda like a UGEN, but not.
    {
//        virtual void update() = 0;
//        virtual double getAvgWindowedVar() = 0;
//        virtual double getAvgWindowedVarDiff()=0;
    protected:
        std::vector<MappingSchema *> mMappingSchemas;
        virtual void createSchemas(std::vector<SignalAnalysis * > *ugens) {};
    public:
        MappingSchema *getSchema(int i)
        {
            assert(i<mMappingSchemas.size() && i>=0);
            return mMappingSchemas[i];
        };

    };
    
    class Dancer : public TangoEntity
    {
    protected:
        std::vector<SignalAnalysis * > rightFoot;
        std::vector<SignalAnalysis * > leftFoot;
        std::vector<SignalAnalysis * > back;
        
        std::vector<SignalAnalysisEventOutput * > vars, varDiffs;
        MotionDataAverage *totalVariance, *totalVarianceDiff;
        
        FootOnset *onsets;
        bool rightFootInit, leftFootInit, backInit, stepDInit, isSchemaInit;
        int rightFootID, leftFootID, backID;
        BeatTiming *timer;
        
    public:
        enum BodyPart{ RightFoot=0, LeftFoot=1, Back=2 };
        enum WhichSignals{ INPUT_SIGNAL=0, LPFILTER5Hz=1, SNAP_PEAKS=2, LPFILTER15Hz=3, WINDOWED_VAR=4, WINDOWED_VARDIFF=5 };
        Dancer(BeatTiming *btimer){
            rightFootInit = false;
            leftFootInit = false;
            backInit = false;
            stepDInit = false;
            isSchemaInit = false;
            timer = btimer;
        };
        ~Dancer()
        {
            if(onsets != NULL)
            {
                delete onsets;
            }

            if(totalVariance != NULL)
            {
                delete totalVariance;
            }
            
            if(totalVarianceDiff != NULL)
            {
                delete totalVarianceDiff;
            }
            
            for(int i=0; i<vars.size(); i++)
            {
                if( vars[i] != NULL )
                {
                    delete vars[i];
                }
            }
            
            for(int i=0; i<varDiffs.size(); i++)
            {
                if( varDiffs[i] != NULL )
                {
                    delete varDiffs[i];
                }
            }
            
        };
        bool isInit()
        {
            return rightFootInit && leftFootInit && backInit;
        };
        
        int getRightFootSensorID()
        {
            return rightFootID;
        };
        
        int getLeftFootSensorID()
        {
            return leftFootID;
        };
        
        int getBackSensorID()
        {
            return backID;
        };
        
        void addSensorBodyPart(int idz, SensorData *sensor, std::vector<SignalAnalysis * > *ugens, BodyPart whichBody )
        {
            //which body part is the sensor of?
            std::vector<SignalAnalysis * > *bUgens;
            switch (whichBody)
            {
                case RightFoot:
                    bUgens = &rightFoot;
                    rightFootInit = true;
                    rightFootID = idz;
                    break;
                case LeftFoot:
                    bUgens = &leftFoot;
                    leftFootInit = true;
                    leftFootID = idz;
                    break;
                case Back:
                    bUgens = &back;
                    backInit = true;
                    backID = idz;
                    break;
                default:
                    break;
            };
            
        
            //add all the basic signal analysis that happens for ea. sensor
            InteractiveTango::InputSignal *signal_input = new InteractiveTango::InputSignal(idz, sensor->getWhichSensor() == ANDROID_SENSOR);
        
            signal_input->setInput(sensor);
        
            bUgens->push_back( signal_input );
            
        
            //was 20... worked with 20
            InteractiveTango::LPFilter5Hz5d *filter5 = new InteractiveTango::LPFilter5Hz5d(signal_input);
            InteractiveTango::SnapPeaks *peaks = new InteractiveTango::SnapPeaks( idz, sensor->getPort(), filter5, timer );
        
            bUgens->push_back( filter5 );
            bUgens->push_back( peaks );
        
            InteractiveTango::LPFilter15Hz *filter15 = new InteractiveTango::LPFilter15Hz(signal_input);
            InteractiveTango::WindowedVariance *variance = new InteractiveTango::WindowedVariance(filter15,
                                                                                              InteractiveTango::SR / 3, idz, sensor->getPort() ) ;
            
            InteractiveTango::WindowedVarianceDifference *varianceDiff = new InteractiveTango::WindowedVarianceDifference(variance, idz, sensor->getPort());
            
            bUgens->push_back(filter15);
            bUgens->push_back(variance);
            bUgens->push_back(varianceDiff);
            
            //copy over to main signal processing chain
            for(int i=0; i<bUgens->size(); i++)
            {
                ugens->push_back( bUgens->at(i) );
            }
            
            //add the step onset once both legs are added
            if( rightFootInit && leftFootInit && !stepDInit)
            {
                onsets = new InteractiveTango::FootOnset( (SnapPeaks *) leftFoot.at(SNAP_PEAKS),(SnapPeaks *)rightFoot.at(SNAP_PEAKS), leftFootID , rightFootID);
                ugens->push_back(onsets);
                
                float bps = timer->getBPM() / (60.0) ;
                onsets->setMotionAnalysisMinMax(FootOnset::MotionDataIndices::STEP_COUNT, 0, (bps/2)*4 ); //estimates for now -- max is steping every beat for now, need to change constant number window 4 to a variable to, and also have this linked to change in bpm
                onsets->setMotionAnalysisMinMax(FootOnset::MotionDataIndices::AVG_PEAK, 0.5, 3); //estimates for now
//              onsets->setMotionAnalysisMinMax(FootOnset::MotionDataIndices::TIME_BETWEEN_STEP);
                
                stepDInit = true;

            }
            
            //create the mapping shemas
            if(stepDInit && backInit && !isSchemaInit)
            {
                //assign var and dvar HERE
                vars.push_back( (SignalAnalysisEventOutput *) leftFoot.at(WhichSignals::WINDOWED_VAR) );
                vars.push_back( (SignalAnalysisEventOutput *) rightFoot.at(WhichSignals::WINDOWED_VAR) );
                vars.push_back( (SignalAnalysisEventOutput *) back.at(WhichSignals::WINDOWED_VAR) );
                
                varDiffs.push_back( (SignalAnalysisEventOutput *) leftFoot.at(WhichSignals::WINDOWED_VARDIFF) );
                varDiffs.push_back( (SignalAnalysisEventOutput *) rightFoot.at(WhichSignals::WINDOWED_VARDIFF) );
                varDiffs.push_back( (SignalAnalysisEventOutput *) back.at(WhichSignals::WINDOWED_VARDIFF) );
                
                totalVariance = new MotionDataAverage( vars );
                totalVarianceDiff = new MotionDataAverage( varDiffs );
                
                //set minMax -- TODO: shimmers have different motion values -- need to normalize (since Android doesn't & back values are
                //different
                totalVarianceDiff->setMotionAnalysisMinMax(0, 0.0, 0.12);
                totalVariance->setMotionAnalysisMinMax(0, 0.0, 1.0);

                ugens->push_back(totalVariance);
                ugens->push_back(totalVarianceDiff);
                
//                createSchemas(ugens); //not creating schemas now...
                isSchemaInit = true;
            }

        };
        std::vector<SignalAnalysis * > * getRightFoot(){ return &rightFoot; };
        std::vector<SignalAnalysis * > * getLeftFoot(){ return &leftFoot; };
        std::vector<SignalAnalysis * > * getBack(){ return &leftFoot; };
        FootOnset * getOnsets(){ return onsets; };
        SignalAnalysisEventOutput *getAvgVarianceDiff(){ return totalVarianceDiff; };
        SignalAnalysisEventOutput *getAvgVariance(){ return totalVariance; };

        
        virtual std::vector<ci::osc::Message> getOSC()
        {
            std::vector<ci::osc::Message> msgs;
            return msgs;
        };
        
        //TODO: FILL IN
        virtual void update(float seconds = 0)
        {
//            std::cout << ( ( MotionAnalysisEvent * ) totalVariance->getMotionData().at(0) )->asDouble() << std::endl;
//            playAccompaniment->update(seconds);
        };
        
    };
    
    class Pareja : public TangoEntity
    {
        protected:
            Dancer *leader;
            Dancer *follower;
        
            CrossCoVar *leaderLeftFollowerRightFeet;
            CrossCoVar *leaderRightFollowerLeftFeet;
            CrossCoVar *backs;
        
//            MotionIntegerDataAverage * heavyVsLightEventPareja, *busyVsSparsePareja; //schema for both dancers
//            std::vector<SignalAnalysisEventOutput *> heavyVsLights;
//            std::vector<SignalAnalysisEventOutput *> busyVsSparse;

            //this is averaging across dancers
            MotionDataAverage *totalVariance, *totalVarianceDiff, *avgFootOnsets, *avgCrossCoVariance;
            std::vector<SignalAnalysisEventOutput * > vars, varDiffs, onsets, crossco;

            void addCrossCoVar(std::vector<SignalAnalysis * > *ugens)
            {
                 leaderLeftFollowerRightFeet = new CrossCoVar( ( Filter *) leader->getLeftFoot()->at(Dancer::LPFILTER5Hz),
                                                                         ( Filter *) follower->getRightFoot()->at(Dancer::LPFILTER5Hz),
                                                                         ( WindowedVariance *) leader->getLeftFoot()->at(Dancer::WINDOWED_VAR),
                                                                         ( WindowedVariance *) follower->getRightFoot()->at(Dancer::WINDOWED_VAR),
                                                                         leader->getLeftFootSensorID(),
                                                                         follower->getRightFootSensorID(),
                                                                         "", //there is no port
                                                                         ""); //there is no port anymore
                
                
                 leaderRightFollowerLeftFeet = new CrossCoVar(( Filter *) leader->getRightFoot()->at(Dancer::LPFILTER5Hz),
                                                                         ( Filter *) follower->getLeftFoot()->at(Dancer::LPFILTER5Hz),
                                                                         ( WindowedVariance *) leader->getRightFoot()->at(Dancer::WINDOWED_VAR),
                                                                         ( WindowedVariance *) follower->getLeftFoot()->at(Dancer::WINDOWED_VAR),
                                                                         leader->getRightFootSensorID(),
                                                                         follower->getLeftFootSensorID(),
                                                                         "", //there is no port
                                                                         ""); //there is no port anymore
                
                
                 backs = new CrossCoVar(( Filter *) leader->getBack()->at(Dancer::LPFILTER5Hz),
                                                                         ( Filter *) follower->getBack()->at(Dancer::LPFILTER5Hz),
                                                                         ( WindowedVariance *)leader->getBack()->at(Dancer::WINDOWED_VAR),
                                                                         ( WindowedVariance *)follower->getBack()->at(Dancer::WINDOWED_VAR),
                                                                         leader->getBackSensorID(),
                                                                         follower->getBackSensorID(),
                                                                         "", //there is no port
                                                                         ""); //there is no port anymore
                ugens->push_back(leaderLeftFollowerRightFeet);
            };
        
        
        public:
        
            enum WhichSchemas{ POINTY_ROUNDED = 0, BUSY_SPARSE=1, ROOM_SIZE=2, THICK_THIN=3 }; //just one for now

            Pareja(Dancer *l, Dancer *f, std::vector<SignalAnalysis * > *ugens, BeatTiming *timer)
            {
                leader = l;
                follower = f;
                addCrossCoVar(ugens);
                averageAcrossDancers(ugens);
                createSchemas(ugens, timer);
            };
        
            virtual std::vector<ci::osc::Message> getOSC()
            {
                std::vector<ci::osc::Message> msgs;
                return msgs;
            };
        
            virtual void averageAcrossDancers(std::vector<SignalAnalysis * > *ugens)
            {
                
                //push back variance data to create averages
                vars.push_back(leader->getAvgVariance());
                vars.push_back(follower->getAvgVariance());
                
                varDiffs.push_back(leader->getAvgVarianceDiff());
                varDiffs.push_back(follower->getAvgVarianceDiff());
                
                onsets.push_back(leader->getOnsets());
                onsets.push_back(follower->getOnsets());
                
                crossco.push_back(leaderLeftFollowerRightFeet);
                crossco.push_back(leaderRightFollowerLeftFeet);
                crossco.push_back(backs);

                
                //average movement variance across leader and follower
                totalVariance = new MotionDataAverage(vars);
                totalVarianceDiff = new MotionDataAverage(varDiffs);
                avgFootOnsets = new MotionDataAverage(onsets);
                avgCrossCoVariance = new MotionDataAverage(crossco);

                //add to ugens so that it is in the processing loop
                ugens->push_back(totalVariance);
                ugens->push_back(totalVarianceDiff);
                ugens->push_back(avgFootOnsets);
                ugens->push_back(avgCrossCoVariance);
            };
        
            virtual void createSchemas(std::vector<SignalAnalysis * > *ugens, BeatTiming *timer) //TODO set default min and maxes in the signal analysis classes
            {
                
                MotionAnalysisEvent *stepNum = (MotionAnalysisEvent *)avgFootOnsets->getMotionData().at(FootOnset::MotionDataIndices::STEP_COUNT);
                MotionAnalysisEvent *winVarDiff = (MotionAnalysisEvent *)totalVarianceDiff->getMotionData().at(0);
                MotionAnalysisEvent *winVar = (MotionAnalysisEvent *)totalVariance->getMotionData().at(0);
                MotionAnalysisEvent *crosscovar = (MotionAnalysisEvent *)avgCrossCoVariance->getMotionData().at(0);
//                MotionAnalysisEvent *onsetPeak = (MotionAnalysisEvent *)onsets->getMotionData().at(FootOnset::MotionDataIndices::AVG_PEAK); //not using...
//                MotionAnalysisEvent *crosscovar = (MotionAnalysisEvent *)foot_onsets->getMotionData().at(FootOnset::MotionDataIndices::AVG_PEAK); //not using...
                
             
                //this is only for couple... for now
                PointyVsSmoothEvent *pv = new PointyVsSmoothEvent(timer,  stepNum,   winVarDiff );
                BusyVsSparseEvent *bs = new BusyVsSparseEvent(timer,  winVar,   stepNum,   crosscovar );
                RoomSizeAreaEvent *rs = new RoomSizeAreaEvent(timer,  stepNum,   crosscovar );
                OrchThickVsThinEvent *ot =  new OrchThickVsThinEvent(timer,  stepNum,  winVar,  crosscovar );
                
                //add to signal processing tree
                ugens->push_back(pv);
                ugens->push_back(bs);
                ugens->push_back(rs);
                ugens->push_back(ot);
                
                mMappingSchemas.push_back(pv);
                mMappingSchemas.push_back(bs);
                mMappingSchemas.push_back(rs);
                mMappingSchemas.push_back(ot);

            //
            //
            //              PointyVsSmoothEvent *ev = new PointyVsSmoothEvent(timer, stepNum, winVarDiff) ;
            //              mMappingSchemas.push_back( ev );
            //              ugens->push_back( ev );
            //
            //              MotionAnalysisEvent *winVar,  MotionAnalysisEvent *stepNum,  MotionAnalysisEvent *crosscovar_avg
            //
            //              //add Busy vs. Sparse Schema
            //              MotionAnalysisEvent *variance = (MotionAnalysisEvent *)totalVariance->getMotionData().at(0);
            //              BusyVsSparseEvent *bv = new BusyVsSparseEvent(timer, variance, stepNum) ;
            //              mMappingSchemas.push_back( bv );
            //              ugens->push_back( bv );
            
            //TEMPORARY PLACEMENT!!! Should have a SONG/WORK class that is handling diff. parts and CONNECTS to dancer, but here we go
            //              playAccompaniment = new SoundFile(timer);
            //              playAccompaniment->addSchema(ev);
            //              playAccompaniment->addSchema(bv);
            //              ugens->push_back(playAccompaniment);
        };
        
//            void createSchemas(std::vector<SignalAnalysis * > *ugens) //TODO set default min and maxes in the signal analysis classes
//            {
//
//                
//            };
        
            virtual void update(float seconds = 0)
            {
            };
    
    };

};

#endif
