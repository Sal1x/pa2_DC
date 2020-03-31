#ifndef _UTILS_
#define _UTILS_

#include "banking.h"
#include "ipc.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

enum {
    MAX_PROCESSES = 10,
};

size_t num_children;
size_t num_processes;
balance_t initial_balances[MAX_PROCESSES];

size_t reader[MAX_PROCESSES][MAX_PROCESSES];
size_t writer[MAX_PROCESSES][MAX_PROCESSES];

pid_t process_pids[MAX_PROCESSES];
balance_t initial_balances[MAX_PROCESSES];

typedef struct {
    local_id process_id;
    timestamp_t time;
} PriorityQueueElement;

typedef struct {
    PriorityQueueElement elements[MAX_PROCESSES];
    size_t size;
} PriorityQueue;

typedef struct {
    BalanceHistory history;
    local_id id;
    timestamp_t lamport_time;
    int fork[MAX_PROCESSES+1];
    int dirty[MAX_PROCESSES+1];
    int waiting_for_fork[MAX_PROCESSES+1];
} Process;


// delete if not needed
Process myself;



int receive_from_all_children(Process* self, Message* msg, MessageType type);
int send_started_to_all(Process* self);
int send_done_to_all(Process* self);
int send_stop_to_all(Process* self);
int send_history(Process* self);
void send_request_to_all(Process* self);
void send_cs_reply(Process* self, local_id to);
void send_cs_request(Process* self, local_id to);
void send_cs_release_to_all(Process* self);
void fill_gaps(Process* self, timestamp_t current_time);
void close_pipes_that_dont_belong_to_us(Process *self);
void take_max_time_and_inc(Process *self, timestamp_t time);
int get_right_fork_index(Process* self);
int get_left_fork_index(Process* self);
int have_all_forks(Process* self);

#endif

