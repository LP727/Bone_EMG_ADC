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

#include "ADC.h"

struct adc_s aAdc; //!< Move buffer in there eventually

int main(int argc, char **argv)
{
    if(ADC_init(&aAdc, PROBE_NB, LATENCY_TARGET, DISPLAY)){
        printf("ADC init failed\n");
        return EXIT_FAILURE;
    }

    ADC_acquisition_start(&aAdc);

    ADC_display(&aAdc);

    ADC_acquisition_stop(&aAdc);

    ADC_destroy(&aAdc);

    return EXIT_SUCCESS;
}
