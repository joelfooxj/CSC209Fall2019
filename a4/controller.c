#include <stdio.h>
#include "controller.h"
/* Checks if the message has a valid type. The gateway should only receive
 * HANDSHAKE or UPDATE messages from sensors
 */
int is_valid_type(struct cignal *cig) {
    if ((cig->hdr.type == HANDSHAKE) || (cig->hdr.type == UPDATE)) {
        return 1;
    }
    return -1;
}

/* Returns 1 if the gateway seen this device before?
 */

// Not sure how this check works.... 
int is_registered(int id, int *device_record) {
if (device_record[id - LOWEST_ID] == 1) {
        return 1;
    }
    return -1;
}

/* Add a new device to the device_record.  Return the new device id.
 * Note that device ids will never be "de-registered" so they cannot be reused.
 */
int register_device(int *device_record) {
    for (int i = 0; i < MAXDEV; i++) {
        if (device_record[i] == 0) {
            device_record[i] = 1;
            return i + LOWEST_ID;
        }
    }
    return -1;
}

/* Turns on or off the cooler or dehumidifier based on the
 * current temperature or humidity.j
 */
void adjust_fan(struct cignal *cig) {
    cig->hdr.type = 3;
    if (cig->hdr.device_type == TEMPERATURE) {
        if (cig->value >= TEMPSET) {
            cig->cooler = ON;
        } else {
            cig->cooler = OFF;
        }
    } else if (cig->hdr.device_type == HUMIDITY) {
        if (cig->value >= HUMSET) {
            cig->dehumid = ON;
        } else {
            cig->dehumid = OFF;
        }
    }
}


/* Check each field of the incoming header to ensure that it is valid, print
 * information process about the state of the sensor using the printf messages
 * below, and adjust the fan.
 * 
 * Error messages must be printed to stderr, but the contents of the messages
 * are unspecified so you can choose good messages.
 * 
 * Print the following before returning from handling a valid event.
 * printf("********************END EVENT********************\n\n");
 * 
 * Print the following after getting a value from the relevant sensor.
 * printf("Temperature: %.4f --> Device_ID: %d\n", YOUR VARIABLES HERE);
 * printf("Humidity: %.4f --> Device_ID: %d\n", YOUR VARIABLES HERE);
 */

int process_message(struct cignal *cig, int *device_record) {
    
    // Check cignal type
    if (is_valid_type(cig) == -1){ 
        fprintf(stderr, "Cignal is not HANDSHAKE or SENSOR UPDATE.\n");
        return -1; 
    } 

    // if HANDSHAKE
    if (cig->hdr.type == HANDSHAKE){
        // check that device_id is not set yet 
        if (cig->hdr.device_id != -1){
            fprintf(stderr, "Device ID is invalid.\n");
            return -1;  
        } else {
            // register device
            cig->hdr.device_id = register_device(device_record);
            return 0;
        }
    }

    // if SENSOR UPDATE
    if (cig->hdr.type == UPDATE){
        // check if device_id registered
        if (is_registered(cig->hdr.device_id, device_record) == -1){
            fprintf(stderr, "device_id [%d] trying to UPDATE is not registered.\n", cig->hdr.device_id); 
            return -1; 
        } else {
            switch (cig->hdr.device_type){
                case TEMPERATURE: 
                    printf("Temperature: %.4f --> Device_ID: %d\n", cig->value, cig->hdr.device_id);    
                    adjust_fan(cig);
                    return 0;
                case HUMIDITY: 
                    printf("Humidity: %.4f --> Device_ID: %d\n", cig->value, cig->hdr.device_id);
                    adjust_fan(cig);
                    return 0; 
                default: 
                    fprintf(stderr, "device_type is invalid\n");
                    return -1; 
            }
        }
    }

    // shouldn't get here
    return -1;

}
