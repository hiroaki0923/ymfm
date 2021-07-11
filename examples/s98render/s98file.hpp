#ifndef s98file_hpp
#define s98file_hpp

#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <list>
#include <string>
#include <vector>
#include <iconv.h>
#include <filesystem>

using namespace std;

typedef struct {
    uint32_t type;
    uint32_t clock;
    uint32_t pan;
    char reserved[4];
} DeviceInfo;

typedef struct {
    char magic[3];char format;
    uint32_t timer1;
    uint32_t timer2;
    uint32_t compress;
    uint32_t nameptr;
    uint32_t dataptr;
    uint32_t loopptr;
    uint32_t devicecount;
    DeviceInfo deviceInfo[];
} S98Header;

class S98File {
private:
    void extractHeader();
public:
    S98File();
    ~S98File();

    bool setFilePath(const char* filepath);

    S98Header* header;
    uint8_t* data;
    uint32_t filesize;
    string songName;
    string gameName;
    string artistName;


    enum DeviceType {
        TYPE_NONE   = 0,
        TYPE_PSG    = 1,
        TYPE_OPN    = 2,
        TYPE_OPN2   = 3,
        TYPE_OPNA   = 4,
        TYPE_OPM    = 5,
        TYPE_OPLL   = 6,
        TYPE_OPL    = 7,
        TYPE_OPL2   = 8,
        TYPE_OPL3   = 9,
        TYPE_DCSG   = 16
    };
};


#endif /* s98file_hpp */
