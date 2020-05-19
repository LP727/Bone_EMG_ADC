 /*********************************************************************************************************
 *
 * Filename    : Main.c
 * Author      : Louis-Philip Beliveau
 * Description : This program runs an ADC aquisition of EMG signals and extracts finger movements from them
 *               It runs on a beaglebone green wireless (BBGW) and uses the libpruio from : 
 *                  http://users.freebasic-portal.de/tjf/Projekte/libpruio/doc/html/ChaPreparation.html
 *               It should be compiled on the BBGW or a computer with the appropriate libraries setup on it
 *
 *********************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>

#include "libpruio/pruio.h"

//TODO: Add error codes in an enum and in function returns

#define PROBE_NB    1           //!< The number of active steps (must match setStep calls and mask).
#define ACQ_RATE_HZ 1000        //!< ADC acquisition rate in Hz
#define SEC_IN_BUF  2           //!< Number of seconds of acquisition in buffer
#define SEC_IN_DISP 4           //!< Number of seconds displayed in ADC_display test function
#define NANO_IN_SEC 1000000000  //!< Number of nanoseconds in a second

uint32_t ADC_init();
uint32_t ADC_display(pruIo *io, uint16_t *pStart, uint16_t *pMid, uint32_t chanNum);

int main(int argc, char **argv)
{
    //TODO: Move necessary in ADC_init() function

    /////////////////////////////////////////////////
    /// ... PRU ADC Config variables ...
    /////////////////////////////////////////////////
    //!< Currently going for a 1hHz acquisition, with 8 seconds of data in buffer
    const uint32_t tSamp = SEC_IN_BUF * ACQ_RATE_HZ;    //!< The number of samples in memory
    const uint32_t sRate = NANO_IN_SEC / ACQ_RATE_HZ;   //!< The sampling rate in ns (1000000 -> 1 kHz).
    //!< To keep in mind: Default maximum memory of the Eram is 256 kByte attributed by kernel and each sample is 2Bytes
    //!< Keep in mind augmenting the number of Probes will augment memory usage

    uint32_t mask = 0;                    //!< The active steps (9 to 11).
    uint32_t tInd = tSamp * PROBE_NB;     //!< The maximum total index.
    uint32_t half = ((io->ESize >> 2) / PROBE_NB) * PROBE_NB; //!< The maximum index of the half ring buffer.
    uint32_t samp = (half << 1) / PROBE_NB; //!< The number of samples (per step).
    /////////////////////////////////////////////////
    /// ... Misc variables ...
    /////////////////////////////////////////////////
    struct timespec mSec;
    mSec.tv_nsec = 1000000;
    uint32_t i;
    uint16_t *pStart;  //!< Pointer to the start of the ring buffer.
    uint16_t *pMid;     //!< Pointer to the middle of the ring buffer.
    /////////////////////////////////////////////////
    /// ... Initialization ...
    /////////////////////////////////////////////////
    pruIo *io = pruio_new(PRUIO_DEF_ACTIVE, 0, 0, 0); //! create new driver
    if (io->Errr){
               printf("Failed creating io with error: (%s)\n", io->Errr); return 1;
    }

    /////////////////////////////////////////////////
    /// ... Step config ...
    /////////////////////////////////////////////////
    for(i = 0; i < PROBE_NB; i++)
    {
        //!< NOTE: This dynamicall allocation is un-ideal and should be replace with Maccros
        uint32_t firstStep = 1; //!< The first step is step 1 as step 0 is charging step (see ARM doc)
        uint32_t firstChan = 0; //!< The first Channel is AIN0
        uint32_t Averaging = 0; //!< No averaging for now
        uint32_t SDelay = 0;    //!< No sampling delay for now
        uint32_t ODelay = 0;    //!< No open delay for now
        
        pruio_adc_setStep(io, firstStep+i, firstChan+i, Averaging, SDelay, ODelay)//!< Initiates number of steps accodring to numbe rof probes
        if (io->Err){ 
            printf("Step %d configuration failed: (%s)\n", firstStep, io->Errr); 
            return EXIT_FAILURE;
        }
    }
    
    /////////////////////////////////////////////////
    /// ... Driver config ...
    /////////////////////////////////////////////////
    for(i = 0; i < PROBE_NB; i++)
    {
        mask |= (1 << i); //!< Sets all bits corresponding to active channels
    }
    if (half > tInd){ 
        half = tInd;
        samp = (half << 1) / PROBE_NB;  //!< Adapt size of the half to the buffer size
    }
    else{
        printf("Number of sample exceeds half of buffer size, time to increase Eram size!\n"); //!< Configures the driver
        return EXIT_FAILURE;
    }

    if (pruio_config(io, samp, mask, sRate, 0)){
        printf("Config failed (%s)\n", io->Errr); //!< Configures the driver
        return EXIT_FAILURE;
    }

    if (pruio_rb_start(io)){
        printf("RB_start failed (%s)\n", io->Errr); 
        return EXIT_FAILURE;
    }
    pStart = io->Adc->Value;
    pMid = p0 + half;

    

    pruio_destroy(io); //!< Destroys driver before exit
    return EXIT_SUCCESS;
}

uint32_t ADC_init()
{
    //TODO: move init code here
    return EXIT_SUCCESS;
}

uint32_t ADC_acquisition_start()
{
    //TODO: Include code to run ADC buffering in separate thread
    //      Will require pthread and mutex
    return EXIT_SUCCESS;
}

//! ADC_display: displays the last x seconds of ADC acquisition for a single channel
/*!
  Brief: 
    This function is used to display the ADC acquisition while it is running in
    RB mode (round buffer). Currently only supports a single step acquisition
  param io:         pruIo *,    pre-initialized pruIo driver
  param pStart:     uint16_t *, pointer to the start of the ADC circular buffer
  param pMid:       uint16_t *, pointer to the middle of the ADC circular buffer
  param chanNum:    uint32_t,   number of active ADC channels 
  return: uint32_t, error code
*/
uint32_t ADC_display(pruIo *io, uint16_t *pStart, uint16_t *pMid, uint32_t chanNum)
{
    //TODO: Fill in display function
    //!< This function retrieves the latest x samples from
    //!< the ADC driver and stores and displays them in
    //!< a target rate of 10 updates for seconds.

    if(chanNum == 1)//!< Currently only supports single channel
    {

    }
    else
    {
        return EXIT_FAILURE;
    }
    

    return EXIT_SUCCESS;
}