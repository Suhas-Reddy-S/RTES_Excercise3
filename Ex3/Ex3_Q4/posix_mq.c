/*
 * File: posix.c
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
#define MAX_MSG_SIZE 128

struct mq_attr mq_attr;

void *receiver(void *arg)
{
    mqd_t mymq;
    char buffer[MAX_MSG_SIZE];
    int prio;
    int nbytes;

    // Open the message queue for reading
    mymq = mq_open(SNDRCV_MQ, O_CREAT | O_RDWR, 777, &mq_attr);
    if (mymq == (mqd_t)-1)
    {
        perror("mq_open");
        exit(1);
    }

    // Read oldest, highest priority message from the message queue
    if ((nbytes = mq_receive(mymq, buffer, MAX_MSG_SIZE, &prio)) == -1)
    {
        perror("mq_receive");
    }
    else
    {
        buffer[nbytes] = '\0';
        printf("Receiver: Message '%s' received with priority = %d, length = %d\n", buffer, prio, nbytes);
    }

    // Close the message queue
    mq_close(mymq);
    return NULL;
}

static char canned_msg[] = "this is a test, and only a test, in the event of a real emergency, you would be instructed ...";

void *sender(void *arg)
{
    mqd_t mymq;
    int prio;
    int nbytes;

    // Open the message queue for writing
    mymq = mq_open(SNDRCV_MQ, O_RDWR, 777, &mq_attr);
    if (mymq == (mqd_t)-1)
    {
        perror("mq_open");
        exit(1);
    }

    // Send message with priority=30
    if ((nbytes = mq_send(mymq, canned_msg, sizeof(canned_msg), 30)) == -1)
    {
        perror("mq_send");
    }
    else
    {
        printf("Sender: Message successfully sent\n");
    }

    // Close the message queue
    mq_close(mymq);
    return NULL;
}

void mq_demo(void)
{
    pthread_t receiver_thread, sender_thread;
    pthread_attr_t receiver_attr, sender_attr;
    struct sched_param receiver_param, sender_param;

    // Setup common message queue attributes
    mq_attr.mq_maxmsg = 100;
    mq_attr.mq_msgsize = MAX_MSG_SIZE;
    mq_attr.mq_flags = 0;

    // Initialize attributes
    pthread_attr_init(&receiver_attr);
    pthread_attr_init(&sender_attr);

    // Set scheduling parameters
    pthread_attr_setschedpolicy(&receiver_attr, SCHED_FIFO);
    pthread_attr_setschedpolicy(&sender_attr, SCHED_FIFO);

    // Set priority for receiver and sender threads
    receiver_param.sched_priority = 100;
    sender_param.sched_priority = 90;

    pthread_attr_setschedparam(&receiver_attr, &receiver_param);
    pthread_attr_setschedparam(&sender_attr, &sender_param);

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
}

int main()
{
    mq_unlink(SNDRCV_MQ);   // Make sure that SNDRCV_MQ is cleanly available
    mq_demo();
    return 0;
}
