#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "network.h"
#include "utils.h"
#include "server.h"
#include <stdio.h>
#include <dirent.h>
#include <pthread.h>


void *server(void *arg){
  serverArg_t *_arg = (serverArg_t *)arg;
  IDlist_t *idList = _arg->idList;
  int portNum = _arg->port;
  char *directory = _arg->directory;
  neighbors_t *neighbors = _arg->neighbors;
  pthread_mutex_t *mutex = _arg->lock;

  int socketfd = createNewSock();
  packet_t packet;
  unsigned long clientIpAddress;

  if (bindSocket(socketfd, portNum) == -1){
    ERROR("Error: wrong port binding. \n");
  }
  printf("[CONNECT] Successfully bind with the port: %d\n", portNum);

  while (1) {
    // TODO: receive packet
    unsigned long clientIp = recvFromSocket(socketfd, &packet, sizeof(packet));
    unsigned char type = packet.descriptor;

    char *packetType;
    switch(type) {
      case 0: packetType = "CONNECT"; break;
      case 1: packetType = "QUERY"; break;
      case 2: packetType = "RESPONSE"; break;
    }
    printf("[SERVER INFO] Received new packet: %s packet\n", packetType);

    if (type == CONNECT){
      // TODO: handle server connect
      serverHandleConnect(neighbors, packet, mutex);
    }
    else if (type == QUERY){
      // TODO: handle server query
      serverQuery(_arg, packet);

    }
    else if (type == RESPONSE){
      // TODO: handle server response
      serverHandleResponse(packet);

    }
  }
}

int serverHandleConnect(neighbors_t *neighbors, packet_t *packet, pthread_mutex_t *mutex){
  char *hostname;
  unsigned long hostIpAddress;
  int portNum;

  /*
   * TODO: update neighbor list
   * Hint: functions in utils.c and network.c may be helpful
   * Hint: use mutex to ensure thread safety
   */

  // Your code starts here

  printf("[NEIGHBOR] New neighbor connecting request. From address: %s:%d\n", 
           getHostIp(hostIpAddress), portNum);
  pthread_mutex_lock(mutex);
  unsigned long ip = getHostAddr(packet->hostname);
  int port = packet->port;
  if (!isNeighborFound(ip, port, neighbors)) {
      addNeighbor(neighbors, ip, port);
  }
  pthread_mutex_unlock(mutex);

  // Your code ends here

  printf("[NEIGHBOR] New neighbor connected!\n");
  return 0;
}

int serverQuery(serverArg_t *args, packet_t *packet) {

  /*
   * TODO: handle server query
   * Hint: always remember thread safety
   */

  packet_t respon_packet;

  char *Dir;
  IDlist_t *IDlist;
  int port;
  neighbors_t *neighbors;
  pthread_mutex_t *lock;

  // TODO: if found in ID list
  if (findInList(args->idList, packet->ID)) return -1;
  addToDataList(args->idList, packet->ID);

  printf("[DUPLICATE ID] Duplicate packet with ID %d detected. Ignoring and discarding.\n", packet->ID);
  
  // TODO: if file found in directory, return the msg back to query host
  if (findInDirectory(args->directory, packet->message)) {
      packet_t resp;
      generatePacket(&resp, packet->message, RESPONSE, 0);
      resp.port = args->port;
      sendToSocket(packet->inaddr, packet->port, &resp, sizeof(resp));
      return 1;
  }

  printf("[RESPON] File %s found, responding to query.\n", packet->message);

  // TODO: TTL check
  if (packet->TTL <= 0) {
        printf("[TTL] Packet TTL expired\n");
        return -1;
    }
  packet->TTL--;

  printf("[TTL] Packet TTL expired, stopping forwarding.\n");

  // TODO: else not found, flood to neighbors
  pthread_mutex_lock(args->lock);
  floodRequest(args->neighbors, args->port, packet, sizeof(*packet));
  pthread_mutex_unlock(args->lock);

  printf("[TTL] Forwarding packet with TTL=%d\n", packet->TTL);

  return 0;
}

int serverHandleResponse(packet_t *packet){
    printf("\n[FILE] File %s found on address %s, with port %d\n\n",
         packet->message, packet->hostname,
         packet->port);
  return 0;
}
