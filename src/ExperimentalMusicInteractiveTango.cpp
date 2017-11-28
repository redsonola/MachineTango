
//OK, originally I was going to do a magnetic time piece, but now I am going to do an experimental music piece which is more general
//This is my plan:
//Create a collection of melodies (ie, melody generators) to respond to busy/sparse
//create a section 1 and a section 2... maybe but have it all be determined by dancer movement... an analysis of dancer movement.....

//create melody generators which can respond more gesturally to foot movement?
//eg. mode of 1 to 1 gesture correspondence?
//vs. more weighted responses? (liike currently?)

//after melody generation really need to experiment on just myself.

#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
//#include "cinder/Capture.h"
#include "cinder/ip/Resize.h"
#include "cinder/Rand.h"
#include <fstream>
#include <sstream>

#include <assert.h>

#define _COUT_DEBUG_ 0


#include "OscSender.h"
#include "OscListener.h"

#include "InteractiveTangoOSCMessages.h"


#include "MIDIUtility.h"

#include "ReadCSV.h"
#include "ShimmerData.h"
#include "VisualizeShimmer.h"
#include "ShimmerWrapper.h"

#include "SensorData.h"

#include "Containers.h"
#include "MotionAnalysisParameterRanges.h"
#include "MotionAnalysisOuput.h"

#include "BeatTiming.h"
#include "InteractiveTangoSignalAnalysis.h"
#include "InteractiveTangoStepDetection.h"
#include "PerceptualMappingScheme.h"
#include "PerceptualSchemas.h"
#include "GestureRecognizerUGENs.h"

#include "Instruments.h"
#include "SoundFile.h"
#include "MusicPlayer.h"
#include "MelodyGeneratorAlgorithm.h"
#include "FactorOracle.h"

#include "MelodyGenerator.h"

#include "SendOSCUGENs.h"
#include "MappingSchemaEventInContinuousOut.h"

#include "TangoFragmentsSectionALoader.h"

#include "Dancers.h"

//#include "MagneticTime.h"

#define MAX_PORT 89898



using namespace ci;
using namespace ci::app;
using namespace std;

//InteractiveTango::MidiFileUtility midiFile;


//void ExperimentalMusicInteractiveTango::setup()
//{
//    InteractiveTango::MelodyGenerator generator;
//    InteractiveTango::FactorOracle fo;
//    
//    fo.train("/Users/courtney/Documents/Interactive Tango Milonga/tango_melody_midi/Piazzola_Astor_one_line_mel.mid");
//    generator.addGeneratorAlgorithm(&fo);
//    
//}


class ExperimentalMusicInteractiveTango : public AppNative {
public:
    void setup();
    void mouseDown( MouseEvent event );
    void keyDown( KeyEvent event );
    void update();
    void draw();
    SensorData * getSensor(std::string whichShimmer);
    
private:
    osc::Listener mListener;
    SensorData * getSensor(string deviceID, int which);
    void drawGrid(float size=100.0f, float step=2.0f);
    void sendPhotoSync(float seconds);
    
    std::vector<SensorData *> mSensorData;
    std::vector<VisualizeShimmer *> visualizers;
    
    std::vector<InteractiveTango::UGEN *> mUGENs;
    int mHoldShimmerIndex;
    int mHoldShimmerIndex2;
    
    ci::ColorA generateRandomColor();
    void screenshot();
    void loadSensor(std::string input);
    void parseFilenameForInfo(string fname, string *deviceID, int *whichID);
    void addStepDetection(int id1, int id2, std::string port1, std::string port2);
    virtual void prepareSettings(Settings *settings);
    void handleOSC(float seconds);
    void saveAllSensors();
    void updateSignalTree(float seconds, float pastSeconds);
    std::vector<ci::osc::Message> collectMessagesforDifferentPorts(std::string addr, std::vector<ci::osc::Message> *msgs, std::vector<ci::osc::Message> nmsgs);
    std::vector<ci::osc::Message> collectMessagesforDifferentPortsBack(std::vector<ci::osc::Message> *msgs);
    
    void loadChosenFiles();
    void loadFragmentsStaccatoSavedFiles();
    void loadPorUnaCabezaNormSavedFiles();
    void loadPorUnaCabezaStaccatoSavedFiles();
    void playPorUnaCabeza();
    void playFragments();
    void addPareja();
    void addDancer();
    void stopOSCToAbleton();
    void startOSCToAbleton();
    void stopStartOSCToAbleton();
    void restartSong();
    InteractiveTango::Pareja::SongIDs whichSongIsPlaying;
    void cycleThroughPlaylist();
    void changeSensorColor();
    void addMarkerToSensorFile(std::string marker);
    
    void setupSensorsForTest();
    void setupSensorsForTestTwoDancers();
    
    void printMenu();
    
    //for handling wii
    void handleWiiOSC(osc::Message message); // turn into commands in application
    void wiiToWekinator(osc::Message message); // forward commands to wekinator
    void printNormalWiiMenu();
    bool isWiiButtonDown(std::string addr, osc::Message message);
    
    //for machine learning in Wekinator, etc.
    bool isSendingToWekinator; //whether wii is sending messages to wekinator
    std::vector<osc::Sender *> wekinatorSenders;
    std::vector<osc::Sender *> wiiWekinatorSenders;
    std::vector<InteractiveTango::SendOSCToWek *> sendOSCToWekinators;
    int whichDTWWekinatorOutput;
    int selectedWekinatorSenderIndex;
    
    void printWekModeMenu();
    void printChangingParamsMenu();
    int addSendtoWekinator();
    void sendOSCInputsToWekinator(std::vector<ci::osc::Message> msgs, float seconds, int index );
    void handleOutputWekOSC( osc::Message message, float seconds );
    
    
    //use wii to change movement min/max params....
    bool isChangingMovementParams;
    void changeMovementParamsWithWii(osc::Message message);
    
    
    //if there are any OSC messages here, clear before proceeding.
    void clearResidualData();
    void clearListeningUDPSocket();
    
    int beatIndex; //for debug
    
    std::string mSaveFilePath;
    
    ci::CameraPersp     mCamera;
    Timer               mTimer;
    long                mIndex;
    bool                mShowMovie;
    bool				mPlayPrevious;
    
    bool                playing_done;
    
    std::string         whichShimmer;
    osc::Sender         syncSender;
    int                 selectedPareja; //for changing motion params, etc.
    
    //different senders for different messages
    //why? bc m4l is CRAZY
    osc::Sender         sender;
    osc::Sender         pVSsSender;
    osc::Sender         ornamentSender; //this sends the ornaments... OSC is getting dropped, so testing if this is why (too much sending at once);
    osc::Sender         circlingSender; //send circling steps
    osc::Sender         longStepSender; //send long step info (not yet mapped)
    osc::Sender         reverbSender;
    osc::Sender         backSender;
    osc::Sender         backSenderFollower;
    
    
    void sendOSCMessages( std::vector<ci::osc::Message> msgs, float seconds  );
    
    //to send OSC messages, etc. differing rates, keep Hz down
    float pastseconds;
    float lastSentOSC;
    
    
    void addCrossCoVar(int id1, int id2, std::string p1, std::string p2);
    
    std::vector<InteractiveTango::Dancer *> mDancers;
    std::vector<InteractiveTango::Pareja *> mParejas;
    InteractiveTango::DanceFloor *danceFloor;
    
    InteractiveTango::BeatTiming mBeatTimer;
    bool    receivedBeat;
    
    //add dancers automatically
    bool addLiveDancers;
    void addLivePareja(std::map<int, int> &l, std::map<int, int> &f, int lID, int fID);
    void addLiveDancer(SensorData *sensor, int sid);
    std::map<int, int> leader, follower, leader2, follower2;
    
    bool shouldSendOSC;
    
    
    //    Engine *ep; //matlab engine
    
};





CINDER_APP_NATIVE( ExperimentalMusicInteractiveTango, RendererGl )
