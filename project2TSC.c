/*Tyler Cooper
 * 2/28/18
 * project 2- producer and consumer telemarketing: threads (the producers) sell items, manager (consumer) places the orders*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUFF_SIZE 20      //the size of the buffer that holds unprocessed orders
#define NUM_SALES 10      //number of salespeople (producers)
#define NUM_ORDERS 20     //each salesperson generates this many orders
#define NUM_PRODS 30      //number of different products in stock
#define SEED 42

pthread_mutex_t the_mutex;
pthread_cond_t condc, condp;
int order_buffer[NUM_PRODS]; //storing what products need to be ordered
int cirqueue[BUFF_SIZE]; //circular queue for salespeople to place orders and for manager to receive orders
int count = 0; //counting so that we know when the queue is full/empty
int front = 0; //keep track of front of circular queue
int rear = 0; //keep track of rear of circular queue
int total_orders = 0;

//manager is the consumer
void *manager(void *tid) {
    int i = 0;
    int TOTAL_NUM_ORDERS = NUM_ORDERS * (NUM_SALES) *(NUM_SALES +1)/2;
    printf("Manager clocking in.\n");
    for(i=0; i<TOTAL_NUM_ORDERS; i++) {
        pthread_mutex_lock(&the_mutex);
        //do critical region stuff
        printf("Manager locked the mutex.\n");

        while (count == 0) {
            printf("Manager is waiting.\n");
            pthread_cond_wait(&condc, &the_mutex);
            printf("Manager woke up and is rechecking the buffer.\n");
        }
        order_buffer[cirqueue[front]]++; //store the front number from cirqueue in order_buffer
        printf("Manager retrieving product %d from index %d.\n", cirqueue[front], front);
        total_orders = total_orders +1; //counting the total number of orders placed
        front = (front + 1) % BUFF_SIZE; //make sure front wraps around when incremented
        count--; //decrement count (making sure cirqueue isn't full or empty)

        pthread_cond_signal(&condp); //salesperson wakeup signal
        printf("Manager unlocking mutex\n");
        pthread_mutex_unlock(&the_mutex);
    }
    printf("Manager clocking out.\n");
    printf("Manager is finished!\n");
    pthread_exit(0);
}

//salesperson is the producer
void *salesperson(void *tid){
    int i = 0;
    int p = 0;
    int num = 0;
    int thread_id = (int) tid +1;
    printf("Salesperson %d clocking in.\n", thread_id);
    for(i=0; i<NUM_ORDERS; i++) { //create the orders
        for (p = 1; p <= thread_id; p++) {
            printf("Salesperson %d starting order number %d.\n", thread_id, i);
            num = (int) random() % NUM_PRODS; //generate product number to order
            printf("Salesperson %d, order number %d, ready with product %d.\n", thread_id, i, num);
            pthread_mutex_lock(&the_mutex);
            //do critical region stuff
            printf("Salesperson %d locked the mutex.\n", thread_id);
            while (count == BUFF_SIZE) {
                printf("Salesperson %d waiting\n", thread_id);
                pthread_cond_wait(&condp, &the_mutex);
                printf("Salesperson %d woke up\n", thread_id);
            }

            //num = random() % NUM_PRODS; //generate product number to order
            cirqueue[rear] = num;
            printf("Salesperson %d storing the number %d, at index %d.\n", thread_id, num, rear);
            rear = (rear + 1) % BUFF_SIZE; //make sure rear wraps around when incremented
            count++; //increment count

            pthread_cond_signal(&condc); //manager wakeup signal
            printf("Salesperson %d unlocking mutex\n", thread_id);
            pthread_mutex_unlock(&the_mutex);
        }
        printf("Salesperson %d completed order number %d.\n", thread_id, i);
    }
    printf("Salesperson %d clocking out.\n", thread_id);
    pthread_exit(0);
}

int main(void) {
    srandom((unsigned int) SEED); //set seed for random number generator to 42
    pthread_t con;
    pthread_t salespersont[NUM_SALES];
    int status = 0;
    int i = 0;

    pthread_mutex_init(&the_mutex, 0);
    pthread_cond_init(&condc, 0);
    pthread_cond_init(&condp, 0);

    //create the salespeople
    for(i=0; i<NUM_SALES; i++){
        status = pthread_create(&salespersont[i], NULL, salesperson, (void *)i);
        if(status != 0){
            printf("Pthread create error\n");
            exit(-1);
        }
    }

    pthread_create(&con, 0, manager, 0);
    pthread_join(con, 0);

    //join the salespeople
    for(i=0; i<NUM_SALES; i++){
        //printf("joining salesperson %d.\n", i);
        status = pthread_join(salespersont[i], 0);
        if(status != 0){
            printf("Pthread join error\n");
            exit(-1);
        }
    }

    pthread_cond_destroy(&condc);
    pthread_cond_destroy(&condp);
    pthread_mutex_destroy(&the_mutex);

    printf("Results:\n");
    printf("Product number of items ordered.\n");
    for(i=0; i<NUM_PRODS; i++){
        printf("%d: %d\n", i, order_buffer[i]);
    }

    printf("Total number of items ordered: %d.\n", total_orders);
    return 0;
}