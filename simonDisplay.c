/*
 * simonDisplay.c
 *
 *  Created on: Oct 31, 2016
 *      Author: hornej2
 */

#include "simonDisplay.h"
#include <stdio.h>
#include <stdint.h>
#include "supportFiles/display.h"
#include "supportFiles/utils.h"
#include "../interval_timers/buttons.h"

#define TOUCH_PANEL_ANALOG_PROCESSING_DELAY_IN_MS 60 // in ms
#define MAX_STR 255
#define TEXT_SIZE 2
#define TEXT_VERTICAL_POSITION 0
#define TEXT_HORIZONTAL_POSITION (DISPLAY_HEIGHT/2)
#define INSTRUCTION_LINE_1 "Touch and release to start the Simon demo."
#define INSTRUCTION_LINE_2 "Demo will terminate after %d touches."
#define DEMO_OVER_MESSAGE_LINE_1 "Simon demo terminated"
#define DEMO_OVER_MESSAGE_LINE_2 "after %d touches."
#define TEXT_VERTICAL_POSITION 0 // Start at the far left.
#define ERASE_THE_SQUARE true  // drawSquare() erases if this is passed in.
#define DRAW_THE_SQUARE false  // drawSquare() draws the square if this is passed in.

// Width, height of the simon "buttons"
#define SIMON_DISPLAY_BUTTON_WIDTH 60 //button width
#define SIMON_DISPLAY_BUTTON_HEIGHT 60 //button height
#define HALF_BUTTON_HEIGHT_WIDTH 30 //half button width
// WIdth, height of the simon "squares.
// Note that the video shows the squares as larger but you
// can use this smaller value to make the game easier to implement speed-wise.
#define SIMON_DISPLAY_SQUARE_WIDTH  120
#define SIMON_DISPLAY_SQUARE_HEIGHT 120

// Given coordinates from the touch pad, computes the region number.

// The entire touch-screen is divided into 4 rectangular regions, numbered 0 - 3.
// Each region will be drawn with a different color. Colored buttons remind
// the user which square is associated with each color. When you press
// a region, computeRegionNumber returns the region number that is used
// by the other routines.
/*
|----------|----------|
|          |          |
|    0     |     1    |
|  (RED)   | (YELLOW) |
-----------------------
|          |          |
|     2    |    3     |
|  (BLUE)  |  (GREEN) |
-----------------------
 */

// These are the definitions for the regions.
#define SIMON_DISPLAY_REGION_0 0
#define SIMON_DISPLAY_REGION_1 1
#define SIMON_DISPLAY_REGION_2 2
#define SIMON_DISPLAY_REGION_3 3

#define DISPLAY_HALF_WIDTH DISPLAY_WIDTH/2 //half width of display
#define DISPLAY_HALF_HEIGHT DISPLAY_HEIGHT/2 //half heigh of display
#define DISPLAY_SIXTH_HEIGHT DISPLAY_HEIGHT/6 //one sixth the display height
#define DISPLAY_SIXTH_WIDTH DISPLAY_WIDTH/6 //one sixth the display width
#define DISPLAY_FIFTH_EIGHTH_HEIGHT DISPLAY_HEIGHT/8*5 //5/8ths the display height
#define DISPLAY_FIFTH_EIGHTH_WIDTH DISPLAY_WIDTH/8*5 //5/8ths the display width

int8_t simonDisplay_computeRegionNumber(int16_t x, int16_t y){
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

// Draws a colored "button" that the user can touch.
// The colored button is centered in the region but does not fill the region.
void simonDisplay_drawButton(uint8_t regionNumber){
    if (regionNumber == SIMON_DISPLAY_REGION_0){
        //draw region 0 button
        display_fillRect(DISPLAY_SIXTH_WIDTH,DISPLAY_SIXTH_HEIGHT,SIMON_DISPLAY_BUTTON_WIDTH,SIMON_DISPLAY_BUTTON_HEIGHT,DISPLAY_RED);
    }
    //draw region 1 button
    if (regionNumber == SIMON_DISPLAY_REGION_1){
        display_fillRect(DISPLAY_FIFTH_EIGHTH_WIDTH,DISPLAY_SIXTH_HEIGHT,SIMON_DISPLAY_BUTTON_WIDTH,SIMON_DISPLAY_BUTTON_HEIGHT,DISPLAY_YELLOW);
    }
    //draw region 2 button
    if (regionNumber == SIMON_DISPLAY_REGION_2){
        display_fillRect(DISPLAY_SIXTH_WIDTH,DISPLAY_FIFTH_EIGHTH_HEIGHT,SIMON_DISPLAY_BUTTON_WIDTH,SIMON_DISPLAY_BUTTON_HEIGHT,DISPLAY_BLUE);
    }
    //draw region 3 button
    if (regionNumber == SIMON_DISPLAY_REGION_3){
        display_fillRect(DISPLAY_FIFTH_EIGHTH_WIDTH,DISPLAY_FIFTH_EIGHTH_HEIGHT,SIMON_DISPLAY_BUTTON_WIDTH,SIMON_DISPLAY_BUTTON_HEIGHT,DISPLAY_GREEN);
    }
}

// Convenience function that draws all of the buttons.
void simonDisplay_drawAllButtons(){
    //draw region 0
    display_fillRect(DISPLAY_SIXTH_WIDTH,DISPLAY_SIXTH_HEIGHT,SIMON_DISPLAY_BUTTON_WIDTH,SIMON_DISPLAY_BUTTON_HEIGHT,DISPLAY_RED);
    //draw region 1
    display_fillRect(DISPLAY_FIFTH_EIGHTH_WIDTH,DISPLAY_SIXTH_HEIGHT,SIMON_DISPLAY_BUTTON_WIDTH,SIMON_DISPLAY_BUTTON_HEIGHT,DISPLAY_YELLOW);
    //draw region 2
    display_fillRect(DISPLAY_SIXTH_WIDTH,DISPLAY_FIFTH_EIGHTH_HEIGHT,SIMON_DISPLAY_BUTTON_WIDTH,SIMON_DISPLAY_BUTTON_HEIGHT,DISPLAY_BLUE);
    //draw region 3
    display_fillRect(DISPLAY_FIFTH_EIGHTH_WIDTH,DISPLAY_FIFTH_EIGHTH_HEIGHT,SIMON_DISPLAY_BUTTON_WIDTH,SIMON_DISPLAY_BUTTON_HEIGHT,DISPLAY_GREEN);
}

// Convenience function that erases all of the buttons.
void simonDisplay_eraseAllButtons(){
    //draw black region 0
    display_fillRect(DISPLAY_SIXTH_WIDTH,DISPLAY_SIXTH_HEIGHT,SIMON_DISPLAY_BUTTON_WIDTH,SIMON_DISPLAY_BUTTON_HEIGHT,DISPLAY_BLACK);
    //draw black region 1
    display_fillRect(DISPLAY_FIFTH_EIGHTH_WIDTH,DISPLAY_SIXTH_HEIGHT,SIMON_DISPLAY_BUTTON_WIDTH,SIMON_DISPLAY_BUTTON_HEIGHT,DISPLAY_BLACK);
    //draw black region 2
    display_fillRect(DISPLAY_SIXTH_WIDTH,DISPLAY_FIFTH_EIGHTH_HEIGHT,SIMON_DISPLAY_BUTTON_WIDTH,SIMON_DISPLAY_BUTTON_HEIGHT,DISPLAY_BLACK);
    //draw black region 3
    display_fillRect(DISPLAY_FIFTH_EIGHTH_WIDTH,DISPLAY_FIFTH_EIGHTH_HEIGHT,SIMON_DISPLAY_BUTTON_WIDTH,SIMON_DISPLAY_BUTTON_HEIGHT,DISPLAY_BLACK);
}

// Draws a bigger square that completely fills the region.
// If the erase argument is true, it draws the square as black background to "erase" it.
void simonDisplay_drawSquare(uint8_t regionNo, bool erase){
    if (regionNo == SIMON_DISPLAY_REGION_0 && !erase){ //draw button on region 0
        display_fillRect(DISPLAY_SIXTH_WIDTH-HALF_BUTTON_HEIGHT_WIDTH,DISPLAY_SIXTH_HEIGHT-HALF_BUTTON_HEIGHT_WIDTH,SIMON_DISPLAY_SQUARE_WIDTH,SIMON_DISPLAY_SQUARE_HEIGHT,DISPLAY_RED);
    }
    if (regionNo == SIMON_DISPLAY_REGION_0 && erase){ //erase button on region 0
        display_fillRect(DISPLAY_SIXTH_WIDTH-HALF_BUTTON_HEIGHT_WIDTH,DISPLAY_SIXTH_HEIGHT-HALF_BUTTON_HEIGHT_WIDTH,SIMON_DISPLAY_SQUARE_WIDTH,SIMON_DISPLAY_SQUARE_HEIGHT,DISPLAY_BLACK);
    }
    if (regionNo == SIMON_DISPLAY_REGION_1 && !erase){ //draw button on region 1
        display_fillRect(DISPLAY_FIFTH_EIGHTH_WIDTH-HALF_BUTTON_HEIGHT_WIDTH,DISPLAY_SIXTH_HEIGHT-HALF_BUTTON_HEIGHT_WIDTH,SIMON_DISPLAY_SQUARE_WIDTH,SIMON_DISPLAY_SQUARE_HEIGHT,DISPLAY_YELLOW);
    }
    if (regionNo == SIMON_DISPLAY_REGION_1 && erase){ //erase button on region 1
        display_fillRect(DISPLAY_FIFTH_EIGHTH_WIDTH-HALF_BUTTON_HEIGHT_WIDTH,DISPLAY_SIXTH_HEIGHT-HALF_BUTTON_HEIGHT_WIDTH,SIMON_DISPLAY_SQUARE_WIDTH,SIMON_DISPLAY_SQUARE_HEIGHT,DISPLAY_BLACK);
    }
    if (regionNo == SIMON_DISPLAY_REGION_2 && !erase){ //draw button on region 2
        display_fillRect(DISPLAY_SIXTH_WIDTH-HALF_BUTTON_HEIGHT_WIDTH,DISPLAY_FIFTH_EIGHTH_HEIGHT-HALF_BUTTON_HEIGHT_WIDTH,SIMON_DISPLAY_SQUARE_WIDTH,SIMON_DISPLAY_SQUARE_HEIGHT,DISPLAY_BLUE);
    }
    if (regionNo == SIMON_DISPLAY_REGION_2 && erase){ //erase button on region 2
        display_fillRect(DISPLAY_SIXTH_WIDTH-HALF_BUTTON_HEIGHT_WIDTH,DISPLAY_FIFTH_EIGHTH_HEIGHT-HALF_BUTTON_HEIGHT_WIDTH,SIMON_DISPLAY_SQUARE_WIDTH,SIMON_DISPLAY_SQUARE_HEIGHT,DISPLAY_BLACK);
    }
    if (regionNo == SIMON_DISPLAY_REGION_3 && !erase){ //draw button on region 3
        display_fillRect(DISPLAY_FIFTH_EIGHTH_WIDTH-HALF_BUTTON_HEIGHT_WIDTH,DISPLAY_FIFTH_EIGHTH_HEIGHT-HALF_BUTTON_HEIGHT_WIDTH,SIMON_DISPLAY_SQUARE_WIDTH,SIMON_DISPLAY_SQUARE_HEIGHT,DISPLAY_GREEN);
    }
    if (regionNo == SIMON_DISPLAY_REGION_3 && erase){ //erase button on region 3
        display_fillRect(DISPLAY_FIFTH_EIGHTH_WIDTH-HALF_BUTTON_HEIGHT_WIDTH,DISPLAY_FIFTH_EIGHTH_HEIGHT-HALF_BUTTON_HEIGHT_WIDTH,SIMON_DISPLAY_SQUARE_WIDTH,SIMON_DISPLAY_SQUARE_HEIGHT,DISPLAY_BLACK);
    }
}

// Runs a brief demonstration of how buttons can be pressed and squares lit up to implement the user
// interface of the Simon game. The routine will continue to run until the touchCount has been reached, e.g.,
// the user has touched the pad touchCount times.

// I used a busy-wait delay (utils_msDelay) that uses a for-loop and just blocks until the time has passed.
// When you implement the game, you CANNOT use this function as we discussed in class. Implement the delay
// using the non-blocking state-machine approach discussed in class.
void simonDisplay_runTest(uint16_t touchCount) {
    display_init();         // Always initialize the display.
    char str[MAX_STR];      // Enough for some simple printing.
    uint8_t regionNumber = 0;   // Convenience variable.
    uint16_t touches = 0;   // Terminate when you receive so many touches.
    // Write an informational message and wait for the user to touch the LCD.
    display_fillScreen(DISPLAY_BLACK);              // clear the screen.
    display_setCursor(TEXT_VERTICAL_POSITION, TEXT_HORIZONTAL_POSITION); // move to the middle of the screen.
    display_setTextSize(TEXT_SIZE);                 // Set the text size for the instructions.
    display_setTextColor(DISPLAY_RED, DISPLAY_BLACK);   // Reasonable text color.
    sprintf(str, INSTRUCTION_LINE_1);                   // Copy the line to a buffer.
    display_println(str);                               // Print to the LCD.
    display_println();                                  // new-line.
    sprintf(str, INSTRUCTION_LINE_2, touchCount);       // Copy the line to a buffer.
    display_println(str);                               // Print to the LCD.
    while (!display_isTouched());       // Wait here until the screen is touched.
    while (display_isTouched());        // Now wait until the touch is released.
    display_fillScreen(DISPLAY_BLACK);  // Clear the screen.
    simonDisplay_drawAllButtons();      // Draw all of the buttons.
    bool touched = false;         // Keep track of when the pad is touched.
    int16_t x, y;                     // Use these to keep track of coordinates.
    uint8_t z;                        // This is the relative touch pressure.
    while (touches < touchCount) {  // Run the loop according to the number of touches passed in.
        if (!display_isTouched() && touched) {          // user has stopped touching the pad.
            simonDisplay_drawSquare(regionNumber, ERASE_THE_SQUARE);  // Erase the square.
            simonDisplay_drawButton(regionNumber);        // DISPLAY_REDraw the button.
            touched = false;                  // Released the touch, set touched to false.
        } else if (display_isTouched() && !touched) {   // User started touching the pad.
            touched = true;                             // Just touched the pad, set touched = true.
            touches++;                                  // Keep track of the number of touches.
            display_clearOldTouchData();                // Get rid of data from previous touches.
            // Must wait this many milliseconds for the chip to do analog processing.
            utils_msDelay(TOUCH_PANEL_ANALOG_PROCESSING_DELAY_IN_MS);
            display_getTouchedPoint(&x, &y, &z);        // After the wait, get the touched point.
            regionNumber = simonDisplay_computeRegionNumber(x, y);// Compute the region number, see above.
            simonDisplay_drawSquare(regionNumber, DRAW_THE_SQUARE);  // Draw the square (erase = false).
        }
    }
    // Done with the demo, write an informational message to the user.
    display_fillScreen(DISPLAY_BLACK);        // clear the screen.
    // Place the cursor in the middle of the screen.
    display_setCursor(TEXT_VERTICAL_POSITION, TEXT_HORIZONTAL_POSITION);
    display_setTextSize(TEXT_SIZE); // Make it readable.
    display_setTextColor(DISPLAY_RED, DISPLAY_BLACK);  // red is foreground color, black is background color.
    sprintf(str, DEMO_OVER_MESSAGE_LINE_1);    // Format a string using sprintf.
    display_println(str);                     // Print it to the LCD.
    sprintf(str, DEMO_OVER_MESSAGE_LINE_2, touchCount);  // Format the rest of the string.
    display_println(str);  // Print it to the LCD.
}




