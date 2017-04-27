/*
 * buttonHandler.c
 *
 *  Created on: Oct 31, 2016
 *      Author: hornej2
 */

#include <stdio.h>
#include <stdint.h>
#include "supportFiles/display.h"
#include "../interval_timers/buttons.h"
#include "simonDisplay.h"
#include "supportFiles/utils.h"

#define RUN_TEST_TERMINATION_MESSAGE1 "buttonHandler_runTest()"  // Info message.
#define RUN_TEST_TERMINATION_MESSAGE2 "terminated."              // Info message.
#define RUN_TEST_TEXT_SIZE 2            // Make text easy to see.
#define RUN_TEST_TICK_PERIOD_IN_MS 100  // Assume a 100 ms tick period.
#define TEXT_MESSAGE_ORIGIN_X 0                  // Text is written starting at the right, and
#define TEXT_MESSAGE_ORIGIN_Y (DISPLAY_HEIGHT/2) // middle.

// These are the definitions for the regions.
#define SIMON_DISPLAY_REGION_0 0
#define SIMON_DISPLAY_REGION_1 1
#define SIMON_DISPLAY_REGION_2 2
#define SIMON_DISPLAY_REGION_3 3

#define DISPLAY_HALF_WIDTH DISPLAY_WIDTH/2
#define DISPLAY_HALF_HEIGHT DISPLAY_HEIGHT/2

#define RESET 0 //reset for counters to 0
#define ADC_MAX 1 //adc rest
#define CNT_MAX 20 //cnt max

bool buttonHandler_enable_flag; //bool for enabling and disabling tick
uint8_t region; //save region number
bool buttonHandler_releaseDetected_flag = false; //keep track of detected button release

uint32_t adc; //keeps track of adc counts

// Get the simon region numbers. See the source code for the region numbering scheme.
uint8_t buttonHandler_getRegionNumber(){
    int16_t x; //x coordinate
    int16_t y; //y coordinate
    uint8_t z; //pressure (will not be needed)
    display_getTouchedPoint(&x, &y, &z); // Returns the x-y coordinate point and the pressure (z).

    if (x < DISPLAY_HALF_WIDTH && y < DISPLAY_HALF_HEIGHT){ //top left corner
        return SIMON_DISPLAY_REGION_0; //region 0
    }
    if (x > DISPLAY_HALF_WIDTH && y < DISPLAY_HALF_HEIGHT){ //top right corner
        return SIMON_DISPLAY_REGION_1; //region 1
    }
    if (x < DISPLAY_HALF_WIDTH && y > DISPLAY_HALF_HEIGHT){ //bottom left corner
        return SIMON_DISPLAY_REGION_2; //region 2
    }
    if (x > DISPLAY_HALF_WIDTH && y > DISPLAY_HALF_HEIGHT){ //bottom right corner
        return SIMON_DISPLAY_REGION_3; //region 3
    }
}

// Turn on the state machine. Part of the interlock.
void buttonHandler_enable(){
    buttonHandler_enable_flag = true;
}

// The only thing this function does is return a boolean flag set by the buttonHandler state machine. To wit:
// Once enabled, the buttonHandler state-machine first waits for a touch. Once a touch is detected, the
// buttonHandler state-machine computes the region-number for the touched area. Next, the buttonHandler
// state-machine waits until the player removes their finger. At this point, the state-machine should
// set a bool flag that indicates the the player has removed their finger. Once the buttonHandler()
// state-machine is disabled, it should clear this flag.
// All buttonHandler_releasedDetected() does is return the value of this flag.
// As such, the body of this function should only contain a single line of code

// Other state machines can call this function to see if the user has stopped touching the pad.
bool buttonHandler_releaseDetected(){
    return buttonHandler_releaseDetected_flag;
}

// States for the controller state machine.
enum buttonHandler_st_t {
    init_st, // Start here, stay in this state for just one tick.
    wait_for_touch_st, //wait for touch
    adc_timer_st, //let adc settle
    release_st, //once the user has released the button
    end_st, //last state
    wait_for_release_st //waiting for release
} currentState = init_st; //start in the init_st

static buttonHandler_st_t previousState;
static bool firstPass = true;

// Turn off the state machine. Part of the interlock.
void buttonHandler_disable(){
    currentState = init_st;
    buttonHandler_enable_flag = false;
}

// This is a debug state print routine. It will print the names of the states each
// time tick() is called. It only prints states if they are different than the
// previous state.
void debugStatePrint() {
    // Only print the message if:
    // 1. This the first pass and the value for previousState is unknown.
    // 2. previousState != currentState - this prevents reprinting the same state name over and over.
    if (previousState != currentState || firstPass) {
        firstPass = false;                // previousState will be defined, firstPass is false.
        previousState = currentState;     // keep track of the last state that you were in.
        switch(currentState) {            // This prints messages based upon the state that you were in.
        case init_st:
            printf("init_st buttonHandler\n\r"); //print init_st to console
            break;
        case wait_for_touch_st:
            printf("wait_for_touch_st buttonHandler\n\r"); //print wait_for_touch_st to console
            break;
        case adc_timer_st:
            printf("adc_timer_st buttonHandler\n\r"); //print adc_timer_st to console
            break;
        case wait_for_release_st:
            printf("wait_for_release_st buttonHandler\n\r"); //print wait_for_release_st to console
        case release_st:
            printf("release_st buttonHandler\n\r"); //print release_st to console
            break;
        case end_st:
            printf("end_st buttonHandler\n\r"); //print end_st to console
            break;
        }
    }
}

// Standard tick function.
void buttonHandler_tick() {
    //debugStatePrint();

    // Perform state action first
    switch(currentState) {
    case init_st:
        simonDisplay_drawAllButtons(); //draw board
        adc = RESET; //reset adc cnt
        buttonHandler_releaseDetected_flag = false; //reset releae detected
        break;
    case wait_for_touch_st: //wait for touch state
        break;
    case adc_timer_st:
        adc++; //increment adc cnt
        break;
    case wait_for_release_st: //wait for release state
        break;
    case release_st: //release state
        break;
    case end_st: //end state
        break;
    default:
        printf("buttonHandler_tick state action: hit default\n\r");
        break;
    }

    // Perform state update next
    switch(currentState) {
    case init_st:
        if (buttonHandler_enable_flag) //run the state machine if enable is on
            currentState = wait_for_touch_st; //go to wait for touch
        else
            currentState = init_st; //stay in init state
        break;
    case wait_for_touch_st:
        if(!display_isTouched()){ //wait until display is touched
            currentState = wait_for_touch_st;
        }
        if(display_isTouched()){ //if display is touched
            display_clearOldTouchData(); //clear the touch data
            currentState = adc_timer_st; //go to adc timer state
        }
        break;
    case wait_for_release_st:
        if (!display_isTouched()){ //if the display is now untouched
            buttonHandler_releaseDetected_flag = true; //set the release flag true
            currentState = release_st; //go to release state
        }
        else
        {
            currentState = wait_for_release_st; //state in state until released
        }
        break;
    case adc_timer_st:
        if (adc<ADC_MAX){ //wait for adc to settle
            currentState = adc_timer_st;
        }
        if (display_isTouched() && adc==ADC_MAX){ //if settled and touched
            region = buttonHandler_getRegionNumber(); //get region number
            simonDisplay_drawSquare(region,false); //draw appropriate square
            adc = RESET; //reset adc cnt
            currentState = wait_for_release_st; //go back to wait for release
        }
        else {
            currentState = adc_timer_st; //stay in adc timer state
        }
        if (!display_isTouched() && adc==ADC_MAX){ //if not touched and settled
            adc = RESET; //reset timer
            currentState = wait_for_touch_st; //go to wait for touch state
        }
        break;
    case release_st: //release state
        if (buttonHandler_releaseDetected()){ //if the touch has been released
            simonDisplay_drawSquare(region,true); //draw the square black
            simonDisplay_drawButton(region); //draw a new button
            currentState = end_st; //go to end state
        }
        else if (!buttonHandler_releaseDetected()){ //if no release
            currentState = release_st; //stay in release state
        }
        break;
    case end_st:
        if (!buttonHandler_enable_flag) //if the enable flag is down state in end state
            currentState = end_st;
        else
            currentState = init_st; //else go to init state
        break;
    default:
        printf("buttonHandler_tick state update: hit default\n\r");
        break;
    }
}

// buttonHandler_runTest(int16_t touchCount) runs the test until
// the user has touched the screen touchCount times. It indicates
// that a button was pushed by drawing a large square while
// the button is pressed and then erasing the large square and
// redrawing the button when the user releases their touch.
void buttonHandler_runTest(int16_t touchCountArg) {
    int16_t touchCount = 0;                 // Keep track of the number of touches.
    display_init();                         // Always have to init the display.
    display_fillScreen(DISPLAY_BLACK);      // Clear the display.
    simonDisplay_drawAllButtons();          // Draw all the buttons.
    buttonHandler_enable();
    while (touchCount < touchCountArg) {    // Loop here while touchCount is less than the touchCountArg
        buttonHandler_tick();               // Advance the state machine.
        utils_msDelay(RUN_TEST_TICK_PERIOD_IN_MS);
        if (buttonHandler_releaseDetected()) {  // If a release is detected, then the screen was touched.
            touchCount++;                       // Keep track of the number of touches.
            // Get the region number that was touched.
            printf("button released: %d\n\r", buttonHandler_getRegionNumber());
            // Interlocked behavior: handshake with the button handler (now disabled).
            buttonHandler_disable();
            utils_msDelay(RUN_TEST_TICK_PERIOD_IN_MS);
            buttonHandler_tick();               // Advance the state machine.
            buttonHandler_enable();             // Interlocked behavior: enable the buttonHandler.
            utils_msDelay(RUN_TEST_TICK_PERIOD_IN_MS);
            buttonHandler_tick();               // Advance the state machine.
        }
    }
    display_fillScreen(DISPLAY_BLACK);        // clear the screen.
    display_setTextSize(RUN_TEST_TEXT_SIZE);  // Set the text size.
    display_setCursor(TEXT_MESSAGE_ORIGIN_X, TEXT_MESSAGE_ORIGIN_Y); // Move the cursor to a rough center point.
    display_println(RUN_TEST_TERMINATION_MESSAGE1); // Print the termination message on two lines.
    display_println(RUN_TEST_TERMINATION_MESSAGE2);
}


