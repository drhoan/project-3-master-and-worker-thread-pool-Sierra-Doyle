#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int item_to_produce = 0, curr_buf_size = 0;
int total_items, max_buf_size, num_workers, num_masters;

int *buffer;
pthread_mutex_t lock;

void print_produced(int num, int master) {
    printf("Produced %d by master %d\n", num, master);
}

void print_consumed(int num, int worker) {
    printf("Consumed %d by worker %d\n", num, worker);
}

// Master thread function to produce items
void *generate_requests_loop(void *data) {
    int thread_id = *((int *)data);

    while (1) {
        // Lock the thread
        pthread_mutex_lock(&lock);

        // Check if all items have been produced and unlock and break
        if (item_to_produce >= total_items) {
            pthread_mutex_unlock(&lock);
            break;
        }

        // Check if there is room in the buffer
        if (curr_buf_size < max_buf_size) {
            // Add teh tiem to the buffer, print the information and increment item number
            buffer[curr_buf_size++] = item_to_produce;
            print_produced(item_to_produce, thread_id);
            item_to_produce++;
        }

        // Unlock the thread
        pthread_mutex_unlock(&lock);
    }

    return 0;
}

// Worker function for consumer
void *worker_function(void *data) {
    int thread_id = *((int *)data);

    while (1) {
        // Lock the thread
        pthread_mutex_lock(&lock);

        // Check to see if all items have been produced, unlock and break if they have been
        if (curr_buf_size <= 0 && item_to_produce >= total_items) {
            pthread_mutex_unlock(&lock);
            break;
        }

        // Check to see if the buffer is empty
        if (curr_buf_size > 0) {
            // Remove an item, unlock and print
            int item = buffer[--curr_buf_size];
            pthread_mutex_unlock(&lock);
            print_consumed(item, thread_id);
        } 
        // If empty, just unlock
        else {
            pthread_mutex_unlock(&lock);
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {
    int *master_thread_id;
    pthread_t *master_thread;
    int *worker_thread_id;
    pthread_t *worker_thread;

    if (argc < 5) {
        printf("./master-worker #total_items #max_buf_size #num_workers #masters\n");
        exit(1);
    } else {
        total_items = atoi(argv[1]);
        max_buf_size = atoi(argv[2]);
        num_workers = atoi(argv[3]);
        num_masters = atoi(argv[4]);
    }

    // Create buffer and lock
    buffer = (int *)malloc(sizeof(int) * max_buf_size);
    pthread_mutex_init(&lock, NULL);

    // Create master threads
    master_thread_id = (int *)malloc(sizeof(int) * num_masters);
    master_thread = (pthread_t *)malloc(sizeof(pthread_t) * num_masters);

    for (int i = 0; i < num_masters; i++) {
        master_thread_id[i] = i;
        pthread_create(&master_thread[i], NULL, generate_requests_loop, &master_thread_id[i]);
    }

    // Create worker threads
    worker_thread_id = (int *)malloc(sizeof(int) * num_workers);
    worker_thread = (pthread_t *)malloc(sizeof(pthread_t) * num_workers);

    for (int i = 0; i < num_workers; i++) {
        worker_thread_id[i] = i;
        pthread_create(&worker_thread[i], NULL, worker_function, &worker_thread_id[i]);
    }

    // Join and print all threads
    for (int i = 0; i < num_masters; i++) {
        pthread_join(master_thread[i], NULL);
        printf("Master %d joined\n", i);
    }

    for (int i = 0; i < num_workers; i++) {
        pthread_join(worker_thread[i], NULL);
        printf("Worker %d joined\n", i);
    }

    // Destroy lock and free memory
    pthread_mutex_destroy(&lock);
    free(buffer);
    free(master_thread_id);
    free(master_thread);
    free(worker_thread_id);
    free(worker_thread);

    return 0;
}
