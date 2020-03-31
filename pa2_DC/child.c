#include "child.h"
#include "log.h"
#include "pa2345.h"
#include "stdbool.h"
#include "banking.h"
#include <stdio.h>
#include <string.h>

void run_child_routine(Process* self, bool mutex_enabled) {
    Message msg;

    log_started(self);
    self->lamport_time++;
    send_started_to_all(self);
    receive_from_all_children(self, &msg, STARTED);
    log_received_all_started(self);

    initialize_forks(self);
    // printf("Fork test for process %d\n", self->id);
    // printf("[");
    // for (int i = 1; i <= num_children; i++) {
    //     printf("%d, ", self->fork[i]);
    // }
    // printf("]\n");

    int num_other_processes_running = num_children-1;
    if (mutex_enabled){
        num_other_processes_running = request_cs(self);
    }

    int num_prints = self->id * 5;

    char str[128];

     for (int i = 1; i <= num_prints; ++i) {
            memset(str, 0, sizeof(str));
            sprintf(str, log_loop_operation_fmt, self->id, i, num_prints);
            // printf(log_loop_operation_fmt, self->id, i, num_prints);
            print(str);
     }

     if (mutex_enabled)
        release_cs(self);

    self->lamport_time++;
    send_done_to_all(self);
    wait_for_all_done(self, num_other_processes_running);
    log_received_all_done(self);
}

void wait_for_all_done(Process* self, int done_remaining) {
        while (done_remaining > 0) {
            Message msg;
            self->lamport_time++;
            local_id from = receive_any(self, &msg);
            take_max_time_and_inc(self, msg.s_header.s_local_time);
            switch (msg.s_header.s_type) {
                case DONE:
                    done_remaining--;
                    break;
                case CS_REQUEST:
                    self->lamport_time++;
                    send_cs_reply(self, from);
            break;
                default:
                    //we don't care about other messages anymore
                    break;
            }
    }
}

int request_cs(const void * self) {
    Process* process = (Process*) self;
    int num_other_processes_running = num_children-1;
    bool left_fork_missing;
    bool right_fork_missing;
    if (process->fork[get_left_fork_index(process)] == 0){
        left_fork_missing = true;
        process->lamport_time++;
        send_cs_request(process, get_left_fork_index(process));
    }
    if (process->fork[get_right_fork_index(process)] == 0){
        right_fork_missing = true;
        process->lamport_time++;
        send_cs_request(process, get_right_fork_index(process));
    }

    if (have_all_forks(process) == 0)
        num_other_processes_running =  wait_for_forks(process);

    return num_other_processes_running;

}
int wait_for_forks(Process* self) {
    Message msg;
    int num_other_processes_running = num_children-1;
    // debug
    while(have_all_forks(self) == 0) {
        self->lamport_time++;
        local_id from = receive_any(self, &msg); // There is a cycle until message received
        take_max_time_and_inc(self, msg.s_header.s_local_time);
        // printf("Process %d received a message\n", self->id);
        switch (msg.s_header.s_type)
        {
        case CS_REQUEST:
            if (self->fork[from] == 1 && self->dirty[from] == 1){
                self->lamport_time++;
                send_cs_reply(self, from);

                self->fork[from] = 0;
                self->dirty[from] = 0;

                self->lamport_time++;
                send_cs_request(self, from);
            }
            self->waiting_for_fork[from] = 1;
            break;
        case CS_REPLY:
            self->fork[from] = 1;
            self->dirty[from] = 0;
            break;
        case DONE:
            // printf("Its a DONE!\n");
            num_other_processes_running--;
            break;
        default:
            break;
        }
    }
    return num_other_processes_running; 

}
// int request_cs(const void * self) {
//     Process* process = (Process*) self;

//     process->lamport_time++;
//     timestamp_t request_time = process->lamport_time;
//     send_request_to_all(process);

//     return wait_for_replies(process, request_time);
// }

int release_cs(const void* self) {
    Process* process = (Process*) self;
    //This is not needed actually
    process->dirty[get_left_fork_index(process)] = 1;
    process->dirty[get_right_fork_index(process)] = 1;

    if (process->waiting_for_fork[get_left_fork_index(process)]){
        process->lamport_time++;
        send_cs_reply(process, get_left_fork_index(process));
    }


    if (process->waiting_for_fork[get_right_fork_index(process)]){
        process->lamport_time++;
        send_cs_reply(process, get_right_fork_index(process));
    }
    return 0;
}
// int release_cs(const void* self) {
//     Process* process = (Process*) self;
//     while (process->local_queue.size > 0) {
//         PriorityQueueElement process_waiting = pq_pop(process);
//         process->lamport_time++;
//         send_cs_reply(process, process_waiting.process_id);
//     }

//     return 0;
// }

// int wait_for_replies(Process* self, timestamp_t request_time) {
//     Message msg;
//     int num_replies_left = num_children - 1;
//     int num_other_processes_running = num_children-1;
//     while(num_replies_left > 0) {
//         self->lamport_time++;
//         local_id from = receive_any(self, &msg); // There is a cycle until message received
//         take_max_time_and_inc(self, msg.s_header.s_local_time);
//         // printf("Process %d received a message\n", self->id);
//         switch (msg.s_header.s_type)
//         {
//         case CS_REQUEST:
//             if (msg.s_header.s_local_time < request_time || (msg.s_header.s_local_time == request_time && from < self->id)){
//                 self->lamport_time++;
//                 send_cs_reply(self, from);
//             } else {
//                 pq_push(self, from, msg.s_header.s_local_time);
//                 self->lamport_time++;
//             }
//             break;
//         case CS_REPLY:
//             num_replies_left--;
//             break;
//         case DONE:
//             // printf("Its a DONE!\n");
//             num_other_processes_running--;
//             break;
//         default:
//             break;
//         }
//     }
//     return num_other_processes_running; 
// }

int release_cs(const void * self);

int initialize_balance_history(Process* self) {
    self->history.s_id = self->id;
    self->history.s_history[0].s_balance = initial_balances[self->id];
    self->history.s_history[0].s_time = 0;
    self->history.s_history[0].s_balance_pending_in = 0;
    self->history.s_history_len = 1;
    // printf("Process %d balance history initialized with %d\n\n", self->id, initial_balances[self->id]);
    return 0;
}

void initialize_forks(Process* self) {
    if (self->id == 1){
        self->fork[2] = 1;
        self->fork[num_children] = 1;
        self->dirty[2] = 1;
        self->dirty[num_children] = 1;
    } else if (self->id == num_children) {
        self->fork[1] = 0;
        self->fork[num_children-1] = 0;
        self->dirty[1] = 0;
        self->dirty[num_children-1] = 0;
    } else {
        self->fork[self->id-1] = 0;
        self->fork[self->id+1] = 1;
        self->dirty[self->id-1] = 0;
        self->dirty[self->id+1] = 1;
    }
}

int run_bank_routine(Process* self) {
    Message msg;
    bool stop_requested = false;
    int num_other_processes_running = num_children-1;
    while(!stop_requested) {
        self->lamport_time++;
        receive_any(self, &msg); // There is a cycle until message received
        take_max_time_and_inc(self, msg.s_header.s_local_time);
        // printf("Process %d received a message\n", self->id);
        switch (msg.s_header.s_type)
        {
        case TRANSFER:
            // printf("Its a transfer!\n");
            handle_transfer(self, &msg); // TODO
            break;
        case DONE:
            // printf("Its a DONE!\n");
            num_other_processes_running--;
            break;
        case STOP:
            // printf("Its a STOP!\n");
            stop_requested = true;
        default:
            break;
        }
    }
    // STOP message received
    self->lamport_time++;
    log_done(self);
    send_done_to_all(self);
    while(num_other_processes_running > 0) {
        self->lamport_time++;
        receive_any(self, &msg);
        take_max_time_and_inc(self, msg.s_header.s_local_time);
        // printf("Process %d received a message in second cycle\n", self->id);
        switch (msg.s_header.s_type)
        {
        case  TRANSFER:
            // printf("Its a transfer!\n");
            handle_transfer(self, &msg); // TODO but only incoming transfers
            break;
        case DONE:
            // printf("Its a DONE!\n");
            num_other_processes_running--;
            break;
        default:
            break;
        }
    }
    return 0;
}

//TODO ADD LOGS!!!!!!!!!!!!!!!!!!!!!!!
int handle_transfer(Process* self, Message* msg) {
    TransferOrder *transfer = (TransferOrder* ) msg->s_payload;
    timestamp_t transfer_time = msg->s_header.s_local_time;
    balance_t balance_change;


    if (transfer->s_src == self->id){
        // printf("Process %d handles outcoming transfer to %d at time %d\n", self->id, transfer->s_dst, current_time);
        balance_change = -(transfer->s_amount);
        self->lamport_time++;
        msg->s_header.s_local_time = self->lamport_time;
        log_transfer_out(transfer, self->lamport_time);
        send(self, transfer->s_dst, msg);
    }
    else {
        // printf("Process %d handles incoming transfer from %d at time %d\n", self->id, transfer->s_src, current_time);
        balance_change = transfer->s_amount;

        self->lamport_time++;
        Message ack = {
            .s_header =
                {
                    .s_magic = MESSAGE_MAGIC,
                    .s_type = ACK,
                    .s_local_time = self->lamport_time,
                    .s_payload_len = 0,
                },
        };
        log_transfer_in(transfer, self->lamport_time);
        // printf("--------Sending ack\n");
        send(&myself, PARENT_ID, &ack);
    }
    
    // Fill gaps in history
    fill_gaps(self, self->lamport_time);
    if (balance_change > 0)
        fill_pending_in(self, transfer_time, self->lamport_time, balance_change);

    balance_t last_balance = self->history.s_history[self->history.s_history_len - 1].s_balance;

    // Set new balance
    balance_t new_balance = last_balance + balance_change;
    self->history.s_history[self->lamport_time].s_balance = new_balance;
    self->history.s_history[self->lamport_time].s_time = self->lamport_time;
    self->history.s_history[self->lamport_time].s_balance_pending_in = 0;
    self->history.s_history_len = (int) self->lamport_time + 1; //or +1???
    // printf(" -- New balance = %d\n -- New history len = %d\n -- Time = %d\n\n", new_balance, self->history.s_history_len, current_time);
    return 0;
    }

void fill_gaps(Process* self, timestamp_t current_time){
    if (current_time > self->history.s_history_len) {
        // printf("-----Filling gaps for process %d------\n", self->id);
        int last_time_in_history = self->history.s_history_len-1;
        balance_t last_balance = self->history.s_history[last_time_in_history].s_balance;
        // printf("last time: %d, current time: %d\n", last_time_in_history, current_time);
        for (timestamp_t time = self->history.s_history_len; time < current_time; time++) {
            self->history.s_history[time].s_balance = last_balance;
            self->history.s_history[time].s_time = time;
            self->history.s_history[time].s_balance_pending_in = 0;
            // printf("Changing history for time: %d to balance %d\n", time, last_balance);
        } 
    }
}

void fill_pending_in(Process* self, timestamp_t transfer_time, timestamp_t current_time, balance_t income) {
    // printf("--- Feeling pending current time %d, history len %d \n", current_time, self->history.s_history_len);
    if (current_time > transfer_time) {
        for (timestamp_t time = transfer_time; time < current_time; time++) {
            self->history.s_history[time].s_balance_pending_in = income;
        }
    }

}
