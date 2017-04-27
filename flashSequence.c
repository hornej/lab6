/*
 * flashSequence.c
 *
 *  Created on: Oct 31, 2016
 *      Author: hornej2
 */

#include "flashSequence.h"
#include <stdio.h>
#include <stdint.h>
#include "globals.h"
#include "buttonHandler.h"
#include "supportFiles/display.h"
#include "../interval_timers/buttons.h"
#include "simonDisplay.h"
#include "supportFiles/utils.h"
#include <stdbool.h>

#define WAIT_CNT_MAX 5 //how long the buttons will flash for
#define RESET 0 //reset counters

bool flashSequence_enable_flag; //enable flag for flash sequence
bool flashSequence_isComplete_flag; //flag to check if sequence is complete
uint8_t waitcnt; //wait cnt
uint8_t iteration; //counts iteration

// Turns on the state machine. Part of the interlock.
void flashSequence_enable(){
    flashSequence_enable_flag = true;
}

// Turns off the state machine. Part of the interlock.
void flashSequence_disable(){
    flashSequence_enable_flag = false;
}

// Other state machines can call this to determine if this state machine is finished.
bool flashSequence_isComplete(){
    return flashSequence_isComplete_flag;
}

enum flashSequence_st_t {
    init_st, // Start here, stay in this state for just one tick.
    wait_st, //wait state
    draw_st, //draw state
    erase_st, //erase state
    end_st //end state
} current_state = init_st; //start in the init_st

static flashSequence_st_t previous_state;
static bool first_pass = true;

// This is a debug state print routine. It will print the names of the states each
// time tick() is called. It only prints states if they are different than the
// previous state.
void Debug_StatePrint() {
    // Only print the message if:
    // 1. This the first pass and the value for previousState is unknown.
    // 2. previousState != currentState - this prevents reprinting the same state name over and over.
    if (previous_state != current_state || first_pass) {
        first_pass = false;                // previousState will be defined, firstPass is false.
        previous_state = current_state;     // keep track of the last state that you were in.
        switch(current_state) {            // This prints messages based upon the state that you were in.
        case init_st:
            printf("init_st flashSequence\n\r"); //print init_st to console
            break;
        case wait_st:
            printf("wait_st flashSequence\n\r");//print wait_st to console
            break;
        case draw_st:
            printf("draw_st flashSequence\n\r");//print draw_st to console
            break;
        case erase_st:
            printf("erase_st flashSequence\n\r");//print erase_st to console
            break;
        case end_st:
            printf("end_st flashSequence\n\r");//print end_st to console
            break;
        }
    }
}
// Standard tick function.
void flashSequence_tick(){

    Debug_StatePrint();

    // Perform state action first
    switch(current_state) {
    case init_st: //do nothing
        break;
    case wait_st:
        waitcnt++; //increment wait cnt
        break;
    case draw_st:
        simonDisplay_drawSquare(globals_getSequenceValue(iteration),false); //draw square
        break;
    case erase_st:
        simonDisplay_drawSquare(globals_getSequenceValue(iteration),true); //erase square
        iteration++; //increment iteration
        break;
    case end_st:
        flashSequence_isComplete_flag = true; //raise complete flag
        break;
    default:
        printf("flashSequence_tick state action: hit default\n\r");
        break;
    }

    // Perform state update next
    switch(current_state){
    case init_st:
        if (flashSequence_enable_flag) //move on only if flag is up
            current_state = draw_st; //go to draw state
        break;
    case wait_st:
        if (waitcnt > WAIT_CNT_MAX){ //give some time for the user to see square
            waitcnt = RESET; //reset wait cnt
            current_state = erase_st; //go to erase state
        }
        break;
    case draw_st:
        current_state = wait_st; //go to wait state
        break;
    case erase_st:
        if (iteration >= globals_getSequenceIterationLength()){ //if the iteration is done
            current_state = end_st; //go to end state
        }
        else {
            current_state = draw_st; //go to draw state
        }
        break;
    case end_st:
        if (!flashSequence_enable_flag){ //if the flag is down
            iteration = RESET; //reset iteration counter
            flashSequence_isComplete_flag = false; //lower complete flag
            current_state = init_st; //go to init state
        }
        break;
    default:
        printf("flashSequence_tick state action: hit default\n\r");
        break;
    }
}

// This will set the sequence to a simple sequential pattern.
// It starts by flashing the first color, and then increments the index and flashes the first
// two colors and so forth. Along the way it prints info messages to the LCD screen.
#define TEST_SEQUENCE_LENGTH 8  // Just use a short test sequence.
uint8_t flashSequence_testSequence[TEST_SEQUENCE_LENGTH] = {
        SIMON_DISPLAY_REGION_0,
        SIMON_DISPLAY_REGION_1,
        SIMON_DISPLAY_REGION_2,
        SIMON_DISPLAY_REGION_3,
        SIMON_DISPLAY_REGION_3,
        SIMON_DISPLAY_REGION_2,
        SIMON_DISPLAY_REGION_1,
        SIMON_DISPLAY_REGION_0};    // Simple sequence.
#define INCREMENTING_SEQUENCE_MESSAGE1 "Incrementing Sequence"  // Info message.
#define RUN_TEST_COMPLETE_MESSAGE "Runtest() Complete"          // Info message.
#define MESSAGE_TEXT_SIZE 2     // Make the text easy to see.
#define TWO_SECONDS_IN_MS 2000  // Two second delay.
#define TICK_PERIOD 75          // 200 millisecond delay.
#define TEXT_ORIGIN_X 0                  // Text starts from far left and
#define TEXT_ORIGIN_Y (DISPLAY_HEIGHT/2) // middle of screen.

// Print the incrementing sequence message.
void flashSequence_printIncrementingMessage() {
    display_fillScreen(DISPLAY_BLACK);  // Otherwise, tell the user that you are incrementing the sequence.
    display_setCursor(TEXT_ORIGIN_X, TEXT_ORIGIN_Y);// Roughly centered.
    display_println(INCREMENTING_SEQUENCE_MESSAGE1);// Print the message.
    utils_msDelay(TWO_SECONDS_IN_MS);   // Hold on for 2 seconds.
    display_fillScreen(DISPLAY_BLACK);  // Clear the screen.
}

// Run the test: flash the sequence, one square at a time
// with helpful information messages.
void flashSequence_runTest() {
    display_init();                   // We are using the display.
    display_fillScreen(DISPLAY_BLACK);    // Clear the display.
    globals_setSequence(flashSequence_testSequence, TEST_SEQUENCE_LENGTH);    // Set the sequence.
    flashSequence_enable();             // Enable the flashSequence state machine.
    int16_t sequenceLength = 1;         // Start out with a sequence of length 1.
    globals_setSequenceIterationLength(sequenceLength);   // Set the iteration length.
    display_setTextSize(MESSAGE_TEXT_SIZE); // Use a standard text size.
    while (1) {                             // Run forever unless you break.
        flashSequence_tick();             // tick the state machine.
        utils_msDelay(TICK_PERIOD);   // Provide a 1 ms delay.
        if (flashSequence_isComplete()) {   // When you are done flashing the sequence.
            flashSequence_disable();          // Interlock by first disabling the state machine.
            flashSequence_tick();             // tick is necessary to advance the state.
            utils_msDelay(TICK_PERIOD);       // don't really need this here, just for completeness.
            flashSequence_enable();           // Finish the interlock by enabling the state machine.
            utils_msDelay(TICK_PERIOD);       // Wait 1 ms for no good reason.
            sequenceLength++;                 // Increment the length of the sequence.
            if (sequenceLength > TEST_SEQUENCE_LENGTH)  // Stop if you have done the full sequence.
                break;
            // Tell the user that you are going to the next step in the pattern.
            flashSequence_printIncrementingMessage();
            globals_setSequenceIterationLength(sequenceLength);  // Set the length of the pattern.
        }
    }
    // Let the user know that you are finished.
    display_fillScreen(DISPLAY_BLACK);              // Blank the screen.
    display_setCursor(TEXT_ORIGIN_X, TEXT_ORIGIN_Y);// Set the cursor position.
    display_println(RUN_TEST_COMPLETE_MESSAGE);     // Print the message.
}


