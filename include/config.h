 /*********************************************************************************************************
 *
 * Filename    : config.h
 * Author      : Louis-Philip Beliveau
 * Description : This file holds all the configuration to define the parameters of operation for the ADC, 
 *               PWM, display and EMG acquisition. Setting LOCAL_COMP 0  allows to compile the program on
 *               a computer that isn't the BBGW.
 *
 *********************************************************************************************************/

#ifndef CONFIG_H_
#define CONFIG_H_

#define LOCAL_COMP 0 //!< Controls activation of stubs for compiling on local (not on BBGW)


/////////////////////////////////////////////////
/// ... ADC CONFIG ...
/////////////////////////////////////////////////
#define DUMP_BUFFER 0           //!< When put to 1, the function ADC display will display the ADC buffer content then stop
#define PROBE_NB    1           //!< The number of active steps (must match setStep calls and mask).
#define ACQ_RATE_HZ 1000        //!< ADC acquisition rate in Hz
#define SEC_IN_BUF  4           //!< Number of seconds of acquisition in buffer
#define NANO_IN_SEC 1000000000  //!< Number of nanoseconds in a second
#define NANO_IN_MS  1000000     //!< Number of nanoseconds in a milisecond
#define MS_IN_SEC   1000        //!< Number of milliseconds in a second
#define BUFFER_SIZE (SEC_IN_BUF * ACQ_RATE_HZ * PROBE_NB)
#define ADC_TO_MV   2.275       //!< Convertion factor to go from adc (16bits) to ref voltage (1800mV)
#define LATENCY_TARGET  50      //!< Target time between ADC acquisition and accessibility of data
#define MAX_ON_SCREEN   40      //!< The maximum number of values on a single line on screen (in our display format)

/////////////////////////////////////////////////
/// ... PWM CONFIG ...
/////////////////////////////////////////////////
//!< TODO: implement PWM


#endif