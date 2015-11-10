/*
   RF24Ethernet-Lite Simple Web Server

   RF24Ethernet-Lite is partially based on the SerialIP example:
   For more information see the SerialIP page on the Arduino wiki:
     <http://www.arduino.cc/playground/Code/SerialIP>

        -----------------

   This example sets up a web server at a specified IP on port 1000.
   and will send out an HTTP response to a browser making any request.
   It will also respond to ping requests.

   Advanced users can reduce memory and program space usage by editing the RF24Network_config.h file included with that library.
   1. Uncomment #define DISABLE_USER_PAYLOADS to disable internal RF24Network payloads
   2. Adjust the #define MAIN_BUFFER_SIZE to the maximum preferred TCP/IP payload size
*/

#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>
#include <RF24Mesh.h>
#include <RF24EthernetLite.h>
//#include <printf.h>


/************** USER RADIO CONFIG **************/
RF24 radio(7, 8);
/***********************************************/
RF24Network network(radio);
RF24Mesh mesh(radio, network);
RF24EthernetStack RF24Ethernet(radio, network, mesh);

// The connection_data struct is defined in an external file.
#include "HelloWorldData.h"

void setup() {

  Serial.begin(115200);
  //printf_begin(); //Debug only

  // We're going to be handling uIP events ourselves since this is the lite version.
  RF24Ethernet.set_uip_callback(uip_callback);

  Serial.print(F("Starting... "));

  // Set the IP address we'll be using. The last octet of the IP will match the RF24Mesh nodeID.
  IP_ADDR myIP = {10, 10, 3, 5};
  IP_ADDR subnet = {255, 255, 255, 0};
  
  RF24Ethernet.begin(myIP, subnet);

  // Set the gateway IP
  IP_ADDR gwIP = {10, 10, 3, 1};
  RF24Ethernet.set_gateway(gwIP);
  
  if (mesh.begin()) {
    Serial.println(F("OK"));
  } else {
    Serial.println(F("Connection to mesh failed"));
    return;
  }

  // Listen for incoming connections on TCP port 1000.  Each incoming
  // connection will result in the uip_callback() function being called.
  RF24Ethernet.listen(1000);
  //radio.printDetails();
}

uint32_t mesh_timer = 0;

void loop() {

  // Check the serial port and process any incoming data.
  RF24Ethernet.tick();

  // We can do other things in the loop, but be aware that the loop will
  // briefly pause while IP data is being processed.

  // Using RF24Mesh, verify connectivity every 30 seconds
  if (millis() - mesh_timer > 30000) {
    mesh_timer = millis();
    if ( ! mesh.checkConnection() ) {
      Serial.println(F("*** RENEW ***"));
      mesh.renewAddress();
    } else {
      Serial.println(F("*** MESH OK ***"));
    }
  }

}

void uipudp_appcall() {
}

int len, count;

void uip_callback(uip_tcp_appstate_t *s)
{
  if (uip_connected()) {

    // We want to store some data in our connections, so allocate some space
    // for it.  The connection_data struct is defined in a separate .h file
    connection_data *d = (connection_data *)malloc(sizeof(connection_data));

    // Save it as RF24Ethernet user data so we can get to it later.
    s->user = d;

    // The protosocket's read functions need a per-connection buffer to store
    // data they read.  We've got some space for this in our connection_data
    // structure, so we'll tell uIP to use that.
    PSOCK_INIT(&s->p, d->input_buffer, sizeof(d->input_buffer));

  }

  // Call/resume our protosocket handler.
  handle_connection(s, (connection_data *)s->user);

  // If the connection has been closed, release the data we allocated earlier.
  if (uip_closed()) {
    if (s->user) free(s->user);
    s->user = NULL;
  }
}

// This function is going to use uIP's protosockets to handle the connection.
// This means it must return int, because of the way the protosockets work.
// In a nutshell, when a PSOCK_* macro needs to wait for something, it will
// return from handle_connection so that other work can take place.  When the
// event is triggered, uip_callback() will call this function again and the
// switch() statement (see below) will take care of resuming execution where
// it left off.  It *looks* like this function runs from start to finish, but
// that's just an illusion to make it easier to code :-)

int handle_connection(uip_tcp_appstate_t *s, connection_data *d)
{
  // All protosockets must start with this macro.  Its internal implementation
  // is that of a switch() statement, so all code between PSOCK_BEGIN and
  // PSOCK_END is actually inside a switch block.  (This means for example,
  // that you can't declare variables in the middle of it!)
  PSOCK_BEGIN(&s->p);

  // Read some returned text into the input buffer we set in PSOCK_INIT.  Data
  // is read until a newline (\n) is received, or the input buffer gets filled
  // up.
  PSOCK_READTO(&s->p, '\n');
  count = psock_datalen(&s->p);

  //Search for the /r/n/r/n string at the end of an html request
  while (count > 2) {
    Serial.write(d->input_buffer, count);
    PSOCK_READTO(&s->p, '\n');
    count = psock_datalen(&s->p);
  }

  // Send the HTML defined in HelloWorldData.h over the connection.
  memcpy_P(buf, begin_html, sizeof(begin_html));
  PSOCK_SEND(&s->p, buf, sizeof(begin_html));
  memcpy_P(buf, end_html, sizeof(end_html));
  PSOCK_SEND(&s->p, buf, sizeof(end_html));

  // Disconnect.
  PSOCK_CLOSE(&s->p);

  // All protosockets must end with this macro.  It closes the switch().
  PSOCK_END(&s->p);
}

