# Bone_EMG_ADC
Code that acquires EMG signal using a beaglebone ADC 

# Setup
1. Install the lastest debian release for the Beaglebone Black
2. Follow the C programming language steps at: 
http://users.freebasic-portal.de/tjf/Projekte/libpruio/doc/html/ChaPreparation.html
3. In /boot/uEnv.txt on the Beaglebone you are using, comment out the line assigning the PRU to the rproc driver and uncomment the one making the uio_pru the driver instead. This is slightly documented in the PRU driver section of the above link
4. Git clone this repo and start cooking
