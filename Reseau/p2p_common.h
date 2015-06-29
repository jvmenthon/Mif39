/* NDIAYE Mame Bigue
   NGUYEN LE Duc Tan
   3TC2 2011
***/



#ifndef __P2P_COMMON
#define __P2P_COMMON

/****************************************************/
/****************************************************/

#define P2P_OK 0
#define P2P_ERROR -1

/****************************************************/
/****************************************************/
/*	Si on a un fichier "config.h", on compile avec HAVE_CONFIG_H
	pour prendre en compte les constantes ci-dessous. 
	Peut-etre une configuration en fonction du systeme
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#define P2P_SHORT_SIZE SIZEOF_SHORT
#define P2P_INT_SIZE   SIZEOF_INT
#else
#define P2P_SHORT_SIZE 2
#define P2P_INT_SIZE   4
#endif

/****************************************************/
/****************************************************/

#define P2P_DATA_OK                    200
#define P2P_DATA_ERROR                   0

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

#include "p2p_options.h"

/****************************************************/
/****************************************************/

/*************************************
  get_tokens : decoupage d'une chaine en elements
*************************************/

#define MAX_TOK 3
#define MAX_TOKLEN 30
int get_tokens(const char *str, char tok[MAX_TOK][MAX_TOKLEN], int (*test_delim)(char));

/************************************
 VERBOSE : affichage des messages 
       0 - print nothing\n\
       1 - print protocol errors\n\
       2 - trace received msg\n\
       3 - trace msg actions and content\n\
       4 - trace server syscalls\n\
*************************************/

#define VSINFO  0
#define VPROTO  1
#define VMRECV  2
#define VMCTNT  3
#define VSYSCL  4
#define CLIENT 10

void VERBOSE						(server_params* sp, int level, char* fmt, ...); 

p2p_node p2p_node_init	(p2p_addr addr, char *s);

int p2p_node_search			(p2p_node s[16], p2p_addr a);

void p2p_node_delete		(p2p_node a);

void p2p_mutex_lock			(server_params *sp);

void p2p_mutex_unlock		(server_params *sp);

void networkUpdate			(server_params *sp);

int networkRepair				(server_params *sp);

void *p2pthreadget			(void *threadId);

/****************************************************/
/****************************************************/

#endif
