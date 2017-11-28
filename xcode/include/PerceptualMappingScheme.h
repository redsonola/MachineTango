//
//  PerceptualMappingScheme.h
//  InteractiveTangoReadFromAndroid
//
//  Created by courtney on 12/1/14.
//
//

#ifndef InteractiveTangoReadFromAndroid_PerceptualMappingScheme_h
#define InteractiveTangoReadFromAndroid_PerceptualMappingScheme_h


#endif


namespace InteractiveTango
{
    
enum SoundUnitType { Background, MainMelody, OrnateDetail };
    
class MappingSchema : public SignalAnalysisEventOutput
{
protected:
    BeatTiming *beatTimer;
    std::vector<MotionAnalysisData *> mData;
    std::vector<double> mWeights;
public:
    MappingSchema( BeatTiming *timer )
    {
        beatTimer = timer;
    };
    
    std::vector<ci::osc::Message> getOSC()
    {
        std::vector<ci::osc::Message> msgs;
        return msgs;
    };
};
    
class PerceptualEvent : public MappingSchema
{
protected:
    int curMood; //mood on some scale, 1..5 (FOR NOW)
    int minMood, maxMood;
public:
    PerceptualEvent(BeatTiming *timer) :  MappingSchema(timer)
    {
        //defaults
        minMood = 1;
        maxMood = 5;
        motionData.push_back(new MotionAnalysisEvent(MotionAnalysisDataType::IntEvent, 0));
    };
    
    ~PerceptualEvent()
    {
        if( motionData[0] != NULL )
        {
            delete motionData[0];
        }
    };
    virtual void updateMotionData()
    {
        ( (MotionAnalysisEvent *)motionData[0])->setValue(curMood);
    };
    int getCurMood()
    {
        return curMood;
    };
    
    virtual void determineMood() = 0;
    virtual void update(float seconds = 0)
    {
        determineMood();
        updateMotionData();
    };
    
    double findMood()
    {
        double equalWeight = ( (double) maxMood )/( (double) mData.size() );
        double mood = 0;
        for(int i=0; i<mWeights.size(); i++)
        {
            mood = mood + (mWeights[i]* ((MotionAnalysisEvent *)mData[i])->scaledValue()*equalWeight);
        }
        return mood;
    };
};
    
//CONNECT TO DANCER AND PERCEPTUAL EVENT
//TODO: OK SO THIS IS A TEST VERSION -- FULL FUNCTIONALITY WILL INVOLVE DIFFERENT TYPES OF FILES, MELODY, ETC.

//FOR TESTING ONLY 2 MOODS PER PROFILE
    
//represents a clip played in Ableton
class SoundFile
{
protected:
    std::vector<int> moodProfile; //what kind of mood perception does this have?
    std::vector<int> harmonySectionProfile; //what kind of harmony section does this have?
    int trackNumber; //in ableton
    int clipNumber; //in ableton
    double lengthInBeats;
public:
    enum SOUND_FILE_TYPE { accompaniment=0, melody=1 }; //only two for now
    
    SoundFile(std::vector<int> profile, std::vector<int> hsProfile, int trackNum, int clipNum, double length=4) //default one measure length for now
    {
        moodProfile = profile;
        harmonySectionProfile = hsProfile;
        trackNumber = trackNum;
        lengthInBeats = length;
        clipNumber = clipNum;
    };
    int getTrackNum(){ return trackNumber; };
    
    std::vector<int> getProfile()
    {
        return moodProfile;
    };
    
    std::vector<int> getHarmonySectionProfile()
    {
        return harmonySectionProfile;
    };
    
    int getTrackNumber()
    {
        return trackNumber;
    };
    
    int getClipNumber()
    {
        return clipNumber;
    };
    
    double getLength()
    {
        return lengthInBeats;
    };
    virtual SOUND_FILE_TYPE getType()
    {
        return SOUND_FILE_TYPE::accompaniment;
    };
};
    
class SoundFileBank //this holds a lot of files... all with same harmony & mood profiles
{
protected:
    std::vector<int> moodProfile; //what kind of mood perception does this have?
    std::vector<int> harmonySectionProfile; //what kind of harmony section does this have?
    std::vector<SoundFile> soundFiles;
public:
    SoundFileBank(std::vector<int> profile, std::vector<int> hsProfile)
    {
        moodProfile = profile;
        harmonySectionProfile = hsProfile;
    };
    
    void addFile(SoundFile file)
    {
        soundFiles.push_back(file);
    };
    
    SoundFile *findASound()
    {
        if( soundFiles.empty() ) return NULL;
        else if ( soundFiles.size() == 1 ) return &soundFiles[0];
        else //return a random file...
        {
            ci::Rand randGen; //TODO: seed on cur time. soon...
            return &soundFiles[ randGen.nextInt(soundFiles.size()) ];
        }
    };
    
    bool generalProfileFit(std::vector<int>  p1, std::vector<int>  p2)
    {
        bool fits = true;
        int index = 0;
        while( index < p1.size() && fits)
        {
            assert(p1.size() == p2.size());
            
            fits = p1[index] == p2[index];
            index++;
        }
        
        return fits;
    };
    
    virtual bool fitsProfile(std::vector<int> profile, std::vector<int> hsProfile)
    {
        bool fits = generalProfileFit(moodProfile, profile);
        
        if( !hsProfile.empty() && fits ) //if empty, then probably a melody, at this point... which dictates harmony and doesn't take harmony requests, etc.
        {
            fits = generalProfileFit(harmonySectionProfile, hsProfile);
        }
        return fits;
    };
};
    
class SoundFileBanksLoader
{
public:
    virtual std::vector<SoundFileBank> load()
    {
        //load all the shiz
        
        std::vector<SoundFileBank> bank;

        //fill in!!!!!
        
        return bank;
    };
};
    
//will represents master list of soundbanks, of which have many files with a mood and chord (in theory!!) -- have not written code for multiple, but will soon
class SoundFileBanks
{
protected:
    std::vector<SoundFileBank> soundBanks;
    std::vector<std::vector<int>> profiles;
    SoundFileBanksLoader loader;
public:
    SoundFileBanks()
    {
        soundBanks = loader.load();
    };
    
    SoundFile *getFileFittingProfile(std::vector<int> profile, std::vector<int> hsprofile)
    //if hsprofile is empty --> this then is prob. melody or what-have-you but it doesn't matter what the harmony or structure is now -- could add structures later
    {
        bool found = false;
        int index = 0;

        while(index < soundBanks.size() && !found)
        {
            found = soundBanks[index].fitsProfile(profile, hsprofile);
            index++;
        }
        assert(found); // should find. should always exist;
        
        return soundBanks[index-1].findASound(); 
    };
};
    
//same as soundfile except for references the next soundfile in a melodic phrase
//NOTE: if/when specific melodic bits are used instead of phrases, will use harmony/section profile to organize the bits into cadences, which is already built-in
//TODO : could also have choices for the next bits, but ehhhhhhhhhhhhhh for now
class MelodySoundFile : public SoundFile
{
protected:
    SoundFileBanks *nextMelodyBit;
        
public:
    MelodySoundFile(SoundFileBanks *nextSound, std::vector<int> profile, std::vector<int> hsProfile, int trackNum, int clipNum, double length=4) :
    SoundFile(profile, hsProfile, trackNum, clipNum, length) //default one measure length for now -- melody bits will be smaller
    {
        nextMelodyBit = nextSound;
    };
        
    SoundFile *getNextMelodySound(std::vector<int> profile)
    {
        std::vector<int> hsprofile; //empty, doesn't matter
        return nextMelodyBit->getFileFittingProfile(profile, hsprofile); //null if last one in melody
    };
    
    virtual SoundFile::SOUND_FILE_TYPE getType()
    {
        return SoundFile::SOUND_FILE_TYPE::melody;
    };
};
    
//generic class containing a soundbank / schema information for a section of music (accompaniment, melody, ornamentation)
//in charge of selecting which sound to play next, when to play each sound, what other control information to send ableton (if applicable -- not needed yet)
//TODO: change Max4LIve code to include harmony/sections.
class MusicSection : public SignalAnalysisEventOutput
{
protected:
    std::vector<SignalAnalysisEventOutput *> mMappingSchemas;
    SoundFileBanks soundFileBank;
    BeatTiming *beatTimer;
    double beatsPlayed;
    SoundFile *curSoundFile;
    bool shouldStartFile;
public:
    
    bool shouldPlayNewFile()
    {
        if( curSoundFile != NULL )
        {
            return ( beatsPlayed > curSoundFile->getLength() );
        }
        else return true;
        
    };
    
    void addSchema(SignalAnalysisEventOutput *schema)
    {
        mMappingSchemas.push_back(schema);
    }
    
    MusicSection(BeatTiming *timer)
    {
        beatTimer = timer;
        curSoundFile = NULL;
        beatsPlayed = 0;
        shouldStartFile = false;
    };
    
    virtual SoundFile *getCurFile()
    {
//        //there should be ONLY one but, building for more
//        
//        //build mood profile
//        std::vector<int> profile;
//        for(int i=0; i<mMappingSchemas.size(); i++)
//        {
//            profile.push_back(  ((MotionAnalysisEvent * )mMappingSchemas[i]->getMotionData().at(0))->asInt() );
//        }
//        
//        return soundFileBank.getFileFittingProfile( profile );
    };
    
    virtual void update(float seconds = 0)
    {
        if( beatTimer->isOnBeat(0.0, seconds) ) //exactly on beat
        {
            beatsPlayed++;
        }
        if(shouldPlayNewFile())
        {
            shouldStartFile = true;
            curSoundFile = getCurFile();
            beatsPlayed = 0;
        } else shouldStartFile = false;
    };
    
    virtual void updateMotionData()
    {
        //nothing for now... maybe have a isPlaying type
    };
    
    virtual std::vector<ci::osc::Message> getOSC()
    {
        std::vector<ci::osc::Message> msgs;
        
        if( shouldStartFile )
        {
            //create a message that starts a file -- testing
            std::vector<int> profile = curSoundFile->getProfile();
            std::cout << "Play now: " << curSoundFile->getTrackNumber() << ",  " << curSoundFile->getClipNumber() << std:: endl ;
            
            //for now message - MUST FIX
            ci::osc::Message msg;
            msg.setAddress(PLAY_CLIP);
            msg.addIntArg(profile[0]);
            msg.addIntArg(profile[1]);
            msgs.push_back(msg);
        }
        
        return msgs;
    };

};
    
class AccompanimentSection : public MusicSection
{
    public:
    virtual SoundFile *getCurFile(std::vector<int> hsprofile) // receive hsprofile
    {
        //there should be ONLY one but, building for more

        //build mood profile
        std::vector<int> profile;
        for(int i=0; i<mMappingSchemas.size(); i++)
        {
            profile.push_back(  ((MotionAnalysisEvent * )mMappingSchemas[i]->getMotionData().at(0))->asInt() );
        }
        
        return soundFileBank.getFileFittingProfile( profile, hsprofile );
    };
    
    //has to be updated with the harmony/section profile
    virtual void update(std::vector<int> hsprofile, float seconds = 0 )
    {
        if( beatTimer->isOnBeat(0.0, seconds) ) //exactly on beat
        {
            beatsPlayed++;
        }
        if(shouldPlayNewFile())
        {
            shouldStartFile = true;
            curSoundFile = getCurFile(hsprofile);
            beatsPlayed = 0;
        } else shouldStartFile = false;
    };
};
    
    
class MelodySection : public MusicSection
{
protected:
    FootOnset *fo;
public:
    MelodySection(BeatTiming *timer, FootOnset *onset) : MusicSection(timer)
    {
        fo = onset;
    };
    
    virtual void update(std::vector<int> hsprofile, float seconds = 0)
    {
        if( beatTimer->isOnBeat(0.0, seconds) ) //exactly on beat
        {
            beatsPlayed++;
        }
        if(shouldPlayNewFile() && fo->isStepping() ) //wait for a foot onset to start
        {
            shouldStartFile = true;
            curSoundFile = getCurFile(hsprofile);
            beatsPlayed = 0;
        } else shouldStartFile = false;
    };

    virtual SoundFile *getCurFile(std::vector<int> hsprofile) //if a counter melody, needs to be sync'd with main melody
    {
        // build mood profile
        std::vector<int> profile;
        for(int i=0; i<mMappingSchemas.size(); i++)
        {
            profile.push_back(  ((MotionAnalysisEvent * )mMappingSchemas[i]->getMotionData().at(0))->asInt() );
        }

        
        //first, is there a next file in the melody?
        MelodySoundFile *nextFile = ((MelodySoundFile *)( (MelodySoundFile *) curSoundFile )->getNextMelodySound(profile));
        
        if(nextFile == NULL)
        {
            return soundFileBank.getFileFittingProfile( profile, hsprofile );
        }
        else
        {
            return (SoundFile *) nextFile;
        }
    };
        
    virtual std::vector<int> getHarmonySectionProfile()
    {
        return curSoundFile->getHarmonySectionProfile();
    };
};

    
class OrnamentationSection : public MusicSection
{
    //TODO: implement!!!!!!!!
    
};
    
//TODO: TRY... length btw steps controlling reverb tails.... could be interesting!!!
    
class MusicPlayer //should have a song structure, will see
{
protected:
    MelodySection main_melody; //determines structure (for now)
    std::vector<MelodySection> c_melodies;
    std::vector<AccompanimentSection> accompaniments;
    
public:
    virtual void update(float seconds = 0)
    {
        std::vector<int> hsprofile; //empty one for main melody... should fix that setup
        main_melody.update(hsprofile, seconds);
        
        hsprofile = main_melody.getHarmonySectionProfile(); //get harmony and section from current
        
        for(int i=0; i<c_melodies.size(); i++)
        {
            c_melodies[i].update(hsprofile, seconds);
        }
        
        for(int i=0; i<accompaniments.size(); i++)
        {
            accompaniments[i].update(hsprofile, seconds);
        }
    };
    
    virtual std::vector<ci::osc::Message> getOSC()
    {
        //collect all the OSC messages to send
        
        std::vector<ci::osc::Message> msgs;
        std::vector<ci::osc::Message> cmsgs;
        std::vector<ci::osc::Message> amsgs;
        
        msgs = main_melody.getOSC();
        
        for(int i=0; i<c_melodies.size(); i++)
        {
            cmsgs = c_melodies[i].getOSC();
            for(int i=0; i<cmsgs.size(); i++)
                msgs.push_back(cmsgs[i]);
        }
        
        for(int i=0; i<accompaniments.size(); i++)
        {
            amsgs =  accompaniments[i].getOSC();
            for(int i=0; i<amsgs.size(); i++)
                msgs.push_back(amsgs[i]);
        }
        
        return msgs;
    };
    
    virtual void setMainMelody(MelodySection melody)
    {
        main_melody = melody;
    };
    virtual void addCounterMelody(MelodySection cMelody)
    {
        c_melodies.push_back(cMelody);
    };
    virtual void addAccompaniment(AccompanimentSection accomp)
    {
        accompaniments.push_back(accomp);
    };
};

    /*
class PlaySoundFile : public SignalAnalysisEventOutput
{
protected:
    std::vector<SignalAnalysisEventOutput *> mMappingSchemas;
    SoundFileBank soundFileBank;
    BeatTiming *beatTimer;
    double beatsPlayed;
    SoundFile *curSoundFile;
    bool shouldStartFile;
public:
    bool shouldPlayNewFile()
    {
        if( curSoundFile != NULL )
        {
            return ( beatsPlayed > curSoundFile->getLength() );
        }
        else return true;
    
    };
    
    void addSchema(SignalAnalysisEventOutput *schema)
    {
        mMappingSchemas.push_back(schema);
    }
    
    PlaySoundFile(BeatTiming *timer)
    {
        beatTimer = timer;
        curSoundFile = NULL;
        beatsPlayed = 0;
        shouldStartFile = false;
    };
    
    SoundFile *getCurFile()
    {
        //there should be ONLY one but, building for more
        
        //build mood profile
        std::vector<int> profile;
        for(int i=0; i<mMappingSchemas.size(); i++)
        {
            profile.push_back(  ((MotionAnalysisEvent * )mMappingSchemas[i]->getMotionData().at(0))->asInt() );
        }
        
        return soundFileBank.getFileFittingProfile( profile );
    };
    
    virtual void update(float seconds = 0)
    {
        if( beatTimer->isOnBeat(0.0, seconds) ) //exactly on beat
        {
            beatsPlayed++;
        }
        if(shouldPlayNewFile())
        {
            shouldStartFile = true;
            curSoundFile = getCurFile();
            beatsPlayed = 0;
        } else shouldStartFile = false;
    };
    
    virtual void updateMotionData()
    {
        //nothing for now... maybe have a isPlaying type
    };
    
    std::vector<ci::osc::Message> getOSC()
    {
        std::vector<ci::osc::Message> msgs;
        
        if( shouldStartFile )
        {
            //create a message that starts a file -- testing
            std::vector<int> profile = curSoundFile->getProfile();
            std::cout << "Play now: " << curSoundFile->getTrackNumber() << ",  " << curSoundFile->getClipNumber() << std:: endl ;
            
            //for now message - MUST FIX
            ci::osc::Message msg;
            msg.setAddress(PLAY_CLIP);
            msg.addIntArg(profile[0]);
            msg.addIntArg(profile[1]);
            msgs.push_back(msg);
        }
        
        return msgs;
    };
};
    
    //melody -- only difference is that waits for step onset (FOR NOW)
class PlaySoundFileMelody : public PlaySoundFile
{
protected:
    FootOnset *fo;
public:
    PlaySoundFileMelody(BeatTiming *timer, FootOnset *onset) : PlaySoundFile(timer)
    {
        fo = onset;
    };
        
        
    virtual void update(float seconds = 0)
    {
        if( beatTimer->isOnBeat(0.0, seconds) ) //exactly on beat
        {
            beatsPlayed++;
        }
        if(shouldPlayNewFile() && fo->isStepping() ) //wait for a foot onset to start
        {
            shouldStartFile = true;
            curSoundFile = getCurFile();
            beatsPlayed = 0; 
        } else shouldStartFile = false;
    };
        
};
    
    */
    
};
