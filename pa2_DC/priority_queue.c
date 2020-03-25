#include "utils.h"
#include <stddef.h>

int compare_elements(PriorityQueueElement first_element, PriorityQueueElement second_element);
int find_index_with_highest_priority(PriorityQueue* queue);

void pq_push(Process* self, local_id process_id, timestamp_t time) {
    PriorityQueueElement element = {
        .process_id = process_id,
        .time = time,
    };
    
    PriorityQueue* queue = &self->local_queue;
    queue->elements[queue->size] = element;
    queue->size++;
}

PriorityQueueElement pq_pop(Process* self) {
    PriorityQueue* queue = &self->local_queue;
    int index_of_extracted = find_index_with_highest_priority(queue);
    PriorityQueueElement extracted_element = queue->elements[index_of_extracted];

    // Replace the extracted element with the last element
    queue->elements[index_of_extracted] = queue->elements[queue->size - 1];
    queue->size--;

    return extracted_element;
}

PriorityQueueElement pq_peek(Process* self) {
    int index = find_index_with_highest_priority(&self->local_queue);
    return self->local_queue.elements[index];
}

int find_index_with_highest_priority(PriorityQueue* queue) {
    int index_of_max = 0;

    for (int i = 1; i < queue->size; ++i) {
        if (compare_elements(queue->elements[i], queue->elements[index_of_max]) > 0) {
            index_of_max = i;
        } 
    }

    return index_of_max;
}

int compare_elements(PriorityQueueElement first_element, PriorityQueueElement second_element) {
    if (first_element.time > second_element.time) {
        return 1;
    } else if (first_element.time < second_element.time) {
        return -1;
    } else {
        if (first_element.process_id < second_element.process_id) {
            return 1;
        } else if (first_element.process_id > second_element.process_id) {
            return -1;
        } else {
            return 0;
        }
    }
}
