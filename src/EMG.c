 /*********************************************************************************************************
 *
 * Filename    : EMG.c
 * Author      : Louis-Philip Beliveau
 * Description : This file holds all functions to define the EMG translation of the ADC acquisition.
 *               It is meant as an interface between the ADC input and the PWM output.
 *
 *********************************************************************************************************/

#include "EMG.h"

//!< Local functions
static void EMG_ack_muscles(struct emg_s* emg, struct adc_s *adc);
static void EMG_emit_movement(struct emg_s* emg, void *pwm);

//! EMG_decode_movement: Analyses ADC inputs and outputs PWM values
/*!
  Brief: 
    This function reads the ADC buffer, decodes the movements from ADC readings
    and outputs the PWM values to support the finger movements.
    
  param emg: struct emg_s *, interface between ADC input and PWM output
  param adc: struct adc_s *, pre-initialized ADC struct with started acquisition thread
  param pwm: struct pwm_s *, currently void as unused TODO: implement PWM 

  return: none
*/
void EMG_decode_movement(struct emg_s *emg, struct adc_s *adc, void *pwm)
{
    EMG_ack_muscles(emg, adc);
    EMG_emit_movement(emg, pwm);
}


//! EMG_ack_muscles: Analyses ADC inputs and deduces muscle action
/*!
  Brief: 
    This function reads the ADC buffer and computes what muscle action is 
    done by the user and encodes it in the EMG struct.
    NOTE: Currently only act as a display to test EMG values around the arm.
    
  param emg: struct emg_s *, interface between ADC input and PWM output
  param adc: struct adc_s *, pre-initialized ADC struct with started acquisition thread

  return: none
*/
static void EMG_ack_muscles(struct emg_s* emg, struct adc_s *adc)
{
    ADC_display(adc); //!< TODO: replace ADC_display by appropriate decoding
}

//! EMG_emit_movement: Determines the PWM output from the EMG values
/*!
  Brief: 
    This function uses EMG decoded movements and encodes it in PWM actions
    
  param emg: struct emg_s *, interface between ADC input and PWM output
  param pwm: struct pwm_s *, currently void as unused TODO: implement PWM 

  return: none
*/
static void EMG_emit_movement(struct emg_s* emg, void *pwm)
{
    //!< TODO: IMPLEMENT PWM
}