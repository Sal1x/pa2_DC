#include "banking.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>

void transfer(void * parent_data, local_id src, local_id dst,
              balance_t amount)
{
    // student, please implement me
}

int main(int argc, char * argv[])
{
    if (argc >= 3 && strcmp(argv[1], "-p") == 0) {
        num_children = strtol(argv[2], NULL, 10);
        num_processes = num_children + 1;

        if (num_children >= MAX_PROCESSES) {
            fprintf(stderr, "ERROR: Too many children requested.\n");
            return 1;
        }

        if (argc != 3 + num_children) {
            fprintf(stderr, "ERROR: Expected %ld balances after `%s %s'\n",
                    num_children, argv[1], argv[2]);
            return 1;
        }

        for (size_t i = 1; i <= num_children; i++) {
            initial_balances[i] = strtol(argv[2 + i], NULL, 10);
        }
    } else {
        fprintf(stderr, "ERROR: Key '-p NUMBER_OF_CHILDREN' is mandatory\n");
        return 1;
    }
    //bank_robbery(parent_data);
    //print_history(all);

    return 0;
}
