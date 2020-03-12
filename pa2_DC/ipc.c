#define _XOPEN_SOURCE 600 /* needed for timespec in <time.h> */
#include "ipc.h"
#include "utils.h"
#include <stdbool.h>
#include "log.h"
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include "pa2345.h"

#include <stdio.h>
#include <time.h>
#include <unistd.h>

int send(void * self, local_id dst, const Message * msg) {
    Process* current_process = self;
    TransferOrder* transfer = msg->s_payload;
    printf("Sending transfer from %d to %d amount %d\n", transfer->s_src, transfer->s_dst, transfer->s_amount);
    int bytes_written;
    bytes_written = write(writer[current_process->id][dst], &msg->s_header, sizeof(MessageHeader));
    printf("Bytes written first time is : %d\n", bytes_written);
    bytes_written = write(writer[current_process->id][dst], &msg->s_payload, msg->s_header.s_payload_len);
    printf("Bytes written second time is : %d\n", bytes_written);
}

int receive(void * self, local_id from, Message * msg) {
    Process* current_process = self;
    int bytes_read;
    while(true){
        bytes_read = read(reader[from][current_process->id], &msg->s_header, sizeof(MessageHeader));
        if (bytes_read == -1) {
            continue;
        }
        printf("Bytes read first time is : %d\n", bytes_read);
        do
        {
            bytes_read  = read(reader[from][current_process->id], &msg->s_payload, msg->s_header.s_payload_len);
            printf("Bytes read second time is : %d\n", bytes_read);
        } while (bytes_read == -1);
        
        TransferOrder* transfer = msg->s_payload;
        printf("Received transfer from %d to %d amount %d\n", transfer->s_src, transfer->s_dst, transfer->s_amount);
        return 0;
    }
}

// TEST WORKS
int receive_from_all_children(Process* self, Message* msg){
    for (int i = 1; i <= num_children; i++) {
        if (i != self->id){
            receive(self, i, msg);
            printf("%d Received start from to %d\n", self->id, i);
        }
    }
    return 0;
}

/**
 * Sends message to ALL processes
 */ 
int send_multicast(void * self, const Message * msg) {
    //This is done to cast void pointer to Process. Because we cannot modify ipc.h
    Process* current_process = self;
    for (int i = 0; i < num_processes; i++) {
        if (i != current_process->id){
            printf("Send start from %d to %d\n", current_process->id, i);
            send(current_process, i, msg);
        }
    }
    return 0;
}

// TEST WORKS
int send_started_to_all(Process* self) {
    Message msg = {
        .s_header =
            {
                .s_magic = MESSAGE_MAGIC,
                .s_type = STARTED,
            },
    };
    sprintf(msg.s_payload, log_started_fmt, get_physical_time(), self->id, getpid(), getppid);
    send_multicast(self, &msg);
}

int send_done_to_all(Process* self) {
    Message msg = {
        .s_header =
            {
                .s_magic = MESSAGE_MAGIC,
                .s_type = STARTED,
            },
    };
    sprintf(msg.s_payload, log_done_fmt, get_physical_time(), self->id, self->history.s_history->s_balance);
    send_multicast(self, &msg);
}

int receive_any(void * self, Message * msg) {
    Process* current_process = self;
    while(true) {
        int bytes_read;
        for (int from = 0; from < num_processes; from++) {
            if (from == current_process->id)
                continue;
            bytes_read = read(reader[from][current_process->id], &msg->s_header, sizeof(MessageHeader));
            if (bytes_read == -1) {
                continue;
            }
            read(reader[from][current_process->id], &msg->s_payload, msg->s_header.s_payload_len);
            return 0;
        }
    }
}