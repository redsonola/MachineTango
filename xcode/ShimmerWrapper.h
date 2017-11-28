#include <sstream>

#define MAX_MATLAB_OUTPUT_LEN 8192


#ifdef __WINDOWS__

/* get_characteristics figures out the size, and category
of the input array_ptr, and then displays all this information. */
void
get_characteristics(const mxArray *array_ptr)
{
	const char    *class_name;
	const mwSize  *dims;
	char          *shape_string;
	char          *temp_string;
	mwSize        c;
	mwSize        number_of_dimensions;
	size_t        length_of_shape_string;

	/* Display the mxArray's Dimensions; for example, 5x7x3.
	If the mxArray's Dimensions are too long to fit, then just
	display the number of dimensions; for example, 12-D. */
	number_of_dimensions = mxGetNumberOfDimensions(array_ptr);
	dims = mxGetDimensions(array_ptr);

	/* alloc memory for shape_string w.r.t thrice the number of dimensions */
	/* (so that we can also add the 'x')                                   */
	shape_string = (char *)mxCalloc(number_of_dimensions * 3, sizeof(char));
	shape_string[0] = '\0';
	temp_string = (char *)mxCalloc(64, sizeof(char));

	for (c = 0; c<number_of_dimensions; c++) {
		sprintf(temp_string, "%"FMT_SIZE_T"dx", dims[c]);
		strcat(shape_string, temp_string);
	}

	length_of_shape_string = strlen(shape_string);
	/* replace the last 'x' with a space */
	shape_string[length_of_shape_string - 1] = '\0';
	if (length_of_shape_string > 16) {
		sprintf(shape_string, "%"FMT_SIZE_T"u-D", number_of_dimensions);
	}
	std::cout << "Dimensions: " << shape_string << std::endl;

	/* Display the mxArray's class (category). */
	class_name = mxGetClassName(array_ptr);
	std::cout << "Class Name: " << class_name << std::endl;


	/* Display a bottom banner. */
	std::cout << "------------------------------------------------\n";

	/* free up memory for shape_string */
	mxFree(shape_string);
}

/* Display the subscript associated with the given index. */
void
display_subscript(const mxArray *array_ptr, mwSize index)
{
	mwSize     inner, subindex, total, d, q, number_of_dimensions;
	mwSize       *subscript;
	const mwSize *dims;

	number_of_dimensions = mxGetNumberOfDimensions(array_ptr);
	subscript = (mwSize *) mxCalloc(number_of_dimensions, sizeof(mwSize));
	dims = mxGetDimensions(array_ptr);

	std::cout << "(";
	subindex = index;
	for (d = number_of_dimensions - 1;; d--) { /* loop termination is at the end */

		for (total = 1, inner = 0; inner<d; inner++)
			total *= dims[inner];

		subscript[d] = subindex / total;
		subindex = subindex % total;
		if (d == 0) {
			break;
		}
	}

	for (q = 0; q<number_of_dimensions - 1; q++) {
		std::cout << subscript[q] << ",";
	}
	std::cout << subscript[number_of_dimensions - 1] << " )  ";

	mxFree(subscript);
}

static void
analyze_double(const mxArray *array_ptr)
{
	double *pr, *pi;
	mwSize total_num_of_elements, index;

	pr = mxGetPr(array_ptr);
	pi = mxGetPi(array_ptr);
	total_num_of_elements = mxGetNumberOfElements(array_ptr);

	for (index = 0; index<total_num_of_elements; index++)  {
		std::cout << "\t";
		display_subscript(array_ptr, index);
		std::cout << " = " << *pr++ << std::endl;
	}
}


/* Pass analyze_string a pointer to a char mxArray.  Each element
in a char mxArray holds one 2-byte character (an mxChar);
analyze_string displays the contents of the input char mxArray
one row at a time.  Since adjoining row elements are NOT stored in
successive indices, analyze_string has to do a bit of math to
figure out where the next letter in a string is stored. */
static void
analyze_string(const mxArray *string_array_ptr)
{
	char *buf;
	mwSize number_of_dimensions, buflen;
	const mwSize *dims;
	mwSize d, page, total_number_of_pages, elements_per_page;

	/* Allocate enough memory to hold the converted string. */
	buflen = mxGetNumberOfElements(string_array_ptr) + 1;
	buf = (char *) mxCalloc(buflen, sizeof(char));

	/* Copy the string data from string_array_ptr and place it into buf. */
	if (mxGetString(string_array_ptr, buf, buflen) != 0)
		std::cout << "MATLAB:explore:invalidStringArray",
		"Could not convert string data\n.";
	else std::cout << "what is inside so far: " << buf << std::endl; 
}

#endif

class ShimmerSaveToFile
//save the Shimmer data 
{
protected:
	std::string baseFilename;
	std::string folder;
	std::string filename;
	int dataIndex;
    int fileMarker;
public: 
	ShimmerSaveToFile(std::string f, std::string port)
	{
		baseFilename = f;
		filename = f.append(".");
		filename.append(port); 
		filename.append(".csv");
		dataIndex = 0;
        fileMarker = 0;
	};
	
	virtual void save(std::vector<ShimmerData *> data)
	{
		std::ofstream myfile(filename, std::ios::app);
		for (int i = 0; i < data.size(); i++)
		{
			myfile << dataIndex << ",";
			for (int j = 0; j < SHIMMER_NUM_ARGS; j++)
			{
				myfile << data[i]->getData(j);
				if (j < SHIMMER_NUM_ARGS - 1) myfile << ",";
			}
			myfile << std::endl;
			dataIndex++;
		}

		myfile.close();
	};
    
    void markerInFile()
	{
		std::ofstream myfile(filename, std::ios::app);
        myfile << dataIndex << -1 << ", FILE MARKER, " <<fileMarker << std::endl;
		myfile.close();
        fileMarker++; 
	};
};

class ShimmerLoadFromFile
{
protected:
	std::vector<ShimmerData *> sData; //data loaded from file
	ci::Timer timer; 
	int index; 
	std::string port;
    float modifier_num;
    std::string fname;
    float looping; // -1 means not looping yo, looping holds end of loop, then returns to start (whether modified or not)
    

	virtual void loadFromFile(std::string filename)
	{
        
		std::fstream fstream;
		std::string data;
		const int BUF_NUM = 1024 * 4;
		char str[BUF_NUM];
		int lastComma, fileIndex;
        
        fname = filename;
		fstream.open(filename, std::ios::in);
		if (fstream.is_open()) {
            
			while (!fstream.eof())// && lastComma!=-1
			{
				fstream.getline(str, BUF_NUM);
				data.assign(str);
                
				//do again if only these characters
				if (data.compare("\n") == 0 && data.compare("\r") == 0)
				{
					fstream.getline(str, BUF_NUM);
					data.assign(str);
				}
                
				lastComma = data.find_first_of(",");
				if (lastComma == -1)
				{
					std::cout << "Incorrect Format\n";
					return; //get out
				}
				fileIndex = lastComma;
                
				int curFrame = atoi(data.substr(0, lastComma).c_str());
				if (curFrame != -1) //ignore markers
				{
					ShimmerData *shimmer = new ShimmerData;
					for (int i = 0; i < SHIMMER_NUM_ARGS+1; i++) //plus one for cam.
					{
						fileIndex = data.find(",", lastComma + 1);
						if (fileIndex != -1)
							shimmer->setData(i, atof(data.substr(lastComma + 1, fileIndex - 1 - lastComma).c_str()));
						else shimmer->setData(i, atof(data.substr(lastComma + 1, data.length() - lastComma - 1).c_str()));
						lastComma = fileIndex;
					}
					sData.push_back(shimmer);
				}
			}
			std::cout << "Loaded " <<  sData.size() << "frames of data\n";
		}
		else std::cout << "Was not able to open file: " << filename << std::endl;
        
		fstream.close();
        
	};

public:
	//TODO: read as we go? instead of keeping in memory???
    ShimmerLoadFromFile()
    {};
    
	ShimmerLoadFromFile(std::string filename)
	{
		index = -1;
        modifier_num = 0;
        looping = -1;

		//get the port number
		int findex, gindex = 0; 
		findex = filename.find_first_of(".");
		gindex = filename.find(".", findex + 1);
		port = filename.substr( findex + 1, gindex - findex - 1 );

		//load from file now
		loadFromFile(filename);
	};
	~ShimmerLoadFromFile()
	{
		for (int i = 0; i < sData.size(); i++)
		{
			delete sData[i];
			sData[i] = NULL; 
		}
		sData.clear(); 
	};
	virtual std::vector<ShimmerData *> getData()
	{
		assert(index > -1 && index < sData.size());

		std::vector<ShimmerData *> shimmers;
        
        if( timer.getSeconds()+modifier_num > looping && looping > -1)
        {
            reset();
        }
        
        while (index < sData.size() && sData[index]->getCameraTimeStamp() <= ((timer.getSeconds()+modifier_num) ) )
        {
            //float t = timer.getSeconds() * 1000;
            shimmers.push_back(sData[index]);
            index++;
        }
		return shimmers;
	};

	inline std::string getPort(){ return port; };

	void start()
	{ 
		index = 0; 
		timer.start();
	};
    
    void reset()
    {
        timer.stop();
        index = 0;
        timer = ci::Timer();
        
        //TODO: this part is a hack that should be fixed
        sData.clear();
        loadFromFile(fname);
        
        timer.start();
    }

    void setLooping(float loop)
    {
        looping = loop;
    };
	void stop()
	{
		index = -1;
		timer.stop();
	};

	inline bool started(){ return (index != -1) && !done(); };
	inline bool done(){ return index >= sData.size(); };
    inline void setTimeModifierNum(float m){ modifier_num = m; };

};

class ShimmerWrapper
{
private:
    
#ifdef _WINDOWS_
	Engine *ep; //MATLAB engine driving this code
    mxArray *shimmerData;
	mxArray *shimmerNames;
    ci::Vec2i getSubscript(const mxArray *array_ptr, mwSize index);

#endif
    
	int _ID; 
	std::string port;
	std::string varName;
	std::string deviceID; 
	std::vector<ShimmerData *> curData; 
	bool init; 
	bool started;
	bool useLoadFile; 
	char buffer[MAX_MATLAB_OUTPUT_LEN + 1];
	ShimmerSaveToFile *saveFile; 
	ShimmerLoadFromFile *loadFile; 
	VisualizeShimmer *visualizer; 

	void handleData(); 
	void printNames();
	void eraseOldData();
	std::vector<ci::osc::Message> getOSCScalarData(int whichData);
	void scaleData(); 

public:
#ifdef _WINDOWS_
	ShimmerWrapper(Engine *e, int id, std::string port_num = "3", VisualizeShimmer *v = NULL);
    void connect();
	void logOutput();
	void stop();
	void disconnect();
#endif
    
    void start();
    ShimmerWrapper(std::string filename, int id1, VisualizeShimmer *v = NULL); //load from file
	std::string getDeviceID();
	inline bool isStarted(){ return started; };
	inline std::vector<ShimmerData *> getCurData(){ return curData; };
	std::vector<ci::osc::Message> getOSC();
	std::vector<ci::osc::Message> getOSCAccelToSCBusX();
	std::vector<ci::osc::Message> getOSCAccelToSCBusY();
	std::vector<ci::osc::Message> getOSCAccelToSCBusZ();
	bool samePort(std::string p){ return port.compare( p ) == 0; };
	bool samePort(ShimmerWrapper *s){ return port.compare( s->getPort() ) == 0; };
	inline std::string getPort(){ return port; };
	double getAccelMax(); 
	double getAccelMin();
    void run();
	void saveToFile(std::string fname) 
	{ 
		if (saveFile != NULL) delete saveFile; 
		saveFile = new ShimmerSaveToFile(fname, port); 
	};
    void markerInFile()
    {
        if (saveFile != NULL)
        {
            saveFile->markerInFile();
        }
    };

	void loadFromFile(std::string fname)
	{
		if (loadFile != NULL) delete loadFile;
		loadFile = new ShimmerLoadFromFile(fname);
		useLoadFile = true;
		init = true;
		port = loadFile->getPort();
	};

	bool hasData(){ return curData.size() != 0;  };
	inline void setVisualizer(VisualizeShimmer *v){ visualizer = v; };
    inline void setTimeModifierNum(float m)
    {
        if (useLoadFile and loadFile!=NULL)
        {
            loadFile->setTimeModifierNum(m);
        }
    };
    
    void resetPlaybackTimer()
    {
        if (useLoadFile and loadFile!=NULL)
        {
            loadFile->reset();
        }
    };
    
    void loop(float l)
    {
        if (useLoadFile and loadFile!=NULL)
        {
            loadFile->setLooping(l);
        }
    };
};

#ifdef _WINDOWS_
ShimmerWrapper::ShimmerWrapper(Engine *e, int id, std::string port_num, VisualizeShimmer *v)
{
	ep = e; 
	_ID = id;
	port = port_num;

	std::stringstream ss; ss << "shimmer" << id;
	varName = ss.str(); 
	init = false; 
	started = false; 
	deviceID = "";
	saveFile = NULL;
	useLoadFile = false; 
	loadFile = NULL;
	setVisualizer(v);
	
	//set up buffer
	buffer[MAX_MATLAB_OUTPUT_LEN] = '\0';
	int res = engOutputBuffer(ep, buffer, MAX_MATLAB_OUTPUT_LEN);
	logOutput();
}

ci::Vec2i ShimmerWrapper::getSubscript(const mxArray *array_ptr, mwSize index)
{
	mwSize     inner, subindex, total, d, q, number_of_dimensions;
	mwSize       *subscript;
	const mwSize *dims;
    
	number_of_dimensions = mxGetNumberOfDimensions(array_ptr);
	subscript = (mwSize *)mxCalloc(number_of_dimensions, sizeof(mwSize));
	dims = mxGetDimensions(array_ptr);
	ci::Vec2i result;
    
	subindex = index;
	for (d = number_of_dimensions - 1;; d--) { /* loop termination is at the end */
        
		for (total = 1, inner = 0; inner<d; inner++)
			total *= dims[inner];
        
		subscript[d] = subindex / total;
		subindex = subindex % total;
		if (d == 0) {
			break;
		}
	}
    
	for (q = 0; q<number_of_dimensions - 1; q++) {
		result.x = subscript[q];
	}
	result.y = subscript[number_of_dimensions - 1];
    
	mxFree(subscript);
    
	return result;
}

void ShimmerWrapper::connect()
{												//TODO: pass as param!!!
	std::stringstream ss;
	ss << varName << " = ShimmerHandleClass('" << port << "');";
	engEvalString(ep, ss.str().c_str());              //Define shimmer as a ShimmerHandle Class instance with comPort1;
	std::cout << ss.str() << std::endl;
	logOutput();
	
	std::stringstream s2; s2 << "started = connectShimmer(" << varName << ");";
	std::cout << s2.str() << std::endl;
	engEvalString(ep, s2.str().c_str());
	logOutput();
}

void ShimmerWrapper::printNames()
{
	mwSize s = mxGetNumberOfElements(shimmerNames);
	//std::cout << "string array dims=" << s << std::endl;
	analyze_string(shimmerNames);
}
#endif

ShimmerWrapper::ShimmerWrapper(std::string filename, int id1, VisualizeShimmer *v)
{
	init = true;
	deviceID = "";
	saveFile = NULL;
	loadFile = NULL;
	setVisualizer(v);
	loadFromFile(filename);
    _ID = id1;
}

double ShimmerWrapper::getAccelMax(){ return G_FORCE * 2.5; }
double ShimmerWrapper::getAccelMin(){ return G_FORCE * -2.5; }

void ShimmerWrapper::eraseOldData()
{
    if( !useLoadFile )
    {
        for (int i = 0; i < curData.size(); i++)
        {
            if (curData[i] != NULL)
                delete curData[i];
            curData[i] = NULL;
        }
    }
	curData.clear();
}

std::vector<ci::osc::Message> ShimmerWrapper::getOSC()
{
	std::vector<ci::osc::Message> msgVector;
	const int MAX_MSG = 100; 

	int  i = 0; 
	int  howManyTimes = curData.size() / MAX_MSG; 
	int  leftover = curData.size() % MAX_MSG;

	for (int k = 0; k < howManyTimes; k++)
	{
		ci::osc::Message msg;
		msg.setAddress(SHIMMER_DATA_OSC_ADDR);
		msg.addStringArg(port);
		msg.addIntArg(MAX_MSG);
		for (i = k*MAX_MSG; i < MAX_MSG*(k+1); i++)
		for (int j = 0; j < SHIMMER_NUM_ARGS; j++)
		{
			msg.addFloatArg(curData[i]->getData(j));
		}
		msgVector.push_back(msg);
	}

	if( leftover > 0)
	{
		ci::osc::Message msg;
		msg.setAddress(SHIMMER_DATA_OSC_ADDR);
		msg.addStringArg(port);
		msg.addIntArg(leftover);
		for (i = howManyTimes*MAX_MSG; i < curData.size(); i++)
		for (int j = 0; j < SHIMMER_NUM_ARGS; j++)
		{
			msg.addFloatArg(curData[i]->getData(j));
		}
		msgVector.push_back(msg);
	}

//	std::cout << "Number of OSC messages which will be sent: " << msgVector.size() << std::endl;

	return msgVector;
}

std::vector<ci::osc::Message> ShimmerWrapper::getOSCScalarData(int whichData)
{
	//data needs to be bundled since it is overflowing the buffers, as is
	std::vector<ci::osc::Message> msgVector;
	const int MAX_MSG = 1024;

	int  i = 0;
	int  howManyTimes = curData.size() / MAX_MSG;
	int  leftover = curData.size() % MAX_MSG;

	for (int k = 0; k < howManyTimes; k++)
	{
		for (i = k*MAX_MSG; i < MAX_MSG*(k + 1); i++)
		{
			ci::osc::Message msg;
			msg.setAddress(BUS_OSC_ADDR);
			msg.addIntArg(whichData - 1 + _ID*(SHIMMER_NUM_ARGS-1)); // which input bus on supercollider
			msg.addFloatArg(curData[i]->getData(whichData));
			msgVector.push_back(msg);
//            std::cout << "which bus #" << whichData - 1 + _ID*(SHIMMER_NUM_ARGS-1) << std::endl;
		}
	}

	if (leftover > 0)
	{
		for (i = howManyTimes*MAX_MSG; i < curData.size(); i++)
		{
			ci::osc::Message msg;
			msg.setAddress(BUS_OSC_ADDR);
			msg.addIntArg(whichData - 1 + _ID*(SHIMMER_NUM_ARGS-1));
			msg.addFloatArg(curData[i]->getData(whichData)); // which input bus on supercollider
			msgVector.push_back(msg);
            std::cout << "which bus #" << whichData - 1 + _ID*(SHIMMER_NUM_ARGS-1) << std::endl;
		}
	}
    
    
	return msgVector;
}

std::vector<ci::osc::Message> ShimmerWrapper::getOSCAccelToSCBusX()
{
	return getOSCScalarData(1);
}

std::vector<ci::osc::Message> ShimmerWrapper::getOSCAccelToSCBusY()
{
	return getOSCScalarData(2);
}

std::vector<ci::osc::Message> ShimmerWrapper::getOSCAccelToSCBusZ()
{
	return getOSCScalarData(3);
}

void ShimmerWrapper::scaleData()
{
	for (int i = 0; i < curData.size(); i++)
	{
		curData[i]->scaleAccel(); 
	}
}

#ifdef _WINDOWS_

void ShimmerWrapper::handleData()
{

	eraseOldData(); //clear the current data -- REMEMBER I AM DOING THIS
    
    mwSize s = mxGetNumberOfElements(shimmerData);
	if (s > 0){
		//get_characteristics(shimmerData);
		//analyze_double(shimmerData);

		double *pr, *pi;
		mwSize total_num_of_elements, index, numOfDataPackets, indexNum;

		pr = mxGetPr(shimmerData);
		numOfDataPackets = mxGetM(shimmerData);
		indexNum = mxGetN(shimmerData);

		total_num_of_elements = mxGetNumberOfElements(shimmerData);

		for (index = 0; index<total_num_of_elements; index++)  {
			ci::Vec2i indexVector = getSubscript(shimmerData, index);
			int i = indexVector.x;
			int j = indexVector.y;

			if (j == 0) 
			{
				curData.push_back(new ShimmerData); 
			}
			curData[i]->setData(j, *pr++);
			//display_subscript(shimmerData, index);
			//std::cout << "  " << i<<","<<j <<" = " << *pr << std::endl;
		}

		if (saveFile != NULL) saveFile->save(curData);
	}
}
#endif


void ShimmerWrapper::run()
{
	// Read the latest data from shimmer data buffer, signalFormatArray defines the format of the data and signalUnitArray the unit
	if (!useLoadFile)
	{
#ifdef _WINDOWS_
		std::stringstream ss;
		ss << "[newData, signalNameArray, signalFormatArray, signalUnitArray] = " << varName << ".getdata('Time Stamp', 'c', 'Accelerometer', 'c', 'Gyroscope', 'c', 'Magnetometer', 'u', 'Battery Voltage', 'a');";
		engEvalString(ep, ss.str().c_str());
		//		logOutput();

		shimmerData = engGetVariable(ep, "newData");
		if (shimmerData == NULL)
		{
			if (init){
				std::cout << "ERROR! No data received from " << varName << ":  " << deviceID << std::endl;
			}
		}
		else if (!init && mxGetNumberOfElements(shimmerData) > 0)
		{
			engEvalString(ep, "signalNamesString = char(signalNameArray(1,1)); ");
			engEvalString(ep, "for i = 2:length(signalNameArray)\ntabbedNextSignalName = [char(9), char(signalNameArray(1, i))];\n signalNamesString = strcat(signalNamesString, tabbedNextSignalName); end");

			shimmerNames = engGetVariable(ep, "signalNamesString");
			printNames();
			init = true;
		}
		else
		{
			handleData();
		}
#endif
	}
	else
	{
		assert(loadFile != NULL);

		eraseOldData();
		if (loadFile->started() && !loadFile->done())
		{
 			std::vector<ShimmerData *> shimmers = loadFile->getData();
			for (int i = 0; i < shimmers.size(); i++)
			{
				curData.push_back(shimmers[i]);
				//std::cout << shimmers[i]->str();
			}
		}
	}
	scaleData(); //change to 0 - 1 values
	if (visualizer != NULL) 
		visualizer->update(curData);
}

#ifdef _WINDOWS_
void ShimmerWrapper::logOutput()
{
	std::string buf(buffer);
	if ( buf.length() > 0 )
		std::cout << "MATLAB output: " << buffer << std::endl; 
}

void ShimmerWrapper::stop()
{
	std::stringstream ss;
	ss << varName << ".stop;\n";
	logOutput();
	started = false; 
}

void ShimmerWrapper::disconnect()
{
	std::stringstream ss;
	ss << varName << ".disconnect;";
	logOutput();
}
#endif

void ShimmerWrapper::start()
{
#ifdef _WINDOWS_
	if (!useLoadFile)
	{
		std::stringstream s2; s2 << "res = " << varName << ".start;";
		engEvalString(ep, s2.str().c_str());
		logOutput();
	}
	else
	{
#endif
        
		loadFile->start();
#ifdef _WINDOWS_
	}
#endif
	started = true;
}

