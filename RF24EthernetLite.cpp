/*
  RF24EthernetLite by TMRh20 2015
*/

#include <Arduino.h>
#include "RF24EthernetLite.h"


extern "C" {
#include "uip-conf.h"
#include "uip.h"
#include "uip_arp.h"
#include "timer.h"
}  


#if defined (RF24_TAP) 
RF24EthernetStack::RF24EthernetStack(RF24& _radio, RF24Network& _network): radio(_radio),network(_network)
	//fn_uip_cb(NULL)
{
}
#else // Using RF24Mesh
RF24EthernetStack::RF24EthernetStack(RF24& _radio, RF24Network& _network, RF24Mesh& _mesh): radio(_radio),network(_network), mesh(_mesh)
	//fn_uip_cb(NULL)
{
}
#endif


void RF24EthernetStack::use_device()
{

  //_radio.begin();
  //_network.begin(97,01);
  //_radio.setPALevel(RF24_PA_MIN);
  //_radio.printDetails();
	
    //::slip_device = &_network;
	//network = _network;
}

void RF24EthernetStack::begin(IP_ADDR myIP, IP_ADDR subnet)
{
#if defined UIP_CONF_EXTERNAL_BUFFER    
    uip_buf = (uint8_t*)&network.frag_ptr->message_buffer[0];
#endif
	uip_ipaddr_t ipaddr;

	//timer_set(&this->periodic_timer, CLOCK_SECOND / 4);
	timer_set(&this->periodic_timer, CLOCK_SECOND / 8);
	
	#if defined (TAP)
		timer_set(&this->arp_timer, CLOCK_SECOND * 10);
		
		uint8_t mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05};
		uip_seteth_addr(mac);
		uip_init();
		uip_arp_init();
		
	#else
      
      mesh.setNodeID(myIP.d);
      mesh.begin();
	  uip_init();
	#endif

	uip_ipaddr(ipaddr, myIP.a, myIP.b, myIP.c, myIP.d);
	uip_sethostaddr(ipaddr);
	uip_ipaddr(ipaddr, subnet.a, subnet.b, subnet.c, subnet.d);
	uip_setnetmask(ipaddr);

}

void RF24EthernetStack::set_gateway(IP_ADDR myIP)
{
  uip_ipaddr_t ipaddr;
  uip_ipaddr(ipaddr, myIP.a, myIP.b, myIP.c, myIP.d);
  uip_setdraddr(ipaddr);
}

void RF24EthernetStack::listen(uint16_t port)
{
  uip_listen(HTONS(port));
}

void RF24EthernetStack::tick()
{

	RF24NetworkHeader headerOut(00,EXTERNAL_DATA_TYPE);

	if(network.update() == EXTERNAL_DATA_TYPE){
		uip_len = network.frag_ptr->message_size;
        /*Serial.print("got ext ");
            for(int i=0; i<uip_len; i++){
              Serial.print(uip_buf[i],HEX);
              Serial.print(":");
            }                
            Serial.println();  */      

	}    
	if(uip_len > 0) {
	#if defined (TAP)
	  if(BUF->type == htons(UIP_ETHTYPE_IP)) {
		uip_arp_ipin();
	#endif
		uip_input();
		// If the above function invocation resulted in data that
		// should be sent out on the network, the global variable
		// uip_len is set to a value > 0.
		if (uip_len > 0){
		    #if defined (TAP)
            uip_arp_out();
            #endif
			size_t totalSize = 0;

			if(network.write(headerOut,uip_buf,uip_len)){
                /*for(int i=0; i<uip_len; i++){
                  Serial.print(uip_buf[i],HEX);
                  Serial.print(":");
                }                
                Serial.println();*/
            }else{ //Serial.println("net write fail");
            }

		}
		
		

	#if defined (TAP)	
		 }else if(BUF->type == htons(UIP_ETHTYPE_ARP)) {
         uip_arp_arpin();	Serial.println("ARp");
         // If the above function invocation resulted in data that
         //   should be sent out on the network, the global variable
         //   uip_len is set to a value > 0. 
           if(uip_len > 0) {
		     network.write(headerOut,uip_buf,uip_len);
           }
	    }
       
	#endif
	
	
	 }else if (timer_expired(&periodic_timer)) {
		timer_reset(&periodic_timer);
		for (int i = 0; i < UIP_CONNS; i++) {
			uip_periodic(i);
			// If the above function invocation resulted in data that
			// should be sent out on the network, the global variable
			// uip_len is set to a value > 0.
			if (uip_len > 0) {
			#if defined (TAP)
              uip_arp_out();
            #endif
			  network.write(headerOut,uip_buf,uip_len);			
			}
		}
	 


#if UIP_UDP
		for (int i = 0; i < UIP_UDP_CONNS; i++) {
			uip_udp_periodic(i);
			// If the above function invocation resulted in data that
			// should be sent out on the network, the global variable
			// uip_len is set to a value > 0. */
			if (uip_len > 0){
			network.write(headerOut,uip_buf,uip_len);
			}
		}
#endif /* UIP_UDP */
       /* Call the ARP timer function every 10 seconds. */
#if defined (TAP)	
       if(timer_expired(&arp_timer)) {
         timer_reset(&arp_timer);
         uip_arp_timer();
       }
#endif
    }
	
}

void RF24EthernetStack::set_uip_callback(fn_uip_cb_t fn)
{
	this->fn_uip_cb = fn;
}

void RF24EthernetStack::uip_callback()
{
	struct serialip_state *s = &(uip_conn->appstate);
	if (this->fn_uip_cb) {
		// The sketch wants to handle all uIP events itself, using uIP functions.
		this->fn_uip_cb(s);//->p, &s->user);
	} else {

	}
}


// uIP callback function
void serialip_appcall(void)
{
	RF24Ethernet.uip_callback();
}

