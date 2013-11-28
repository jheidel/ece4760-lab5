// ildaparser.c
// Jeff Heidel 2013
#include "ildaparser.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ENDIANSWAP16(x) \
    do { \
        x = (((x << 8) & 0xFF00) | ((x >> 8) & 0x00FF)); \
    } \
    while(0)

FILE *fp;

// Values read from header
char ilda_formattype;
char ilda_name[9];
char ilda_companyname[9];
uint16_t ilda_numentries;
uint16_t ilda_currentframe;
uint16_t ilda_totalframes;
char ilda_scannerhead;

char isDataFrame() {
    return ilda_formattype == FORMAT_3D ||
       ilda_formattype == FORMAT_2D; 
}

char* getName() { return ilda_name; }
char* getCompanyName() { return ilda_companyname; }
uint16_t getEntries() { return ilda_numentries; }
uint16_t getFrameNum() { return ilda_currentframe; }
uint16_t getTotalFrames() { return ilda_totalframes; }

int readHeader() {
    char buffer[5];
    memset(buffer, 0, sizeof(buffer));
    memset(ilda_name, 0, sizeof(ilda_name));
    memset(ilda_companyname, 0, sizeof(ilda_companyname));

    // Read & verify magic number (bytes 1-4)
    if (!fread(buffer, 1, 4, fp)) return ERR_FAILED_READ;
    if (strcmp(buffer, "ILDA") != 0) return ERR_BAD_FORMAT;

    // Unused (bytes 5-7)
    fseek(fp, 3, SEEK_CUR); 
    
    // Format type (byte 8)
    if (!fread(&ilda_formattype, 1, 1, fp)) return ERR_FAILED_READ;

    // Name (bytes 9-16)
    if (!fread(ilda_name, 1, 8, fp)) return ERR_FAILED_READ;

    // Company Name (bytes 17-24)
    if (!fread(ilda_companyname, 1, 8, fp)) return ERR_FAILED_READ;

    // Number of entries in data section (bytes 25-26)
    if (!fread(&ilda_numentries, 2, 1, fp)) return ERR_FAILED_READ;
    ENDIANSWAP16(ilda_numentries);

    // Number of entries in data section (bytes 27-28)
    if (!fread(&ilda_currentframe, 2, 1, fp)) return ERR_FAILED_READ;
    ENDIANSWAP16(ilda_currentframe);

    // Number of entries in data section (bytes 29-30)
    if (!fread(&ilda_totalframes, 2, 1, fp)) return ERR_FAILED_READ;
    ENDIANSWAP16(ilda_totalframes);

    // Read scanner head (byte 31)
    if (!fread(&ilda_scannerhead, 1, 1, fp)) return ERR_FAILED_READ;

    // Unused (byte 32)
    fseek(fp, 1, SEEK_CUR); 

    printf("--HEADER PARSE--\n"
            "ILDA format:\t%d\n" 
            "Name:\t\t%s\n"
            "Company:\t%s\n"
            "Entries:\t%d\n"
            "Current Frame:\t%d\n"
            "Total Frames:\t%d\n"
            "Scanner Head:\t%d\n"
            "--END HEADER--\n",
            ilda_formattype,
            ilda_name,
            ilda_companyname,
            ilda_numentries,
            ilda_currentframe,
            ilda_totalframes,
            ilda_scannerhead);
    return SUCCESS;
}

// Format: x, y, z
int16_t pointData[3];

// Format blank (boolean), lastpoint (boolean), color
uint8_t laserData[3];

int16_t* getPointData() {
    return pointData;
}

uint8_t* getLaserData() {
    return laserData;
}

int loadNextPoint() {
    // Read x, y, z laser data
    if(!fread(pointData, sizeof(uint16_t),
        ilda_formattype == FORMAT_3D ? 3 : 2, fp)) return ERR_FAILED_READ;
    ENDIANSWAP16(pointData[0]);
    ENDIANSWAP16(pointData[1]);
    if (ilda_formattype == FORMAT_3D) {
        ENDIANSWAP16(pointData[2]);
    } else {
        pointData[2] = 0;
    }

    // Read blanking & lastpoint bits
    if (!fread(laserData, sizeof(uint8_t), 2, fp)) return ERR_FAILED_READ;
    laserData[2] = laserData[1]; // color data
    laserData[1] = (laserData[0] & 0x80) >> 7; //lastpoint
    laserData[0] = (laserData[0] & 0x40) >> 6; //blanking
    return SUCCESS;
}


int loadILDAFile(char* filename) {
    // Load ILDA file
    fp = fopen(filename, "r");
    if (fp == NULL) return ERR_FAILED_OPEN;

    return SUCCESS;
}


