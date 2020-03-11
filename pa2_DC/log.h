#ifndef __LOG__
#define __LOG__

#include "stdio.h"
#include "stdarg.h"
#include "common.h"
#include "ipc.h"
#include "utils.h"

FILE *events_log_file;
FILE *pipe_log_file;

static const char * const log_pipe_opened_fmt = "Pipe from process %1d to %1d OPENED\n";

void log_init();

void logprintf(const char *format, ...);

void log_pipe_opened(int from, int to);

void log_started(Process* self);

void log_received_all_started();

void log_done();

void log_received_all_done();

void log_close();

void log_transfer_out(TransferOrder *transfer);

void log_transfer_in(TransferOrder *transfer);

#endif 
