#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "armspi.h"
#define MAX_ARMS 3

char *spi_devices[MAX_ARMS] =
    { "/dev/unipispi", "/dev/unipispi", "/dev/unipispi" };
int spi_speed[MAX_ARMS] = { 0, 0, 0 };

int do_check_fw = 0;

int add_arm(arm_handle * armh, uint8_t index, const char *device, int speed)
{
        if (index >= MAX_ARMS) {        // Too many devices
                return -1;
        }
        arm_handle *arm = calloc(1, sizeof(arm_handle));        // Allocate and zero-init the arm_handle struct

        if (arm == NULL) {
                return -1;
        }

        if (arm_init(arm, device, speed, index) == 0) {
                armh[index] = *arm;
        } else {
                free(arm);
                return -1;
        }
        return 0;
}

int main()
{
        arm_handle *arm[MAX_ARMS];

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
                        add_arm(*arm, ai, dev, speed);
                }
        }
        printf("Finished init\n");
        return 0;
}
