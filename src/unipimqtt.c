#define NINPUT_GROUPS 3
#define MAX_ARMS 3
#define UNIPIMQTT_DEBUG true
#define _POSIX_C_SOURCE 199309L // nanosleep

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "armspi.h"

char *spi_devices[MAX_ARMS] =
    { "/dev/unipispi", "/dev/unipispi", "/dev/unipispi" };
int spi_speed[MAX_ARMS] = { 0, 0, 0 };

typedef struct {
        uint8_t address;
        uint8_t length;
        uint8_t slave;
        uint8_t *values;
} input_group;

// Find the next power of two for a uint8_t
// This works in the sense that we propagate the MSB to all other bits
uint8_t nextpow2(uint8_t v)
{
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v++;
        return v;
}

int add_arm(arm_handle ** armh, uint8_t index, const char *device, int speed)
{
        if (index >= MAX_ARMS) {        // Too many devices
                return -1;
        }
        arm_handle *arm = calloc(1, sizeof(arm_handle));        // Allocate and zero-init the arm_handle struct

        if (arm == NULL) {
                return -1;
        }

        if (arm_init(arm, device, speed, index) == 0) {
                armh[index] = arm;
        } else {
                free(arm);
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
        arm_handle *arm[MAX_ARMS];
        input_group input_groups[NINPUT_GROUPS];

        // init arm devices
        int ai;
        for (ai = 0; ai < MAX_ARMS; ai++) {
                arm[ai] = NULL;
                char *dev = spi_devices[ai];
                if ((dev != NULL) && (strlen(dev) > 0)) {
                        int speed = spi_speed[ai];
                        if (!(speed > 0)) {
                                speed = spi_speed[0];
                        }
                        if (add_arm(arm, ai, dev, speed)) {
                                printf("Init arm devices %d\n failed", ai);
                        }
                }
        }
        // init input groups (manual for now)
        input_groups[0].address = 0;
        input_groups[0].length = 4;
        input_groups[0].slave = 0;
        input_groups[2].address = 14;
        input_groups[2].length = 16;
        input_groups[2].slave = 2;

        input_groups[0].values =
            malloc(nextpow2(input_groups[0].length) * sizeof(uint8_t));
        if (input_groups[0].values == NULL) {
                printf("Could not set up values array\n");
                return -1;
        }
        input_groups[2].values =
            malloc(nextpow2(input_groups[2].length) * sizeof(uint8_t));
        if (input_groups[2].values == NULL) {
                printf("Could not set up values array\n");
                return -1;
        }

        // Check for group 2
        int group_n = 2;
        int nbits = input_groups[group_n].length / 8;
        nbits = nbits > 0 ? nbits : 1;
        uint8_t *response = malloc(nbits * sizeof(uint8_t *));
        if (response == NULL) {
                printf("Could not set up response array\n");
                return -1;
        }
        int ctr;
        int i, j;
        for (ctr = 0; ctr < 100; ctr++) {
                int n = read_bits(arm[input_groups[group_n].slave],
                                  input_groups[group_n].address,
                                  input_groups[group_n].length,
                                  response);
                if (n < 0) {
                        printf("Error reading response: %d\n", n);
                        return n;
                }
                printf("Response: %x %x\n", response[0], response[1]);
                // Bit shift to get individual values
                for (i = 0; i < nbits; i++) {
                        for (j = 0; j < 8; j++) {
                                input_groups[group_n].values[(8 * i) + j] =
                                    (response[i] >> j) & 1;
                        }
                }
                for (i = 0; i < input_groups[group_n].length; i++) {
                        if (response[1] != 0) {
                                printf("(%d - %d) ", i,
                                       input_groups[group_n].values[i]);
                        }
                }
                printf("\n");
                nanosleep(&((struct timespec) { 0, 250e6 }), NULL);
        }
        free(response);
        free(input_groups[0].values);
        free(input_groups[2].values);
        // free up arm handles
        if (free_arm(arm, MAX_ARMS)) {
                printf("Issue freeing up arm handles\n");
        }
        return 0;
}
