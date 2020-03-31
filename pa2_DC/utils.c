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

int get_right_fork_index(Process* self) {
    if (self->id == num_children)
        return 1;
    return self->id+1;
}

int get_left_fork_index(Process* self) {
    if (self->id == 1)
        return num_children;
    return self->id-1;
}

int have_all_forks(Process* self) {
    if (self->fork[get_left_fork_index(self)] == 1 && self->fork[get_right_fork_index(self)] == 1) {
        return 1;
    }
    return 0;
}
