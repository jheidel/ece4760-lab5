#ifndef ILDA_PARSER_H
#define ILDA_PARSER_H

#include <stdint.h>

#define SUCCESS             0
#define ERR_FAILED_OPEN     1
#define ERR_FAILED_READ     2
#define ERR_BAD_FORMAT      3

#define FORMAT_3D           0
#define FORMAT_2D           1

typedef struct openildafile *openildafile_t;

// Loads an ILDA file
int loadILDAFile(char* filename);

// Gets a pointer to the last opened ilda file struct object
openildafile_t getIldaFilePointer();

// Reads the header from the ILDA file
int readHeader(openildafile_t f);

// Getters for frame header info
char isDataFrame(openildafile_t f);
char* getName(openildafile_t f);
uint16_t getFormatType(openildafile_t f);
char* getCompanyName(openildafile_t f);
uint16_t getEntries(openildafile_t f);
uint16_t getFrameNum(openildafile_t f);
uint16_t getTotalFrames(openildafile_t f);

// Provides access to the point / laser data fields
int16_t* getPointData();
uint8_t* getLaserData();

// Loads data into the point & laser data locations
int loadNextPoint(openildafile_t f);

void closeFile(openildafile_t f);

#endif // ILDA_PARSER_H

