#ifndef _ROESTI_DECODER
#define _ROESTI_DECODER 1

#include <arpa/inet.h>
#include <err.h>
#include <string.h>

/*

 buf[]
save header, data_packet and trailer in one buffer.
+-----------+---------------+-------------+-------------+----------+
|           |               |             |             |          |
+-----------+---------------+-------------+-------------+----------+
<-----------><--------------><------------><------------><--------->
   HEADER      DATA_PACKET       DATA       DATA_PACKET    TRAILER
                  HEADER                     TRAILER
             <------------------------------------------>
                             DATA_PACKET

user program (read twice)
<-----------><----------------------------------------------------->
  1st read                          2nd read

*/

// LENGTH definition
const static int HEADER_LEN               = 4*4;
const static int DATA_PACKET_HEADER_LEN   = 2*4;
const static int DATA_PACKET_TRAILER_LEN  = 2 + 1 + 1;
const static int DATA_PACKET_FOOTER_LEN   = 2;
const static int DATA_PACKET_STATUS_LEN   = 1;
const static int DATA_PACKET_CHECKSUM_LEN = 1;
const static int TRAILER_LEN              = DATA_PACKET_FOOTER_LEN + DATA_PACKET_STATUS_LEN + DATA_PACKET_CHECKSUM_LEN;
const static int ONE_EVENT_DATA_LEN       = 2;

// valid numbers in the header
const static int VALID_MAGIC_WORD        = 0x89abcdef;
const static int VALID_TYPE_UPPER_24_BIT = 0xff0000;
const static int VALID_TRAILER           = 0xfedcba98;

// valid numbers in the data packet header
const static short VALID_DATA_PACKET_HEADER_MAGIC = 0x1234;
const static short VALID_STOP_ID                  = 0x3f;
const static short VALID_DATA_PACKET_FOOTER       = 0x5678;
const static short VALID_DATA_PACKET_STATUS       = 0xcc;
const static short VALID_DATA_PACKET_CHECKSUM     = 0x00;

struct roesti_header {
    int magic_word;
    int type_upper_24_bit;
    int board_num_in_header;
    int data_packet_len;
    int event_num;
    // data packet
};

struct roesti_data_packet_header {
    short data_packet_header_magic;
    char board_num;
    char channel_num;
    char stop_id;
    short stop_num;
    short data_len;
    // data
    //short data_packet_footer_magic;
    //char status;
    //char checksum;
};

struct roesti_data_packet_trailer {
    short footer;
    char  status;
    char  checksum;
};

int decode_roesti_header(struct roesti_header *header, unsigned char *buf)
{
    int x;

    // magic word
    memcpy(&x, &buf[0], sizeof(int));
    header->magic_word = ntohl(x);

    // type upper 24 bit
    header->type_upper_24_bit = (buf[4] << 16) + (buf[5] << 8) + buf[6];

    // board number in the header
    header->board_num_in_header = buf[7];

    // data packet len
    memcpy(&x, &buf[8], sizeof(int));
    header->data_packet_len = ntohl(x);

    // event number
    memcpy(&x, &buf[12], sizeof(int));
    header->event_num = ntohl(x);

    // data packet after this

    return 0;
}

bool is_valid_header(struct roesti_header *header)
{
    if (header->magic_word != VALID_MAGIC_WORD) {
        warnx("invalid magic_word: %08x (should be %08x)", header->magic_word,
            VALID_MAGIC_WORD);
        return false;
    }

    if (header->type_upper_24_bit != VALID_TYPE_UPPER_24_BIT) {
        warnx("invalid type upper 24 bit: %06x (should be %06x)",
            header->type_upper_24_bit, VALID_TYPE_UPPER_24_BIT);
        return false;
    }
    
    return true;
}

int decode_roesti_data_packet_header(struct roesti_data_packet_header *header, unsigned char *buf)
{
    short x;

    // data packet header
    memcpy(&x, &buf[HEADER_LEN + 0], sizeof(short));
    header->data_packet_header_magic = ntohs(x);

    // board number
    header->board_num = buf[HEADER_LEN + 2];

    // channel number
    header->channel_num = buf[HEADER_LEN + 3];

    // stop id 
    header->stop_id = buf[HEADER_LEN + 4] >> 2;

    // stop number
    header->stop_num = ((buf[HEADER_LEN + 4] & 0x3) << 8) + buf[HEADER_LEN + 5];

    // data length
    memcpy(&x, &buf[HEADER_LEN + 6], sizeof(short));
    header->data_len = ntohs(x);

    return 0;
}

bool is_valid_data_packet_header(struct roesti_data_packet_header *data_packet_header)
{
    if (data_packet_header->data_packet_header_magic != VALID_DATA_PACKET_HEADER_MAGIC) {
        warnx("invalid data packet header magic: %04x (should be %04x)",
            data_packet_header->data_packet_header_magic,
            VALID_DATA_PACKET_HEADER_MAGIC);
        return false;
    }
    if (data_packet_header->stop_id != VALID_STOP_ID) {
        warnx("invalid stop_id %02x (should be %02x)",
            data_packet_header->stop_id,
            VALID_STOP_ID);
        return false;
    }

    return true;
}

int decode_roesti_data_packet_trailer(struct roesti_data_packet_trailer *data_packet_trailer, unsigned char *buf, struct roesti_data_packet_header *data_packet_header)
{
    short x;
    short footer;

    int footer_pos = HEADER_LEN + DATA_PACKET_HEADER_LEN + data_packet_header->data_len;
    memcpy(&x, &buf[footer_pos], DATA_PACKET_FOOTER_LEN);
    footer = htons(x);
    data_packet_trailer->footer   = footer;
    data_packet_trailer->status   = buf[footer_pos + DATA_PACKET_FOOTER_LEN];
    data_packet_trailer->checksum = buf[footer_pos + DATA_PACKET_FOOTER_LEN + DATA_PACKET_CHECKSUM_LEN];

    return 0;
}

bool is_valid_data_packet_trailer(struct roesti_data_packet_trailer *data_packet_trailer)
{
    if (data_packet_trailer->footer != VALID_DATA_PACKET_FOOTER) {
        warnx("invalid data packet footer: %02x (should be %02x)",
            data_packet_trailer->footer, VALID_DATA_PACKET_FOOTER);
        return false;
    }
    
    if (data_packet_trailer->status != VALID_DATA_PACKET_STATUS) {
        warnx("invalid data packet status: %02x (should be %02x)",
            data_packet_trailer->status, VALID_DATA_PACKET_STATUS);
        return false;
    }

    if (data_packet_trailer->checksum != VALID_DATA_PACKET_CHECKSUM) {
        warnx("invalid data packet checksum: %02x (should be %02x)",
            data_packet_trailer->checksum, VALID_DATA_PACKET_CHECKSUM);
        return false;
    }

    return true;
}

bool is_valid_data_packet_footer(unsigned char *buf, struct roesti_data_packet_header *data_packet_header)
{
    short x;
    short footer;
    memcpy(&x, &buf[HEADER_LEN + DATA_PACKET_HEADER_LEN + data_packet_header->data_len], DATA_PACKET_FOOTER_LEN);
    footer = ntohs(x);
    if (footer != VALID_DATA_PACKET_FOOTER) {
        warnx("invalid data packet footer: %04x (should be %04x",
            footer,
            VALID_DATA_PACKET_FOOTER);
        return false;
    }

    return true;
}

int get_data_at(unsigned char *buf, roesti_data_packet_header *header, int n)
{
    short x;
    short data;
    memcpy(&x, &buf[HEADER_LEN + DATA_PACKET_HEADER_LEN + ONE_EVENT_DATA_LEN*n], ONE_EVENT_DATA_LEN);

    data = ntohs(x);
    
    return data;
}
#endif
