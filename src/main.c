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

#define PROBE_NB    1

int main(int argc, char **argv)
{
    /////////////////////////////////////////////////
    /// ... PRU ADC Config variables ...
    /////////////////////////////////////////////////
    //TODO: verify: tSamp, sRate, mask and half(buffer separation will probably vary)
    const uint32_t tSamp = 123401;      //!< The number of samples in the files (per step).
    const uint32_t sRate = 20000;       //!< The sampling rate in ns (20000 -> 50 kHz).
    const uint32_t NoStep = PROBE_NB;   //!< The number of active steps (must match setStep calls and mask).

    uint32_t mask = 7 << 9;             //!< The active steps (9 to 11).
    uint32_t tInd = tSamp * NoStep;     //!< The maximum total index.
    uint32_t half = ((io->ESize >> 2) / NoStep) * NoStep; //!< The maximum index of the half ring buffer.
    
    /////////////////////////////////////////////////
    /// ... Misc variables ...
    /////////////////////////////////////////////////
    struct timespec mSec;
    mSec.tv_nsec = 1000000;
    uint32_t i;

    pruIo *io = pruio_new(PRUIO_DEF_ACTIVE, 0, 0, 0); //! create new driver
    if (io->Errr){
               printf("Failed creating io with error: (%s)\n", io->Errr); return 1;
    }

    /////////////////////////////////////////////////
    /// ... Step config ...
    /////////////////////////////////////////////////
    for(i = 0; i < )
    {
        uint32_t firstStep = 1; //!< The first step is step 1 as step 0 is charging step (see ARM doc)
        uint32_t firstChan = 0; //!< The first Channel is AIN0
        uint32_t Averaging = 0; //!< No averaging for now
        uint32_t SDelay = 0;    //!< No sampling delay for now
        uint32_t ODelay = 0;    //!< No open delay for now
        
        pruio_adc_setStep(io, firstStep, firstChan, Averaging, SDelay, ODelay)//!< Initiates number of steps accodring to numbe rof probes
        if (io->Err){ 
            printf("step 9 configuration failed: (%s)\n", io->Errr); 
            return EXIT_FAILURE;
        }
    }
    
    /////////////////////////////////////////////////
    /// ... Driver config ...
    /////////////////////////////////////////////////

    //TODO: write driver config

    return EXIT_SUCCESS;
}
