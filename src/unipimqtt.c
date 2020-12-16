#define NINPUT_GROUPS 3
#define MAX_ARMS 3
#define UNIPIMQTT_DEBUG true
#define _POSIX_C_SOURCE 199309L // nanosleep
#define MAX_VALUE_LENGTH 30     // Max length of values to query

#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "armspi.h"
#include "armutil.h"

static char *spi_devices[MAX_ARMS] =
    { "/dev/unipispi", "/dev/unipispi", "/dev/unipispi" };
static int spi_speed[MAX_ARMS] = { 0, 0, 0 };

static arm_handle *arm[MAX_ARMS];       // TODO: do we want this global?

typedef struct {
        unsigned char address;
        unsigned char length;
        unsigned char slave;
        unsigned char *values;
} input_group;

// millis gives the current time in milliseconds
long millis()
{
        struct timespec _t;
        clock_gettime(CLOCK_REALTIME, &_t);
        return _t.tv_sec * 1000 + lround(_t.tv_nsec / 1.0e6);
}

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

// Unwrap response array into values array individual values
void unwrap(unsigned char *values, uint8_t * response, unsigned char length)
{
        int nbits = length / 8;
        nbits = nbits > 0 ? nbits : 1;
        for (int i = 0; i < nbits; i++) {
                for (int j = 0; j < 8; j++) {
                        values[(i * 8) + j] = (response[i] >> j) & 1;
                }
        }
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

        for (int i = 0; i < n_arms; i++) {
                free(armh[i]);
        }
        return 0;
}

void *read_group(void *void_grp)
{
        input_group *grp = (input_group *) void_grp;

        int nbits = (int)ceil(grp->length / 8.0);
        uint8_t *response = malloc(nbits * sizeof(uint8_t *));
        if (response == NULL) {
                printf("Could not set up response array\n");
                return NULL;
                //return -1; // TODO: proper error handling
        }
        unsigned long reg = 0;
        long counters[grp->length];
        // init counters to zero
        for (int i; i < grp->length; i++) {
                counters[i] = 0;
        }
        long prevtime = 0;
        long delta = 0;
        for (int ctr = 0; ctr < 100; ctr++) {
                int n = read_bits(arm[grp->slave], grp->address, grp->length,
                                  response);
                if (n < 0) {
                        printf("Error reading response: %d\n", n);
                        return NULL;
                        //return n;
                }
                //printf("Response: %x %x %x %x\n", response[0], response[1],
                //       response[2], response[3]);
                //unwrap(grp->values, response, grp->length);
                //for (int i = 0; i < grp->length; i++) {
                //        if (response[0] != 0 || response[1] != 0) {
                //                printf("(%d - %d) ", i, grp->values[i]);
                //        }
                //}
                //printf("\n");
                unsigned long result = 0;
                for (int i = 0; i < nbits; i++) {
                        result |= response[i] << (i * 8);
                }
                //printf("%lu\n", result);
                unsigned char prev, cur = 0;
                delta = millis() - prevtime;
                //if (result != reg) {
                for (int i = 0; i < 8 * sizeof(unsigned long); i++) {
                        prev = (reg >> i) & 0x1;
                        cur = (result >> i) & 0x1;
                        if (cur != 0) {
                                counters[i] += delta;
                                printf("CNT: %d - %li\n", i, counters[i]);
                        }
                        if (prev != cur) {
                                printf("%d -- %d %d %li\n", i, prev,
                                       cur, counters[i]);
                                // reset counter
                                counters[i] = 0;
                        }
                }
                //}
                reg = result;
                prevtime = millis();
                nanosleep(&((struct timespec) { 0, 250e6 }), NULL);
        }
        free(response);
        return NULL;
}

int main()
{
        input_group input_groups[NINPUT_GROUPS];

        // init arm devices
        for (int ai = 0; ai < MAX_ARMS; ai++) {
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

        // Dump names
        for (int i = 0; i < MAX_ARMS; i++) {
                printf("Device %d - %s\n", i, arm_name(arm[i]->bv.hw_version));
        }

        // init input groups (manual for now)
        input_groups[0].address = 0;
        input_groups[0].length = 4;
        input_groups[0].slave = 0;
        input_groups[1].address = 0;    // 14 for inputs
        input_groups[1].length = 30;
        input_groups[1].slave = 1;
        input_groups[2].address = 0;    // 14 for inputs
        input_groups[2].length = 30;
        input_groups[2].slave = 2;

        for (int group_n = 0; group_n < NINPUT_GROUPS; group_n++) {
                input_groups[group_n].values =
                    malloc(nextpow2(input_groups[group_n].length) *
                           sizeof(uint8_t));
                if (input_groups[group_n].values == NULL) {
                        printf("Could not set up values array\n");
                        return -1;
                }
        }

        // Check for group 2
        // Thread
        pthread_t poll_thread[NINPUT_GROUPS];
        for (int group_n = 0; group_n < NINPUT_GROUPS; group_n++) {
                // Split into threads
                if (pthread_create
                    (&poll_thread[group_n], NULL, read_group,
                     &input_groups[group_n])) {
                        fprintf(stderr, "Error creating thread\n");
                        return -1;
                }
        }
        // Join all thread again
        for (int group_n = 0; group_n < NINPUT_GROUPS; group_n++) {
                // Wait for all 3 to have finished
                if (pthread_join(poll_thread[group_n], NULL)) {
                        fprintf(stderr, "Error joining thread\n");
                        return -1;
                }
        }

        for (int group_n = 0; group_n < NINPUT_GROUPS; group_n++) {
                free(input_groups[group_n].values);
        }
        // free up arm handles
        if (free_arm(arm, MAX_ARMS)) {
                printf("Issue freeing up arm handles\n");
        }
        return 0;
}
