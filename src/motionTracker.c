/*
 *
 *
 * This file contains all session logic, motion event recording,
 * metric calculations, and serial output for the wrist motion tracker.
 *
 * main.c will call the functions defined here to drive the system.
 * The state machine lives in main.c — this file handles the data and math.
 */


/* ----------------------------------------------------------------------------
                                    INCLUDES
   ----------------------------------------------------------------------------
   Include the following
     - <zephyr/kernel.h>          for k_uptime_get() to capture timestamps
     - <zephyr/sys/printk.h>      for printk() to print to serial console
     - "motionTrackerInterface.h" your own header — gives access to the
                                  eventTracker and eventSession structs,
                                  MAX_EVENTS constant, and function declarations
     - "BTN.h"                    the course button driver

   Note: use quotes "" for your own local files, angle brackets <> for Zephyr system files
---------------------------------------------------------------------------- */
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "motionTrackerInterface.h"
#include "BTN.h"



/* ----------------------------------------------------------------------------
                                    CONSTANTS AND DATA TYPES
   ----------------------------------------------------------------------------
   MAX_EVENTS, struct eventTracker, and struct eventSession are all already
   defined in motionTrackerInterface.h which you included above.
   Do not redefine them here — including the header gives you full access.
---------------------------------------------------------------------------- */

static void motionTrackerPrintResults(void);



/* ----------------------------------------------------------------------------
                                    PRIVATE VARIABLE
   ----------------------------------------------------------------------------
   Declare one static variable of type struct eventSession to hold the current
   session data. Making it static means it is private to this file — main.c
   cannot access the raw data directly, only through your functions.

   Example:
     static struct eventSession _currentSession;

   This one variable holds everything:
     - motionEvents[]         — the array of up to MAX_EVENTS recorded events
     - eventCounter           — how many events have been recorded so far
     - sessionStartTimestamp  — when the session started
     - sessionEndTimestamp    — when the session ended
---------------------------------------------------------------------------- */

static struct eventSession currentSession;
static bool sessionActive = false;
static float wrist_roll = 0.0f;   // wrist rotating as ball is released
static float wrist_snap = 0.0f;   // wrist snapping forward on follow through



/* ----------------------------------------------------------------------------
                            SECTION 1 — INITIALIZATION
   ----------------------------------------------------------------------------
   Define the motionTrackerInit() function declared in motionTrackerInterface.h

   This function should:
     1. Call BTN_init() to initialize all buttons
        - store the return value in an int variable called rv (or similar)
        - if rv is less than 0, initialization failed:
            print an error message using printk()
            return rv so the caller knows something went wrong
     2. Reset _currentSession to a clean state:
        - set _currentSession.eventCounter to 0
        - set _currentSession.sessionStartTimestamp to 0
        - set _currentSession.sessionEndTimestamp to 0
        - loop through _currentSession.motionEvents[] from 0 to MAX_EVENTS
          and set each .eventidx and .timestamp to 0
     3. Print "Motion tracker ready" using printk()
     4. Return 0 to indicate success

   Function signature (must match motionTrackerInterface.h exactly):
     int motionTrackerInit(void)
---------------------------------------------------------------------------- */

// the following function sets the buttons to detect presses or input and creates a clean session
int motionTrackerInit(){
    int btn = BTN_init();
    if (0 > btn) {
        printk("initialization failed");
        return btn;      
    }

    currentSession.eventCounter = 0;
    currentSession.sessionStartTimestamp = 0;
    currentSession.sessionEndTimestamp = 0;

    for (int i = 0; i < MAX_EVENTS; i++){
        currentSession.motionEvents[i].eventidx = 0;
        currentSession.motionEvents[i].timestamp = 0;
    }
  
    printk("motion tracker ready");
    return 0;
    
}

/* ----------------------------------------------------------------------------
                            SECTION 2 — SESSION CONTROL
   ----------------------------------------------------------------------------
   Define two functions: startMotionSessions() and endMotionSession()
   Both signatures are declared in motionTrackerInterface.h

   START SESSION — startMotionSessions():
     1. Use k_uptime_get() to get the current time in ms and store it into
        _currentSession.sessionStartTimestamp
     2. Reset _currentSession.eventCounter to 0 so old data doesn't carry over
     3. Print "Session started" using printk()

   ---

   END SESSION — endMotionSession():
     1. Use k_uptime_get() to get the current time in ms and store it into
        _currentSession.sessionEndTimestamp
     2. Print "Session ended" using printk()
     3. Call motionTrackerPrintResults() which is defined below in Section 4
        This automatically calculates and prints all metrics when session ends

   Function signatures (must match motionTrackerInterface.h exactly):
     void startMotionSessions(void)
     void endMotionSession(void)
---------------------------------------------------------------------------- */
void startMotionSessions(void) {
    sessionActive = true;
    currentSession.sessionStartTimestamp = k_uptime_get();
    currentSession.eventCounter = 0; // why?
    printk("Session started\n");

}

void endMotionSession(void){
    sessionActive = false;
    currentSession.sessionEndTimestamp = k_uptime_get();
    printk("Session ended\n");
    motionTrackerPrintResults();
    
}


/* ----------------------------------------------------------------------------
                            SECTION 3 — MOTION EVENT RECORDING
   ----------------------------------------------------------------------------
   Define the recordMotionEvents() function declared in motionTrackerInterface.h

   This function is called by main.c whenever BTN0 is pressed during
   an active session. Each call records one motion event.

   The function should:
     1. Check if _currentSession.eventCounter has already reached MAX_EVENTS
        - if so: print "Max events reached" using printk() and return
        - this prevents writing past the end of the motionEvents array
     2. Capture the current timestamp:
        - declare a local int64_t variable
        - assign it the return value of k_uptime_get()
     3. Store the timestamp into the next available slot:
        _currentSession.motionEvents[_currentSession.eventCounter].timestamp
     4. Store the event index into the same slot:
        _currentSession.motionEvents[_currentSession.eventCounter].eventidx
        use the current value of eventCounter as the index value
     5. Increment _currentSession.eventCounter by 1
     6. Print the event number and timestamp using printk()
        Example: "Event 1 recorded at 1340 ms"
        Tip: print eventCounter after incrementing so it reads 1, 2, 3...
             instead of 0, 1, 2...

   Function signature (must match motionTrackerInterface.h exactly):
     void recordMotionEvents(void)
---------------------------------------------------------------------------- */

void recordMotionEvents(void){
    if (!sessionActive) {
      printk("No active session\n");
      return;
    }
    if(currentSession.eventCounter == MAX_EVENTS){
      printk("Max Events Reached\n");
      return;
    }

    int64_t currTime = k_uptime_get();

    currentSession.motionEvents[currentSession.eventCounter].timestamp = currTime;
    currentSession.motionEvents[currentSession.eventCounter].eventidx = currentSession.eventCounter;
    
    currentSession.eventCounter += 1;
    printk("Event %d recorded at %lld ms\n", currentSession.eventCounter, currTime);



}

/* ----------------------------------------------------------------------------
                            SECTION 4 — METRICS CALCULATION AND OUTPUT
   ----------------------------------------------------------------------------
   Define a static function called motionTrackerPrintResults()
   
   It is static because it is NOT in motionTrackerInterface.h — main.c never
   calls this directly. Only endMotionSession() calls it internally.

   The function should do the following in order:

   STEP 1 — Print total events:
     printk("Total events: %d\n", _currentSession.eventCounter);

   STEP 2 — Guard clause: if eventCounter is less than 2, print
     "Not enough data for metrics"
     and return — you need at least 2 events to compute intervals

   STEP 3 — Reaction time:
     - declare int64_t reactionTime
     - reactionTime = motionEvents[0].timestamp - sessionStartTimestamp
     - print it: "Reaction time: %lld ms\n"

   STEP 4 — Average interval:
     - declare int64_t intervals[MAX_EVENTS] to store each gap between events
     - declare int64_t totalInterval = 0 to accumulate the sum
     - loop from i = 1 to eventCounter - 1:
         intervals[i-1] = motionEvents[i].timestamp - motionEvents[i-1].timestamp
         add intervals[i-1] to totalInterval
     - int64_t numIntervals = eventCounter - 1
     - int64_t avgInterval = totalInterval / numIntervals
     - print it: "Average interval: %lld ms\n"

   STEP 5 — Consistency (variance):
     - declare int64_t varianceSum = 0
     - loop through intervals[] again:
         int64_t diff = intervals[i] - avgInterval
         varianceSum += diff * diff
     - int64_t variance = varianceSum / numIntervals
     - print variance: "Variance: %lld\n"
     - print consistency label:
         if variance < 2000  → "Consistency: HIGH\n"
         if variance < 8000  → "Consistency: MEDIUM\n"
         otherwise           → "Consistency: LOW\n"

   STEP 6 — Leave this placeholder comment exactly as written:
     // TODO: IMU wrist deviation results will be printed here (Lesson 10)

   Function signature (static, not declared in header):
     static void motionTrackerPrintResults(void)
---------------------------------------------------------------------------- */
static void motionTrackerPrintResults(void){

      printk("Total events: %d\n", currentSession.eventCounter);

      if (2 > currentSession.eventCounter){
        printk("not enough data\n");
        return;
      }

      int64_t reactionTime = currentSession.motionEvents[0].timestamp - currentSession.sessionStartTimestamp;
      printk("Reaction time: %lld ms\n", reactionTime);

      int64_t intervals[MAX_EVENTS];
      int64_t totalInterval = 0;
      int64_t avgIntervals = 0;
      int64_t numIntervals = 0;


      for (int i = 1; i < currentSession.eventCounter; i++){
        intervals[i - 1] = currentSession.motionEvents[i].timestamp - currentSession.motionEvents[i - 1].timestamp;
        totalInterval += intervals[i - 1];
      }

      numIntervals = currentSession.eventCounter - 1;
      avgIntervals = totalInterval / numIntervals;
          
      printk("Avg Interval: %lld ms\n", avgIntervals);
     
      int64_t varianceSum = 0;

      for (int i = 0; i < numIntervals; i++) {
          int64_t difference = intervals[i] - avgIntervals;
          varianceSum += difference * difference;
      }
      int64_t variance = varianceSum / numIntervals;
      printk("Variance: %lld\n", variance);

      if (variance < 2000) {
        printk("Consistency: HIGH\n");
      } else if (variance < 8000) {
        printk("Consistency: MEDIUM\n");
      } else {
        printk("Consistency: LOW\n");
      }

// TODO: IMU wrist deviation results will be printed here (Lesson 10)

}

/* ----------------------------------------------------------------------------
                            SECTION 5 — IMU PLACEHOLDER
   ----------------------------------------------------------------------------
   Leave this section empty for now with this TODO comment:

   TODO: When IMU arrives (Lesson 10), add the following:
     - include the IMU driver header at the top of this file
     - record baseline orientation at the start of startMotionSessions()
     - calculate wrist deviation per event inside recordMotionEvents()
     - print deviation results inside motionTrackerPrintResults() at STEP 6
---------------------------------------------------------------------------- */