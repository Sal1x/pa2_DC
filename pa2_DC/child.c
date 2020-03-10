#include "child.h"
#include "log.h"
#include "pa2345.h"
#include "stdbool.h"

void run_child_routine(Process* self) {
    Message msg;
    //TODO initialize balances а нужно ли? хм
    initialize_balance_history(self);
    log_started(self);
    send_started_to_all(self);
    receive_from_all_children(self, &msg);
    log_received_all_started();

    run_bank_routine(self);

    receive_from_all_children(self, &msg);
    log_received_all_done();
    //send history
    //ADD LOGS Check if works add send history
}

int run_bank_routine(Process* self) {
    Message msg;
    bool stop_requested = false;
    int num_other_processes_running = num_children-1;
    while(!stop_requested) {
        receive_any(self, &msg); // There is a cycle until message received
        switch (msg.s_header.s_type)
        {
        case TRANSFER:
            handle_transfer(self, &msg); // TODO
            break;
        case DONE:
            num_other_processes_running--;
            break;
        case STOP:
            stop_requested = true;
        default:
            break;
        }
    }
    // STOP message received
    send_done_to_all(self);
    while(num_other_processes_running !=0) {
        receive_any(self, &msg);
        switch (msg.s_header.s_type)
        {
        case  TRANSFER:
            handle_transfer(self, &msg); // TODO but only incoming transfers
            break;
        case DONE:
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
    TransferOrder* transfer = msg->s_payload;
    timestamp_t current_time = get_physical_time();
    balance_t balance_change;

    if (transfer->s_src == self->id){
        balance_change = -(transfer->s_amount);
        send(self, transfer->s_dst, msg);
    }
    else {
        balance_change = transfer->s_amount;

        Message acknowledgement;
        acknowledgement.s_header = (MessageHeader) {
            .s_magic = MESSAGE_MAGIC,
            .s_type = ACK,
            .s_local_time = current_time,
            .s_payload_len = 0,
        };
        send(&myself, PARENT_ID, &acknowledgement);
    }
    // Fill gaps in history
    if (current_time > self->history.s_history_len) {
        int last_time_in_history = self->history.s_history_len-1;
        balance_t last_balance = self->history.s_history[last_time_in_history].s_balance;
        for (int time = self->history.s_history_len; time < current_time; time++) {
            self->history.s_history[time].s_balance = last_balance;
        }

        // Set new balance
        balance_t new_balance = last_balance + balance_change;
        self->history.s_history[current_time].s_balance = new_balance;
        self->history.s_history_len = current_time; //or +1???
    }
    return 0;
}

//Человек в другой лабе зачем то заполняет все время от момента трансфера до макс Т балансом, я не понимаю зачем это делать
// initialize_balance_history(Process* self) {
//     for (int i = 0; i < MAX_T;  i++)
//         self->history[i] = 
// }