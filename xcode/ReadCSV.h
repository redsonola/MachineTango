//
//  ReadCSV.h
//  ReadCSVClass
//
//  Created by courtney on 8/15/14.
//
//

#ifndef ReadCSVClass_ReadCSV_h
#define ReadCSVClass_ReadCSV_h

#pragma once

#include <sstream>
#include <fstream>


/*
 //use case
 ReadCSV rCSV(filename);
 rCSV.getLine();

 while( !rCSV.eof() && rCSV.correctFormat() )
 {
    std::vector<char *> rCSV.getTokensLine();
    processToken()
 }
 
 rCSV.close();
 
 */

const int BUF_NUM = 1024 * 4;
class ReadCSV
{
public:
    ReadCSV();
    ReadCSV(std::string filename);
    void init(std::string filename);
    inline bool opened(){ return open; };
    inline bool eof(){ return fstr.eof(); };
    inline void close(){ fstr.close(); };
    std::vector<std::string> getTokensInLine();

    
private:
    std::fstream fstr;
    bool open;
};



#endif
