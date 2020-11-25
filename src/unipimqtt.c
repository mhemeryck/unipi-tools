#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "armspi.h"
#define MAX_ARMS 3
#define UNIPIMQTT_DEBUG true

char *spi_devices[MAX_ARMS] =
    { "/dev/unipispi", "/dev/unipispi", "/dev/unipispi" };
int spi_speed[MAX_ARMS] = { 0, 0, 0 };

int add_arm(arm_handle ** armh, uint8_t index, const char *device, int speed)
{
        if (index >= MAX_ARMS) {        // Too many devices
                return -1;
        }
        arm_handle *arm = calloc(1, sizeof(arm_handle));        // Allocate and zero-init the arm_handle struct

        if (arm == NULL) {
                return -1;
        }

        printf("Start arm init\n");
        if (arm_init(arm, device, speed, index) == 0) {
                printf("arm init passed\n");
                armh[index] = arm;
        } else {
                printf("arm init failed\n");
                free(arm);
                printf("arm init failed, free passed\n");
                return -1;
        }
        return 0;
}

int free_arm(arm_handle ** armh, int n_arms)
{
        if (armh == NULL) {
                return -1;
        }

        int i;
        for (i = 0; i < n_arms; i++) {
                free(armh[i]);
        }
        return 0;
}

int main()
{
        printf("Start init\n");
        arm_handle *arm[MAX_ARMS];

        // init arm devices
        int ai;
        for (ai = 0; ai < MAX_ARMS; ai++) {
                arm[ai] = NULL;
                char *dev = spi_devices[ai];
                if ((dev != NULL) && (strlen(dev) > 0)) {
                        int speed = spi_speed[ai];
                        printf("Speed check passed\n");
                        if (!(speed > 0)) {
                                speed = spi_speed[0];
                        }
                        if (add_arm(arm, ai, dev, speed) < 0) {
                                printf("Init arm devices %d\n failed", ai);
                        }
                }
        }
        printf("Finished init\n");
        // read discrete bits
        //int address = 1004;
        //int slave = 2;
        //int length = 1;
        //uint16_t *response = malloc(1 * sizeof(uint16_t *));
        int address = 14;
        int slave = 2;
        int length = 16;
        uint8_t *response = malloc(2 * sizeof(uint8_t *));
        if (response == NULL) {
                printf("Could not set up response array\n");
                return -1;
        }
        int counter = 0;
        while (counter < 10) {
                int n = read_bits(arm[slave], address, length, response);
                //int n = read_regs(arm[slave], address, length, response);
                if (n < 0) {
                        printf("Error reading response: %d\n", n);
                        return n;
                }
                //printf("Response: %d\n", response[0]);
                printf("Response: %d - %d\n", response[0], response[1]);
                struct timespec ts = { 0, 250e6 };
                nanosleep(&ts, NULL);
                counter++;
        }
        free(response);
        if (free_arm(arm, MAX_ARMS) != 0) {
		printf("Issue freeing up arm handles\n");
        }
        // free up arm handles
        return 0;
}
