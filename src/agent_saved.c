#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "geolocation.h"
#include "payload.h"


int main(int argc , char *argv[])
{
    int t = 1, 	   // round
	port, len,
	sock = -1,
	delay = 1; // send location every 'delay' seconds.
    struct sockaddr_in server;
    char server_reply[2000];
    char name[10];

    srand((unsigned int)time(NULL));
    
    // checking commandline parameter
    if (argc != 4)
    {
		printf("Usage: %s hostname_ip port my_name\n", argv[0]);
		return -1;
    }

        // obtain port number
    if (sscanf(argv[3], "%s", name) <= 0)
    {
    	fprintf(stderr, "%s: error: wrong parameter: name\n", argv[3]);
    	return -2;
    }
        
    // obtain port number
    if (sscanf(argv[2], "%d", &port) <= 0)
    {
    	fprintf(stderr, "%s: error: wrong parameter: port\n", argv[0]);
    	return -2;
    }
        
    // create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
	fprintf(stderr, "%s: error: cannot create socket\n", argv[0]);
    }
    
    printf("[%s] Socket created.\n", name);

    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_family = AF_INET;
    server.sin_port = htons( port );

    // connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed");
        return 1;
    }

    printf("[%s] Connected\n", name);
    
    // keep communicating with server
    
    while((len = recv(sock, server_reply, sizeof(server_reply), 0)) > 0)
    {
        printf("[%s] Server reply: %.*s\n", name, len, server_reply);
        printf("_______________________________________________________________________________________________________\n");

	// build the payload with random lat/lng
	payload_t payload = { "precType", t, random_float(100), random_float(100), name};
	char* payload_string = NULL;
        set_payload(payload, &payload_string);

        printf("[%s] Sending location: %s\n", name, payload_string);
	sleep(delay);

        // send some data
        if( send(sock , payload_string , strlen(payload_string) , 0) < 0)
        {
            printf("[%s] Send failed", name);
            return 1;
        }
	t++;
    }

    close(sock);

    return 0;
}