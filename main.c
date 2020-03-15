#include <libusb-1.0/libusb.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define VENDOR_ID 0x1b1c
#define NODE_ID 0x0c0b

#define UPDATE_FREQ_SEC 1

#define R 0xff
#define G 0xff
#define B 0xff

#define NUMBER_OF_FANS 6
#define RETRIES_ON_TIMEOUT 5

uint8_t headers[2][64];
uint8_t colors[NUMBER_OF_FANS][3][64];

libusb_device_handle* dev_handle = NULL;


void generate_headers()
{
   headers[0][0] = 0x33;
   headers[0][1] = 0xff;

   headers[1][0] = 0x38;
   headers[1][2] = 0x2;
}

void generate_payloads()
{
    for(int i = 0; i< NUMBER_OF_FANS; ++i)
    {
        // Red channel
        colors[i][0][0] = 0x32;
        colors[i][0][2] = i * 0x10;
        colors[i][0][3] = 0x10;
        colors[i][0][4] = 0x0;
        memset(colors[i][0] + 5, R, 16);

        // Green channel
        colors[i][1][0] = 0x32;
        colors[i][1][2] = i * 0x10;
        colors[i][1][3] = 0x10;
        colors[i][1][4] = 0x1;
        memset(colors[i][1] + 5, G, 16);

        // Blue channel
        colors[i][2][0] = 0x32;
        colors[i][2][2] = i * 0x10;
        colors[i][2][3] = 0x10;
        colors[i][2][4] = 0x2;
        memset(colors[i][2] + 5, B, 16);
    }
}

int write_payload(unsigned char* data, int length) {
    assert(dev_handle != NULL);
    assert(length == 64);

    int res = LIBUSB_ERROR_TIMEOUT;
    int attempts = RETRIES_ON_TIMEOUT;
    int written = 0;

    while(res == LIBUSB_ERROR_TIMEOUT && attempts > 0)
    {
        res = libusb_interrupt_transfer(dev_handle, LIBUSB_ENDPOINT_OUT | 1, data, length, &written, 100);
        attempts--;
    }

    if (attempts < 0 || written != length)
       return -1;

    if (res != 0)
       return res;

    return 0;
}

int main()
{
    // Initialize usb lib
    if (libusb_init(NULL) != 0) {
        fprintf(stderr, "Could not initialize lib usb");
        return -1;
    }

    // Get device handle
    dev_handle = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, NODE_ID);	
    if (dev_handle == NULL) {
        fprintf(stderr, "Could not get device handle, check vendor id and node id");
        return -1;
    }

    // Generate payloads
    generate_headers();
    generate_payloads();


    for(;;) {
        // Write headers
        if (write_payload(headers[0], 64) != 0) {
            fprintf(stderr, "Could not write header");
            return -1;
        }

        if (write_payload(headers[1], 64) != 0) {
            fprintf(stderr, "Could not write header");
            return -1;
        }

        for(int i = 0; i < NUMBER_OF_FANS; ++i)
        {
            for(int j = 0; j < 3; ++j) {
               if (write_payload(colors[i][j], 64) != 0)
               {
                  fprintf(stderr, "Could not write color payload");
                  return -1;
               }
            }
        }
        usleep(UPDATE_FREQ_SEC * 1000 * 1000); // To us
    }
}
