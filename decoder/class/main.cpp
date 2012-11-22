#include <stdlib.h>

#include "RoestiDataFile.h"

int main(int argc, char *argv[])
{
    //struct roesti_header header;

    std::string file = "sample.dat";
    if (argc == 2) {
        file = argv[1];
    }

    RoestiDataFile data(file);

    for ( ; ; ) {
        int n;
        n = data.read_header();
        if (n == 0) {
            break; // EOF
        }
        if (data.is_valid_header()) {
            printf("header OK\n");
        }
        else {
            exit(1);
        }

#if 0
        // specify data packet len 
        int len = data.get_total_data_packet_len();
        n = data.read_data_packet(len);
        n = data.read_trailer(len);
#endif

        // or
        n = data.read_data_packet();
        n = data.read_trailer();

        if (data.is_valid_trailer()) {
            printf("trailer OK\n");
        }
        else {
            exit(1);
        }
        
        char board_num = data.get_board_num();
        printf("board_num: %d\n", board_num);

        int event_num = data.get_event_num();
        printf("event_num: %d\n", event_num);
    
        // process data packet
        data.init_process_data();
        while (data.has_left_data()) {
#if 0
            if (data.is_valid_data_packet_header()) {
                printf("data header OK\n");
            else {
                exit(1);
            }
#endif
            int n_data = data.get_data_len();
            printf("n_data: %d\n", n_data);
            n_data = n_data / 2;
            for (int i = 0; i < n_data; i++) {
                short value = data.get_data_at(i);
                printf("data: %d\n", value);
            }
//            if (data.is_valid_data_packet_trailer) {
//                printf("data trailer OK\n");
//            }
//            else {
//                exit(1);
//            }
            data.seek_to_next_data();
        }
    }
    return 0;
}
