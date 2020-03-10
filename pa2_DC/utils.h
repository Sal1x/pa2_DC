#ifndef _UTILS_
#define _UTILS_

#include "banking.h"

enum {
    MAX_PROCESSES = 10,
};

size_t num_children;
size_t num_processes;
balance_t initial_balances[MAX_PROCESSES];

#endif

