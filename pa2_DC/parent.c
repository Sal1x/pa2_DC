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
    self->lamport_time++;
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

    timestamp_t max_time;
    all_history->s_history_len = num_children;
    for(size_t from = 1; from <= num_children ; ++from){
        self->lamport_time++;
        receive(self, from, &msg);
        take_max_time_and_inc(self, msg.s_header.s_local_time);

        if(msg.s_header.s_type != BALANCE_HISTORY)
            printf("ERROR: Wrong type in BALANCE_HISTORY");

        if (max_time < msg.s_header.s_local_time)
            max_time = msg.s_header.s_local_time;

        BalanceHistory* balanceHistory = (BalanceHistory*) msg.s_payload;
        all_history->s_history[from - 1] = *balanceHistory;
    }

    for(size_t child_id = 1; child_id <= num_children ; child_id++) {
        BalanceHistory* balance_history = (BalanceHistory*) &all_history->s_history[child_id-1];
        // printf("Process %d history len is %d\n", child_id, balance_history->s_history_len);
        if (max_time >= balance_history->s_history_len) {
            int last_time_in_history = balance_history->s_history_len-1;
            balance_t last_balance = balance_history->s_history[last_time_in_history].s_balance;
            // printf("last time: %d, current time: %d\n", last_time_in_history, current_time);
            for (timestamp_t time = balance_history->s_history_len; time <= max_time; time++) {
                balance_history->s_history[time].s_balance = last_balance;
                balance_history->s_history[time].s_time = time;
                balance_history->s_history[time].s_balance_pending_in = 0;
                // printf(">>>Process %d Changing history for time: %d to balance %d\n", child_id, time, last_balance);
            }
            balance_history->s_history_len = max_time+1;
        }
    }
}
