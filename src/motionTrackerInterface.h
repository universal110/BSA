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


/* ----------------------------------------------------------------------------
                                   INCLUDES
   ----------------------------------------------------------------------------
   You will need:
     - <zephyr/kernel.h>   because your structs use int64_t for timestamps
---------------------------------------------------------------------------- */
#include <zephyr/kernel.h>
#include <stdint.h>


/* ----------------------------------------------------------------------------
                                   CONSTANTS
   ----------------------------------------------------------------------------
   Declare MAX_EVENTS here (not in motionTracker.c) so that both files
   agree on the same array size.
   
   Example: #define MAX_EVENTS 10
---------------------------------------------------------------------------- */
#define MAX_EVENTS 5 // number of events displayed and tracked 

/* ----------------------------------------------------------------------------
                                   TYPES
   ----------------------------------------------------------------------------
   Declare your two structs here so that both motionTracker.c and main.c
   can see the same type definitions.

   STRUCT 1 — a single motion event:
     - an int for the event index
     - an int64_t for the timestamp in milliseconds

   STRUCT 2 — the full session:
     - an array of motion events, sized to MAX_EVENTS
     - an int to count how many events have been recorded
     - an int64_t for the session start timestamp
     - an int64_t for the session end timestamp
---------------------------------------------------------------------------- */
struct eventTracker {
    int eventidx;
    int64_t timestamp;
    int64_t wristDeviation;
};

struct eventSession {
    struct eventTracker motionEvents[MAX_EVENTS]; // eventSession contains a struct of motionEvents which is of type eventTracker and takes in n arguments defined by max_events
    int eventCounter;
    int64_t sessionStartTimestamp;
    int64_t sessionEndTimestamp;
};

/* ----------------------------------------------------------------------------
                              PUBLIC FUNCTION DECLARATIONS
   ----------------------------------------------------------------------------
   Declare the four functions that main.c is allowed to call.
   No implementation — just the signature followed by a semicolon.
   Include a one-line comment above each explaining what it does.

     1. motionTracker_init       — initialises buttons and resets session data
     2. motionTracker_startSession  — records start time, resets event counter
     3. motionTracker_endSession    — records end time, triggers result printing
     4. motionTracker_recordEvent   — records one motion event with timestamp
---------------------------------------------------------------------------- */

// functions are void because they dont return anything to main
int motionTrackerInit(void);
void startMotionSessions(void);
void endMotionSession(void);
void recordMotionEvents(void);


#endif