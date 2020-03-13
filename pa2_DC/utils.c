#include "utils.h"

void take_max_time_and_inc(Process *self, timestamp_t time) {
    if (self->lamport_time < time) {
        self->lamport_time = time;
    }
    self->lamport_time++;
}

timestamp_t get_lamport_time(){
    return myself.lamport_time;
}
