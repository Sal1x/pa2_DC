#include "utils.h"

void run_child_routine(Process* self);
int run_bank_routine(Process* self);
int handle_transfer(Process* self, Message* order);
int initialize_balance_history(Process* self);
