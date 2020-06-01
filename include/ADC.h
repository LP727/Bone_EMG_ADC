#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "config.h"

#if (LOCAL_COMP < 1)
    #include "libpruio/pruio.h"
#endif

//TODO: Add error codes in an enum and in function returns

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


#endif

typedef enum {
    FREE,       //!< ADC acquisition will run freely with synchronizing  
    DISPLAY,    //!< ADC acquisition will synchronize itself with the display
    EXTERNAL    //!< ADC acquisition will be syncrhonized with a semaphore defined in another file
} synch_mode_e;

struct adc_s
{
    // **** Driver **** //
    pruIo *io;          //!< The driver

    // **** Config variables **** //
    uint32_t aChan;     //!< The numbe rof active channels
    uint32_t sRate;     //!< The sampling rate in Hz
    uint32_t sTime;     //!< The time of a sample ns (1000000 -> 1 kHz).
    uint32_t mask;      //!< The active steps (9 to 11).
    uint32_t tInd;      //!< The maximum total index.
    uint32_t half;      //!< The maximum index of the half ring buffer.
    uint32_t samp;      //!< The number of samples in the ring buffer
    uint32_t lat;       //!< The target latency (ms) between acquisition and storage
    uint32_t res;       //!< The number of sample per unit of latency time
    synch_mode_e sMode; //!< Defines what your acquisition will synch on

    // **** Ring buffer pointer **** //
    uint16_t *pStart;   //!< Pointer to the start of the ring buffer.
    uint16_t *pTrack;   //!< Pointer to track position the ring buffer.
    uint16_t *pTarget;  //!< Pointer to define next increment of pTrack.
    uint16_t *pEnd;     //!< Pointer to the end of the ring buffer.

    // **** Thread and sync **** //
    pthread_t adcThread;    //!< ADC background thread
    pthread_mutex_t bufMtx; //!< mutex for buffer access
    pthread_mutex_t stpMtx; //!< mutex for stop of adcThread
    sem_t adcDispSem;        //!< semaphore to syncrhonise end of acquisition with display
    sem_t acdAckSem;        //!< semaphore to syncrhonise end of acquisition with display
    sem_t adcExtSem;
    uint32_t adcStp;        //!< value to send stop signal
};

//!< Public buffer
extern uint16_t ADC_buffer[BUFFER_SIZE]; //!< Public buffer, needs mutex synchronization to access

//!< Public functions
uint32_t ADC_init(struct adc_s *actAdc, uint32_t chanNum, uint32_t latency, synch_mode_e mode);
uint32_t ADC_acquisition_start(struct adc_s *actAdc);
uint32_t ADC_acquisition_stop(struct adc_s *actAdc);
uint32_t ADC_display(struct adc_s *actAdc);
void ADC_destroy(struct adc_s *actAdc);

#endif