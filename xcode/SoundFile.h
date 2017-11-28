///
//  SoundFile.h
//  InteractiveTangoReadFromAndroid
//
//  Created by courtney on 5/9/15.
//
//

#ifndef InteractiveTangoReadFromAndroid_SoundFile_h
#define InteractiveTangoReadFromAndroid_SoundFile_h


#endif

namespace InteractiveTango {

    //CONNECT TO DANCER AND PERCEPTUAL EVENT
    //TODO: OK SO THIS IS A TEST VERSION -- FULL FUNCTIONALITY WILL INVOLVE DIFFERENT TYPES OF FILES, MELODY, ETC.
    
    //FOR TESTING ONLY 2 MOODS PER PROFILE
    
    //represents a clip played in Ableton
    class SoundFile
    {
    public:
        enum SOUND_FILE_TYPE { accompaniment=0, melody=1 }; //only two for now
    protected:
        long fileID; //unique id for the file -- important when it comes to assigning things.
        boost::shared_ptr<std::vector<int>> moodProfile; //what kind of mood perception does this have?
        boost::shared_ptr<std::vector<int>> harmonySectionProfile; //what kind of harmony section does this have?
        int trackNumber; //in ableton
        int clipNumber; //in ableton
        int AbletonID;
        double lengthInSeconds, lengthInBeats;
        SoundFile::SOUND_FILE_TYPE soundType;
        std::string filename; // for ref.
        int order; //what order in a section?
        Orchestra orchestration; //list of instruments in its orchestration (for determining which file can come next.. now only in the accomp. but could be in the melody, SOON.)

    public:
        SoundFile(boost::shared_ptr<std::vector<int>> profile, boost::shared_ptr<std::vector<int>> hsProfile, int trackNum, int clipNum, int aid, double length=4, float slen=1, long fID=0, std::string filename="", int ord=1) //default one measure length for now
        {
            moodProfile = profile;
            harmonySectionProfile = hsProfile;
            trackNumber = trackNum;
            lengthInBeats = length;
            lengthInSeconds = slen;
            clipNumber = clipNum;
            soundType = SOUND_FILE_TYPE::accompaniment;
            fileID = fID;
            order = ord;
            AbletonID = aid;
        };
        
        Orchestra *getOrchestration()
        {
            return &orchestration;
        };
        
        int getTrackNum(){ return trackNumber; };
        
        bool includesInstrument( int instrumentID )
        {
            return orchestration.includesInstrument(instrumentID);
        };
        
        bool includesOrchestration( std::vector<int> orchs )
        {
            bool hasOrch = false;
            int index = 0;
            while ( !hasOrch && ( index < orchs.size() ) )
            {
                hasOrch = includesInstrument( orchs[index] );
                index++;
            }
            return hasOrch; 
        };
        
        bool includesOrchestration( Orchestra *orch )
        {
            return orchestration.includesOrchestration(orch);
        };
        
        boost::shared_ptr<std::vector<int>> getProfile()
        {
            return moodProfile;
        };
        
        boost::shared_ptr<std::vector<int>> getHarmonySectionProfile()
        {
            return harmonySectionProfile;
        };
        
        inline int getOrder()
        {
            return order;
        };
        
        int getTrackNumber()
        {
            return trackNumber;
        };
        
        int getClipNumber()
        {
            return clipNumber;
        };
        
        int getAbletonID()
        {
            return AbletonID;
        };
        
        virtual double getLength()
        {
            return lengthInBeats;
        };
        virtual SOUND_FILE_TYPE getType()
        {
            return soundType;
        };
        virtual std::string getTypeAsString()
        {
            if( soundType == SOUND_FILE_TYPE::melody )
            {
                return "melody";
            }
            else return "accompaniment";
        };
        inline long getFileID()
        {
            return fileID;
        };
        
        virtual double getLengthSeconds()
        {
            return lengthInSeconds;
        };
    };
    
    class SoundFileBank //this holds a lot of files... all with same harmony & mood profiles
    {
    protected:
        boost::shared_ptr<std::vector<int>> moodProfile; //what kind of mood perception does this have?
        boost::shared_ptr<std::vector<int>> harmonySectionProfile; //what kind of harmony section does this have?
        std::vector<SoundFile *> soundFiles;
    public:
        SoundFileBank()
        {};
        SoundFileBank(boost::shared_ptr<std::vector<int>> profile, boost::shared_ptr<std::vector<int>> hsProfile)
        {
            setProfiles(profile, hsProfile);
        };
        
        boost::shared_ptr<std::vector<int>> getProfile(){ return moodProfile; };
        boost::shared_ptr<std::vector<int>> getHSProfile(){ return harmonySectionProfile; };
        size_t size(){ return soundFiles.size(); };
        
        void setProfiles(boost::shared_ptr<std::vector<int>> profile, boost::shared_ptr<std::vector<int>> hsProfile)
        {
            moodProfile = profile;
            harmonySectionProfile = hsProfile;
        };
        
        void addFile(SoundFile *file)
        {
            soundFiles.push_back(file);
        };
        
        SoundFile *findASound(std::vector<SoundFile *> *files)
        {
            if( files->empty() )
            {
                return NULL;
            }
            else if ( files->size() == 1 )
            {
                return files->at(0);
            }
            else
            {
                return files->at( std::rand() % files->size() );
            }
        };
        
        SoundFile *findASound()
        {
            return findASound(&soundFiles);
        };
        
        //finds one sharing at least one instrument, not entire orchestration -- MATCHING PROFILE IF POSSIBLE
        SoundFile *findASound(Orchestra *orchestration, boost::shared_ptr<std::vector<int>> profile_)
        {
            assert(soundFiles.size() > 0 );
            
            //since a soundbank has files of all the same profile, the number of instruments of ea. file will be the same
            //if this assumption doesn't hold, then this fails... note.
            int numberOfInstruments = soundFiles[0]->getOrchestration()->size();
            if( numberOfInstruments == orchestration->size() )
            {
                return findASound(orchestration);
            }
            else
            {
                std::vector<SoundFile *> fittingFiles, sameOrchFiles;
                
                //ok, now match the correct instrumentation
                for(int i=0; i<soundFiles.size(); i++)
                {
                    
                    if( soundFiles[i]->includesOrchestration( orchestration ) )
                        sameOrchFiles.push_back( soundFiles[i] );
                }
                
                
                //ok, see if we can find a perfect match within correct instrumentation
                for(int i=0; i<sameOrchFiles.size(); i++)
                {
                    
                    if( sameOrchFiles[i]->getOrchestration()->fitsProfile( profile_ ) )
                        fittingFiles.push_back( sameOrchFiles[i] );
                }
                
                if( fittingFiles.empty() )
                {
                    //ok, going to have to fudge it now
                    for(int i=0; i<sameOrchFiles.size(); i++)
                    {
                        
                        if( sameOrchFiles[i]->getOrchestration()->fitsProfileExcludingTHESEInstruments( orchestration, profile_ ) )
                            fittingFiles.push_back( sameOrchFiles[i] );
                    }
                }
            
                return findASound(&fittingFiles);
            }
        };
        
        SoundFile *findASound(Orchestra *orchestration)
        {
            std::vector<SoundFile *> sameOrchFiles;
            for(int i=0; i<soundFiles.size(); i++)
            {
                
                if( soundFiles[i]->includesOrchestration( orchestration ) )
                    sameOrchFiles.push_back( soundFiles[i] );
            }
            
            return findASound(&sameOrchFiles);
        };
        
        
        //finds a sound using one or more instruments instruments specified
        SoundFile *findASound( boost::shared_ptr<std::vector<int>> profile_ )
        {
            std::vector<SoundFile *> orchFilesFittingProfile;
            for(int i=0; i<soundFiles.size(); i++)
            {
                
                if( soundFiles[i]->getOrchestration()->fitsProfile( profile_ ) )
                    orchFilesFittingProfile.push_back( soundFiles[i] );
            }
            return findASound(&orchFilesFittingProfile);
        };
        
        bool empty(){ return soundFiles.empty(); };
        
        bool generalProfileFit(boost::shared_ptr<std::vector<int>>  p1, boost::shared_ptr<std::vector<int>>  p2)
        {
            bool fits = true;
            int index = 0;
            
            while( index < p1->size() && fits)
            {
                assert(p1->size() == p2->size());
                
                if( p1->size() != p2->size() )
                {
                    std::cout << "p1 size: " << p1->size() << "p2 size: " << p2->size() << " index: " << index << std::endl;
                    std::abort();
                }
                
                fits = p1->at(index) == p2->at(index);
                index++;
            }
            
            
            return fits;
        };
        
        
        virtual bool fitsProfile(boost::shared_ptr<std::vector<int>> profile, boost::shared_ptr<std::vector<int>> hsProfile)
        {
            bool fits;
            
            if( !profile->empty() )
            {
                fits = generalProfileFit(moodProfile, profile);
            }
            else fits = true; 
            
            if( !hsProfile->empty() && fits ) //if empty, then probably a melody, at this point... which dictates harmony and doesn't take harmony requests, etc.
            {
                //            std::cout << "harmony profile size: " << hsProfile->size() << "  profile: " << hsProfile->at(0) << "  profile: " << hsProfile->at(1) << std::endl;
                //            std::cout << "profile size: " << profile->size() << "  profile: " << profile->at(0) << "  profile: " << profile->at(1) << std::endl;
                //            std::cout.flush();
                
                fits = generalProfileFit(harmonySectionProfile, hsProfile);
            }
            return fits;
        };
        
    };
    
    class SoundFileBanksLoader
    {
    protected:
        ReadCSV readCSVWithSoundFileData;
        std::vector<SoundFileBank *> bank;
        std::vector<SoundFile *> files;
        Instruments *availableInstruments;
    public:
        SoundFileBanksLoader(Instruments *instruments)
        {
            availableInstruments = instruments;
        };
        
        virtual Instruments *getInstruments()
        {
            return availableInstruments;
        }
        
        
        virtual std::vector<SoundFileBank *> load()
        {
            //load all the shiz
            
            
            //fill in!!!!!
            
            return bank;
        };
        
        virtual void loadStartingFileBanks(int whichOrd = 1)
        {
            for(int i=0; i<files.size(); i++)
            {
                if(files[i]->getOrder()==whichOrd)
                {
                    //check if a new profile, if not, append to the existing bank.
                    bool bankExists = false;
                    int index = 0;
                    while ( index < bank.size() && !bankExists )
                    {
                        bankExists = bank[index]->fitsProfile( files[i]->getProfile(), files[i]->getHarmonySectionProfile() );
                        index++;
                    }
                    
                    if( bankExists )
                    {
                        bank[index-1]->addFile(files[i]);
                    }
                    else
                    {
                        SoundFileBank * b = new SoundFileBank(files[i]->getProfile(), files[i]->getHarmonySectionProfile() );
                        b->addFile(files[i]);
                        bank.push_back(b);
                    }
                }
            }
        };
    };
    
    //will represents master list of soundbanks, of which have many files with a mood and chord (in theory!!) -- have not written code for multiple, but will soon
    class SoundFileBanks
    {
    protected:
        std::vector<SoundFileBank *> soundBanks;
        SoundFileBanksLoader *loader;
        Instruments *availableInstruments;
    public:
        SoundFileBanks()
        {
        };
        
        ~SoundFileBanks()
        {
            //destroy all the banks TODO
        };
        
        SoundFileBanks(std::vector<SoundFileBank *> banks)
        {
            soundBanks = banks;
        };
        
        bool endOfPhrase()
        {
            bool found = false;
            int index = 0;
            while (!found && index<soundBanks.size() )
            {
                found = !(soundBanks[index]->empty());
                index++;
            }
            return !found;
        };
        
        void addSoundFile(SoundFile *sound)
        {
            //check if a new profile, if not, append to the existing bank.
            bool bankExists = false;
            int index = 0;
            while ( index < soundBanks.size() && !bankExists )
            {
                bankExists = soundBanks[index]->fitsProfile( sound->getProfile(), sound->getHarmonySectionProfile() );
                index++;
            }
            
            if( bankExists )
            {
                soundBanks[index-1]->addFile(sound);
            }
            else
            {
                SoundFileBank * bank = new SoundFileBank(sound->getProfile(), sound->getHarmonySectionProfile() );
                bank->addFile(sound);
                soundBanks.push_back(bank);
            }
        };
        
        SoundFileBanks(SoundFileBanksLoader *l)
        {
            loader = l;
            soundBanks = loader->load();
            availableInstruments = loader->getInstruments(); 
        };
        
        //used for continuing accompaniment lines
        SoundFile *getFileFittingProfile( boost::shared_ptr<std::vector<int>> profile, boost::shared_ptr<std::vector<int>> hsprofile, Orchestra *orch, boost::shared_ptr<std::vector<int>> orchProfile )
        {
            bool found = false;
            int index = 0;
            
            while(index < soundBanks.size() && !found)
            {
                found = soundBanks[index]->fitsProfile(profile, hsprofile );
                index++;
            }
            
            if(!found)
            {
                return NULL;
            }
            else
            {
                SoundFile *file = soundBanks[index-1]->findASound( orch, orchProfile );
                return file;
            }
        };
        
        //used only for ornaments at the moment
        SoundFile *getFileFittingProfile( boost::shared_ptr<std::vector<int>> profile, boost::shared_ptr<std::vector<int>> hsprofile, Orchestra *orch )
        {
            bool found = false;
            int index = 0;
            
            //for ornaments -- can still play if song has not started... for testing with them when have no schemas attached....
            if( hsprofile->empty() && profile->empty())
            {
                int whichSoundBank = std::rand() % soundBanks.size();
                SoundFile *file = soundBanks[whichSoundBank]->findASound( orch );
                return file;
            }
            
            while(index < soundBanks.size() && !found)
            {
                found = soundBanks[index]->fitsProfile(profile, hsprofile );
                index++;
            }
            
            if(!found)
            {
                return NULL;
            }
            else
            {
                SoundFile *file = soundBanks[index-1]->findASound( orch );
                return file;
            }
        };
        
        //this is not for only phrase start -- phrase start for orch but also handles continuing melodies... whoops... refactor later
        SoundFile *getFileFittingProfileForPhraseStart(boost::shared_ptr<std::vector<int>> profile, boost::shared_ptr<std::vector<int>> hsprofile, boost::shared_ptr<std::vector<int>> instrumentProfile=NULL )
        //if hsprofile is empty --> this then is prob. melody or what-have-you but it doesn't matter what the harmony or structure is now -- could add structures later
        {
            bool found = false;
            int index = 0;
            
            assert( !hsprofile->empty() && !profile->empty() );

            
            while(index < soundBanks.size() && !found)
            {
                found = soundBanks[index]->fitsProfile(profile, hsprofile);
                index++;
            }
            
            if(!found)
            {
                return NULL;
            }
            else
            {
                SoundFile *file;
                if( instrumentProfile!=NULL )
                    file = soundBanks[index-1]->findASound(instrumentProfile);
                else file = soundBanks[index-1]->findASound();
//                    if( profile->size() >= 2 )
//                    {
//                        std::cout << "index - " << index  << std::endl;
//                        std::cout << "size - " << soundBanks.size()  << std::endl;
//
//                        std::cout << " profile->at(0): " << profile->at(0);
//                        std::cout << " profile->at(1): " << profile->at(1);
//                        std::cout << " hsprofile->at(0): " << hsprofile->at(0);
//                        std::cout << " hsprofile->at(1): " << hsprofile->at(1);
//                        std::cout << " Sound Mood: ";
//                        std::cout << file->getProfile()->at(0);
//                        std::cout << "  FileID: " << file->getFileID();
//                        std::cout << " sound type: " << file->getTypeAsString();
//                        std::cout << " track number: " << file->getTrackNumber();
//                        std::cout << " clip number: " << file->getClipNumber();
//                        std::cout << " ableton id : " << file->getAbletonID();
//
//                        std::cout << std::endl;
//                    }
                    return file;
                }
//            }
            
        };
        
        
        //this is not for only phrase start -- phrase start for orch but also handles continuing melodies... whoops... refactor later
        SoundFile *getFileFittingProfileForPhraseStart(boost::shared_ptr<std::vector<int>> hsprofile, boost::shared_ptr<std::vector<int>> profile, Orchestra *orch, boost::shared_ptr<std::vector<int>> instrumentProfile )
        //if hsprofile is empty --> this then is prob. melody or what-have-you but it doesn't matter what the harmony or structure is now -- could add structures later
        {
            bool found = false;
            int index = 0;
            
            assert( !hsprofile->empty() && !profile->empty() );
            
            
            while(index < soundBanks.size() && !found)
            {
                found = soundBanks[index]->fitsProfile(profile, hsprofile);
                index++;
            }
            
            if(!found)
            {
                return NULL;
            }
            else
            {
                SoundFile *file;
                file = soundBanks[index-1]->findASound(orch, instrumentProfile);
                return file;
            }
            //            }
            
        };

        
        int getSize()
        {
            return soundBanks.size();
        };
    };
    
    //TODO AND NOTE:  melody sound files are structured, in that they can only go to certain other melody sound files. Due to tests, I've realized that accompaniment sound files ALSO must be restricted in this manner (for a different reason). Some accompaniments just don't sound that good... and constant orchestration switching btw solo instruments sounds bad. Ergo, I am creating a file called structuredSoundFile and MelodySoundFile will inherit from that, and include the (small) additional features of melody sound files. 4/23/2015 --> CDB
    
    //same as soundfile except for references the next soundfile in a melodic phrase
    //NOTE: if/when specific melodic bits are used instead of phrases, will use harmony/section profile to organize the bits into cadences, which is already built-in
    //TODO : could also have choices for the next bits, but ehhhhhhhhhhhhhh for now <-- this would be good to implement soon as I have some choices... 4/23
    
    class StructuredSoundFile : public SoundFile
    {
    protected:
        SoundFileBanks *nextSoundBit; //possibilities for the next melody
        
    public:
        StructuredSoundFile(SoundFileBanks *nextSound, boost::shared_ptr<std::vector<int>> profile, boost::shared_ptr<std::vector<int>> hsProfile, int trackNum, int clipNum, int aid, double length=4, double slen=0, long fID=1,  std::string filename="", int ord=1) :
        SoundFile(profile, hsProfile, trackNum, clipNum, aid, length, slen, fID, filename, ord) //default one measure length for now -- melody bits will be smaller
        {
            nextSoundBit = nextSound;
        };
        
        SoundFile *getNextSound(boost::shared_ptr<std::vector<int>> hsprofile, boost::shared_ptr<std::vector<int>> profile)
        {
//            if(nextSoundBit == NULL){
//                if( order < 9 ) abort();
//                return NULL;
//            }
//            else
//            {                                                                  //TEST
//                return nextSoundBit->getFileFittingProfile(profile, hsprofile); //null if last one in melody -- NOT BEGINNING?
//            }
            return NULL;
        };
        virtual int numNextFiles()
        {
            if( nextSoundBit == NULL ) return 0;
            else return nextSoundBit->getSize();
        };
        
        virtual void addNextSound(SoundFile *sf)
        {
            assert(nextSoundBit != NULL);
            nextSoundBit->addSoundFile(sf);
        };
        
        void setSoundBanks( SoundFileBanks *banks)
        {
            nextSoundBit = banks;
        };
        
        bool endOfPhrase()
        {
            return nextSoundBit->endOfPhrase();
        };
        
        void addInstrument(Instrument *instr)
        {
            orchestration.addInstrument(instr);
        };
        
    };
    
    class MelodySoundFile : public StructuredSoundFile
    {
    protected:
        //melody bits sometimes go over a barline, and ergo, over a harmony, thus the harmony it has in the beginning is different than the end.
        //this is important since the main melody determines all the accompaniment & counter-melodic structures.
        //halfway through, the harmony number will change.
        int harmonyStart;
        int harmonyEnd;
        double halfwayLength;
    public:
        MelodySoundFile(SoundFileBanks *nextSound, boost::shared_ptr<std::vector<int>> profile, boost::shared_ptr<std::vector<int>> hsProfile, int trackNum, int clipNum, int aid, double length=4, double slen=0, int startHarmony=1, int endHarmony=1, long fID=1,  std::string filename="", int ord=1, int instrumentID=1, Instruments *ins=NULL) :
        StructuredSoundFile(nextSound, profile, hsProfile, trackNum, clipNum, aid, length, slen, fID, filename, ord) //default one measure length for now -- melody bits will be smaller
        {
            harmonyStart = startHarmony;
            harmonyEnd = endHarmony;
            halfwayLength = slen / 2.0; //find length 1/2 way through the sample
            soundType = SoundFile::SOUND_FILE_TYPE::melody;
            orchestration.addInstrument( ins->getInstrument(instrumentID) );
        };
        
        SoundFile *getNextSound(boost::shared_ptr<std::vector<int>> profile, boost::shared_ptr<std::vector<int>> hsprofile)
        {
            if(nextSoundBit == NULL) return NULL;
            else
            {
                return nextSoundBit->getFileFittingProfileForPhraseStart(profile, hsprofile, NULL); //null if last one in melody
            }
        };
        
        void changeHarmonyIfHalfway(float seconds)
        //sometimes melody fragments will chnage harmony in the middle... this should help choose more appropriate harmonies
        {
            if( seconds > halfwayLength )
                harmonySectionProfile->at(1) = harmonyEnd;
        };
        
        int endHarmony(){ return harmonyEnd;  };
        
        void setHarmony(int start, int end)
        {
            harmonyStart = start;
            harmonyEnd = end;
        };
    };
    
    class AccompanimentSoundFile : public StructuredSoundFile
    {
    protected:
        //track and clip numbers maybe more than 1, depends on orch
        std::vector<int> tracks;
        std::vector<int> clips;
        std::vector<int> AbletonIDs;
    public:
        AccompanimentSoundFile(SoundFileBanks *nextSound, boost::shared_ptr<std::vector<int>> profile, boost::shared_ptr<std::vector<int>> hsProfile, double length=4, double slen=0, long fID=1,  std::string filename="", int ord = 1 ) :
        StructuredSoundFile(nextSound, profile, hsProfile, 0, 0, 0, length, slen, fID, filename, ord) //default one measure length for now -- melody bits will be smaller
        {
            soundType = SoundFile::SOUND_FILE_TYPE::accompaniment;
        };
        
        virtual void addTrackAndClip(int track, int clip, int aid)
        {
            tracks.push_back(track);
            clips.push_back(clip);
            AbletonIDs.push_back(aid);
            
        };
        
        virtual void addNextSound(SoundFile *sf)
        {
            assert(nextSoundBit != NULL);
            
            bool includesAnInstrument = false;
            int index = 0;
            while (index < orchestration.size() && !includesAnInstrument)
            {
                includesAnInstrument = ((StructuredSoundFile *)sf)->includesInstrument(orchestration.getInstrViaIndex(index)->getInstrumentID());
                index++;
            }
            bool sameSection = harmonySectionProfile->at(0) == sf->getHarmonySectionProfile()->at(0);
            
            if ( includesAnInstrument && sameSection )
                StructuredSoundFile::addNextSound(sf);
            
        };
        
        //this gets the next sound so that it fits the instrument profile, as much as it can...
        SoundFile *getNextSound(boost::shared_ptr<std::vector<int>> hsprofile, boost::shared_ptr<std::vector<int>> profile, Orchestra *orch, boost::shared_ptr<std::vector<int>> orchProfile)
        {
            if(nextSoundBit == NULL){
                if( order < 9 ) abort();
                return NULL;
            }
            else
            {                                                                  //TEST
                return nextSoundBit->getFileFittingProfile(profile, hsprofile, orch, orchProfile); //null if last one in melody -- NOT BEGINNING?
            }
        };
        
        virtual int getTracks(int i)
        {
            return tracks[i];
        };
        
        virtual int getClips(int i)
        {
            return clips[i];
        };
        
        virtual int getAbletonIDs(int i)
        {
            return AbletonIDs[i];
        };
        
        virtual long getClipSize()
        {
            return clips.size();
        }
    };
    
    class OrnamentSoundFile : public AccompanimentSoundFile
    {
    
        
        
        
    };
    
};
