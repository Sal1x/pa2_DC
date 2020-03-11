#include "parent.h"
#include "log.h"

void run_parent_routine(Process* self){
    Message msg;

    //Start
    // log_started(self);
    receive_from_all_children(self, &msg);
    // log_received_all_started();

    //Bank Robbery
    transfer(self, 1, 2, 10);

    //bank robbery
    //send stop
    //receive done
    //recieve history
    //print history

}