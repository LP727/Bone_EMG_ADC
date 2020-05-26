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
/////////////////////////////////////////////////
/// ... DEBUG define ...
/////////////////////////////////////////////////

      

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

//!< If 1, activates stub for functions relying on Pruio
#define LOCAL_COMP 0

#if (LOCAL_COMP < 1)
    #include "libpruio/pruio.h"
#endif

//TODO: Add error codes in an enum and in function returns

#define PROBE_NB    1           //!< The number of active steps (must match setStep calls and mask).
#define ACQ_RATE_HZ 1000        //!< ADC acquisition rate in Hz
#define SEC_IN_BUF  4           //!< Number of seconds of acquisition in buffer
#define NANO_IN_SEC 1000000000  //!< Number of nanoseconds in a second
#define BUFFER_SIZE (SEC_IN_BUF * ACQ_RATE_HZ)
#define ADC_TO_MV   2.275       //!< Convertion factor to go from adc (12bits) to ref voltage (1800mV)

/////////////////////////////////////////////////
/// ... Stub ...
/////////////////////////////////////////////////
#if (LOCAL_COMP > 0)

#define PRUIO_DEF_ACTIVE 1
//!< Stubs to allow compiling without the Beaglebone
//!< VERY IMPORTANT: MUST NOT BE ACTIVE WHEN RUNNING ON BEAGLEBONE
typedef struct ADC_s_
{
    uint16_t *Value;
}ADC_s;
typedef struct pruIos
{
    uint32_t    ESize;
    char        *Errr;
    uint32_t    DRam[10];
    ADC_s       *Adc;
} pruIo;

pruIo*   pruio_new(uint32_t foo, uint32_t bar, uint32_t baz, uint32_t qux){pruIo *i = 0; return i;}
uint32_t pruio_adc_setStep(pruIo *foo, uint32_t bar, uint32_t baz, uint32_t qux, uint32_t quux, uint32_t corge){return EXIT_SUCCESS;}
uint32_t pruio_config(pruIo *foo, uint32_t bar, uint32_t baz, uint32_t qux, uint32_t quux){return EXIT_SUCCESS;}
uint32_t pruio_rb_start(pruIo *foo){return EXIT_SUCCESS;}
uint32_t pruio_destroy(pruIo *foo){return EXIT_SUCCESS;}
#endif

struct adc_s
{
    pruIo *io;      //!< The driver
    uint32_t aChan; //!< The numbe rof active channels
    uint32_t tSamp; //!< The number of samples in memory
    uint32_t sRate; //!< The sampling rate in ns (1000000 -> 1 kHz).
    uint32_t mask;  //!< The active steps (9 to 11).
    uint32_t tInd;  //!< The maximum total index.
    uint32_t half;  //!< The maximum index of the half ring buffer.
    uint32_t samp;  //!< The number of samples (per step).
    uint16_t *pStart;//!< Pointer to the start of the ring buffer.
    uint16_t *pMid;  //!< Pointer to the middle of the ring buffer.
};

//!< Future public functions
uint32_t ADC_init(struct adc_s *actAdc, uint32_t chanNum);
uint32_t ADC_acquisition_start(struct adc_s *actAdc);
uint32_t ADC_display(struct adc_s *actAdc);

void ADC_destroy(struct adc_s *actAdc);

//!< Future local functions
static void swap(uint16_t **p0, uint16_t **p1);


uint16_t ADC_buffer[BUFFER_SIZE] = {0};//!< Future public buffer, will need semaphore or mutex to monitor access
static struct adc_s aAdc; //!< Move buffer in there eventually

int main(int argc, char **argv)
{
    if(ADC_init(&aAdc, PROBE_NB)){
        printf("ADC init failed\n");
        return EXIT_FAILURE;
    }

    
    ADC_display(&aAdc);

    ADC_destroy(&aAdc);

    return EXIT_SUCCESS;
}

uint32_t ADC_init(struct adc_s *actAdc, uint32_t chanNum)
{
    /////////////////////////////////////////////////
    /// ... Misc variables ...
    /////////////////////////////////////////////////
    uint32_t i;

    /////////////////////////////////////////////////
    /// ... PRU ADC Config variables ...
    /////////////////////////////////////////////////
    //!< Currently going for a 1hHz acquisition, with 8 seconds of data in buffer
    actAdc->tSamp = ACQ_RATE_HZ;    //!< The number of samples in memory
    actAdc->sRate = NANO_IN_SEC / ACQ_RATE_HZ;   //!< The sampling rate in ns (1000000 -> 1 kHz).
    //!< To keep in mind: Default maximum memory of the Eram is 256 kByte attributed by kernel and each sample is 2Bytes
    //!< Keep in mind augmenting the number of Probes will augment memory usage
    actAdc->aChan = chanNum;              //!< Number of active ADC channels
    actAdc->mask = 0;                     //!< The active steps (9 to 11).
    actAdc->tInd = actAdc->tSamp * actAdc->aChan;     //!< The maximum total index.


    /////////////////////////////////////////////////
    /// ... Initialization ...
    /////////////////////////////////////////////////
    actAdc->io = pruio_new(PRUIO_DEF_ACTIVE, 0, 0, 0); //!< create new driver
    if (actAdc->io->Errr){
               printf("Failed creating io with error: (%s)\n", actAdc->io->Errr);
               return EXIT_FAILURE;
    }
    
    /////////////////////////////////////////////////
    /// ... Step config ...
    /////////////////////////////////////////////////
    for(i = 0; i < actAdc->aChan; i++)
    {
        //!< NOTE: This dynamicall allocation is un-ideal and should be replace with Maccros
        uint32_t firstStep = 1; //!< The first step is step 1 as step 0 is charging step (see ARM doc)
        uint32_t firstChan = 0; //!< The first Channel is AIN0
        uint32_t Averaging = 0; //!< No averaging for now
        uint32_t SDelay = 0;    //!< No sampling delay for now
        uint32_t ODelay = 0;    //!< No open delay for now
        
        pruio_adc_setStep(actAdc->io, firstStep+i, firstChan+i, Averaging, SDelay, ODelay);//!< Initiates number of steps accodring to numbe rof probes
        if(actAdc->io->Errr){ 
            printf("Step %d configuration failed: (%s)\n", firstStep, actAdc->io->Errr); 
            return EXIT_FAILURE;
        }
    }
    
    actAdc->half = ((actAdc->io->ESize >> 2) / actAdc->aChan) * actAdc->aChan; //!< The maximum index of the half ring buffer.
    actAdc->samp = (actAdc->half << 1) / actAdc->aChan; //!< The number of samples (per step).
    /////////////////////////////////////////////////
    /// ... Driver config ...
    /////////////////////////////////////////////////
    for(i = 0; i < actAdc->aChan; i++)
    {
        actAdc->mask |= (1 << (i+1)); //!< Sets all bits corresponding to active channels
    }
    if (actAdc->half > actAdc->tInd){ 
        actAdc->half = actAdc->tInd;
        actAdc->samp = (actAdc->half << 1) / actAdc->aChan;  //!< Adapt size of the half to the buffer size
    }
    else{
        printf("Number of sample exceeds half of buffer size, time to increase Eram size!\n");
        return EXIT_FAILURE;
    }

    if (pruio_config(actAdc->io, actAdc->samp, actAdc->mask, actAdc->sRate, 0)){
        printf("Config failed (%s)\n", actAdc->io->Errr); //!< Configures the driver
        return EXIT_FAILURE;
    }

    if (pruio_rb_start(actAdc->io)){
        printf("RB_start failed (%s)\n", actAdc->io->Errr); 
        return EXIT_FAILURE;
    }
    actAdc->pStart = actAdc->io->Adc->Value;//!< Pointer to the start of the ring buffer.
    actAdc->pMid = actAdc->pStart + actAdc->half;       //!< Pointer to the middle of the ring buffer.

    return EXIT_SUCCESS;
}

uint32_t ADC_acquisition_start(struct adc_s *actAdc)
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
uint32_t ADC_display(struct adc_s *actAdc)
{
    //!< This function retrieves the latest x samples from
    //!< the ADC driver and stores and displays them in
    //!< a target rate of 10 updates for seconds.
    uint32_t secInd = 0;
    int32_t dispInd = 0;
    uint32_t i;
    uint32_t j = 0;
    struct timespec mSec;
    mSec.tv_nsec = 1000000;

    printf("Starting dislay:\n");
    if(actAdc->aChan == 1)//!< Currently only supports single channel
    {
        while(j < 10)//!< Stops after 10 seconds TODO: Add proper stop condition
        {
            //!< Fetch new ADC data
            if(actAdc->pStart > actAdc->pMid){
                while(actAdc->io->DRam[0] < actAdc->half) nanosleep(&mSec, NULL);
            }
            else{
                while(actAdc->io->DRam[0] > actAdc->half) nanosleep(&mSec, NULL);
            }
            memcpy(&ADC_buffer[(secInd * ACQ_RATE_HZ)], actAdc->pStart, ACQ_RATE_HZ*2); //!< Since copy is made once per second, uses acquisition rate

            //!< Very rudimentary display of 4 last second of ADC acquisition
            printf("\r");
            for(i = 0; i < BUFFER_SIZE/100; i++)//!< Replace hardcoded value
            {
                dispInd = (i * 100);
                dispInd = (dispInd + (((secInd+1) % SEC_IN_BUF) * ACQ_RATE_HZ)) % BUFFER_SIZE;
                printf("%4.0f|",ADC_buffer[dispInd]/ADC_TO_MV); //!< adjust index to move the display as new data comes in
            }
            fflush(stdout);
            swap(&actAdc->pStart,&actAdc->pMid);
            secInd = (secInd+1) % SEC_IN_BUF;
            j++;
        }
    }
    else
    {
        return EXIT_FAILURE;
    }
    
    printf("\n");
    return EXIT_SUCCESS;
}

void ADC_destroy(struct adc_s *actAdc)
{
    pruio_destroy(actAdc->io); //!< Destroys driver before exit
}

static void swap(uint16_t **p0, uint16_t **p1)
{
    uint16_t *swap = *p0;
    *p0 = *p1;
    *p1 = swap;
}
