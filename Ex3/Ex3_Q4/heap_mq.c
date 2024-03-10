/*
 * File: heap_mq.c
 * Author: Krishna Suhagiya and Suhas Reddy
 * Description: This file ports the provided VxWorks posix_mq.c implementation to POSIX with SCHED_FIFO scheduling.
 * Date: 9th March 2023
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <mqueue.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>

#define SNDRCV_MQ "/send_receive_mq"

struct mq_attr mq_attr;
mqd_t mymq;

void *receiver(void *arg)
{
    char buffer[sizeof(void *)+sizeof(int)];
    void *buffptr;
    int prio;
    int nbytes;
    int id;

    while (1)
    {
        /* Read oldest, highest priority msg from the message queue */
        if ((nbytes = mq_receive(mymq, buffer, (size_t)sizeof(void *)+sizeof(int), &prio)) == -1)
        {
            perror("mq_receive");
        }
        else
        {
            buffer[nbytes] = '\0';
            memcpy(&buffptr, buffer, sizeof(void *));
            memcpy((void *)&id, &(buffer[sizeof(void *)]), sizeof(int));
            printf("receive: ptr msg %p received with priority = %d, length = %d, id = %d\n", buffptr, prio, nbytes, id);
            printf("contents of ptr = %s\n", (char *)buffptr);
            free(buffptr);
            printf("heap space memory freed\n");
        }
    }
    return NULL;
}

static char imagebuff[4096];

void *sender(void *arg)
{
    char buffer[sizeof(void *)+sizeof(int)];
    void *buffptr;
    int prio;
    int nbytes;
    int id = 999;

    while (1)
    {
        /* Send malloc'd message with priority=30 */
        buffptr = (void *)malloc(sizeof(imagebuff));
        strcpy(buffptr, imagebuff);
        printf("Message to send = %s\n", (char *)buffptr);

        printf("Sending %ld bytes\n", sizeof(buffptr));

        memcpy(buffer, &buffptr, sizeof(void *));
        memcpy(&(buffer[sizeof(void *)]), (void *)&id, sizeof(int));

        if ((nbytes = mq_send(mymq, buffer, (sizeof(void *) + sizeof(int)), 30)) == -1)
        {
            perror("mq_send");
        }
        else
        {
            printf("send: message ptr %p successfully sent\n", buffptr);
        }

        // Introduce a delay
        usleep(3000000); // 3 seconds

    }
    return NULL;
}

static int sid, rid;

void heap_mq(void)
{
    pthread_t receiver_thread, sender_thread;
    pthread_attr_t receiver_attr, sender_attr;
    struct sched_param receiver_param, sender_param;

    int i, j;
    char pixel = 'A';

    for(i=0;i<4096;i+=64) {
    pixel = 'A';
    for(j=i;j<i+64;j++) {
      imagebuff[j] = (char)pixel++;
    }
    imagebuff[j-1] = '\n';
    }
    imagebuff[4095] = '\0';
    imagebuff[63] = '\0';

    printf("buffer =\n%s", imagebuff);

    // Setup common message queue attributes
    mq_attr.mq_maxmsg = 100;
    mq_attr.mq_msgsize = sizeof(void *)+sizeof(int);
    mq_attr.mq_flags = 0;

    // Initialize attributes
    pthread_attr_init(&receiver_attr);
    pthread_attr_init(&sender_attr);

    // Set scheduling parameters
    pthread_attr_setschedpolicy(&receiver_attr, SCHED_FIFO);
    pthread_attr_setschedpolicy(&sender_attr, SCHED_FIFO);

    // Set priority for receiver and sender threads
    receiver_param.sched_priority = 100; // Higher priority for receiver
    sender_param.sched_priority = 90;

    pthread_attr_setschedparam(&receiver_attr, &receiver_param);
    pthread_attr_setschedparam(&sender_attr, &sender_param);

    // Create message queue
    mymq = mq_open(SNDRCV_MQ, O_CREAT | O_RDWR, 777, &mq_attr);
    if (mymq == (mqd_t)-1)
    {
        perror("mq_open");
        exit(1);
    }

    // Create receiver and sender threads with the specified attributes
    if (pthread_create(&receiver_thread, &receiver_attr, receiver, NULL) != 0)
    {
        perror("pthread_create");
        exit(1);
    }
    else
    {
        printf("Receiver thread created\n");
    }

    if (pthread_create(&sender_thread, &sender_attr, sender, NULL) != 0)
    {
        perror("pthread_create");
        exit(1);
    }
    else
    {
        printf("Sender thread created\n");
    }

    // Clean up attributes
    pthread_attr_destroy(&receiver_attr);
    pthread_attr_destroy(&sender_attr);

    // Wait for threads to complete
    pthread_join(receiver_thread, NULL);
    pthread_join(sender_thread, NULL);

    // Close message queue
    mq_close(mymq);
}

void shutdown(void)
{
  mq_close(mymq);
}

int main()
{
	mq_unlink(SNDRCV_MQ);  // Make sure that SNDRCV_MQ is cleanly available
    heap_mq();
    return 0;
}
