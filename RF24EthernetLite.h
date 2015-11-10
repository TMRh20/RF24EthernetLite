/*
  RF24EthernetLite by TMRh20 2015
*/

#ifndef RF24EthernetLite_h
#define RF24EthernetLite_h

#include <Arduino.h>

#define TUN  // Use only the tcp protocol, no ethernet headers or arps
//#define TAP  // Include ethernet headers


#include <RF24.h>
#include <RF24Network.h>
#if !defined (RF24_TAP) // Using RF24Mesh
#include <RF24Mesh.h>
#endif

extern "C" {
  #import "uip-conf.h"
  #import "utility/uip.h"
  #include "utility/timer.h"

}



#if defined (TAP)
#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
#endif

#define uip_seteth_addr(eaddr) do {uip_ethaddr.addr[0] = eaddr[0]; \
                              uip_ethaddr.addr[1] = eaddr[1];\
                              uip_ethaddr.addr[2] = eaddr[2];\
                              uip_ethaddr.addr[3] = eaddr[3];\
                              uip_ethaddr.addr[4] = eaddr[4];\
                              uip_ethaddr.addr[5] = eaddr[5];} while(0)
							  

typedef struct {
	int a, b, c, d;
} IP_ADDR;


typedef void (*fn_uip_cb_t)(uip_tcp_appstate_t *conn);

typedef void (*fn_my_cb_t)(unsigned long a);
extern fn_my_cb_t x;


class RF24EthernetStack {//: public Print {
	public:

        #if !defined (RF24_TAP) // Using RF24Mesh
		RF24EthernetStack(RF24& _radio,RF24Network& _network, RF24Mesh& _mesh);
		#else
        RF24EthernetStack(RF24& _radio,RF24Network& _network);
        #endif

		void use_device();
		void begin(IP_ADDR myIP, IP_ADDR subnet);
		void set_gateway(IP_ADDR myIP);
		void listen(uint16_t port);

		// tick() must be called at regular intervals to process the incoming serial
		// data and issue IP events to the sketch.  It does not return until all IP
		// events have been processed.
		void tick();

		// Set a user function to handle raw uIP events as they happen.  The
		// callback function can only use uIP functions, but it can also use uIP's
		// protosockets.
		void set_uip_callback(fn_uip_cb_t fn);



	private:
		RF24& radio;
		RF24Network& network;
        #if !defined (TAP) // Using RF24Mesh
        RF24Mesh& mesh;
		#endif
        struct timer periodic_timer;
		struct timer arp_timer;
		//, arp_timer;
		struct serialip_state *cur_conn; // current connection (for print etc.)
		fn_uip_cb_t fn_uip_cb;
		void uip_callback();

	friend void serialip_appcall(void);
	friend void uipudp_appcall(void);

};

extern RF24EthernetStack RF24Ethernet;

#endif
