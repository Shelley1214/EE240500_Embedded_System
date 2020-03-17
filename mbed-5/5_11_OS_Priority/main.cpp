#include "mbed.h"

#include "rtos.h"


Thread t2;

Thread t3;

Thread *(t2_);

Thread *(t3_);


void notify(const char* name, int state) {

    printf("%s: %d\n\r", name, state);

}


void test_thread(void const *args) {

    while (true) {

        notify((const char*)args, 0); wait(1);

        notify((const char*)args, 1); wait(1);

    }

}


int main() {

    t2.start(callback(test_thread, (void *)"Th 2"));

    t3.start(callback(test_thread, (void *)"Th 3"));


    // Set osPriority Help to Synchronize each thread !

    t2_ = &t2;

    t3_ = &t3;

    t2_->set_priority(osPriorityHigh);

    t3_->set_priority(osPriorityLow);


    // Default osPriority of Main Thread is 'osPriorityNormal'

    test_thread((void *)"Th 1");

}
