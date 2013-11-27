#ifndef ILDA_PARSER_H
#define ILDA_PARSER_H

#include <stdint.h>

#define SUCCESS             0
#define ERR_FAILED_OPEN     1
#define ERR_FAILED_READ     2
#define ERR_BAD_FORMAT      3

#define FORMAT_3D           0
#define FORMAT_2D           1

// Loads an ILDA file
int loadILDAFile(char* filename);

// Reads the header from the ILDA file
int readHeader();

// Getters for frame header info
char isDataFrame();
char* getName();
char* getCompanyName();
uint16_t getEntries();
uint16_t getFrameNum();
uint16_t getTotalFrames();

// Provides access to the point / laser data fields
uint16_t* getPointData();
uint8_t* getLaserData();

// Loads data into the point & laser data locations
int loadNextPoint();

#endif // ILDA_PARSER_H

