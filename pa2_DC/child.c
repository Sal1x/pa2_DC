#include "child.h"
#include "log.h"
#include "pa2345.h"
#include "stdbool.h"
#include "banking.h"

void run_child_routine(Process* self) {
    Message msg;
    initialize_balance_history(self);

    // log_started(self);
    send_started_to_all(self);
    receive_from_all_children(self, &msg);
    // log_received_all_started();

    run_bank_routine(self);
    send_history(self);

    // printf("----------Recieved all done and stopped \n");

    // log_received_all_done();
    //send history
    //ADD LOGS Check if works add send history
}

int initialize_balance_history(Process* self) {
    self->history.s_id = self->id;
    self->history.s_history[0].s_balance = initial_balances[self->id];
    self->history.s_history[0].s_time = 0;
    self->history.s_history[0].s_balance_pending_in = 0;
    self->history.s_history_len = 1;
    printf("Process %d balance history initialized with %d\n\n", self->id, initial_balances[self->id]);
    return 0;
}

int run_bank_routine(Process* self) {
    Message msg;
    bool stop_requested = false;
    int num_other_processes_running = num_children-1;
    while(!stop_requested) {
        receive_any(self, &msg); // There is a cycle until message received
        printf("Process %d received a message\n", self->id);
        switch (msg.s_header.s_type)
        {
        case TRANSFER:
            printf("Its a transfer!\n");
            handle_transfer(self, &msg); // TODO
            break;
        case DONE:
            printf("Its a DONE!\n");
            num_other_processes_running--;
            break;
        case STOP:
            printf("Its a STOP!\n");
            stop_requested = true;
        default:
            break;
        }
    }
    // STOP message received
    send_done_to_all(self);
    while(num_other_processes_running > 0) {
        receive_any(self, &msg);
        printf("Process %d received a message in second cycle\n", self->id);
        switch (msg.s_header.s_type)
        {
        case  TRANSFER:
            printf("Its a transfer!\n");
            handle_transfer(self, &msg); // TODO but only incoming transfers
            break;
        case DONE:
            printf("Its a DONE!\n");
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
    timestamp_t current_time = get_physical_time();
    balance_t balance_change;

    if (transfer->s_src == self->id){
        printf("Process %d handles outcoming transfer to %d at time %d\n", self->id, transfer->s_dst, current_time);
        balance_change = -(transfer->s_amount);
        log_transfer_out(transfer);
        send(self, transfer->s_dst, msg);
    }
    else {
        printf("Process %d handles incoming transfer from %d at time %d\n", self->id, transfer->s_src, current_time);
        balance_change = transfer->s_amount;

        Message ack = {
            .s_header =
                {
                    .s_magic = MESSAGE_MAGIC,
                    .s_type = ACK,
                    .s_local_time = current_time,
                    .s_payload_len = 0,
                },
        };
        log_transfer_in(transfer);
        // printf("--------Sending ack\n");
        send(&myself, PARENT_ID, &ack);
    }
    
    // Fill gaps in history
    fill_gaps(self, current_time);
    balance_t last_balance = self->history.s_history[self->history.s_history_len - 1].s_balance;
    printf(" -- Last Balance = %d\n", last_balance);

    // Set new balance
    balance_t new_balance = last_balance + balance_change;
    self->history.s_history[current_time].s_balance = new_balance;
    self->history.s_history[current_time].s_time = current_time;
    self->history.s_history[current_time].s_balance_pending_in = 0;
    self->history.s_history_len = (int) current_time + 1; //or +1???
    printf(" -- New balance = %d\n -- New history len = %d\n -- Time = %d\n\n", new_balance, self->history.s_history_len, current_time);
    return 0;
    }

void fill_gaps(Process* self, timestamp_t current_time){
    if (current_time > self->history.s_history_len) {
        printf("-----Filling gaps for process %d------\n", self->id);
        int last_time_in_history = self->history.s_history_len-1;
        balance_t last_balance = self->history.s_history[last_time_in_history].s_balance;
        printf("last time: %d, current time: %d\n", last_time_in_history, current_time);
        for (timestamp_t time = self->history.s_history_len; time < current_time; time++) {
            self->history.s_history[time].s_balance = last_balance;
            self->history.s_history[time].s_time = time;
            self->history.s_history[time].s_balance_pending_in = 0;
            printf("Changing history for time: %d to balance %d\n", time, last_balance);
        } 
    }
}
