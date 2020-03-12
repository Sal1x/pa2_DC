#include "parent.h"
#include "log.h"

void run_parent_routine(Process* self){
    Message msg;

    //Start
    // log_started(self);
    receive_from_all_children(self, &msg);
    // log_received_all_started();

    //Bank Robbery
    // bank_robbery(self, num_children);
    transfer(self, 1, 2, 10);

    //Send stop
    send_stop_to_all(self);
    //receive done
    receive_from_all_children(self, &msg);
    //recieve history
    //print history
}