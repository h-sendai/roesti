#ifndef _ROESTIDATA
#define _ROESTIDATA 1

#include <arpa/inet.h>

#include <err.h>
#include <string.h>

#include <iostream>

struct roesti_header {
    unsigned int magic_word;
    unsigned int type_u_24;
    unsigned char board_num;
    unsigned int data_packet_len;
    unsigned int event_num;
};

struct roesti_data_packet_header {
    unsigned short magic;
    unsigned char  board_num;
    unsigned char  channel_num;
    unsigned char  stop_id;
    unsigned short stop_num;
    unsigned short data_len;
};

struct roesti_data_packet_trailer {
    unsigned short footer;
    unsigned char  status;
    unsigned char  checksum;
};

struct roesti_trailer {
    unsigned int trailer;
};

class RoestiData {

public:
    RoestiData();
    virtual ~RoestiData();
    //
    int decode_header(struct roesti_header *header);
    int decode_data_packet_header();
    int decode_trailer(struct roesti_trailer *trailer);
    //
    bool is_valid_header();
    bool is_valid_data_packet_header();
    bool is_valid_trailer();
    //
    int  get_magic_word();
    int  get_type_u_24();
    char get_board_num();
    int  get_event_num();
    int  get_data_packet_len();
    int  get_trailer();
    int  get_data_packet_header() { return 0; };
    int  get_channel_num() { return 0; };
    int  get_stop_id() { return 0; };
    //
    bool seek_to_next_data();
    int  get_data_len();
    int  get_data_packet_footer() { return 0; };
    int  get_status() { return 0; };
    int  get_checksum() { return 0; };
    // max 65536 bytes data + header size
    const static int BUFFER_SIZE = 128*1024; // 128kB
    unsigned char buf[BUFFER_SIZE];
    // length
    const static int ROESTI_MAGIC_WORD_LEN      = 4;
    const static int ROESTI_TYPE_LEN            = 4;
    const static int ROESTI_DATA_PACKET_LEN_LEN = 4;
    const static int ROESTI_EVENT_NUM_LEN       = 4;
    //
    const static int ROESTI_HEADER_LEN
        = ROESTI_MAGIC_WORD_LEN + ROESTI_TYPE_LEN + ROESTI_DATA_PACKET_LEN_LEN + ROESTI_EVENT_NUM_LEN;
    const static int ROESTI_DATA_PACKET_HEADER_LEN  = 2*4;
    const static int ROESTI_DATA_PACKET_TRAILER_LEN = 2*2;
    const static int ROESTI_TRAILER_LEN             = 4;
    // constant (magic etc.)
    const static int   ROESTI_VALID_MAGIC_WORD         = 0x89abcdef;
    const static int   ROESTI_VALID_TYPE_U_24          = 0xff0000;
    const static int   ROESTI_VALID_TRAILER            = 0xfedcba98;
    const static short ROESTI_VALID_DATA_PACKET_HEADER = 0x1234;
    const static char  ROESTI_VALID_STOP_ID            = 0x3f;
    const static short ROESTI_DATA_PACKET_FOOTER       = 0x5678;
    const static char  ROESTI_VALID_STATUS             = 0xcc;
    const static char  ROESTI_VALID_CHECKSUM           = 0x00;
    // data decode
    int init_process_data();
    bool has_left_data();
    int get_data_at(int n);
    
private:
    bool has_decoded_header;
    bool has_decoded_data_packet_header;
    struct roesti_header header;
    struct roesti_data_packet_header data_packet_header;
    int data_buf_len;
    int data_buf_pos;
};

RoestiData::RoestiData(): has_decoded_header(false), has_decoded_data_packet_header(false)
{
    ;
}

RoestiData::~RoestiData()
{
    ;
}

int RoestiData::get_magic_word()
{
    int x;
    memcpy(&x, &buf[0], sizeof(int));

    return ntohl(x);
}

int RoestiData::get_type_u_24()
{
    int x =   (buf[ROESTI_MAGIC_WORD_LEN    ] << 16)
            + (buf[ROESTI_MAGIC_WORD_LEN + 1] <<  8)
            + (buf[ROESTI_MAGIC_WORD_LEN + 2]      );

    return x;
}

char RoestiData::get_board_num()
{
    char x = buf[ROESTI_MAGIC_WORD_LEN + 3];
    
    return x;
}

int RoestiData::get_event_num()
{
    int x;
    memcpy(&x, &buf[ROESTI_MAGIC_WORD_LEN + ROESTI_TYPE_LEN + ROESTI_DATA_PACKET_LEN_LEN], sizeof(int));

    return ntohl(x);

}

int RoestiData::get_data_packet_len()
{
    //std::cerr << "### get_data_packet_len() ###" << std::endl;
    int x;
    memcpy(&x, &buf[ROESTI_MAGIC_WORD_LEN + ROESTI_TYPE_LEN], sizeof(int));

    return ntohl(x);
}

int RoestiData::get_trailer()
{
    int x;
    int data_packet_len = get_data_packet_len();
    memcpy(&x, &buf[ROESTI_HEADER_LEN + data_packet_len], sizeof(int));

    return ntohl(x);
}

int RoestiData::decode_header(struct roesti_header *header)
{
    header->magic_word      = get_magic_word();
    header->type_u_24       = get_type_u_24();
    header->board_num       = get_board_num();
    header->data_packet_len = get_data_packet_len();
    header->event_num       = get_event_num();

    return 0;
}

int RoestiData::decode_trailer(struct roesti_trailer *trailer)
{
    trailer->trailer = get_trailer();

    return 0;
}

int RoestiData::decode_data_packet_header()
{
    return 0;
}

bool RoestiData::is_valid_header()
{
    int magic_word = get_magic_word();
    if (magic_word != ROESTI_VALID_MAGIC_WORD) {
        warnx("does not have valid magic: %08x (should be %08x)",
            magic_word, ROESTI_VALID_MAGIC_WORD);
        return false;
    }

    int type_u_24 = get_type_u_24();
    if (type_u_24 != ROESTI_VALID_TYPE_U_24) {
        warnx("does not have valid type upper 24 bit: %06x (should be %06x)",
            type_u_24, ROESTI_VALID_TYPE_U_24);
        return false;
    }

    return true;
}

bool RoestiData::is_valid_trailer()
{
    int trailer = get_trailer();
    if (trailer != ROESTI_VALID_TRAILER) {
        warnx("does not have valid trailer: %08x (should be %08x)",
            trailer, ROESTI_VALID_TRAILER);
        return false;
    }

    return true;
}

int RoestiData::init_process_data()
{
    //std::cerr << "### init_process_data ###" << std::endl;
    data_buf_len = get_data_packet_len();
    data_buf_pos = 0;
    return 0;
}

bool RoestiData::has_left_data()
{
    if (data_buf_len == data_buf_pos) {
        return false;
    }
    else if (data_buf_len < data_buf_pos) {
        warnx("bug: data_buf_pos (%d (dec)) exceeds data_buf_len (%d (dec))",
            data_buf_pos, data_buf_len);
        // return false to loop
        return false;
    }
    else {
        return true;
    }
}

int RoestiData::get_data_len()
{
    int short x;
    memcpy(&x, &buf[ROESTI_HEADER_LEN + data_buf_pos + 6], sizeof(short));
    return ntohs(x);
}

bool RoestiData::seek_to_next_data()
{
    int data_len = get_data_len();
    data_buf_pos += (data_len
                     + ROESTI_DATA_PACKET_HEADER_LEN
                     + ROESTI_DATA_PACKET_TRAILER_LEN);
    return true;
}

int RoestiData::get_data_at(int n)
{
    short data;
    memcpy(&data, &buf[ROESTI_HEADER_LEN + data_buf_pos + 8 + n*2], sizeof(short));

    return ntohs(data);
}

bool RoestiData::is_valid_data_packet_header()
{
    return true;
}
#endif
