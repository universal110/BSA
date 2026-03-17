/*
 * motionTrackerInterface.h
 *
 * This is the public interface for the motion tracker module.
 * main.c will include this file to gain access to the session
 * functions it needs to call.
 *
 * Rules for this file:
 *   - NO implementation goes here, only declarations
 *   - Only expose functions that main.c actually needs to call
 *   - Internal functions like motionTracker_printResults() stay hidden
 */


/* ----------------------------------------------------------------------------
                                INCLUDE GUARD
   ----------------------------------------------------------------------------
   Wrap the entire file in an include guard to prevent it being included twice.
   
   Pattern:
     #ifndef MOTION_TRACKER_INTERFACE_H
     #define MOTION_TRACKER_INTERFACE_H
     
     ... everything else goes here ...
     
     #endif

---------------------------------------------------------------------------- */

// This prevents the compiler from processing the interface file twice
// since it'll be included in both main.c and motiontracker.c file
#ifndef MOTION_TRACKER_INTERFACE_H
#define MOTION_TRACKER_INTERFACE_H


#include <zephyr/kernel.h>
#include <stdint.h>

#define MAX_EVENTS 20 // number of events displayed and tracked 
struct eventTracker {
    int eventidx;
    int64_t timestamp;
    //int64_t wristDeviation;
    float lateralDeviation;   // left/right wrist movement — should be near 0°
    float forwardDeviation;   // wrist snap forward — expected ~30°+
};

struct eventSession {
    struct eventTracker motionEvents[MAX_EVENTS]; // eventSession contains a struct of motionEvents which is of type eventTracker and takes in n arguments defined by max_events
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