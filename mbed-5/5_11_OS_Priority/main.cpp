
/*
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
*/

#include "mbed.h"

#include "mbed_events.h"


DigitalOut led1(LED1);

DigitalOut led2(LED2);

InterruptIn btn(SW2);


EventQueue printfQueue;

EventQueue eventQueue;


void blink_led2() {

  // this runs in the normal priority thread

  led2 = !led2;

}


void print_toggle_led() {

  // this runs in the lower priority thread

  printf("Toggle LED!\r\n");

}


void btn_fall_irq() {

  led1 = !led1;


  // defer the printf call to the low priority thread

  printfQueue.call(&print_toggle_led);

}


int main() {

  // low priority thread for calling printf()

  Thread printfThread(osPriorityLow);

  printfThread.start(callback(&printfQueue, &EventQueue::dispatch_forever));


  // normal priority thread for other events

  Thread eventThread(osPriorityHigh);

  eventThread.start(callback(&eventQueue, &EventQueue::dispatch_forever));


  // call blink_led2 every second, automatically defering to the eventThread

  Ticker ledTicker;

  ledTicker.attach(eventQueue.event(&blink_led2), 1.0f);


  // button fall still runs in the ISR

  btn.fall(&btn_fall_irq);


  while (1) {wait(1);}

}
