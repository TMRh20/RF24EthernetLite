/**
 *  RF24Ethernet-Lite Simple Telnet Server
 *  An example of how to write uIP applications
 *         
 *  RF24Ethernet-Lite is partially based on the SerialIP example:
 *  For more information see the SerialIP page on the Arduino wiki:
 *    <http://www.arduino.cc/playground/Code/SerialIP>
 *    
 *  This example sets up a web server at a specified IP on port 1000.
 *  It will also respond to ping requests.
 *  Advanced users can reduce memory and program space usage by editing the RF24Network_config.h file included with that library:
 *     1. Uncomment #define DISABLE_USER_PAYLOADS to disable internal RF24Network payloads   
 *     2. Adjust the #define MAIN_BUFFER_SIZE to the maximum preferred TCP/IP payload size
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

/*
 * Declaration of the protosocket function that handles the connection
 * (defined at the end of the code).
 */
static int handle_connection(uip_tcp_appstate_t *s, connection_data *d);

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

uint32_t mesh_timer=0;

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
/*---------------------------------------------------------------------------*/
/*
 * In hello-world.h we have defined the UIP_APPCALL macro to
 * hello_world_appcall so that this funcion is uIP's application
 * function. This function is called whenever an uIP event occurs
 * (e.g. when a new connection is established, new data arrives, sent
 * data is acknowledged, data needs to be retransmitted, etc.).
 */

void uipudp_appcall() {
}

void
uip_callback(uip_tcp_appstate_t *s)
{

  if(uip_connected()) {
     /*
     * The uip_conn structure has a field called "appstate" that holds
     * the application state of the connection. We make a pointer to
     * our data structure, and link it to the current connection data.
     */
      connection_data *d = (connection_data *)malloc(sizeof(connection_data));
      s->user = d;
      /*
     * If a new connection was just established, we should initialize
     * the protosocket in our applications' state structure.
     */
     PSOCK_INIT(&s->p, d->input_buffer, sizeof(d->input_buffer));
  }

  /*
   * Finally, we run the protosocket function that actually handles
   * the communication. We pass it a pointer to the application state
   * of the current connection.
   */
  handle_connection(s, (connection_data *)s->user);
}


/*---------------------------------------------------------------------------*/
/*
 * This is the protosocket function that handles the communication. A
 * protosocket function must always return an int, but must never
 * explicitly return - all return statements are hidden in the PSOCK
 * macros.
 */
static int
handle_connection(uip_tcp_appstate_t *s, connection_data *d)
{
  PSOCK_BEGIN(&s->p);

  PSOCK_SEND_STR(&s->p, "Hello. What is your name?\n");
  PSOCK_READTO(&s->p, '\n');
  strncpy(d->name, d->input_buffer, sizeof(d->name));
  PSOCK_SEND_STR(&s->p, "Hello ");
  PSOCK_SEND_STR(&s->p, d->name);
  PSOCK_CLOSE(&s->p);
  
  PSOCK_END(&s->p);
}
/*---------------------------------------------------------------------------*/

