char tx_buf[SIZE_GATEWAYTONODESERIAL_PACKET];		// to hold messages sent by the gateway to the sensor network

int main()
{
	GatewayToNodeSerial_Packet gtn_pkt;				// to hold a packet to be sent to the sensor network
	char gw_addr[SIZE_IP_ADDRESS];					// temporary variable 
	
	
	// make an UDP socket to connect to the SLIPStream server
	if( slipstream_open(GATEWAY_ADDRESS, GATEWAY_PORT, 1) == 0 )
	{
		printf("Error in connecting to the gateway server at [%s,%d]\r\n", strcpy(gw_addr, GATEWAY_ADDRESS), GATEWAY_PORT);
		exit(1);
	}
	
	if( slipstream_send(tx_buf, (int)SIZE_GATEWAYTONODESERIAL_PACKET) == 0 ) // send an inital message to the server
	{
		printf("Error in sending message to firefly\r\n");
		exit(1);
	}
	
	
	
	















int main()
{
	int64_t start_time, end_time;				// to mark the start and end of the collection period 
	NodeToGatewaySerial_Packet ntg_pkt;			// to hold a packet received from the attached node
	GatewayToNodeSerial_Packet gtn_pkt;			// to send a packet to the attached node
	int8_t data_collection_flag;				// flag to mark the end of data collection
	int8_t ret;									// holds the return value of function calls
	
	char gw_addr[16];							// temporary variable 
	int8_t cnt = 0;
	int8_t i;
	
	initialise_network_gateway();					// initialise the gateway 
	
	// make an UDP socket to connect to the SLIPStream server
	if( slipstream_open("127.0.0.1", 4000, 1) == 0 )
	{
		printf("Error in connecting to the gateway server at [%s,%d]\r\n", strcpy(gw_addr, GATEWAY_ADDRESS), GATEWAY_PORT);
		exit(1);
	}
	
	// construct a dummy packet
	//sprintf(gtn_pkt.data, "Startup message\r\n");
	printf("tx_buf = [");
	for(i = 0; i < SIZE_GATEWAYTONODESERIAL_PACKET; i++)
	{	tx_buf[i] = 5;
		printf("%d ", tx_buf[i]);
	}
	printf("]\r\n");
	
	gtn_pkt.data[0] = 1;
	gtn_pkt.data[1] = 2;
	gtn_pkt.type = SERIAL_APPLICATION;
	gtn_pkt.length = 2;
	//gtn_pkt.length = strlen(gtn_pkt.data);
	//pack_GatewayToNodeSerial_Packet_header(tx_buf, &gtn_pkt);
	//memcpy(tx_buf + SIZE_GATEWAYTONODESERIAL_PACKET_HEADER, gtn_pkt.data, MAX_GATEWAY_PAYLOAD);
	//printf("Dummy packet = ");
	printf("SIZE_GATEWAYTONODESERIAL_PACKET = %d\r\n", SIZE_GATEWAYTONODESERIAL_PACKET);
	//print_gtn_pkt(&gtn_pkt);
	// send the dummy packet
	tx_buf[0] = 1;
	tx_buf[1] = 2;
	//sprintf (buffer, "This is a sample slip string: Count %d\n", cnt);

	do{
	ret = slipstream_send(tx_buf, (int)SIZE_GATEWAYTONODESERIAL_PACKET); // send an inital message to the server
	//ret = slipstream_send(buffer, strlen(buffer));
	if(ret == 0)
	{
		printf("Error in sending message to firefly\r\n");
		exit(1);
	}
	printf("Sent message to the firefly %d\r\n", cnt);
	cnt = (cnt + 1) % 128;
	sleep(1);
	}while(1);
	
	while(1)												// listen from the attached node forever 
	{
		if(DEBUG_NG == 2)
		{
			printf("Within main while loop\r\n");
		}
		// reset the timers
		start_time = time(NULL);
		end_time = start_time;
		
		// assume no new link has been detected		
		data_collection_flag = FALSE;
		
		// open the topology file
		topologyFile = (FILE*)fopen("SensorTopology.dot", "w");
		if(topologyFile == NULL)
		{
			printf("Topology generation file could not be opened\n");
			exit(1);
		}
		
		if(DEBUG_NG == 2)
			printf("Topology collection started\n");
		
		// begin topology generation				
		begin_topology_file(topologyFile);
		
		// collect data for COLLECTION_PERIOD or till a new link is detected 		
		while( (end_time - start_time < COLLECTION_PERIOD) && (data_collection_flag == FALSE) )	 
		{
			if(DEBUG_NG == 2)
			{
				printf("Within collection loop\r\n");
			}
			ret = receiveFromSerial(rx_buf, SIZE_NODETOGATEWAYSERIAL_PACKET);
			
			if(DEBUG_NG == 2)
			{
				printf("\r\nNo of characters received = %d\r\n", ret);
			}
		
			if(DEBUG_NG >= 0)
			{
				printf("Received rx_buf = ");
				printBuffer(rx_buf, SIZE_NODETOGATEWAYSERIAL_PACKET);
			}
			// unpack the packet
			unpack_NodeToGatewaySerial_Packet_header(&ntg_pkt, rx_buf);
									
			// what kind of packet is it 
			switch(serial_pkt_type(&ntg_pkt))
			{
				case SERIAL_APPLICATION:
					process_serial_app_pkt(&ntg_pkt);
					break;
					
				case SERIAL_NW_CONTROL:
					if(process_serial_nw_ctrl_pkt(&ntg_pkt) == 1)	// should end the topology description file 
						data_collection_flag = TRUE; 
					break;
					
				case INVALID:
					// drop the packet and go receive another one 
					printf("NG: Invalid packet type received = %d\n", ntg_pkt.type);	 
					break;
			}
			
			// update the end time 
			end_time = time(NULL);
			
		} // end while(COLLECTION_PERIOD)
		
		generate_routing_tables();
		disseminate_routing_tables();
				
	} // end while(1) 
	 return 0;
} // end main()
