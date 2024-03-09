#include <pthread.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#define PI 3.14
#define NUM_THREADS 2

bool run_complete = false;

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

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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
        pthread_mutex_unlock(&mutex);
        sleep(1); // Update rate of 1 Hz
    }

    return NULL;
}

void *read_nav_state(void *threadp) {
    thread_param *tp = (thread_param *)threadp;

    for (int i = 0; i < 18; i++) {
        pthread_mutex_lock(&mutex);
        nav_state *state = tp->state;
        printf("\nRead Thread Execution Number: %d", i);
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

int main() {
    printf("RTES Question 2:\n");
    
    pthread_t threads[NUM_THREADS];
    struct timespec timestamp;
    clock_gettime(CLOCK_REALTIME, &timestamp);
    nav_state state = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, timestamp};
     
    thread_param thread0 = {0, &state}, thread1 = {1, &state};

    pthread_create(&threads[0], NULL, update_nav_state, (void *)&thread0);
    pthread_create(&threads[1], NULL, read_nav_state, (void *)&thread1);
    
    // Wait for threads to finish
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
    
    pthread_mutex_destroy(&mutex);

    return 0;
}

