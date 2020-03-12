#include "stdio.h"
#include "stdarg.h"
#include "common.h"
#include "log.h"
#include "pa2345.h"
#include "utils.h"
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void log_init() {
    events_log_file = fopen(events_log, "w");
    pipe_log_file = fopen(pipes_log, "w");
}

void logprintf(const char *format, ...) {
    va_list va;

    va_start(va, format);
    vprintf(format, va);
    va_end(va);

    va_start(va, format);
    vfprintf(events_log_file, format, va);
    va_end(va);
}

void log_started(Process *self) {
    pid_t pid = getpid();
    pid_t parent_pid = getppid();

    logprintf(
        log_started_fmt, get_physical_time, self->id,
        pid, parent_pid,
        self->history.s_history[self->history.s_history_len - 1].s_balance);
}

void log_received_all_started(Process *self) {
    logprintf(log_received_all_started_fmt, get_physical_time(), self->id);
}

void log_done(Process *self) {
    logprintf(log_done_fmt, get_physical_time(), self->id);
}

void log_received_all_done(Process *self) {
    logprintf(log_received_all_done_fmt, get_physical_time(), self->id);
}

void log_close(Process *self) {
    fclose(events_log_file);
}

void log_transfer_out(TransferOrder *transfer) {
    logprintf(log_transfer_out_fmt, get_physical_time(), transfer->s_src, transfer->s_amount, transfer->s_dst);
}

void log_transfer_in(TransferOrder *transfer) {
    logprintf(log_transfer_in_fmt, get_physical_time(), transfer->s_dst, transfer->s_amount, transfer->s_src);
}
