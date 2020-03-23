#include "banking.h"
#include "utils.h"
#include "log.h"
#include "parent.h"
#include "child.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

void transfer(void * parent_data, local_id src, local_id dst,
              balance_t amount) {
    Process *self = parent_data;
    self->lamport_time++;

    Message msg = {
        .s_header =
            {
                .s_magic = MESSAGE_MAGIC,
                .s_payload_len = sizeof(TransferOrder),
                .s_type = TRANSFER,
                .s_local_time = self->lamport_time,
            },
    };
    TransferOrder transferOrder = {
        .s_src = src,
        .s_dst = dst,
        .s_amount = amount,
    };
    memcpy(&msg.s_payload, &transferOrder, sizeof(TransferOrder));
    send(self, src, &msg);
    Message receivedMsg;
    // wait ACK from 
    self->lamport_time++;
    receive(self, dst, &receivedMsg);
    if (receivedMsg.s_header.s_type != ACK)
        fprintf(stderr, "ERROR: Wrong type of Message. Process %d received type %d from %d\n", self->id, receivedMsg.s_header.s_type, dst);

    take_max_time_and_inc(self, receivedMsg.s_header.s_local_time);
    // printf("-------Ack received\n");
}

int main(int argc, char * argv[])
{
    Process* self = &myself;
    // READ COMMANDLINE
    if (argc >= 3 && strcmp(argv[1], "-p") == 0) {
        num_children = strtol(argv[2], NULL, 10);
        num_processes = num_children + 1;

        if (num_children >= MAX_PROCESSES) {
            fprintf(stderr, "ERROR: Too many children requested.\n");
            return 1;
        }

        if (argc != 3 + num_children) {
            fprintf(stderr, "ERROR: Expected %ld balances after `%s %s'\n",
                    num_children, argv[1], argv[2]);
            return 1;
        }

        for (size_t i = 1; i <= num_children; i++) {
            initial_balances[i] = strtol(argv[2 + i], NULL, 10);
        }
    } else {
        fprintf(stderr, "ERROR: Key '-p NUMBER_OF_CHILDREN' is mandatory\n");
        return 1;
    }
    //------------ Create file descriptors. ------------
    for (size_t source = 0; source < num_processes; source++) {
        for (size_t destination = 0; destination < num_processes;
             destination++) {
            if (source != destination) {
                int fildes[2];
                pipe(fildes);
                for (int i = 0; i < 2; ++i) {
                    unsigned int flags = fcntl(fildes[i], F_GETFL, 0);
                    fcntl(fildes[i], F_SETFL, flags | O_NONBLOCK);
                }

                reader[source][destination] = fildes[0];
                writer[source][destination] = fildes[1];
            }
        }
    }
 // Добавить в открытие дискррипторов часть с ассинхрорнщиной 
 // Добавить один сенд и один рессив чтобы 
    log_init();

    process_pids[PARENT_ID] = getpid();

    // Create children processes.
    for (size_t id = 1; id <= num_children; id++) {
        int child_pid = fork();
        if (child_pid > 0) {
            // We are inside the parent process.
            self->id = PARENT_ID;
            process_pids[id] = child_pid;
        } else if (child_pid == 0) {
            // We are inside the child process.
            self->id = id;
            break;
        } else {
            // Forking failed.
            fprintf(stderr, "ERROR: Forking failed");
            perror("main");
            return 1;
        }
    }

    close_pipes_that_dont_belong_to_us(self);

    self->lamport_time = 0;
    if (self->id == PARENT_ID){
        run_parent_routine(self);
    } else
    {
        run_child_routine(self);
    }
    

    for (size_t i = 1; i <= num_processes; i++) {
        waitpid(process_pids[i], NULL, 0);
    }

    //bank_robbery(parent_data);
    //print_history(all);

    return 0;
}

void close_pipes_that_dont_belong_to_us(Process *self) {
    for (size_t source = 0; source < num_processes; source++) {
        for (size_t destination = 0; destination < num_processes; destination++) {
            if (source != self->id && destination != self->id &&
                source != destination) {
                close(writer[source][destination]);
                close(reader[source][destination]);
            }
            if (source == self->id && destination != self->id) {
                close(reader[source][destination]);
            }
            if (destination == self->id && source != self->id) {
                close(writer[source][destination]);
            }
        }
    }
}

