/*
 * File: Q5.c
 * Author: Suhas Reddy and Krishna Suhagiya
 * Description: This program demonstrates a multi-threaded system for updating 
 *		and reading navigation state data. It includes three threads: 
 *		one for updating navigation state, one for reading the state, 
 *		and one for timeout handling. The update thread updates the 
 *		navigation state variables periodically, the read thread reads 
 *		and prints the state, and the timeout thread handles resource 
 *		acquisition timeouts. The program utilizes POSIX threads and 
 *		synchronization mechanisms such as mutexes and condition variables.
 * Date: 9th March 2023
 */

#include <pthread.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>

#define PI 3.14
#define NUM_THREADS 3

bool run_complete = false;
pthread_cond_t signal_read = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    double Latitude;
    double Longitude;
    double Altitude;
    double Roll;
    double Pitch;
    double Yaw;
    struct timespec timestamp;
} nav_state;

typedef struct {
    int thread_idx;
    nav_state *state;
} thread_param;

static nav_state state;

void *update_nav_state(void *threadp) {
    thread_param *tp = (thread_param *)threadp;

    while (!run_complete) {
        pthread_mutex_lock(&mutex);
        nav_state *state = tp->state;
        clock_gettime(CLOCK_REALTIME, &state->timestamp);
        state->Latitude = 0.01 * (state->timestamp.tv_sec);
        state->Longitude = 0.2 * (state->timestamp.tv_sec);
        state->Altitude = 0.25 * (state->timestamp.tv_sec);
        state->Roll = sin(2 * PI * (state->timestamp.tv_sec));
        state->Pitch = cos(2 * PI * (state->timestamp.tv_sec) * (state->timestamp.tv_sec));
        state->Yaw = cos(2 * PI * (state->timestamp.tv_sec));
        pthread_cond_signal(&signal_read);   // Signal read function when update is complete
        pthread_mutex_unlock(&mutex);
        sleep(1); // Update rate of 1 Hz
    }
    
    pthread_mutex_lock(&mutex);
    return NULL;
}

void *read_nav_state(void *threadp) {
    thread_param *tp = (thread_param *)threadp;
    for (int i = 0; i < 18; i++) {
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&signal_read, &mutex);  // Wait until update is complete
        nav_state *state = tp->state;
        printf("\nExecution number: %d", i);
        printf("\nLatitude: %lf", state->Latitude);
        printf("\nLongitude: %lf", state->Longitude);
        printf("\nAltitude: %lf", state->Altitude);
        printf("\nRoll: %lf", state->Roll);
        printf("\nPitch: %lf", state->Pitch);
        printf("\nYaw: %lf", state->Yaw);
        printf("\nTimestamp: %lu\n", state->timestamp.tv_sec);
        pthread_mutex_unlock(&mutex);
        sleep(10); // Read rate of 0.1 Hz
    }
    run_complete = true;
    return NULL;
}

void *timeout_thread(void *threadp) {
    for (int i = 0; i <= 19; i++) {
        struct timespec timeout;
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += 10;
        printf("\nWaiting on Resources");
        int mutex_acquired;
        while ((mutex_acquired = pthread_mutex_timedlock(&mutex, &timeout)) != 0) {
            if (mutex_acquired == ETIMEDOUT) {
                printf("\nNo new data available at %lu", time(NULL));
                pthread_mutex_unlock(&mutex);
            }
        }

        pthread_mutex_unlock(&mutex);
        printf("\nAcquired Resources\n");

        sleep(10);  // Check for data at a rate of 0.1 Hz
    }

    return NULL;
}

int main() {
    printf("RTES Question 5\n");
    
    pthread_t threads[NUM_THREADS];
    struct timespec timestamp;
    clock_gettime(CLOCK_REALTIME, &timestamp);
    state.Latitude = 0.0;
    state.Longitude = 0.0;
    state.Altitude = 0.0;
    state.Roll = 0.0;
    state.Pitch = 0.0;
    state.Yaw = 0.0;
    state.timestamp = timestamp;
    
    thread_param thread0 = {0, &state}, thread1 = {1, &state};

    pthread_create(&threads[0], NULL, update_nav_state, (void *)&thread0);
    pthread_create(&threads[1], NULL, read_nav_state, (void *)&thread1);
    pthread_create(&threads[2], NULL, timeout_thread, NULL);
    
    // Wait for threads to finish
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
    pthread_join(threads[2], NULL);
    
    pthread_mutex_destroy(&mutex);

    return 0;
}

