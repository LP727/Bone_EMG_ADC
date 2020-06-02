 /*********************************************************************************************************
 *
 * Filename    : main.c
 * Author      : Louis-Philip Beliveau
 * Description : This program runs an ADC aquisition of EMG signals and extracts finger movements from them
 *               It runs on a beaglebone green wireless (BBGW) and uses the libpruio from : 
 *                  http://users.freebasic-portal.de/tjf/Projekte/libpruio/doc/html/ChaPreparation.html
 *               It should be compiled on the BBGW or a computer with the appropriate libraries setup on it
 *
 *********************************************************************************************************/
#include <signal.h>
#include "EMG.h"

static struct emg_s aEmg;
static struct adc_s aAdc;

static void sig_handler(int sigNo);
static uint32_t init(uint32_t chanNum, uint32_t latency, synch_mode_e mode);
static uint32_t cleanup();

int main(int argc, char **argv)
{

    if(init(PROBE_NB, LATENCY_TARGET, DISPLAY)){
        return EXIT_SUCCESS; //!< Failed to init ADC, exits
    }

    EMG_decode_movement(&aEmg, &aAdc, NULL); //!< Currently PWM is not implemented

    cleanup();

    return EXIT_SUCCESS;
}

// signal handler to handle Ctrl+C
static void sig_handler(int sigNo) 
{
    cleanup();
    exit(sigNo);
}

static uint32_t init(uint32_t chanNum, uint32_t latency, synch_mode_e mode)
{
    // set up signal handler to handle Ctrl+C
    signal(SIGINT, sig_handler);

    if(ADC_init(&aAdc, chanNum, latency, mode)){
        printf("ADC init failed\n");
        return EXIT_FAILURE;
    }

    if(ADC_acquisition_start(&aAdc))
    {
        printf("ADC start failed\n");
        return EXIT_FAILURE;      
    }

    return EXIT_SUCCESS;
}

static uint32_t cleanup()
{
    if(ADC_acquisition_stop(&aAdc))
    {
        printf("ADC stop failed\n");
        return EXIT_FAILURE;      
    }
    ADC_destroy(&aAdc);

    return EXIT_SUCCESS;
}