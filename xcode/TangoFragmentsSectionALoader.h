//
//  TangoFragmentsSectionALoader.h
//  InteractiveTangoReadFromAndroid
//
//  Created by courtney on 3/24/15.
//for now this file loads melodies from tango fragments
//
//

#ifndef InteractiveTangoReadFromAndroid_TangoFragmentsSectionALoader_h
#define InteractiveTangoReadFromAndroid_TangoFragmentsSectionALoader_h



namespace InteractiveTango {
    

//files for tango section A song (1st tango song composed by me, Courtney Brown
#define SECTION_A_MEL_FILE "/Users/courtney/Documents/Dissertation Work - Interactive Tango/song_data/fragments/melodyTestPiano_vBusySparse.csv"
#define SECTION_A_NEXT_MEL "/Users/courtney/Documents/Dissertation Work - Interactive Tango/song_data/fragments/NextMelodyPianoBusySparse.csv"
#define SECTION_A_ACCOMP_FILE  "/Users/courtney/Documents/Dissertation Work - Interactive Tango/song_data/fragments/Accompaniment.csv"
#define SECTION_A_ORCH_FILE "/Users/courtney/Documents/Dissertation Work - Interactive Tango/song_data/fragments/AccompanimentOrchestration.csv"
    
#define SECTION_A_ORNAMENT_FILE "/Users/courtney/Documents/Dissertation Work - Interactive Tango/song_data/fragments/Ornaments.csv"
#define SECTION_A_ORNAMENT_ORCH_FILE "/Users/courtney/Documents/Dissertation Work - Interactive Tango/song_data/fragments/Ornament_Orchestration.csv"
#define SECTION_A_BOLEO_ORNAMENT_FILE "/Users/courtney/Documents/Dissertation Work - Interactive Tango/song_data/fragments/BoleoOrnaments.csv"
#define SECTION_A_BOLEO_ORNAMENT_ORCH_FILE "/Users/courtney/Documents/Dissertation Work - Interactive Tango/song_data/fragments/BoleoOrnament_Orchestration.csv"
    
//files for Por Una Cabeza song
#define POR_UNA_CABEZA_MEL_FILE "/Users/courtney/Documents/Dissertation Work - Interactive Tango/song_data/por una cabeza/por_una_cabeza_melody.csv"
#define POR_UNA_CABEZA_NEXT_MEL "/Users/courtney/Documents/Dissertation Work - Interactive Tango/song_data/por una cabeza/PorUnaCabeza_NextMelodyPiano.csv"
#define POR_UNA_CABEZA_ACCOMP_FILE  "/Users/courtney/Documents/Dissertation Work - Interactive Tango/song_data/por una cabeza/PorUnaCabeza_Accompaniment.v.SparseBusy.csv"
#define POR_UNA_CABEZA_ORCH_FILE "/Users/courtney/Documents/Dissertation Work - Interactive Tango/song_data/por una cabeza/AccompanimentOrchestration_porUnaCabeza.v.SparseVsBusy.v2.csv"

    

    
//this loads all the sections, in truth
class MelodySoundFileBanksLoader : public SoundFileBanksLoader
{
public:
    MelodySoundFileBanksLoader(std::string melInfoFile_,  std::string nextMelKeyFile_, Instruments *instr) : SoundFileBanksLoader(instr)
    {
        melInfoFile = melInfoFile_;
        nextMelKeyFile = nextMelKeyFile_;
        ins = instr;
    };
    
protected:
        ReadCSV readNextMelodies;
        std::string melInfoFile;
        std::string nextMelKeyFile;
        Instruments *ins;

    
        void loadMelodyFiles()
        {
            MelodySoundFile *mel;
            bool first = true;

            while(  !readCSVWithSoundFileData.eof() )
            {
                std::vector<std::string> tokens = readCSVWithSoundFileData.getTokensInLine();
                
                if (first)
                {
                    first = false;
                }
                else if(tokens.size() > 0)
                {
                    //CDB -- fixed
                    boost::shared_ptr<std::vector<int>> profile( new std::vector<int> );
                    boost::shared_ptr<std::vector<int>> hsProfile( new std::vector<int> );
                    
//                    0       1                       2   3  4  5  6  7  8      9
//                    16,SectionACutUpMelodyStaccato6, 3, 3, 6, 5, 5, 3, 1.047, 1,
//                    FileID,FileName,HarmonyNumberFirst,HarmonyNumberSecond,MelodyOrder,TrackNumber,ClipNumber,Staccato Rating,length (seconds),Section,notes
                    
//FileID,FileName,HarmonyNumberFirst,HarmonyNumberSecond,MelodyOrder,TrackNumber,
//ClipNumber,length (seconds),Section,Busy Rating, AbletonID,notes
                
                    long fileID = std::atoi(tokens[0].c_str());
                    std::string name = tokens[1];
                    int startHarmony = std::atoi(tokens[2].c_str());
                    int endHarmony = std::atoi(tokens[3].c_str());
                    int order = std::atoi(tokens[4].c_str());
                    int trackNum = std::atoi(tokens[5].c_str()); //#4 is melody order, only for info
                    int clipNum = std::atoi(tokens[6].c_str());
                    double slen = std::atof(tokens[7].c_str()); //length in seconds
                    int section = std::atoi(tokens[8].c_str()); //which section? everything should be sectino A for now...
                    int profileRating = std::atoi(tokens[9].c_str()); //how staccato?
                    int AbletonID = std::atoi(tokens[10].c_str());
                    int instrumentID = std::atoi(tokens[11].c_str());

                    double length= 1; //dummy, using second length instead

                
                    profile->push_back( profileRating );
                    
                    hsProfile->push_back(section);
                    hsProfile->push_back(startHarmony);
                
                    mel = new MelodySoundFile( new SoundFileBanks(), profile, hsProfile, trackNum, clipNum, AbletonID, length, slen, startHarmony, endHarmony, fileID, name, order, instrumentID, ins );

                    files.push_back(mel);
                }
            }
        };
    
    void createMelodyBanks()
    {
        //CDB - this is where a database would be nice - also v. specific to circumstances, ie, only one file per profile
        //CDB -- this code exploits what I know about the data files and is not generalizable
        //CDB -- for instance, the fileID is -1 from its index in vector 'files'
        
        //CDB -- making it generalizable... NOW. 4/17/2015
        
        
        //extract next melody key info from file.
        std::vector<std::vector<int>> melodyKeys;
        bool first = true;
        while(  !readNextMelodies.eof() )
        {
            std::vector<std::string> tokens = readNextMelodies.getTokensInLine();
            std::vector<int> keypair;
            if( tokens.size() > 0 )
            {
                for(int i=0; i<2; i++)
                    keypair.push_back( std::atoi(tokens[i].c_str()) );
            
                //discard header but add everything else
                if(! first )
                    melodyKeys.push_back(keypair);
                else first = false;
            }
        }
        
        //create melody banks to chain melody bits together
        for( int i=0; i<melodyKeys.size(); i++ )
        {
            int startFileID = melodyKeys[i].at(0) - 1;
            int endFileID = melodyKeys[i].at(1) - 1;
            
            if( startFileID > -1 && endFileID > -1 )
            {
//                std::cout << "Next Melody IDs:" << startFileID << "," << endFileID << std::endl;
//                std::cout.flush();
                
                MelodySoundFile * melStart = ( MelodySoundFile * ) files[startFileID];
                MelodySoundFile * melEnd = ( MelodySoundFile * ) files[endFileID];
    
                melStart->addNextSound( melEnd );
            }
        }
        
        //add beginning melodies
        loadStartingFileBanks();
        
    };
    
    
public:
    virtual std::vector<SoundFileBank *> load()
    {
        //load all the shiz
        
        readCSVWithSoundFileData.init(melInfoFile);
        readNextMelodies.init(nextMelKeyFile);
        
        if(readCSVWithSoundFileData.opened())
        {
            loadMelodyFiles();
            readCSVWithSoundFileData.close();
        }
        else
        {
            std::cout << "File: "  << melInfoFile  << " could not be opened!  \n";
        }
        
        if(readNextMelodies.opened())
        {
            createMelodyBanks();
            readNextMelodies.close();
        }
        else
        {
            std::cout << "File: "  << nextMelKeyFile  << " could not be opened!  \n";
        }
        
        return bank;
    };
    
    virtual void testMelodies()
    {
        //this functionality is in a different application
//        std::vector<SoundFileBank *> bank;
//        std::vector<SoundFile *> files;
        
        
        
    };
};
    
class AccompanimentSoundFileBanksLoader : public SoundFileBanksLoader
{
protected:
    Instruments *availableInstruments;
public:
    AccompanimentSoundFileBanksLoader(std::string accompFile_, std::string orchFile_, Instruments *instr) : SoundFileBanksLoader(instr)
    {
        accompFile = accompFile_;
        orchFile= orchFile_;
        availableInstruments = instr;
    };

    virtual std::vector<SoundFileBank *> load()
    {
        //load all the shiz
//        std::string accInfoFile = SECTION_A_ACCOMP_FILE;
//        std::string accOrchFile = SECTION_A_ORCH_FILE;
        
        readCSVWithSoundFileData.init(accompFile);
        
        if(readCSVWithSoundFileData.opened())
        {
            loadAccompanimentFiles();
            readCSVWithSoundFileData.close();
            
            accompOrchFileData.init(orchFile);
            if(accompOrchFileData.opened())
            {
                findAllowedNextFilesBasedOnOrchestration();
                loadStartingFileBanks();
                
                accompOrchFileData.close();
            }
            else
            {
                std::cout << "Accompaniment Orchestration file could not be opened!!\n";
            }
        }
        
        return bank;
    };
    
    //so it turns out that melodies can start over previous sections, and then may require to start with a harmony #2
    //makes sense... hmmm, unlikely 3 will be a problem
    virtual void loadStartingFileBanks(int whichOrd = 1)
    {
        //TODO: should have an organic way of querying accompaniment chord/harmony size
        for( int i=1; i<=13; i++ )
            SoundFileBanksLoader::loadStartingFileBanks(i);
    }
    
    
protected:
    ReadCSV accompOrchFileData;
    std::string accompFile, orchFile;
    bool hasBusyVSsparse; // does accompaniment have busy vs. sparse?
    
    void findAllowedNextFilesBasedOnOrchestration()
    {
        bool first = true;
        
        //load the orchestration
        while(  !accompOrchFileData.eof() )
        {
            std::vector<std::string> tokens = accompOrchFileData.getTokensInLine();
            
            if (first)
            {
                first = false;
            }
            else
            {
                int fileID = std::atoi(tokens[0].c_str()) - 1;
                int instrumentID = std::atoi(tokens[1].c_str());
                int track = std::atoi(tokens[2].c_str());
                int clip = std::atoi(tokens[3].c_str());
                int AbletonID = std::atoi(tokens[4].c_str());

                
                assert(fileID > -1 && fileID < files.size()); //make sure data isn't crazy
                
                ((AccompanimentSoundFile *) files[fileID])->addInstrument( availableInstruments->getInstrument(instrumentID) );
                ((AccompanimentSoundFile *) files[fileID])->addTrackAndClip(track, clip, AbletonID);
            }
        }
        
        //ok, add to allowable next files, unfortunately, this is N^2, but it is only happening at load.
        //it tests whether appropriate to add within the accompaniment file class
        for(int i=0;  i<files.size(); i++ )
            for(int j=0; j<files.size(); j++)
            {
                ( ( AccompanimentSoundFile *) files[i] )->addNextSound(files[j]);
            }
    }
    
    void loadAccompanimentFiles()
    {
        StructuredSoundFile *soundFile;
        bool first = true;
        
        while(  !readCSVWithSoundFileData.eof() )
        {
            std::vector<std::string> tokens = readCSVWithSoundFileData.getTokensInLine();
            
            if (first)
            {
                first = false;
            }
            else
            {
                //CDB -- fixed
                boost::shared_ptr<std::vector<int>> profile( new std::vector<int> );
                boost::shared_ptr<std::vector<int>> hsProfile( new std::vector<int> );
                
                //FileID ,FileName,HarmonyNumber,HarmonyOrder,TrackNumber,ClipNumber,Staccato Rating,Orch. Rating,length(beats),Section, slen,notes
                
                long fileID = std::atoi(tokens[0].c_str());
                std::string name = tokens[1];
                int harmony = std::atoi(tokens[2].c_str());
                int order  = std::atoi(tokens[3].c_str());
//                int trackNum = std::atoi(tokens[4].c_str()); //#4 is melody order, only for info
//                int clipNum = std::atoi(tokens[5].c_str());
//                int profileRatingStacc = std::atoi(tokens[6].c_str()); //how staccato?
//                int profileRatingOrch = std::atoi(tokens[4].c_str()); //how much orch? -- NOT USING NOW!!
                double length= std::atoi(tokens[5].c_str()); //dummy, using second length instead
                int section = std::atoi(tokens[6].c_str()); //which section? everything should be sectino A for now...
                double slen = std::atof(tokens[7].c_str()); //length in seconds
                int busyVsparse = std::atof(tokens[8].c_str()); //busy v sparse rating

//                profile->push_back( profileRatingStacc );
//                profile->push_back( profileRatingOrch ); // 7-29-2016 removing for now... 
                profile->push_back( busyVsparse );
                
                hsProfile->push_back(section);
                hsProfile->push_back(harmony);
                
               // SoundFile(profile, hsProfile, trackNum, clipNum, length=4, slen=1, fID=0, filename="") //default one measure length for now
                soundFile = new AccompanimentSoundFile(new SoundFileBanks(), profile, hsProfile, length, slen, fileID, name, order);
                
                files.push_back(soundFile);
            }
        }
    };
};
    
////////------------------------------------------------
    class OrnamentSoundFileBanksLoader : public SoundFileBanksLoader
    {
        
    protected:
        ReadCSV ornamentOrchFileData;
        std::string ornamentFile, orchFile;
//        bool hasBusyVSsparse; // does accompaniment have busy vs. sparse?

        Instruments *availableInstruments;
    public:
        OrnamentSoundFileBanksLoader(std::string ornamentFile_, std::string orchFile_, Instruments *ins) : SoundFileBanksLoader(ins)
        {
            ornamentFile = ornamentFile_;
            orchFile = orchFile_;
            availableInstruments = ins;
        };
        
        virtual std::vector<SoundFileBank *> load()
        {
            readCSVWithSoundFileData.init(ornamentFile);
            
            if(readCSVWithSoundFileData.opened())
            {
                loadOrnamentFiles();
                readCSVWithSoundFileData.close();
                
                ornamentOrchFileData.init(orchFile);
                if(ornamentOrchFileData.opened())
                {
                    findAllowedNextFilesBasedOnOrchestration();
                    loadStartingFileBanks();
                    
                    ornamentOrchFileData.close();
                }
                else
                {
                    std::cout << "Ornament Orchestration file could not be opened!!\n";
                }
            }
            
            return bank;
        };
        
        //so it turns out that melodies can start over previous sections, and then may require to start with a harmony #2
        //makes sense... hmmm, unlikely 3 will be a problem
        virtual void loadStartingFileBanks(int whichOrd = 1)
        {
            for( int i=1; i<=13; i++ )
                SoundFileBanksLoader::loadStartingFileBanks(i);
        }
        
        
    protected:
        
        void findAllowedNextFilesBasedOnOrchestration()
        {
            bool first = true;
            
            //load the orchestration
            while(  !ornamentOrchFileData.eof() )
            {
                std::vector<std::string> tokens = ornamentOrchFileData.getTokensInLine();
                
                if (first)
                {
                    first = false;
                }
                else
                {
                    int fileID = std::atoi(tokens[0].c_str()) - 1;
                    int instrumentID = std::atoi(tokens[1].c_str());
                    int track = std::atoi(tokens[2].c_str());
                    int clip = std::atoi(tokens[3].c_str());
                    int AbletonID = std::atoi(tokens[4].c_str());
                    
                    
                    assert(fileID > -1 && fileID < files.size()); //make sure data isn't crazy
                    
                    ((AccompanimentSoundFile *) files[fileID])->addInstrument(availableInstruments->getInstrument(instrumentID));
                    ((AccompanimentSoundFile *) files[fileID])->addTrackAndClip(track, clip, AbletonID);
                }
            }
            
            //ok, add to allowable next files, unfortunately, this is N^2, but it is only happening at load.
            //it tests whether appropriate to add within the accompaniment file class
            for(int i=0;  i<files.size(); i++ )
                for(int j=0; j<files.size(); j++)
                {
                    ( ( AccompanimentSoundFile *) files[i] )->addNextSound(files[j]);
                }
        }
        
        void loadOrnamentFiles()
        {
            AccompanimentSoundFile *soundFile;
            bool first = true;
            
            while(  !readCSVWithSoundFileData.eof() )
            {
                std::vector<std::string> tokens = readCSVWithSoundFileData.getTokensInLine();
                
                if (first)
                {
                    first = false;
                }
                else
                {
                    
                    //CDB -- fixed
                    boost::shared_ptr<std::vector<int>> profile( new std::vector<int> );
                    boost::shared_ptr<std::vector<int>> hsProfile( new std::vector<int> );
                    
                    // FileID ,FileName,HarmonyStart,HarmonyOrder, Orch. Rating,length(beats),Section,slen,Gesture,notes
                    
                    long fileID = std::atoi(tokens[0].c_str());
                    std::string name = tokens[1];
                    int harmony = std::atoi(tokens[2].c_str());
                    int order  = std::atoi(tokens[3].c_str());
                    //                int trackNum = std::atoi(tokens[4].c_str()); //#4 is melody order, only for info
                    //                int clipNum = std::atoi(tokens[5].c_str());
                    //                int profileRatingStacc = std::atoi(tokens[6].c_str()); //how staccato?
                    int profileRatingOrch = std::atoi(tokens[4].c_str()); //how much orch?
                    double length= std::atoi(tokens[5].c_str()); //dummy, using second length instead
                    int section = std::atoi(tokens[6].c_str()); //which section? everything should be sectino A for now...
                    double slen = std::atof(tokens[7].c_str()); //length in seconds
                    int gesture = std::atof(tokens[8].c_str()); //which gesture //1. circling // 2. boleo
                    

                    profile->push_back( profileRatingOrch );
                    profile->push_back( gesture );
                    
                    hsProfile->push_back(section);
                    hsProfile->push_back(harmony);
                    
                    // SoundFile(profile, hsProfile, trackNum, clipNum, length=4, slen=1, fID=0, filename="") //default one measure length for now
                    soundFile = new AccompanimentSoundFile(new SoundFileBanks(), profile, hsProfile, length, slen, fileID, name, order);
                    
                    files.push_back(soundFile);
                }
            }
        };
    };
//--------------
    
    
    
//TODO: CHANGE so that each song isn't a new class -- instead set up a more flexible structure via a large database
//with main table referencing the appropriate melody and accompaniment files
    
class MelodySectionA : public MainMelodySection
{
public:
    MelodySectionA(BeatTiming *timer, FootOnset *onset, Instruments *instr) : MainMelodySection(timer, onset, instr, 15)
    {
        timesToRepeatSection();
        soundFileBank = new SoundFileBanks( new MelodySoundFileBanksLoader(SECTION_A_MEL_FILE, SECTION_A_NEXT_MEL, instr) );
    };
    
    virtual void timesToRepeatSection()
    {
        //hmmm... this should be loaded from my database
        const int times_to_repeat_section_a = 4;
        for(int i=0; i<times_to_repeat_section_a; i++)
            song_structure.push_back(1);
        
        //only have 2 sections B per one section A
        song_structure.push_back(2);
        song_structure.push_back(2);
    };
};

    
class AccompanimentSectionA : public AccompanimentSection    {
public:
    AccompanimentSectionA(BeatTiming *timer, Instruments *instr) : AccompanimentSection(timer, instr)
    {
        
        soundFileBank = new SoundFileBanks( new AccompanimentSoundFileBanksLoader( SECTION_A_ACCOMP_FILE, SECTION_A_ORCH_FILE, instrumentsAvailable ));
    };
};
    
class OrnamentCirclingSectionA : public OrnamentationSection
{
public:
    OrnamentCirclingSectionA( GestureToMusic *g, BeatTiming *timer, FootOnset *onset, Instruments *instr, float perWindowSize=5 ) : OrnamentationSection( g, timer, onset, instr, perWindowSize )
    {
        soundFileBank = new SoundFileBanks( new OrnamentSoundFileBanksLoader( SECTION_A_ORNAMENT_FILE, SECTION_A_ORNAMENT_ORCH_FILE, instrumentsAvailable ));
    };
};

//NOTE ---> not using at the moment
//class OrnamentBoleoSectionA : public OrnamentationSection
//{
//public:
//    OrnamentBoleoSectionA( GestureToMusic *g, BeatTiming *timer, FootOnset *onset, Instruments *instr, float perWindowSize=5 ) : OrnamentationSection( g, timer, onset, instr, perWindowSize )
//    {
//        soundFileBank = new SoundFileBanks( new OrnamentSoundFileBanksLoader( SECTION_A_BOLEO_ORNAMENT_FILE, SECTION_A_BOLEO_ORNAMENT_ORCH_FILE, instrumentsAvailable ));
//    };
//};
    
class MelodyPorUnaCabeza : public MainMelodySection
{
public:
    MelodyPorUnaCabeza(BeatTiming *timer, FootOnset *onset, Instruments *instr) : MainMelodySection(timer, onset, instr, 15)
    {
        timesToRepeatSection();
        soundFileBank = new SoundFileBanks( new MelodySoundFileBanksLoader(POR_UNA_CABEZA_MEL_FILE, POR_UNA_CABEZA_NEXT_MEL, instr) );
    };
    
    virtual void timesToRepeatSection()
    {
        //hmmm... this should be loaded from my database
        song_structure.push_back(1);
        song_structure.push_back(2);
    };
};
    
class AccompanimentPorUnaCabeza : public AccompanimentSection    {
public:
    AccompanimentPorUnaCabeza(BeatTiming *timer, Instruments *instr) : AccompanimentSection(timer, instr)
    {
        soundFileBank = new SoundFileBanks( new AccompanimentSoundFileBanksLoader( POR_UNA_CABEZA_ACCOMP_FILE, POR_UNA_CABEZA_ORCH_FILE, instrumentsAvailable ));
    };
};
    
}; //end InteractiveTango namespace

#endif
