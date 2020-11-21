#include <unistd.h>
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
        int address = 0;
        int slave = 1;
        int length = 1;
        uint8_t *response = malloc(length * sizeof(uint8_t *));
        if (response == NULL) {
                printf("Could not set up response array\n");
                return -1;
        }
        while (1) {
                int n = read_bits(arm[slave], address, length, response);
                if (n < 0) {
                        printf("Error reading response: %d\n", n);
                        return n;
                }
                printf("Response: %x\n", response[0]);
                sleep(1);
        }
        free(response);

        // free up arm handles
        return 0;
}
