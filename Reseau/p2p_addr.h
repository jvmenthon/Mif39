/* NGUYEN LE Duc Tan
   MIF39 Lyon 1 Informatique
***/


#ifndef __P2P_ADDR
#define __P2P_ADDR

#include <stdio.h>

//Taille d'une adresse P2P en octets.
#define P2P_ADDR_SIZE (P2P_INT_SIZE + 2 * P2P_SHORT_SIZE)

struct p2p_addr_struct {
	struct in_addr ip;
	uint16_t tcp_port;
	uint16_t udp_port;
};

//On n'exporte qu'un pointeur sur la structure p2p_addr qui elle reste
//cachee au sien du module p2p_addr.c
typedef struct p2p_addr_struct *p2p_addr;

//Creer et donc alloue une structure p2p_addr. retourne une p2p_addr,
//donc un pointeur sur la structure cree
p2p_addr p2p_addr_create();

//Supprime une adresse p2p_addr. (i.e., supprime la structure pointe
//par addr
void     p2p_addr_delete(p2p_addr addr);

//Copie l'adresse source src dans l'adress destination dst. La
//structure dst doit etre allouee avant.
void     p2p_addr_copy(p2p_addr dst, p2p_addr src);

//Duplique l'adresse p2p addr et retourne une nouvelle structure
//contenant une copie de addr. Cette fonction alloue donc une nouvelle
//structure. 
p2p_addr p2p_addr_duplicate(p2p_addr addr);

//Compare les 2 adresses P2P. Renvoie 0 si elle sont differentes,
//i.e., si la partie IP n'est pas la meme ou si les ports employes
//sont differents.
int p2p_addr_is_equal(const p2p_addr addr1, const p2p_addr addr2);

//Renvoie 0 si l'adresse addr n'est pas uen adresse de broadcast P2P
int p2p_addr_is_broadcast(const p2p_addr addr);

//Renvoie 0 si l'adresse addr n'est pas l'adresse non definie P2P
int p2p_addr_is_undefined(const p2p_addr addr);

//renvoie un pointeur sur l'adresse P2P de broadcast
p2p_addr p2p_addr_broadcast();

//renvoie un pointeur sur l'adresse P2P non definie
p2p_addr p2p_addr_undefined();

//assigne l'adresse P2P dst. Les parametres sont l'adresse IP ip_str sous
//forme de chaine, l eportt tcp et udp sous la forme d entier short
//non signe. 
int   p2p_addr_set(p2p_addr dst, const char* ip_str, unsigned short tcp, unsigned short udp);

//assigne l'adresse dst a partir d'une adresse sous forme de string,
//i.e., IP:TCP_PORT:UDP_PORT
int   p2p_addr_setstr(p2p_addr dst, const char* p2p_str);

//assigne l'adresse addr a P2P broadcast : 255.255.255.255:0:0
void  p2p_addr_set_broadcast(p2p_addr addr);

//assigne l'adresse addr a P2P undifinied : 0.0.0.0:0:0
void  p2p_addr_set_undefined(p2p_addr addr);

//renvoie addr sous forme de chaine
char*          p2p_addr_get_str     (p2p_addr addr);

//renvoie la partie IP de addr sous forme de chaine
char*          p2p_addr_get_ip_str  (p2p_addr addr);

//renvoie le port TCP de addr
unsigned short p2p_addr_get_tcp_port(p2p_addr addr);

//renvoie le port UDP de addr
unsigned short p2p_addr_get_udp_port(p2p_addr addr);

//ecrit l'adresse addr dans le fichier fd (file descriptor)
void p2p_addr_dump(const p2p_addr addr, int fd);

//ecrit l'adresse formatee addr dans le fichier fd 
void p2p_addr_dumpfile(const p2p_addr addr, const FILE *fd);

#endif
