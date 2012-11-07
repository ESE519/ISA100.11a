 int8_t ret, len, cnt;
  int8_t sock;
  
  printf ("tx_task PID=%d\r\n", nrk_get_pid ());

  sock = create_socket(SOCK_DGRAM);
  if(sock == NRK_ERROR)
  	printf("Error in creating socket %d\n", nrk_errno_get());
  cnt = 0;
 
  while (1)
  {
    	// Build a TX packet
    	sprintf (tx_buf, "This is a test %d", cnt);
    	cnt++;
    	nrk_led_set (BLUE_LED);
		
		if(NODE_ADDR == 1)
	 		send(sock, tx_buf, strlen(tx_buf), 2,  SERVER2_PORT, NORMAL_PRIORITY);
	 	else
	 		send(sock, tx_buf, strlen(tx_buf), 1,  SERVER1_PORT, NORMAL_PRIORITY);
	 	
    	nrk_kprintf (PSTR ("Tx packet enqueued\r\n"));
    
	 	ret = wait_until_send_done(sock);    
    	if(ret == NRK_ERROR)
    		printf("Error in waiting for send done signal %d\n", nrk_errno_get());

    	// Task gets control again after TX complete
    	nrk_kprintf (PSTR ("Tx task sent data!\r\n"));
    	nrk_led_clr (BLUE_LED);
    	nrk_wait_until_next_period ();
  }
	*/
  return;
}


{
  /* int8_t sock;
  int8_t ret;
  uint8_t *rx_buf;
  int8_t len;
  int16_t srcAddr;
  int8_t srcPort;
  int8_t rssi;
  int8_t i;
  
   printf ("rx_task PID=%d\r\n", nrk_get_pid ());
  sock = create_socket(SOCK_DGRAM);
  if(sock == NRK_ERROR)
   	nrk_kprintf(PSTR("Error creating socket in rx_task"));
  	
  if(NODE_ADDR == 1)
  	ret = bind(sock, SERVER1_PORT);
  else
  	ret = bind(sock, SERVER2_PORT);
  
  if(ret == NRK_ERROR)
  		printf("Error in binding to socket %d\n", nrk_errno_get());
  	
  if(NODE_ADDR == 1)
  	ret = set_rx_queue_size(sock, SERVER1_RQS);
  else
  	ret = set_rx_queue_size(sock, SERVER2_RQS);
  
  if(ret == NRK_ERROR)
  		printf("Error in setting rx queue size %d\n", nrk_errno_get());
  
  while(1)
  {
  		rx_buf = receive(sock, &len, &srcAddr, &srcPort, &rssi);
 		
 		printf("Received a pkt from %d,%d with RSSI = %d\n", srcAddr, srcPort, rssi);
 		nrk_led_set(GREEN_LED); 
 		 
  		for(i = 0; i < len; i++)
  			printf("%c ", rx_buf[i]);
  		printf("\r\n");
  			
  		nrk_led_clr(GREEN_LED);
  		release_buffer(sock, rx_buf);
  	}
  	*/
  	
  	return;
}

#include "NWStackConfig.h"
#include "TransportLayerUDP.h"
#include "NetworkLayer.h"
#include "Pack.h"
#include "Serial.h"
#include "BufferManager.h"