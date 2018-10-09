/*
 *
 * re-adapted for simultra
 * 
 **/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include<stdint.h>
#include <time.h>
#include "cJSON.h"

#include "payload.h"
#include "utils.h"
#include "geolocation.h"
#include "config.h"
#include "atypes.h"

node_t* my_hosts; // dynamic list containing the nodes, to be populated using ADD command.
int my_hosts_number = -1;

void* sending (void* destination_node)
{
    int t = 1, 	   // round
	len,
	sock = -1;
    struct sockaddr_in server;
    char server_reply[2000];
  
    srand((unsigned int)time(NULL));
    
    // create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
	fprintf(stderr, "Error: cannot create socket\n");
    }
    
    debug("Socket created.");

    server.sin_addr.s_addr = inet_addr( ((node_t*) destination_node)->ip);
    server.sin_family = AF_INET;
    server.sin_port = htons( ((node_t*) destination_node)->port);

    // connect to remote node
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
	debug("%s is not available !", ((node_t*) destination_node)->name);
        //perror("connect failed");
    }

    debug("I am Connected.");
    
    // keep communicating with server
    
    while((len = recv(sock, server_reply, sizeof(server_reply), 0)) > 0)
    {
        debug("[%s] Server reply: %.*s\n", me, len, server_reply);
        debug("_________________________________________________________________________________________________________________________\n");

	// build the payload with random lat/lng
	payload_t payload = { "precType", t, random_float(100), random_float(100), ((node_t*) destination_node)->name};
	char* payload_string = NULL;
        set_payload(payload, &payload_string);

        debug("[%s] Sending location: %s\n", me, payload_string);
	sleep( ((node_t*) destination_node)->delay);

        // send some data
        if( send(sock , payload_string , strlen(payload_string) , 0) < 0)
        {
            debug("[%s] Send failed", me);
        }
	t++;
    }

    close(sock);

    return 0;
}

//===================================================================================================
// handling an incoming connection

void *new_connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = (intptr_t) socket_desc;
    int read_size = 0;
    char client_message[2000];
    
    // to id the sender
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    char ip_str[INET_ADDRSTRLEN];
    
    // sending an ACK
    char* rdy = malloc(sizeof(char) * 4);
    sprintf(rdy, "%d", ACK);
    write(sock, rdy, sizeof(rdy)-1);
    free(rdy);
    
    // receiving a message from client
    while( (read_size = recv(sock , client_message , sizeof(client_message) , 0)) > 0 )
    {
        // identifying the sender
        if (getsockname(sock, (struct sockaddr *)&sin, &len) == -1)
            perror("getsockname");
        else
            inet_ntop(AF_INET, &(sin.sin_addr), ip_str, INET_ADDRSTRLEN);
	    
	// Parsing the JSON
        debug("Payload received from %s:%d", ip_str, ntohs(sin.sin_port));
	debug("Parsing the JSON content...");
	
	// extract the fields from json,
	payload_t payload = {0, 1, 0., 0., 0, 0, 0};
	parse_payload(client_message, & payload);

        // build a new message, the response.
        char *Response = malloc(  // TODO JSONDATA add here.
				    sizeof(char) *
 				   (strlen(payload.data) +
				   strlen(payload.name) +
				   sizeof(payload.command) +
				   sizeof(payload.lat)+
				   sizeof(payload.lng)+
				   sizeof(payload.round)+
				   strlen(payload.precision)+80)
				   
				   );

	debug("Running command %d for %s", payload.command, payload.name);
	
	switch (payload.command)
	{

	    case MOVING:
	    {
		sprintf(Response, "MOVING %s.", payload.name);
		
		debug("Receiving MOVING from the agent");

		debug("with name %s @(%lf, %lf) @t=%d command=%d data=<%s>.",
		    payload.name,
		    payload.lat,
		    payload.lng,
		    payload.round,
		    payload.command,
		    payload.data
		);

		
		break;
	    }

	    case ACK:
	    {
		sprintf(Response, "ACK %s.", payload.name);
		break;
	    }
	    case ADD:
	    {			
		if (strcmp(payload.data, "\"\"") == 0) // TODO use remove_all_chars()
		{
		    debug("No hosts specified. Nothing to add !");
		}
		else
		{
		    // TODO Control on payload.data...., make sure its either {IP:PORT,}IP:PORT or pure *JSON*
    
		    int i, j, hosts_number;
		    node_t* hosts = NULL;
		    
		    get_hosts( payload.data, // list of hosts.
			      & hosts,
			      & hosts_number);
    
		    // check my list 'my_hosts'
		    
		    if (my_hosts == NULL) // my list is empty
		    {
			debug("Empty List");
			
			if (hosts_number>0)
			{
				debug("Populating List");
				
				my_hosts = malloc( sizeof(node_t) * MAX_NODES ); // TODO realloc() later instead, when i am adding to an already non empty list.
				
				for ( i = 0 ; i < hosts_number ; i++ )
				{
				    my_hosts[i].ip = malloc( sizeof(char) * (strlen(hosts[i].ip)+1));
				    sscanf(hosts[i].ip, "%s", my_hosts[i].ip);
				    my_hosts[i].port = hosts[i].port;
				    // TODO add names?
				    debug("ADDING %s:%d", hosts[i].ip, hosts[i].port);
				}
				
				my_hosts_number = hosts_number;
			}
			
			// response
			sprintf(Response, "DONE %s @(%lf, %lf) @t=%d command=%d data=<%s>.",
				payload.name,
				payload.lat,
				payload.lng,
				payload.round,
				payload.command,
				payload.data
				);
			debug("I finished adding the entries.");
			debug("%s", Response);
		    }
		    else // my list is not empty
		    {
			debug("LISTING: There is someone in my list... %d node(s) :", my_hosts_number);
			for ( i = 0 ; i < my_hosts_number ; i++ )
			    debug("LISTING:   host #%d <%s:%d>", i, my_hosts[i].ip, my_hosts[i].port);
			    
			debug("Let me check if your entries are in my list or not.");
			
			for ( i = 0 ; i < hosts_number ; i++ )
			{
		    	    alert("_______________________________________________");
		    	    alert("Checking your entry %s:%d", hosts[i].ip, hosts[i].port);
			    
			    for ( j = 0 ; j < my_hosts_number ; j++ )
			    {
				if (!strcmp(hosts[i].ip, my_hosts[j].ip) && hosts[i].port == my_hosts[j].port )
				{
				    alert("\t\t    %s:%d == %s:%d    (found !)", hosts[i].ip, hosts[i].port, my_hosts[i].ip, my_hosts[j].port);
				}
				else
				{
				    alert("\t\t    %s:%d <> %s:%d", hosts[i].ip, hosts[i].port, my_hosts[i].ip, my_hosts[j].port);
				    //alert("   ===> %s:%d",  my_hosts[my_hosts_number-1].ip,  my_hosts[my_hosts_number-1].port );
				}
				
			    }
			    
			}
			    
			// check if the payloads sent by agent are in my_hosts or not
			// if they are not, add them,
			// ....
			// if not send
	    		sprintf(Response, "%d", DONE);
			    debug("Sending DONE to %s.", payload.name);
		    }
			
		}
		
		debug("Breaking from 'ADD' case");
		break;
	    }
	    default:
	    {
		debug("Unknown Command");
		sprintf(Response, " %s Your Command %d is unknown.", payload.name, payload.command);
		break;
	    }
	}
	    
        // send back
        write(sock , Response , read_size + strlen(Response));
        
        debug("___________________________________________________________________________________________________");
  
    }
     
    if(read_size == 0)
    {
	debug("Host <%s:%d> disconnected.", ip_str, ntohs(sin.sin_port));
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }

    return 0;
}

//===================================================================================================

void* receiving(void* port_p)
{  
    int port = *((int*) port_p);
    int socket_desc , client_sock;
    struct sockaddr_in server, client;
    socklen_t c = sizeof(client);

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        debug("Could not create socket !");
    }

    // prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    bzero (&server.sin_zero, 8);


    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("bind failed !");
    }

    listen(socket_desc , MAX_NODES);

    // accept an incoming connection
    debug("Waiting for incoming connections on %d", port);

    c = sizeof(client);
    
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, &c)) )
    {
        debug("Connection accepted");
        pthread_t thread_id;

        if( pthread_create( &thread_id, NULL, new_connection_handler, (void*) (intptr_t)client_sock) < 0)
        {
            perror("could not create thread");
        }
        int client_port = ntohs(client.sin_port);
        debug("Handler assigned for client: %s", ip_s(client));
    }

    if (client_sock < 0)
    {
        perror("accept failed");
    }

    return 0;
}

//==========================================================================================================

int main(int argc , char *argv[])
{
    int i,
        port = -1;  // listening port
        node_t* nodes = 0;
    
    // check for command line arguments
    if (argc != 2)
    {
	    fprintf(stderr, "Usage: %s config_file\n", argv[0]);
	    return -1;
    }

    // Load configuration from confX.json
    
    char * buffer = 0;
    load_config(argv[1], &buffer);
    
    int n = set_nodes(&nodes, buffer, & port); // n: number of peers
    
    debug("##############################");
    debug("   Listener");
    debug("          port = %d", port);
    debug("##############################");
    
    pthread_t thread_listener;
    if( pthread_create( &thread_listener , NULL,  receiving, (void*) & port ) < 0)
    {
	fprintf(stderr,"Error : pthread_create() could not create thread.\n");
	return 1;
    }
    
    pthread_join( thread_listener, NULL);

#if 0
    debug("Thread assigned on %d.", port);
    
 
    debug("##############################");
    debug("   Emitter");
    debug("##############################");

    pthread_t* thread_emitter = malloc(n * sizeof(pthread_t));
    for ( i = 0 ; i < n ; i++  )
    {	
	if( pthread_create( &thread_emitter[i] , NULL ,  sending, (void*) (nodes+i) ) < 0)
	{
	    fprintf(stderr,"Error : pthread_create() could not create thread %d\n", i);
	    return 1;
	}
	debug("Thread assigned for %s", nodes[i].name);
    }

//    for ( i = 0 ; i < n ; i++  )
//	  pthread_join( thread_emitter[i], NULL);    

#endif

    
}