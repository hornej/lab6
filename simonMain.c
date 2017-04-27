/*
 * simonMain.c
 *
 *  Created on: Oct 31, 2016
 *      Author: hornej2
 */

#include "simonDisplay.h"
#include "buttonHandler.h"
#include "verifySequence.h"
#include "flashSequence.h"

//#define RUN_SIMON_DISPLAY_TEST
#ifdef RUN_SIMON_DISPLAY_TEST
#define MAX_TOUCH_COUNT 10
int main() {
  simonDisplay_runTest(MAX_TOUCH_COUNT);
}
#endif

//#define RUN_BUTTON_HANDLER_TEST
#ifdef RUN_BUTTON_HANDLER_TEST
#define MAX_TOUCH_COUNT 10
int main() {
  buttonHandler_runTest(MAX_TOUCH_COUNT);
}
#endif

#define RUN_FLASH_SEQUENCE_TEST
#ifdef RUN_FLASH_SEQUENCE_TEST
int main() {
  flashSequence_runTest();
}
#endif

//#define RUN_VERIFY_SEQUENCE_TEST
#ifdef RUN_VERIFY_SEQUENCE_TEST
int main() {
  verifySequence_runTest();
}
#endif


void isr_function() {
    // Empty for now.
}
