/* NGUYEN LE Duc Tan
   MIF39 Lyon 1 Informatique
***/

// Contient les declarations, les definitions pour les type 
// p2p_search -> file_search
// *p2p_node
// p2p_topology
// server_params* sp
// pthread_mutex_t les_mutex



#ifndef __P2P_OPTIONS
#define __P2P_OPTIONS

/****************************************************/
/****************************************************/

#include <stdio.h>       
#include <sys/types.h>   
#include <sys/time.h>    
#include <sys/select.h>  	/* pour fd_set      */
#include <netinet/in.h>  	/* pour sockaddr_in */
#include <pthread.h> 	 		/* pour le mutex    */
#include <sys/timeb.h>

/****************************************************/
/****************************************************/

#define P2P_OK 0
#define P2P_ERROR -1

/****************************************************/
/****************************************************/

#define P2P_SHORT_SIZE 2
#define P2P_INT_SIZE   4

/****************************************************/
/****************************************************/

#define P2P_DATA_ERROR                   0
#define P2P_DATA_OK                    200
#define P2P_DATA_BAD_REQUEST           400
#define P2P_DATA_UNAUTHORIZED          401
#define P2P_DATA_FORBIDDEN             402
#define P2P_DATA_NOT_FOUND             403
#define P2P_BAD_REQUEST                410
#define P2P_REQUEST_URI_TOO_LARGE      414
#define P2P_INTERNAL_SERVER_ERROR      500
#define P2P_NOT_IMPLEMENTED            501
#define P2P_SERVICE_UNAVAILABLE        502

/****************************************************/
/****************************************************/

#define P2P_NEIGNBOR_LEFT 0xFFFF0000
#define P2P_NEIGNBOR_RIGHT 0x0000FFFF
#define P2P_FILE_NAME_SIZE 200 	// longueur maximal du nom du fichier
#define P2P_MAX_PATH 1024 			// longueur maximal du dir_name et server_name
#define P2P_MAX_THREAD 8 				// le nombre maximal des threads
#define P2P_MAX_SOCKET 64 			// le nombre maximal des sockets

#include "p2p_addr.h"

struct file_search{  // structure contenant des infos sur les recherches
  char *file_name;
  int filesize;
  int nb_result;  
  char* result[33]; 
  int file_size[33];
};

struct p2p_search_t{  // structure pour la recherche
  int nbsearch;
  struct file_search search[100]; // nombre des recherches est limite a 100
};

typedef struct p2p_search_t p2p_search;

struct p2p_node_t {
	p2p_addr addr;
	char     *name;
};

typedef struct p2p_node_t *p2p_node;

struct p2p_topology_t {
	p2p_node Left[16];
	p2p_node Right[16];
};

typedef struct p2p_topology_t p2p_topology;

//Structure contenant tous les parametres d'un noeud P2P. Permet
//d'avoir toutes les varaibles globales definissant l'environnement
//dans une seule structure que l'on peut ainsi passer a toutes les
//fonctions. 
/* server parameters */
struct server_params_t {
  char *server_name;		/* son nom */
  char *dir_name;				/* le directory ou l'on copie les fichiers */
  int verbosity;				/* le niveau de verbosite */

  int port_ui;					/* le numero de port pour l'interface
				   									utilisateur */
  int port_p2p_tcp;			/* le numero de port TCP du noeud */
  int port_p2p_udp;			/* le numero de port UDP du noeud */

  int client_ui;        /* socket connectee par telnet */
	int sock_ui;					/* socket mise en ecoute pour UI */
	int sock_tcp;					/* socket en ecoute pour traffic tcp */
	int sock_udp;					/* socket en ecoute pour traffic udp */


  /* Topology */
  p2p_addr p2pMyId;	        /* son adresse P2P */
	
	// Les adresses des voisins
	p2p_addr p2pLeft;
	p2p_addr p2pRight;

	// Topology 
  p2p_topology p2pTopology;	/* Ses voisins */
	int checkNetwork;
	int repairNetwork;

 	// mutex pour la synchronisation
	pthread_mutex_t log_mutex;

	// Les threads tcp
  pthread_t p2pThread[P2P_MAX_THREAD];
  pthread_t p2pGetThread[P2P_MAX_THREAD];
  struct timeb *bDownload;  //
  struct timeb *eDownload;  //

	int fileId; // id du fichier a telecharger
	
	// Les sockets a servir
	int p2pSocket[P2P_MAX_SOCKET];
	int nextSock; // dernier socket mise en queue p2pSocket
	int lastDone; // dernier socket etant servi
	
  /* Search */
  p2p_search  p2pSearchList;	/* la liste des recherches */
};

typedef struct server_params_t server_params;


server_params* sp;
pthread_mutex_t sock_mutex;  // mutex pour les socket
pthread_mutex_t get_mutex;   // mutex pour pour le telechargement

pthread_cond_t var_cond;    
pthread_cond_t get_cond;

#endif

