/*
 * SerialIP Hello World example.
 *
 * This file contains the definition of the per-connection data that will be
 * allocated on each incoming connection.  Unfortunately it has to be in a
 * separate file because of the way the Arduino IDE works.  (Typedefs can't
 * be listed before function declarations.)
 *
 * You can edit this structure if you want more variables available when
 * processing a connection.  Be careful not to allocate too much data or
 * you might run out of memory (which manifests as the Arduino rebooting for
 * no apparent reason, or never making it to the loop() function after reset.)
 *
 */

typedef struct {
  char input_buffer[MAX_PAYLOAD_SIZE];
  char name[20];
} connection_data;


// Our HTML data is saved in Program Memory to save on SRAM
static const char begin_html[] PROGMEM =
  "HTTP/1.1 200 OK\n"
  "Content-Type:text/html\n\n"
  "<!DOCTYPE";

static const char end_html[] PROGMEM =
  " HTML>\n<html>\n"
  "HELLO FROM ARDUINO!\n"
  "</html>\n";

// A character buffer is used to load data in from program memory before sending
char buf[50];


