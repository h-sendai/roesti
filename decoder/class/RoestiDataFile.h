#ifndef _RoestiDataFile
#define _RoestiDataFile 1

#include "RoestiData.h"

#include <err.h>
#include <stdio.h>

class RoestiDataFile : public RoestiData
{
    public:
        RoestiDataFile();
        RoestiDataFile(std::string filename);
        virtual ~RoestiDataFile();
        int read_header();
        int read_data_packet();
        int read_data_packet(int data_packet_len);
        int read_trailer();
        int read_trailer(int data_packet_len);
    private:
        FILE *fp;
};

RoestiDataFile::RoestiDataFile(): fp(NULL)
{
}

RoestiDataFile::RoestiDataFile(std::string filename): fp(NULL)
{
    fp = fopen(filename.c_str(), "r");
    if (fp == NULL) {
        err(1, "fopen");
    }
}

RoestiDataFile::~RoestiDataFile()
{
    if (fp != NULL) {
        if (fclose(fp) != 0) {
            err(1, "fclose");
        }
    }
}

int RoestiDataFile::read_header()
{
    int n = fread(buf, 1, ROESTI_HEADER_LEN, fp);
    if (n != ROESTI_HEADER_LEN) {
        if (feof(fp)) {
            return 0;
        }
        else {
            err(1, "read error");
        }
    }
    return n;
}

int RoestiDataFile::read_data_packet()
{
    int len = get_data_packet_len();
    int n = read_data_packet(len);

    return n;
}

int RoestiDataFile::read_data_packet(int data_packet_len)
{
    int n = fread(&buf[ROESTI_HEADER_LEN], 1, data_packet_len, fp);
    if (n != data_packet_len) {
        if (feof(fp)) {
            return 0;
        }
        else {
            err(1, "read error for data packet");
        }
    }
    return n;
}

int RoestiDataFile::read_trailer()
{
    int len = get_data_packet_len();
    int n = read_trailer(len);

    return n;
}

int RoestiDataFile::read_trailer(int data_packet_len)
{
    int n = fread(&buf[ROESTI_HEADER_LEN + data_packet_len], 1, ROESTI_TRAILER_LEN, fp);
    if (n != ROESTI_TRAILER_LEN) {
        if (feof(fp)) {
            return 0;
        }
        else {
            err(1, "read error for trailer");
        }
    }
    return n;
}
#endif
