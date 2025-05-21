
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "network.h"
#include "utils.h"
#include "pthread.h"

pthread_mutex_t globalLock = PTHREAD_MUTEX_INITIALIZER; // global mutex (not necessarily used)

int generateIdentifier(){
  srand(time(NULL));
  return rand();
}


int generatePacket(packet_t *dataPacket, char *filename, int type, int TTL){
  char hostName[MAX_STRLEN];

  dataPacket->ID = generateIdentifier();
  dataPacket->TTL = (unsigned char)TTL;
  dataPacket->descriptor = (unsigned char)type;
  
  gethostname(hostName, MAX_STRLEN);

  strcpy(dataPacket->hostname, hostName);

  if (filename != NULL){
    strcpy(dataPacket->message, filename);
  }

  return dataPacket->ID;
}

int initializeList(IDlist_t *dataList){
  dataList->num = 0;
  return 0;
}

int addToDataList(IDlist_t *dataList, int identifier){
  /* 
   * TODO: check if the ID is already in the list; if not, add it to the list
   * Hint: you can use mutex to ensure thread safety
   */

  // Your code starts here
  int addToDataList(IDlist_t *dataList, int identifier) {
    pthread_mutex_lock(&globalLock);
    for (int i=0; i<dataList->num; i++) {
        if (dataList->IDs[i] == identifier) {
            pthread_mutex_unlock(&globalLock);
            return -1;
        }
    }
    dataList->IDs[dataList->num++] = identifier;
    pthread_mutex_unlock(&globalLock);
    return 0;
  }
  // Your code ends here

  return dataList->num;
}

int findInList(IDlist_t *dataList, int identifier){
  int num = dataList->num;
  int i;

  /*
   * TODO: check if the ID is already in the list; if not, return 0
   */

  // Your code starts here
  for (i = 0; i < num; ++i) {
      if (dataList->IDs[i] == identifier) {
        return 1;
      }
  }
  // Your code ends here

  return 0;
}

void printList(IDlist_t *idList) {
  printf("[DEBUG] Current ID list: [");
  for (int i = 0; i < idList->num; ++i) {
      printf("%d", idList->IDs[i]);
        if (i != idList->num - 1) {
          printf(", ");
      }
  }
  printf("]\n");
}

int initializeNeighbors(neighbors_t *neighborData){
  neighborData->num_neighbors = 0;
  memset(neighborData->neighbor_list, 0, sizeof(neighborData->neighbor_list));
  return 0;
}

int addNeighbor(neighbors_t *neighborData, unsigned long hostIPAddress, int port){
  int index = neighborData->num_neighbors;
  neighbor_t *newNeighbor = &(neighborData->neighbor_list[index]);

  if (neighborData->num_neighbors >= MAX_NEIGHBOR){
    ERROR("ERROR: Exceeds maximum neighbor number\n");
    return -1;
  }
  // TODO: Assign the IP Address and Port to the new neighbor
  neighbor_t *n = &neighborData->neighbor_list[neighborData->num_neighbors];
    n->inaddr = hostIPAddress;
    n->port = port;
    neighborData->num_neighbors++;
  // TODO: update the number of neighbors

  return index;
}

// Contact neighbors and send CONNECT packet to all neighbors
int connectToNeighbors(neighbors_t *neighborData, int myPortNumber){
  printf("[DEBUG] Hello? I am trying to connect ... \n");
  int i;
  packet_t dataPacket;
  int num_neighbors = neighborData->num_neighbors;
  neighbor_t *neighbor;

  // connect to all neighbors stored
  for (i = 0; i < num_neighbors; ++i){
    // TODO: generate packet and send CONNECT packet to the neighbor
    // Hint: functions in network.c may be helpful
        packet_t pkt;
        generatePacket(&pkt, NULL, CONNECT, 0);
        sendToSocket(neighborData->neighbor_list[i].inaddr, 
                    neighborData->neighbor_list[i].port, 
                    &pkt, sizeof(pkt));
  }
  
  return 0;
}

int isNeighborFound(unsigned long hostIPAddress, int port, neighbors_t *neighborData){
  int i;

  for (i = 0; i < neighborData->num_neighbors; ++i){
    if (neighborData->neighbor_list[i].inaddr == hostIPAddress &&
        neighborData->neighbor_list[i].port == port){
      return 1;
    }
  }

  return 0;
} 

void printNeighborData(neighbors_t *neighborData) {
    printf("[NEIGHBOR] Current neighbors\n");
    printf("[NEIGHBOR] Number of neighbors: %d\n", neighborData->num_neighbors);
    printf("[NEIGHBOR] Neighbor list:\n");
    for (int i = 0; i < neighborData->num_neighbors; ++i) {
        printf("[NEIGHBOR] Neighbor %d: IP %s, Port %d\n", i + 1, 
               getHostIp(neighborData->neighbor_list[i].inaddr),
               neighborData->neighbor_list[i].port);
    }
}

int findInDirectory(char *directory, char *file){
  DIR *dp;
  struct dirent *directoryStruct;
  if ( (dp = opendir(directory)) == NULL ){
    return -1;
  }
  // loop through directory, search for the file
  while ( (directoryStruct = readdir(dp)) != NULL ){
    if ( strcmp(file, directoryStruct->d_name) == 0 ){
      return 1;
    }
  }
  closedir(dp);
  return 0;
}

int floodRequest(neighbors_t *neighbors, int portno, packet_t *packet, int size){
  int i;
  int num_neighbors = neighbors->num_neighbors;
  unsigned long inaddr;
    
  for (i = 0; i < num_neighbors; ++ i){
    // TODO: send packet to the neighbor
    // Hint: functions in network.c may be helpful
    if (neighbors->neighbor_list[i].port == portno) continue;
        sendToSocket(neighbors->neighbor_list[i].inaddr,
                    neighbors->neighbor_list[i].port,
                    packet, size);
        }
  return 0;
}