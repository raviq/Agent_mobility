
/*
 *
 * re-adapted for simultra
 *
 **/

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#include "atypes.h"
#include "geolocation.h"
#include "payload.h"
#include "debug.h"

// Sends payloads to manager

void* send_to (void* destination_node)
{
    int t = 0, // round
	T = 100,   // total number of rounds
	len,
	sock = -1;
    double x = 0, y = 0; // initial positions (x, y)
    struct sockaddr_in server;
    char server_reply[2000];
  
    srand((unsigned int)time(NULL));

    // Create socket.
    if ((sock = socket(AF_INET , SOCK_STREAM , 0)) == -1)
    {
		fprintf(stderr, "Error: cannot create socket!\n");
    }
    
    debug("Socket created.");

    server.sin_addr.s_addr = inet_addr( ((node_t*) destination_node)->ip);
    server.sin_family = AF_INET;
    server.sin_port = htons( ((node_t*) destination_node)->port);

    // connect to remote node
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("Connection failed!");
    }

    debug("Connected");
    
    // kKeep communicating with the server
    while((len = recv(sock, server_reply, sizeof(server_reply), 0)) > 0)
    {	
		command_t command;
		sscanf(server_reply, "%d", &command);

		switch (command)
		{
			case DONE:
			{
				debug("Server reply: DONE");
				break;
			}
		
			case ACK:
			{
				debug("Server reply: ACK");
				break;
			}
		
			default:
			{
				debug("Server reply: <%.*s>", len, server_reply);
				break;
			}
		}
	
		payload_t payload = { "precType",
					   t, // round
					   x, // lat
					   y, // lng
					   ((node_t*) destination_node)->name,
					   MOVING,
					   "127.0.0.1:8058,127.0.0.1:8019,127.0.0.1:8003,127.0.0.1:8065" // TODO JSON
					};
				
		char* payload_string = NULL;
		set_payload_latlon(payload, &payload_string, x, y, t);
		x += 1.0;
		y += 1.0;
		debug("Sending: %s", payload_string);
	
		sleep( ((node_t*) destination_node)->delay);

		// Alter data with new lat/lon, and command (to NEWLOCATION)	
		// send some data
		if( send(sock , payload_string , strlen(payload_string) , 0) < 0)
		{
			debug("Send failed");
		}
		
		t++;
    }

    close(sock);

    return 0;
}


int main(int argc , char *argv[])
{
    if (argc != 2)
    {
		printf(" Argument missing!\n Usage: ./agent index\n index = 1, 2,... \n");
		exit(0);
    }
    
    char* agent_name = malloc(sizeof(char) * 10);
    sprintf(agent_name,"%s%s", "Agent", argv[1]);
    
    node_t node =
    {
		"127.0.0.1",	
		8033, 	// port we are connecting to.
		1, 		// delay
		agent_name
	};

    debug("Sending to manager.");
    
    pthread_t thread = malloc(sizeof(pthread_t));
    
    if( pthread_create( &thread , NULL ,  send_to, (void*) & (node) ) < 0)
    {
	    fprintf(stderr,"Error : pthread_create() could not create thread.\n");
	    return 1;
    }
    debug("Thread assigned for %s", node.name);
    
    pthread_join( thread, NULL);  
    
    free(agent_name);
    
return 0;

}
