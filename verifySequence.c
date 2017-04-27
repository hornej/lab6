/*
 * verifySequence.c
 *
 *  Created on: Oct 31, 2016
 *      Author: hornej2
 */

#include <stdio.h>
#include <stdint.h>
#include "globals.h"
#include "buttonHandler.h"
#include "verifySequence.h"
#include "supportFiles/display.h"
#include "../interval_timers/buttons.h"
#include "simonDisplay.h"
#include "supportFiles/utils.h"
#include <stdbool.h>

#define CNT_MAX 30 //max cnt
#define INDEX_START -1
#define RESET 0
#define ONE 1 //used for adding one

uint8_t cnt; //counter
bool verifySequence_enable_flag; //tick enable flag
bool verifySequence_isTimeOutError_flag; //flag to see if the player took too long
bool verifySequence_isUserInputError_flag; //flag to see if the player pressed wrong square
bool verifySequence_isComplete_flag; //flag to see if the sequence is done
bool middleError_flag = false; //set the middle error flag
uint16_t index = INDEX_START; //init index
uint8_t Region; //saves region number

// State machine will run when enabled.
void verifySequence_enable(){
    verifySequence_enable_flag = true;
}

// This is part of the interlock. You disable the state-machine and then enable it again.
void verifySequence_disable(){
    verifySequence_enable_flag = false;
}

// Used to detect if there has been a time-out error.
bool verifySequence_isTimeOutError(){
    return verifySequence_isTimeOutError_flag;
}

// Used to detect if the user tapped the incorrect sequence.
bool verifySequence_isUserInputError(){
    return verifySequence_isUserInputError_flag;
}

// Used to detect if the verifySequence state machine has finished verifying.
bool verifySequence_isComplete(){
    return verifySequence_isComplete_flag;
}

// States for the controller state machine.
enum verifySequence_st_t {
    init_st, // Start here, stay in this state for just one tick.
    wait_for_touch_st, //wait for touch state
    touch_st, //touch state
    end_st //end state
} CurrentState = init_st; //start in the init_st

static verifySequence_st_t PreviousState;
static bool firstPass = true;

// This is a debug state print routine. It will print the names of the states each
// time tick() is called. It only prints states if they are different than the
// previous state.
void DebugStatePrint() {
    // Only print the message if:
    // 1. This the first pass and the value for previousState is unknown.
    // 2. previousState != currentState - this prevents reprinting the same state name over and over.
    if (PreviousState != CurrentState || firstPass) {
        firstPass = false;                // previousState will be defined, firstPass is false.
        PreviousState = CurrentState;     // keep track of the last state that you were in.
        switch(CurrentState) {            // This prints messages based upon the state that you were in.
        case init_st:
            printf("init_st verifySequence\n\r"); //print init_st to console
            break;
        case wait_for_touch_st:
            printf("wait_for_touch_st verifySequence\n\r");//print wait_for_touch_st to console
            break;
        case touch_st:
            printf("touch_st verifySequence\n\r");//print touch_st to console
            break;
        case end_st:
            printf("end_st verifySequence\n\r"); //print end_st to console
            break;
        }
    }
}

// Standard tick function.
void verifySequence_tick() {
    DebugStatePrint();

    // Perform state action first
    switch(CurrentState) {
    case init_st:
        buttonHandler_enable(); //enable button handler
        index = INDEX_START; //initialize index
        cnt = RESET; //reset cnt
        break;
    case wait_for_touch_st:
        cnt++; //increment cnt
        buttonHandler_enable(); //enable button handler
        break;
    case touch_st:
        index++; //increment index
        buttonHandler_disable(); //disble button handler
        break;
    case end_st:
        verifySequence_isTimeOutError_flag = false; //reset time out error flag
        break;
    default:
        printf("verifySequence_tick state action: hit default\n\r");
        break;
    }

    // Perform state update next
    switch(CurrentState) {
    case init_st:
        if (verifySequence_enable_flag) //if the flag is enabled continue state machine
            CurrentState = wait_for_touch_st; //go to wait for touch state
        else
            CurrentState = init_st; //stay in init state
        break;
    case wait_for_touch_st:
        if(cnt > CNT_MAX){ //if user takes too long
            verifySequence_isTimeOutError_flag = true; //time out flag goes up
            verifySequence_isComplete_flag = true; //completed game
            cnt = RESET; //reset cnt
            buttonHandler_disable(); //disble button handler
            CurrentState = end_st; //go to end state
        }
        else if(!buttonHandler_releaseDetected()){ //if no release
            CurrentState = wait_for_touch_st; //stay in wait for touch
        }
        else if(buttonHandler_releaseDetected()){ //if release detected
            cnt = RESET; //reset cnt
            Region = buttonHandler_getRegionNumber(); //get region
            simonDisplay_drawSquare(Region,false); //draw square in that region
            CurrentState = touch_st; //go to touch state
        }
        break;
    case touch_st:
        //if the player pressed the correct square and the sequence is not over
        if (globals_getSequenceValue(index)==Region&&index<globals_getSequenceIterationLength()){
            if (!middleError_flag){ //if they have not previously messed up
                verifySequence_isUserInputError_flag = false; //no mistake has been made
            }
            CurrentState = wait_for_touch_st; //go to wait for touch state
        }
        if (globals_getSequenceValue(index)!=Region){ //if the player presses incorrect square
            verifySequence_isUserInputError_flag = true; //raise input error flag
            middleError_flag = true; //middle error flag is raised
            CurrentState = end_st; //go to end state
        }
        if (index+ONE==globals_getSequenceIterationLength()){ //if the sequence is over
            verifySequence_isComplete_flag = true; //the sequence is complete
            CurrentState = end_st; //go to end state
        }
        break;
    case end_st:
    buttonHandler_enable(); //enable button handler
    //if the sequence is complete or there was no error
    if (verifySequence_isComplete()||!verifySequence_isUserInputError()){
            verifySequence_isComplete_flag = false; //reset completed flag
            middleError_flag = false; //reset middle error flag
            CurrentState = init_st; //go to init state
        }
        else if (buttons_read() & BUTTONS_BTN0_MASK){ //if btn0 is pressed
            CurrentState = init_st; //go to init state
        }
        else {
            CurrentState = wait_for_touch_st; //if not go back to wait for touch state. keep playing
        }
        break;
    default:
        printf("verifySequence_tick state update: hit default\n\r");
        break;
    }
}

#define MESSAGE_X 0
//#define MESSAGE_Y (display_width()/4)
#define MESSAGE_Y (display_height()/2)
#define MESSAGE_TEXT_SIZE 2
//#define MESSAGE_STARTING_OVER
#define BUTTON_0 0  // Index for button 0
#define BUTTON_1 1  // Index for button 1
#define BUTTON_2 2  // Index for button 2
#define BUTTON_3 3  // Index for button 3
// Prints the instructions that the user should follow when
// testing the verifySequence state machine.
// Takes an argument that specifies the length of the sequence so that
// the instructions are tailored for the length of the sequence.
// This assumes a simple incrementing pattern so that it is simple to
// instruct the user.
void verifySequence_printInstructions(uint8_t length, bool startingOver) {
    display_fillScreen(DISPLAY_BLACK);              // Clear the screen.
    display_setTextSize(MESSAGE_TEXT_SIZE);     // Make it readable.
    display_setCursor(MESSAGE_X, MESSAGE_Y);    // Rough center.
    if (startingOver) {                                             // Print a message if you start over.
        display_fillScreen(DISPLAY_BLACK);          // Clear the screen if starting over.
        display_setTextColor(DISPLAY_WHITE);        // Print whit text.
        display_println("Starting Over. ");         // Starting over message.
    }
    // Print messages are self-explanatory, no comments needed.
    // These messages request that the user touch the buttons in a specific sequence.
    display_println("Tap: ");
    display_println();
    switch (length) {
    case 1:
        display_println("red");
        break;
    case 2:
        display_println("red, yellow ");
        break;
    case 3:
        display_println("red, yellow, blue ");
        break;
    case 4:
        display_println("red, yellow, blue, green ");
        break;
    default:
        break;
    }
    display_println("in that order.");
    display_println();
    display_println("hold BTN0 to quit.");
}

// Just clears the screen and draws the four buttons used in Simon.
void verifySequence_drawButtons() {
    display_fillScreen(DISPLAY_BLACK);  // Clear the screen.
    simonDisplay_drawButton(BUTTON_0);  // Draw the four buttons.
    simonDisplay_drawButton(BUTTON_1);
    simonDisplay_drawButton(BUTTON_2);
    simonDisplay_drawButton(BUTTON_3);
}

// This will set the sequence to a simple sequential pattern.
#define MAX_TEST_SEQUENCE_LENGTH 4  // the maximum length of the pattern
uint8_t verifySequence_testSequence[MAX_TEST_SEQUENCE_LENGTH] = {0, 1, 2, 3};  // A simple pattern.
#define MESSAGE_WAIT_MS 4000  // Display messages for this long.

// Increment the sequence length making sure to skip over 0.
// Used to change the sequence length during the test.
int16_t incrementSequenceLength(int16_t sequenceLength) {
    int16_t value = (sequenceLength + 1) % (MAX_TEST_SEQUENCE_LENGTH+1);
    if (value == 0) value++;
    return value;
}

// Used to select from a variety of informational messages.
enum verifySequence_infoMessage_t {
    user_time_out_e,            // means that the user waited too long to tap a color.
    user_wrong_sequence_e,      // means that the user tapped the wrong color.
    user_correct_sequence_e,    // means that the user tapped the correct sequence.
    user_quit_e                 // means that the user wants to quite.
};

// Prints out informational messages based upon a message type (see above).
void verifySequence_printInfoMessage(verifySequence_infoMessage_t messageType) {
    // Setup text color, position and clear the screen.
    display_setTextColor(DISPLAY_WHITE);
    display_setCursor(MESSAGE_X, MESSAGE_Y);
    display_fillScreen(DISPLAY_BLACK);
    switch(messageType) {
    case user_time_out_e:  // Tell the user that they typed too slowly.
        display_println("Error:");
        display_println();
        display_println("  User tapped sequence");
        display_println("  too slowly.");
        break;
    case user_wrong_sequence_e:  // Tell the user that they tapped the wrong color.
        display_println("Error: ");
        display_println();
        display_println("  User tapped the");
        display_println("  wrong sequence.");
        break;
    case user_correct_sequence_e:  // Tell the user that they were correct.
        display_println("User tapped");
        display_println("the correct sequence.");
        break;
    case user_quit_e:             // Acknowledge that you are quitting the test.
        display_println("quitting runTest().");
        break;
    default:
        break;
    }
}

#define TICK_PERIOD_IN_MS 100
// Tests the verifySequence state machine.
// It prints instructions to the touch-screen. The user responds by tapping the
// correct colors to match the sequence.
// Users can test the error conditions by waiting too long to tap a color or
// by tapping an incorrect color.
void verifySequence_runTest() {
    display_init();  // Always must do this.
    buttons_init();  // Need to use the push-button package so user can quit.
    int16_t sequenceLength = 1;  // Start out with a sequence length of 1.
    verifySequence_printInstructions(sequenceLength, false);  // Tell the user what to do.
    utils_msDelay(MESSAGE_WAIT_MS);  // Give them a few seconds to read the instructions.
    verifySequence_drawButtons();    // Now, draw the buttons.
    // Set the test sequence and it's length.
    globals_setSequence(verifySequence_testSequence, MAX_TEST_SEQUENCE_LENGTH);
    globals_setSequenceIterationLength(sequenceLength);
    // Enable the verifySequence state machine.
    verifySequence_enable();  // Everything is interlocked, so first enable the machine.
    // Need to hold button until it quits as you might be stuck in a delay.
    while (!(buttons_read() & BUTTONS_BTN0_MASK)) {
        // verifySequence uses the buttonHandler state machine so you need to "tick" both of them.
        verifySequence_tick();  // Advance the verifySequence state machine.
        buttonHandler_tick();   // Advance the buttonHandler state machine.
        utils_msDelay(TICK_PERIOD_IN_MS);       // Wait for a tick period.
        // If the verifySequence state machine has finished, check the result,
        // otherwise just keep ticking both machines.
        if (verifySequence_isComplete()) {
            if (verifySequence_isTimeOutError()) {                // Was the user too slow?
                verifySequence_printInfoMessage(user_time_out_e); // Yes, tell the user that they were too slow.
            } else if (verifySequence_isUserInputError()) {       // Did the user tap the wrong color?
                verifySequence_printInfoMessage(user_wrong_sequence_e); // Yes, tell them so.
            } else {
                verifySequence_printInfoMessage(user_correct_sequence_e); // User was correct if you get here.
            }
            utils_msDelay(MESSAGE_WAIT_MS);                            // Allow the user to read the message.
            sequenceLength = incrementSequenceLength(sequenceLength);  // Increment the sequence.
            globals_setSequenceIterationLength(sequenceLength);        // Set the length for the verifySequence state machine.
            verifySequence_printInstructions(sequenceLength, true);    // Print the instructions.
            utils_msDelay(MESSAGE_WAIT_MS);                            // Let the user read the instructions.
            verifySequence_drawButtons();                              // Draw the buttons.
            verifySequence_disable();                                  // Interlock: first step of handshake.
            verifySequence_tick();                                     // Advance the verifySequence machine.
            utils_msDelay(TICK_PERIOD_IN_MS);                          // Wait for tick period.
            verifySequence_enable();                                   // Interlock: second step of handshake.
            utils_msDelay(TICK_PERIOD_IN_MS);                          // Wait for tick period.
        }
    }
    verifySequence_printInfoMessage(user_quit_e);  // Quitting, print out an informational message.
}



