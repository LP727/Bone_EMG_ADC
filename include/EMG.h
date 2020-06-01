 /*********************************************************************************************************
 *
 * Filename    : EMG.h
 * Author      : Louis-Philip Beliveau
 * Description : This file holds all functions to define the EMG translation of the ADC acquisition.
 *               It is meant as an interface between the ADC input and the PWM output.
 *
 *********************************************************************************************************/
#ifndef EMG_H_
#define EMG_H_

#include "ADC.h"

/////////////////////////////////////////////////
/// ... OUTPUT structs and enum ...
/////////////////////////////////////////////////

typedef enum {
    CLOSE,
    OPEN,
    HOLD
} direction_e;

struct axis_s //!< defines behavior of a servo in an axis
{
    direction_e dir;    //!< Is the servo opening or closing the hand
    uint32_t magnitude; //!< how fast to move the servo in the appropriate direction
};

struct finger_s
{
    uint32_t thumb; //!< Defines if the finger is the thumb
    struct axis_s grip_axis; //!< gripping direction and force
    struct axis_s lat_axis; //!< only the thumb will uses this one (for now...)
};

/////////////////////////////////////////////////
/// ... INPUT structs and enum ...
/////////////////////////////////////////////////

struct muscle_s
{
    uint32_t EMG_reading;
};

/////////////////////////////////////////////////
/// ... MAIN struct ...
/////////////////////////////////////////////////

//!< NOTE: currently implemented for a single finger
struct emg_s
{
    // **** Output variables **** //
    struct finger_s index;
    struct finger_s middle;
    struct finger_s ring;
    struct finger_s pinky;
    struct finger_s thumb;

    // **** Input variables **** //
    struct muscle_s muscle[PROBE_NB]; //!< NOTE: Currently just an array of EMG placement, further knowledge will improve identification of what muscle to read
};

//!< Public functions
void EMG_decode_movement(struct emg_s* emg, struct adc_s *adc, void *pwm);

#endif