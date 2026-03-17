#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "motionTrackerInterface.h"
#include "BTN.h"
#include "sensor.h"

static void motionTrackerPrintResults(void);


static struct eventSession currentSession;
static bool sessionActive = false;
static float wrist_roll = 0.0f;   // wrist rotating as ball is released
static float wrist_snap = 0.0f;   // wrist snapping forward on follow through


// the following function sets the buttons to detect presses or input and creates a clean session
int motionTrackerInit(){
    int btn = BTN_init();
    if (0 > btn) {
        printk("initialization failed\n");
        return btn;      
    }

    int sens = sensorInit();
    if (0 > sens){
        printk("sensor start failed\n");
        return sens;
    }

    currentSession.eventCounter = 0;
    currentSession.sessionStartTimestamp = 0;
    currentSession.sessionEndTimestamp = 0;

    for (int i = 0; i < MAX_EVENTS; i++){
        currentSession.motionEvents[i].eventidx = 0;
        currentSession.motionEvents[i].timestamp = 0;
    }
  
    printk("motion tracker ready\n");
    return 0;
    
}

void startMotionSessions(void) {
    sessionActive = true;

    wrist_roll = sensorWristTurn();
    wrist_snap = sensorWristBend();

    currentSession.sessionStartTimestamp = k_uptime_get();
    currentSession.eventCounter = 0;
    printk("Session started\n");


}

void endMotionSession(void){
    sessionActive = false;
    currentSession.sessionEndTimestamp = k_uptime_get();
    printk("Session ended\n");
    motionTrackerPrintResults();
    
}

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
    
    // Store lateral (left/right) and forward deviation separately
    float currentTurn = sensorWristTurn();
    float currentBend = sensorWristBend();

    currentSession.motionEvents[currentSession.eventCounter].lateralDeviation = currentTurn - wrist_roll;

    currentSession.motionEvents[currentSession.eventCounter].forwardDeviation = currentBend - wrist_snap;

    currentSession.eventCounter += 1;
    printk("Event %d recorded at %lld ms\n", currentSession.eventCounter, currTime);



}

static void motionTrackerPrintResults(void){

    printk("Total events: %d\n", currentSession.eventCounter);

    if (2 > currentSession.eventCounter){
        printk("not enough data\n");
        return;
    }

    int64_t reactionTime = currentSession.motionEvents[0].timestamp - currentSession.sessionStartTimestamp;

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

    int64_t varianceSum = 0;
    for (int i = 0; i < numIntervals; i++) {
        int64_t difference = intervals[i] - avgIntervals;
        varianceSum += difference * difference;
    }
    int64_t variance = varianceSum / numIntervals;

    // Per shot breakdown
    printk("--- Shot Analysis ---\n");

    int64_t totalLateral = 0;
    int64_t totalForward = 0;

    for (int i = 0; i < currentSession.eventCounter; i++) {

        float lat = currentSession.motionEvents[i].lateralDeviation;
        float fwd = currentSession.motionEvents[i].forwardDeviation;

        // absolute value — negative just means left vs right direction
        if (lat < 0) lat = -lat;
        if (fwd < 0) fwd = -fwd;

        int latWhole   = (int)lat;
        int latDecimal = (int)((lat - latWhole) * 100);
        int fwdWhole   = (int)fwd;
        int fwdDecimal = (int)((fwd - fwdWhole) * 100);

        const char *label;
        if (lat < 5.0f) {
            label = "Straight release";
        } else if (lat < 15.0f) {
            label = "Slight deviation";
        } else {
            label = "Significant deviation";
        }

        printk("Shot %d - Lateral: %d.%02d  Forward: %d.%02d — %s\n",
               currentSession.motionEvents[i].eventidx + 1,
               latWhole, latDecimal,
               fwdWhole, fwdDecimal,
               label);

        totalLateral += (int64_t)lat;
        totalForward += (int64_t)fwd;
    }

    // Session summary
    int64_t avgLateral = totalLateral / currentSession.eventCounter;
    int64_t avgForward = totalForward / currentSession.eventCounter;

    const char *overallLabel;
    if (avgLateral < 5) {
        overallLabel = "Straight release";
    } else if (avgLateral < 15) {
        overallLabel = "Slight deviation";
    } else {
        overallLabel = "Significant deviation";
    }

    printk("--- Session Summary ---\n");
    printk("Total shots: %d\n", currentSession.eventCounter);
    printk("Reaction time: %lld ms\n", reactionTime);
    printk("Avg interval: %lld ms\n", avgIntervals);
    printk("Avg lateral deviation: %lld degrees\n", avgLateral);
    printk("Avg forward deviation: %lld degrees\n", avgForward);
    printk("Overall release: %s\n", overallLabel);
    printk("Consistency: ");
    if (variance < 2000) {
        printk("HIGH\n");
    } else if (variance < 8000) {
        printk("MEDIUM\n");
    } else {
        printk("LOW\n");
    }
}

bool motionTrackerIsActive(void) {
    return sessionActive;
}
