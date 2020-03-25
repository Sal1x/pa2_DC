#ifndef _CHILD_
#define _CHILD_
#include "utils.h"

void run_child_routine(Process* self, bool mutex_enabled);
int run_bank_routine(Process* self);
int handle_transfer(Process* self, Message* order);
int initialize_balance_history(Process* self);
void fill_pending_in(Process* self, timestamp_t transfer_time, timestamp_t current_time, balance_t income);
int wait_for_replies(Process* self);
void wait_for_all_done(Process* self, int done_remaining);

#endif
