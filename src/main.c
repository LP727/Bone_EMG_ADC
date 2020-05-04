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
#include <pthread.h>

#include "libpruio/pruio.h"

int main(int argc, char **argv)
{
    printf("\nHello BBGW world!\n");
    return 0;
}
