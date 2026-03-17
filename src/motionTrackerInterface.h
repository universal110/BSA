#ifndef MOTION_TRACKER_INTERFACE_H
#define MOTION_TRACKER_INTERFACE_H


#include <zephyr/kernel.h>
#include <stdint.h>

#define MAX_EVENTS 20 // number of events displayed and tracked 
struct eventTracker {
    int eventidx;
    int64_t timestamp;
    //int64_t wristDeviation;
    float lateralDeviation;   // left/right wrist movement
    float forwardDeviation;   // wrist snap forward
};

struct eventSession {
    struct eventTracker motionEvents[MAX_EVENTS];
    int eventCounter;
    int64_t sessionStartTimestamp;
    int64_t sessionEndTimestamp;
};


int motionTrackerInit(void);
void startMotionSessions(void);
void endMotionSession(void);
void recordMotionEvents(void);
bool motionTrackerIsActive(void);


#endif