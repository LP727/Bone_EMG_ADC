#include <time.h>
#include <string.h>
#include "ADC.h"

/////////////////////////////////////////////////
/// ... Stub ...
/////////////////////////////////////////////////
#if (LOCAL_COMP > 0)
pruIo*   pruio_new(uint32_t foo, uint32_t bar, uint32_t baz, uint32_t qux){pruIo *i = 0; return i;}
uint32_t pruio_adc_setStep(pruIo *foo, uint32_t bar, uint32_t baz, uint32_t qux, uint32_t quux, uint32_t corge){return EXIT_SUCCESS;}
uint32_t pruio_config(pruIo *foo, uint32_t bar, uint32_t baz, uint32_t qux, uint32_t quux){return EXIT_SUCCESS;}
uint32_t pruio_rb_start(pruIo *foo){return EXIT_SUCCESS;}
uint32_t pruio_destroy(pruIo *foo){return EXIT_SUCCESS;}
#endif

//!< Local functions
//static void swap(uint16_t **p0, uint16_t **p1);
static void ADC_acquisition(void *arg);
static void display_channel(struct adc_s *actAdc, uint32_t chan, uint32_t startIndex);
static void display_buffer();


uint16_t ADC_buffer[BUFFER_SIZE] = {0};//!< Future public buffer, will need semaphore or mutex to monitor access

//! ADC_init: Initilization of ADC driver
/*!
  Brief: 
    This function initializes the ADC driver to work at (currently hardcoded) a
    sampling rate of 1kHz, storing the values in a ring buffer holding the last
    (currently hardcoded) 2 seconds of data. The number of ADC channels and the
    rate at which the ADC will be refreshed in the program buffer is specified 
    in parameters.
  param actAdc:     struct adc_s *,    ADC struct holding pruio driver
  param chanNum:    uint32_t,   number of ADC probes that 
  param latency:    uint32_t,   time before a reading will be accessible
  return: uint32_t, error code
*/
uint32_t ADC_init(struct adc_s *actAdc, uint32_t chanNum, uint32_t latency, synch_mode_e mode)
{
    uint32_t i;

    /////////////////////////////////////////////////
    /// ... PRU ADC Config variables ...
    /////////////////////////////////////////////////
    //!< Currently going for a 1hHz acquisition, with 8 seconds of data in buffer
    actAdc->aChan = chanNum;
    actAdc->sRate = ACQ_RATE_HZ;    //!< The number of samples per seconds
    actAdc->sTime = NANO_IN_SEC / actAdc->sRate;   //!< A sample time in ns (1000000 -> 1 kHz).
    actAdc->lat = latency;
    actAdc->res = (actAdc->lat * NANO_IN_MS * actAdc->aChan)/actAdc->sTime;
    //!< To keep in mind: Default maximum memory of the Eram is 256 kByte attributed by kernel and each sample is 2Bytes
    //!< Keep in mind augmenting the number of Probes will augment memory usage
                  //!< Number of active ADC channels
    actAdc->mask = 0;
    actAdc->tInd = actAdc->sRate * actAdc->aChan;     //!< The maximum total index per second.
    actAdc->sMode = mode;

    /////////////////////////////////////////////////
    /// ... Initialization ...
    /////////////////////////////////////////////////
    actAdc->adcStp = 0;

    switch (actAdc->sMode)
    {
        case FREE:
        break;

        case DISPLAY:
            sem_init(&actAdc->adcDispSem, 0, 1);
        break;

        case EXTERNAL:
            sem_init(&actAdc->adcExtSem, 0, 1);
        break;

        default:
        break;
    }
    
    sem_init(&actAdc->acdAckSem, 0, 0);
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
    
    actAdc->half = ((actAdc->io->ESize / 4) / actAdc->aChan) * actAdc->aChan; //!< The maximum index of the half ring buffer.
    actAdc->samp = (actAdc->half * 2) / actAdc->aChan; //!< The number of samples in buffer.

    /////////////////////////////////////////////////
    /// ... Driver config ...
    /////////////////////////////////////////////////
    for(i = 0; i < actAdc->aChan; i++)
    {
        actAdc->mask |= (1 << (i+1)); //!< Sets all bits corresponding to active channels
    }
    if (actAdc->half > actAdc->tInd){ 
        actAdc->half = actAdc->tInd; //!< Adapt size of the half to the buffer size
        actAdc->samp = (actAdc->half * 2) / actAdc->aChan;  
    }
    else{
        printf("Number of sample exceeds half of buffer size, time to increase Eram size!\n");
        return EXIT_FAILURE;
    }

    if (pruio_config(actAdc->io, actAdc->samp, actAdc->mask, actAdc->sTime, 0)){
        printf("Config failed (%s)\n", actAdc->io->Errr); //!< Configures the driver
        return EXIT_FAILURE;
    }

    if (pruio_rb_start(actAdc->io)){
        printf("RB_start failed (%s)\n", actAdc->io->Errr); 
        return EXIT_FAILURE;
    }
    actAdc->pStart = actAdc->io->Adc->Value;            //!< Pointer to the start of the ring buffer.
    actAdc->pTrack = actAdc->pStart;                    //!< Pointer to track position in ring buffer.
    actAdc->pEnd = actAdc->pStart + (actAdc->half * 2);//!< Pointer to the end of the ring buffer.
    return EXIT_SUCCESS;
}

uint32_t ADC_acquisition_start(struct adc_s *actAdc)
{
    int iRet;
    iRet = pthread_mutex_init(&actAdc->bufMtx,NULL);
    if(iRet){
        printf("Buffer mutex init failed with error %d\n", iRet);
        return EXIT_FAILURE;
    }
    iRet = pthread_mutex_init(&actAdc->stpMtx,NULL);
    if(iRet){
        printf("Stop mutex init failed with error %d\n", iRet);
        return EXIT_FAILURE;
    }
    iRet = pthread_create(&actAdc->adcThread, NULL, (void*) &ADC_acquisition, (void *)actAdc);
    if(iRet){
        printf("ADC thread creation failed with error %d\n", iRet);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

uint32_t ADC_acquisition_stop(struct adc_s *actAdc)
{
    int iRet;

    iRet = pthread_mutex_lock(&actAdc->stpMtx);
    if(iRet){
        printf("Failed to lock stop mutex with error %d\n", iRet);
        return EXIT_FAILURE;
    }
    else{
        actAdc->adcStp = 1;
        pthread_mutex_unlock(&actAdc->stpMtx);
        switch (actAdc->sMode)
        {
        case FREE:
        break;

        case DISPLAY:
            sem_post(&actAdc->adcDispSem);
        break;

        case EXTERNAL:
            sem_post(&actAdc->adcExtSem);
        break;

        default:
        break;
        }
        pthread_join(actAdc->adcThread, NULL);
    }
    
    return EXIT_SUCCESS;
}

//! ADC_display: displays the last x seconds of ADC acquisition for a single channel
/*!
  Brief: 
    This function is used to display the ADC acquisition while it is running in
    RB mode (round buffer). Currently only supports a single step acquisition
  param io:  struct adc_s *,    pre-initialized ADC struct holding pruio driver
  param dispRate: uint32_t, dependent

  return: uint32_t, error code
*/
uint32_t ADC_display(struct adc_s *actAdc)
{
    //!< This function retrieves the latest x samples from
    //!< the ADC driver and stores and displays them in
    //!< a target rate of 10 updates for seconds.
    uint32_t startInd = 0;
    uint32_t i;
    uint32_t k = 0;
  
    if(DUMP_BUFFER)
    {
        display_buffer();
	return EXIT_SUCCESS;
    }
    printf("Starting dislay:\n");
    for(i = 0; i < actAdc->aChan; i++)
    {
        printf("\n");
    }
    while(k < 10 * (MS_IN_SEC/actAdc->lat))//!< Stops after 10 seconds TODO: Add proper stop condition
    {
        sem_wait(&actAdc->acdAckSem);
        for(i = 0; i < actAdc->aChan; i++)
        {
            display_channel(actAdc, i, startInd);
        }
        
        startInd = (startInd+actAdc->res) % BUFFER_SIZE;
        k++;
	    sem_post(&actAdc->adcDispSem);
    }
    
    printf("\n");
    return EXIT_SUCCESS;
}

void ADC_destroy(struct adc_s *actAdc)
{
    //!< Destroys initialized semaphores
    switch (actAdc->sMode)
    {
        case FREE:
        break;

        case DISPLAY:
            sem_destroy(&actAdc->adcDispSem);
        break;

        case EXTERNAL:
            sem_destroy(&actAdc->adcExtSem);
        break;

        default:
        break;
    }
    
    //!< Destroys mutexes
    pthread_mutex_destroy(&actAdc->stpMtx);
    pthread_mutex_destroy(&actAdc->bufMtx);

    pruio_destroy(actAdc->io); //!< Destroys driver before exit
}

// TODO: completely remove if pointer swap juged unnecessary
//static void swap(uint16_t **p0, uint16_t **p1)
//{
//    uint16_t *swap = *p0;
//    *p0 = *p1;
//    *p1 = swap;
//}

//! ADC_acquisition: stores ADC acquisition in the ADC buffer
/*!
  Brief: 
    This function is used to store the ADC acquisition of the last SEC_IN_BUF
    seconds of ADC acquisition. It is meant to be run in a background pthread 
    as it contains an infitine loop. The ADC must be initialized in a RB mode
    for this to work properly. The loop posts a semaphore at a rate defined at
    initialization. 

  param arg:    void *, pre-initialized ADC struct holding pruio driver

  return: none
*/
static void ADC_acquisition(void *arg)
{
    uint32_t ackInd = 0;
    uint32_t bufInd = 0;
    uint32_t run = 1;
    struct adc_s * actAdc = (struct adc_s *)arg;
    struct timespec mSec;
    mSec.tv_nsec = NANO_IN_MS;

    while(run)
    {
        switch (actAdc->sMode)
        {
        case FREE:
        break;

        case DISPLAY:
            sem_wait(&actAdc->adcDispSem);
        break;

        case EXTERNAL:
            sem_wait(&actAdc->adcExtSem);
        break;

        default:
        break;
        }

        
        actAdc->pTarget = actAdc->pTrack + actAdc->res;//!< increment of pointer
        if(actAdc->pTarget >= actAdc->pEnd)
        {
            actAdc->pTarget = actAdc->pStart; //!< For circularity
        }
        if(actAdc->pTarget > actAdc->pTrack){
            while(actAdc->io->DRam[0] < ackInd) nanosleep(&mSec, NULL);
        }
        else{
            while(actAdc->io->DRam[0] > actAdc->res) nanosleep(&mSec, NULL); //!< For circularity
        }
        
        if(!pthread_mutex_lock(&actAdc->bufMtx)){
            memcpy(&ADC_buffer[bufInd], actAdc->pTrack, actAdc->res*2);
            pthread_mutex_unlock(&actAdc->bufMtx);
        }
        actAdc->pTrack = actAdc->pTarget;
        ackInd = (ackInd+actAdc->res) % (actAdc->half*2);
	    bufInd = (bufInd+actAdc->res) % BUFFER_SIZE;
	
	    //!< Check for stop condition on every end of loop
        if(!pthread_mutex_trylock(&actAdc->stpMtx)){
            if(actAdc->adcStp == 1){
                run = 0;
            }
            pthread_mutex_unlock(&actAdc->stpMtx);
        }	    
        //printf("%d",actAdc->io->DRam[0]); //!< NOTE: left as means of sanity check, remove when necessary
	    sem_post(&actAdc->acdAckSem);
    }

    pthread_exit(NULL);
}

static void display_channel(struct adc_s *actAdc, uint32_t chan, uint32_t startIndex)
{
    int32_t dispInd = 0;
    uint32_t i;
    double displayRatio = BUFFER_SIZE/actAdc->res / MAX_ON_SCREEN; //!< time resolution compared to on screen display capability 
    
    if(chan == 0)
    {
        for(i = chan; i < actAdc->aChan; i++)
        {
            printf("\033[F");
        }
    }
    printf("%d|",chan);
    for(i = 1; i < MAX_ON_SCREEN; i++)
    {
        dispInd = i * actAdc->res * displayRatio; //!< TODO: Implement mean values instead of downsampling if fast enough
        dispInd = ((dispInd + startIndex + chan) % BUFFER_SIZE);
        if(!pthread_mutex_lock(&actAdc->bufMtx)){
            printf("%4.0f:",ADC_buffer[dispInd]/ADC_TO_MV); //!< adjust index to move the display as new data comes in
            pthread_mutex_unlock(&actAdc->bufMtx);
        }
    }
    printf("\n");
    fflush(stdout);
}

static void display_buffer()
{
    int i;
    printf("Buffer content starts below:\n");
    for(i = 0; i< BUFFER_SIZE; i++)
    {
        if(!(i%PROBE_NB))
	{
        printf("\n%d:",(i/2)+1);
	}
        printf("%d;", ADC_buffer[i]);
    }
    printf("\n");
}
