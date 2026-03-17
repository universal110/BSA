#include <zephyr/kernel.h> 
#include <zephyr/sys/printk.h>
#include "BTN.h"
#include "motionTrackerInterface.h"
#include "sensor.h"

#define SLEEP_MS 1

int main(void) {

    k_msleep(3000);
    int rv = motionTrackerInit();
    if (0 > rv) {
        printk("Motion tracker initialization failed\n");
        return 0;
    }

    // tracks the last time an event was recorded
    // prevents one flick from recording multiple events
    int64_t lastEventTime = 0;

    while (1) {

        // start a new session, records baseline orientation
        if (BTN_check_clear_pressed(BTN0)) {
            startMotionSessions();
        }

        // automatic motion detection which replaces BTN1
        // reads total accelerometer force and records event
        // if it exceeds threshold and cooldown has passed
        if (motionTrackerIsActive()) {
            float magnitude = sensorGetMagnitude();
            int64_t now = k_uptime_get();

            // threshold 
            if (magnitude > 28000.0f && (now - lastEventTime) > 500) {
                recordMotionEvents();
                lastEventTime = now;
            }
        }

        // BTN2: end the session and print all metrics
        if (BTN_check_clear_pressed(BTN2)) {
            endMotionSession();
        }

        k_msleep(SLEEP_MS);
    }

    return 0;
}