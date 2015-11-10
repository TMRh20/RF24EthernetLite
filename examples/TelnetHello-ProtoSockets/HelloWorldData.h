
#ifndef __HELLO_WORLDDATA_H__
#define __HELLO_WORLDDATA_H__

/* Next, we define the state of our application, and the memory required for this state is
   allocated together with each TCP connection. One application state
   for each TCP connection. */

typedef struct {
  char input_buffer[MAX_PAYLOAD_SIZE];
  char name[20];
} connection_data;


#endif /* __HELLO_WORLD_H__ */


