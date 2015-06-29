/* NDIAYE Mame Bigue
   NGUYEN LE Duc Tan
   3TC2 2011
***/


#ifndef __P2P_MSG
#define __P2P_MSG

#include <stdio.h>
#include "p2p_addr.h"

#define P2P_VERSION              1

#define P2P_MSG_UNDEFINED        0
#define P2P_MSG_JOIN_REQ         4
#define P2P_MSG_JOIN_ACK         5
#define P2P_MSG_LINK_UPDATE      6
#define P2P_MSG_SEARCH           8
#define P2P_MSG_REPLY            9
#define P2P_MSG_NEIGHBORS_REQ   10
#define P2P_MSG_NEIGHBORS_LIST  11 
#define P2P_MSG_GET             12
#define P2P_MSG_DATA            13

#define P2P_MSG_TTL_NULL         0
#define P2P_MSG_TTL_ONE_HOP      1
#define P2P_MSG_TTL_MAX          16

#define P2P_BAD_MSG_TYPE   0xFF000000

#define P2P_HDR_BITFIELD_SIZE   4 /* bytes */
#define P2P_HDR_SIZE            (P2P_HDR_BITFIELD_SIZE + 2 * P2P_ADDR_SIZE)


typedef struct p2p_msg_struct *p2p_msg;

/*** create / destroy msg ***/

//Cree (alloue) un message P2P dont le payload peut contenir une
//payload_size octets.
p2p_msg  p2p_msg_create      (void);

//Detruit le message msg, i.e., l'ensemble des structures allouees
//(les addresses, l epaylod et le msg lui meme)
void     p2p_msg_delete      (p2p_msg msg);

//Duplique le message dans un nouveau message
p2p_msg  p2p_msg_duplicate   (const p2p_msg msg);

//initialise un message avec type, TTL, src et dest
int      p2p_msg_init        (p2p_msg msg, unsigned int type, unsigned int ttl, const p2p_addr src, const p2p_addr dst);

//renvoie le payload
unsigned char* p2p_msg_get_payload(p2p_msg);

//initialise le payload de msg avec payload de taille length
int      p2p_msg_init_payload(p2p_msg msg, unsigned short int length, unsigned char* payload);

/*** header ***/
//renvoie la version de msg
unsigned char  p2p_msg_get_version (const p2p_msg msg);

//initialise la version de msg a version
void           p2p_msg_set_version (p2p_msg msg, unsigned char version);

//renvoie le type de msg
unsigned char  p2p_msg_get_type    (const p2p_msg msg);

//convertir le type de msg en string
char *p2p_msg_get_type_str(const p2p_msg msg);

//initialise le type de msg a type
void           p2p_msg_set_type    (p2p_msg msg, unsigned char type);

//renvoie le TTL de msg
unsigned char  p2p_msg_get_ttl     (const p2p_msg msg);

//initialise le TTL de msg a ttl
void           p2p_msg_set_ttl     (p2p_msg msg, unsigned char ttl);

//renvoie la longueur de l'entete de msg
unsigned short p2p_msg_get_length  (const p2p_msg msg);

//initialise la longueur de l'entete de msg a length
void           p2p_msg_set_length  (p2p_msg msg, unsigned short length);

//renvoie l'adresse source de msg
p2p_addr       p2p_msg_get_src     (const p2p_msg msg);

//initialise l'adresse source de msg a src
void           p2p_msg_set_src     (p2p_msg msg, p2p_addr src);

//renvoie l'adresse destination de msg
p2p_addr       p2p_msg_get_dst     (const p2p_msg msg);

//initialise l'adrersse destination de msg a dst
void           p2p_msg_set_dst     (p2p_msg msg, p2p_addr dst);

/*** debug ***/
//ecrit le message msg dans le fichier fd. Si print_payload != 0 ecrit
//aussi le payload du message sinon on n'ecrit que l'entete.
//ajouter le parametre sp pour pouvoir afficher sur l'ecran
int p2p_msg_dumpfile       (server_params *sp, const p2p_msg msg, const FILE* fd, int print_payload);

//ecrit l'entete du message msg en hexa. 
int p2p_msg_hexdumpheader  (server_params *sp, unsigned char* msg, int size, const FILE* fs);


//Cree une socket tcp ou udp selon parametre d'entree: 
//mode "tcp" ou "udp"
//numero du port
int p2p_socket_create			 (char *mode, int port);

int p2p_socket_blocking_enable (int sockfd);

/*** tcp ***/
// Cree une socket TCP en ecoute pour le serveur 
// La fonction renvoie le descripteur du socket
// s'il y a l'erreur, elle renvoie -1 
int p2p_tcp_listen_create	 (server_params* sp);


//Cree une socket TCP vers le noeud P2P dst.
int p2p_tcp_socket_create  (server_params* sp, p2p_addr dst);

//Ferme la socket donnee par le descripteur fd
int p2p_tcp_socket_close   (server_params* sp, int fd);

//Envoie le message msg via la socket tcp fd
int p2p_tcp_msg_sendfd     (server_params* sp, p2p_msg msg, int fd);

//recoie dans msg un message depuis la socket fd
int p2p_tcp_msg_recvfd     (server_params* sp, p2p_msg msg, int fd);

//envoie le message msg via tcp au noeud destination indique dans le
//champ dst de msg
int p2p_tcp_msg_send       (server_params* sp, const p2p_msg msg);

/*** udp ***/
//Cree une socket UDP en ecoute pour le serveur
//La fonction renvoie le descripteur de la socket
int p2p_udp_listen_create  (server_params* sp);
//Cree une socket UDP vers le noeud P2P dst.
int p2p_udp_socket_create  (server_params* sp, p2p_addr dst);

//Ferme la socket donnee par le descripteur fd
int p2p_udp_socket_close   (server_params* sp, int fd);

//Envoie le message msg via la socket UDP fd vers dst
int p2p_udp_msg_sendfd     (server_params* sp, p2p_msg msg, int fd, p2p_addr dst);

//recoie dans msg un message depuis la socket UDP fd
int p2p_udp_msg_recvfd     (server_params* sp, p2p_msg msg, int fd);

//envoie le message msg via udp au noeud destination indique dans le
//champ dst de msg
int p2p_udp_msg_send       (server_params* sp, p2p_msg msg);

//rebroadcast le message msg
int p2p_udp_msg_rebroadcast(server_params* sp, p2p_msg msg);

//+++

/* ui */
// creer socket pour l'interface utilisatuer
int p2p_ui_socket_create	 (server_params* sp);

int p2p_ui_socket_close 	 (server_params* sp, int fd);

//+++
#endif
