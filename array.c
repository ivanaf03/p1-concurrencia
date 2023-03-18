#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <threads.h>
#include "options.h"

#define DELAY_SCALE 1000


struct array {
    int size;
    int *arr;
};

struct parameters{
    int id;
    int iterations;
    int delay;
    struct array arr;
    mtx_t *mt;
};

void apply_delay(int delay) {
    for(int i = 0; i < delay * DELAY_SCALE; i++); // waste time
}


int increment(void *args)
{
    int pos, val;
    struct parameters* parameters=(struct parameters*) args;

    for(int i = 0; i < parameters->iterations; i++) {
        pos = rand() % parameters->arr.size;
        mtx_lock(&parameters->mt[pos]);
        printf("%d increasing position %d\n", parameters->id, pos);
        val = parameters->arr.arr[pos];
        apply_delay(parameters->delay);
        val ++;
        apply_delay(parameters->delay);
        parameters->arr.arr[pos] = val;
        mtx_unlock(&parameters->mt[pos]);
        apply_delay(parameters->delay);
    }
    return 0;
}

int sum_res(void *args){
    int pos1, pos2, val;
    struct parameters* parameters=(struct parameters*) args;
    for(int i = 0; i < parameters->iterations; i++) {

        pos1 = rand() % parameters->arr.size;
        pos2 = rand() % parameters->arr.size;

        if(pos1!=pos2) {
            if(pos1<pos2){
                mtx_lock(&parameters->mt[pos1]);
                mtx_lock(&parameters->mt[pos2]);
            } else{
                mtx_lock(&parameters->mt[pos2]);
                mtx_lock(&parameters->mt[pos1]);
            }
                printf("%d(2) increasing position %d\n", parameters->id, pos1);
                val = parameters->arr.arr[pos1];
                apply_delay(parameters->delay);
                val++;
                apply_delay(parameters->delay);
                parameters->arr.arr[pos1] = val;
                apply_delay(parameters->delay);

                printf("%d(2) decreasing position %d\n", parameters->id, pos2);
                val = parameters->arr.arr[pos2];
                apply_delay(parameters->delay);
                val--;
                apply_delay(parameters->delay);
                parameters->arr.arr[pos2] = val;
                apply_delay(parameters->delay);

                mtx_unlock(&parameters->mt[pos1]);
                mtx_unlock(&parameters->mt[pos2]);
            } else{
            printf("%d(2) increasing and decreasing position %d\n", parameters->id, pos2);
        }
        }
    return 0;
}



void print_array(struct array arr) {
    int total = 0;
    for(int i = 0; i < arr.size; i++) {
        total += arr.arr[i];
        printf("%d ", arr.arr[i]);
    }

    printf("\nTotal: %d\n", total);
}

int main (int argc, char **argv)
{
    struct options       opt;
    struct array         arr;
    srand(time(NULL));

    // Default values for the options
    opt.num_threads  = 5;
    opt.size         = 10;
    opt.iterations   = 100;
    opt.delay        = 1000;

    read_options(argc, argv, &opt);
    arr.size = opt.size;
    arr.arr  = malloc(arr.size * sizeof(int));
    memset(arr.arr, 0, arr.size * sizeof(int));

    thrd_t t[opt.num_threads];
    thrd_t t2[opt.num_threads];
    struct parameters p[opt.num_threads];
    mtx_t *mutex=malloc(sizeof(mtx_t)*opt.size);

    for(int i=0;i<opt.size;i++){
        mtx_init(&mutex[i], mtx_plain);
    }

    for(int i=0;i<opt.num_threads;i++){
        p[i].arr=arr;
        p[i].delay=opt.delay;
        p[i].iterations=opt.iterations;
        p[i].mt=mutex;
        p[i].id=i;

        if(thrd_create(&t[i], increment,&p[i])!=0){
            return -1;
        }
        if(thrd_create(&t2[i], sum_res,&p[i])!=0){
            return -1;
        }
    }

    for(int i=0;i<opt.num_threads;i++){
        if(thrd_join(t[i], NULL)!=0){
            return -1;
        }
        if(thrd_join(t2[i], NULL)!=0){
            return -1;
        }

    }

    print_array(arr);

    for(int i=0;i<opt.size;i++){
        mtx_destroy(&mutex[i]);
    }
    free(mutex);
    free(arr.arr);

    return 0;
}