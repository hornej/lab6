/*
 * global.c
 *
 *  Created on: Oct 31, 2016
 *      Author: hornej2
 */

#include "globals.h"
#include <stdio.h>
#include <stdint.h>

#define INIT_ZERO 0
// The length of the sequence.
// The static keyword means that globals_sequenceLength can only be accessed
// by functions contained in this file.
static uint16_t globals_sequenceLength = INIT_ZERO;  // The length of the sequence.

static uint16_t globals_sequenceIterationLength;
static uint8_t global_sequence[GLOBALS_MAX_FLASH_SEQUENCE];

// This is the length of the sequence that you are currently working on,
// not the maximum length but the interim length as
// the user works through the pattern one color at a time.
void globals_setSequenceIterationLength(uint16_t length) {
    globals_sequenceIterationLength = length; //keep track of length of sequence currently working on
}

// This is the length of the complete sequence at maximum length.
// You must copy the contents of the sequence[] array into the global variable that you maintain.
// Do not just grab the pointer as this will fail.
void globals_setSequence(const uint8_t sequence[], uint16_t length){
    for(uint32_t i = INIT_ZERO; i < length; i++){ //iterate through sequence
        global_sequence[i] = sequence[i]; //copy sequence to global sequence
    }
    globals_sequenceLength = length; //set the new length
}

// This returns the value of the sequence at the index.
uint8_t globals_getSequenceValue(uint16_t index){
    return global_sequence[index];
}

// Retrieve the sequence length.
uint16_t globals_getSequenceLength(){
    return globals_sequenceLength;
}

// This is the length of the sequence that you are currently working on,
// not the maximum length but the interim length as
// the use works through the pattern one color at a time.
uint16_t globals_getSequenceIterationLength(){
    return globals_sequenceIterationLength;
}

