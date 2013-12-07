// ildaparser.c
// Jeff Heidel 2013
#include "ildaparser.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

//#define DEBUG 

#define ENDIANSWAP16(x) \
    do { \
        x = (((x << 8) & 0xFF00) | ((x >> 8) & 0x00FF)); \
    } \
    while(0)

struct openildafile {
    FILE *fp;

    // Values read from header
    char ilda_formattype;
    char ilda_name[9];
    char ilda_companyname[9];
    uint16_t ilda_numentries;
    uint16_t ilda_currentframe;
    uint16_t ilda_totalframes;
    char ilda_scannerhead;
};

char isDataFrame(openildafile_t f) {
    return f->ilda_formattype == FORMAT_3D ||
       f->ilda_formattype == FORMAT_2D; 
}

char* getName(openildafile_t f) { return f->ilda_name; }
char* getCompanyName(openildafile_t f) { return f->ilda_companyname; }
uint16_t getEntries(openildafile_t f) { return f->ilda_numentries; }
uint16_t getFormatType(openildafile_t f) { return f->ilda_formattype; }
uint16_t getFrameNum(openildafile_t f) { return f->ilda_currentframe; }
uint16_t getTotalFrames(openildafile_t f) { return f->ilda_totalframes; }

int readHeader(openildafile_t f) {
    char buffer[5];
    memset(buffer, 0, sizeof(buffer));
    memset(f->ilda_name, 0, sizeof(f->ilda_name));
    memset(f->ilda_companyname, 0, sizeof(f->ilda_companyname));

    // Read & verify magic number (bytes 1-4)
    if (!fread(buffer, 1, 4, f->fp)) return ERR_FAILED_READ;
    if (strcmp(buffer, "ILDA") != 0) return ERR_BAD_FORMAT;

    // Unused (bytes 5-7)
    fseek(f->fp, 3, SEEK_CUR); 
    
    // Format type (byte 8)
    if (!fread(&f->ilda_formattype, 1, 1, f->fp)) return ERR_FAILED_READ;

    // Name (bytes 9-16)
    if (!fread(f->ilda_name, 1, 8, f->fp)) return ERR_FAILED_READ;

    // Company Name (bytes 17-24)
    if (!fread(f->ilda_companyname, 1, 8, f->fp)) return ERR_FAILED_READ;

    // Number of entries in data section (bytes 25-26)
    if (!fread(&f->ilda_numentries, 2, 1, f->fp)) return ERR_FAILED_READ;
    ENDIANSWAP16(f->ilda_numentries);

    // Number of entries in data section (bytes 27-28)
    if (!fread(&f->ilda_currentframe, 2, 1, f->fp)) return ERR_FAILED_READ;
    ENDIANSWAP16(f->ilda_currentframe);

    // Number of entries in data section (bytes 29-30)
    if (!fread(&f->ilda_totalframes, 2, 1, f->fp)) return ERR_FAILED_READ;
    ENDIANSWAP16(f->ilda_totalframes);

    // Read scanner head (byte 31)
    if (!fread(&f->ilda_scannerhead, 1, 1, f->fp)) return ERR_FAILED_READ;

    // Unused (byte 32)
    fseek(f->fp, 1, SEEK_CUR); 

#ifdef DEBUG
    printf("--HEADER PARSE--\n"
            "ILDA format:\t%d\n" 
            "Name:\t\t%s\n"
            "Company:\t%s\n"
            "Entries:\t%d\n"
            "Current Frame:\t%d\n"
            "Total Frames:\t%d\n"
            "Scanner Head:\t%d\n"
            "--END HEADER--\n",
            f->ilda_formattype,
            f->ilda_name,
            f->ilda_companyname,
            f->ilda_numentries,
            f->ilda_currentframe,
            f->ilda_totalframes,
            f->ilda_scannerhead);
#endif
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

int loadNextPoint(openildafile_t f) {
    // Read x, y, z laser data
    if(!fread(pointData, sizeof(uint16_t),
        f->ilda_formattype == FORMAT_3D ? 3 : 2, f->fp)) return ERR_FAILED_READ;
    ENDIANSWAP16(pointData[0]);
    ENDIANSWAP16(pointData[1]);
    if (f->ilda_formattype == FORMAT_3D) {
        ENDIANSWAP16(pointData[2]);
    } else {
        pointData[2] = 0;
    }

    // Read blanking & lastpoint bits
    if (!fread(laserData, sizeof(uint8_t), 2, f->fp)) return ERR_FAILED_READ;
    laserData[2] = laserData[1]; // color data
    laserData[1] = (laserData[0] & 0x80) >> 7; //lastpoint
    laserData[0] = (laserData[0] & 0x40) >> 6; //blanking
    return SUCCESS;
}

openildafile_t last_opened_file;

openildafile_t getIldaFilePointer() {
    return last_opened_file;
}

int loadILDAFile(char* filename) {
    
    last_opened_file = (openildafile_t) malloc(sizeof(struct openildafile));

    // Load ILDA file
    last_opened_file->fp = fopen(filename, "r");
    if (last_opened_file->fp == NULL) {
        free(last_opened_file);
        return ERR_FAILED_OPEN;
    }

    return SUCCESS;
}

void closeFile(openildafile_t f) {
    fclose(f->fp);
    free(f);
}


