#include "child.h"
#include "log.h"
#include "pa2345.h"
#include "stdbool.h"
#include "banking.h"
#include "priority_queue.h"

void run_child_routine(Process* self) {
    Message msg;
    initialize_balance_history(self);

    log_started(self);
    self->lamport_time++;
    send_started_to_all(self);
    receive_from_all_children(self, &msg);
    log_received_all_started(self);

    int num_other_processes_running = request_cs(self);
    //print
    release_cs(self);

    log_received_all_done(self);
    self->lamport_time++;
    send_history(self);
}

int request_cs(const void * self) {
    Process* process = (Process*) self;
    Message msg;

    process->lamport_time++;
    pq_push(self, process->id, process->lamport_time);

    process->lamport_time++;
    send_request_to_all(process);

    return wait_for_replies(process);
}

int release_cs(const void* self) {
    Process* process = (Process*) self;

    pq_pop(self);
    process->lamport_time++;
    send_cs_release_to_all(self);

    return 0;
}

int wait_for_replies(Process* self) {
       Message msg;
    bool stop_requested = false;
    int num_replies_left = num_children - 1;
    int num_other_processes_running = num_children-1;
    while(num_replies_left > 0 && pq_peek(self).process_id != self->id) {
        self->lamport_time++;
        local_id from = receive_any(self, &msg); // There is a cycle until message received
        take_max_time_and_inc(self, msg.s_header.s_local_time);
        // printf("Process %d received a message\n", self->id);
        switch (msg.s_header.s_type)
        {
        case CS_REQUEST:
            pq_push(self, from, msg.s_header.s_local_time);
            self->lamport_time++;
            send_cs_reply(self, from);
            break;
        case CS_REPLY:
            num_replies_left--;
            break;
        case CS_RELEASE:
            pq_pop(self);
            num_replies_left--;
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
