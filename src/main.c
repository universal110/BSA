/*
 * main.c
 *
 * This file is the entry point of the program.
 * Its only jobs are:
 *   1. Initialize the motion tracker system
 *   2. Run the main loop that checks buttons and calls the right functions
 *
 * All session logic, data, and metrics live in motionTracker.c
 * This file just drives the state machine.
 */


#include <zephyr/kernel.h> 
#include <zephyr/sys/printk.h>
#include "BTN.h"
#include "motionTrackerInterface.h"


#define SLEEP_MS 1

int main(void) {

    // Initialize the motion tracker, sets up buttons and clears session data
    // If it returns less than 0, something went wrong 
    int rv = motionTrackerInit();
    if (0 > rv) {
        printk("Motion tracker initialization failed\n");
        return 0;
    }

    // Main loop — runs forever, checking each button on every iteration
    while (1) {

        // BTN1 — start a new session
        if (BTN_check_clear_pressed(BTN0)) {
            startMotionSessions();
        }

        // BTN0 — record a motion event
        if (BTN_check_clear_pressed(BTN1)) {
            recordMotionEvents();
        }

        // BTN2 — end the session and print all metrics
        if (BTN_check_clear_pressed(BTN2)) {
            endMotionSession();
        }

        k_msleep(SLEEP_MS);
    }

    return 0;
}
