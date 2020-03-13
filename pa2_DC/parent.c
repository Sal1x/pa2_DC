#include "parent.h"
#include "log.h"

void run_parent_routine(Process* self){
    Message msg;

    //Start
    log_started(self);
    receive_from_all_children(self, &msg);
    log_received_all_started(self);

    //Bank Robbery
    bank_robbery(self, num_children);

    //Send stop
    send_stop_to_all(self);

    //receive done
    receive_from_all_children(self, &msg);
    log_received_all_done(self);

    AllHistory all_history;
    receive_all_history(self, &all_history);
    print_history(&all_history);
}

void receive_all_history(Process* self, AllHistory* all_history){
    Message msg;
    all_history->s_history_len = num_children;
    for(size_t from = 1; from <= num_children ; ++from){
        receive(self, from, &msg);
        if(msg.s_header.s_type != BALANCE_HISTORY)
            printf("ERROR: Wrong type in BALANCE_HISTORY");
        BalanceHistory* balanceHistory = (BalanceHistory*) msg.s_payload;
        all_history->s_history[from - 1] = *balanceHistory;
    }
}
