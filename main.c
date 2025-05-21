#include "utils.h"
#include "server.h"
#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>


int main(int argc, char **argv) {
    // Read in port number, TTL, directory, and neighbors from arguments
    int portNumber; // host port number
    int ttl; // host ttl
    char directory[128]; // host directory

    neighbors_t neighbors;
    pthread_mutex_t lock;

    if (argc < 4) {
        ERROR("ERROR: Insufficient arguments provided. Usage: ./p2p_server [PORT_NUM] [TTL] [DIR] [NEIGHBOR_HOST, ...]\n");
    }

    if (argc >= 4) {
        // Parse arguments
        portNumber = atoi(argv[1]);
        ttl = atoi(argv[2]);
        strcpy(directory, argv[3]);

        // Initialize neighbors
        initializeNeighbors(&neighbors);
        int i;
        for (i = 4; i < argc - 1; i += 2) {  // Increment by 2 to process both IP and port at the same time
            unsigned long hostIpAddress = getHostAddr(argv[i]);
            printf("[NEIGHBOR] HostIPaddr: %lu\n", hostIpAddress);
            portNumber = atoi(argv[i+1]);
            if (hostIpAddress < 0) {
                ERROR("ERROR: Could not find host\n");
            }
            // TODO: Save to neighbor data structure
            for (i = 4; i < argc - 1; i += 2) { 
                unsigned long hostIpAddress = getHostAddr(argv[i]);
                int neighborPort = atoi(argv[i+1]); 
                addNeighbor(&neighbors, hostIpAddress, neighborPort);
            }
            
            // print neighbor information
            printNeighborData(&neighbors);
            printf("[NEIGHBOR] Sent connection request to neighbor with address %s:%d \n", argv[i], portNumber);
        }
    }

    IDlist_t idList;
    serverArg_t serverArg;

    // Initialize ID list
    initializeList(&idList);

    /*
     * TODO: Send CONNECT packet to all neighbors
     * Hint: functions in utils.c may be helpful
     */
    // Your code starts here
    connectToNeighbors(&neighbors, portNumber);

    // Your code ends here

    // Add values to server arguments
    serverArg.idList = &idList;
    serverArg.port = portNumber;
    serverArg.directory = directory;
    serverArg.lock = &lock;
    serverArg.neighbors = &neighbors;

    // TODO: create a thread to handle server operations
    pthread_t server_thread;
    pthread_create(&server_thread, NULL, server, &serverArg);

    // Main thread while loop: host sender
    while (1) {
        // Read input from user
        char input[MAX_STRLEN];

        // Input filename for search
        printf("[FILE] Enter the File Name for Search: \n");
        fgets(input, MAX_STRLEN, stdin);
        int len = strlen(input);
        if (input[0] != '\0' && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }
      
        // First search local server directory
        if (findInDirectory(directory, input)) {
            printf("[FILE] Found file '%s' in local directory \n", input);
            continue;
        }
        
        /*
         * TODO: flooding
         * Hint: Ensure thread safety by using a mutex
         */
        
        // Your code starts here
        pthread_mutex_lock(&lock);
        packet_t queryPacket;
        generatePacket(&queryPacket, input, QUERY, ttl);
        floodRequest(neighbors, portNumber, &queryPacket, sizeof(packet_t)); 
        pthread_mutex_unlock(&lock);
        // Your code ends here
    }

    return 0;
}
