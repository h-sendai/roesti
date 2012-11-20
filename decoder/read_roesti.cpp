#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "roesti_decoder.h"

int read_fp(FILE *fp, unsigned char *buf, int read_len)
{
    int n = fread(buf, 1, read_len, fp);
    if (n == 0) {
        if (feof(fp)) {
            ;
        }
        else {
            warnx("fread in read_header");
        }
    }
    else if (n != read_len) {
        warnx("short read");
        n = 0;
    }

    return n;
}

int read_header(FILE *fp, unsigned char *buf, int read_len)
{
    int n;
    n = read_fp(fp, buf, read_len);
    return n;
}

int read_data_packet_and_trailer(FILE *fp, unsigned char *buf, int read_len)
{
    int n;
    n = read_fp(fp, buf, read_len);
    return n;
}

int usage()
{
    fprintf(stderr, "Usage: read_roesti data_file\n");
    
    return 0;
}

int main(int argc, char *argv[])
{
    struct roesti_header header;
    struct roesti_data_packet_header  data_packet_header;
    struct roesti_data_packet_trailer data_packet_trailer;
    unsigned char buf[128*1024];
    memset(buf, 'X', 128*1024);

    FILE *fp;
    if (argc != 2) {
        usage();
        exit(1);
    }

    if ( (fp = fopen(argv[1], "r")) == NULL) {
        err(1, "fopen");
    }

    int n;
    for ( ; ; ) {
        n = read_header(fp, buf, HEADER_LEN);
        if (n == 0) {
            break;
        }
        decode_roesti_header(&header, buf);
        fprintf(stderr, "event number: %d\n", header.event_num);

        if (! is_valid_header(&header)) {
            errx(1, "invalid header");
        }
     
        n = read_data_packet_and_trailer(fp, &buf[HEADER_LEN], header.data_packet_len + TRAILER_LEN);
        if (n == 0) {
            break;
        }
        decode_roesti_data_packet_header(&data_packet_header, buf);
        if (! is_valid_data_packet_header(&data_packet_header)) {
             errx(1, "invalid data packet header");
        }

        decode_roesti_data_packet_trailer(&data_packet_trailer, buf, &data_packet_header);
        if (! is_valid_data_packet_footer(buf, &data_packet_header)) {
            errx(1, "invalid data_packet_footer");
        }
        
        fprintf(stderr, "n_data: %d\n", data_packet_header.data_len / ONE_EVENT_DATA_LEN);
        for (int i = 0; i < data_packet_header.data_len / ONE_EVENT_DATA_LEN; i++) {
            short data = get_data_at(buf, &data_packet_header, i);
            printf("%d\n", data);
        }

    }
    return 0;
}
