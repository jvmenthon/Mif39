/* NGUYEN LE Duc Tan
   MIF39 Lyon 1 Informatique
***/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h>  // pour la structure sockaddr_in
#include <netdb.h> 
#include <assert.h>


/* pour la fonction inet_aton() */
#include <arpa/inet.h>

/* pour la fonction perror affichant l'erreur */
#include <errno.h>
#include <sys/ioctl.h>

#define BACKLOG 3 // nombre de connexion en attente


#include "p2p_common.h"
#include "p2p_addr.h"
#include "p2p_msg.h"
#include "p2p_file.h"

#define MIN(a,b)    (((a)<(b))?(a):(b))

//Definition de la structure de l'entete d'un message P2P
struct p2p_msg_hdr_struct {
  unsigned char  version_type;	/* Les champs Version et CmdType sont
				   codes tous les deux sur 1 octet */ 
  unsigned char  ttl;		/* Le champ  TTTL*/
  unsigned short length;	/* Le champ longueur */
  p2p_addr src;			/* Le champ adresse source */
  p2p_addr dst;			/* Le champ adresse destination */
};

//Definition du type p2p_msg_hdr qui est un pointeur sur la structure
//p2p_msg_hdr_struct 
typedef struct p2p_msg_hdr_struct p2p_msg_hdr;

//Definition de la structure d'un message P2P
struct p2p_msg_struct {
  p2p_msg_hdr hdr;		/* Un entete */
  unsigned char *payload;	/* Un payload qui un pointeur sur une
				   zone memoire de unsigned char */
};

p2p_msg
p2p_msg_create()
{
  p2p_msg msg;
  if ((msg = (p2p_msg) malloc(P2P_HDR_SIZE)) == NULL)
    return NULL;
  p2p_msg_set_version (msg,P2P_VERSION);
  p2p_msg_set_type    (msg,P2P_MSG_UNDEFINED);
  assert(msg->hdr.src = p2p_addr_create());
  assert(msg->hdr.dst = p2p_addr_create());
  msg->payload        = NULL;

  return msg;
} 


void 
p2p_msg_delete(p2p_msg msg)
{
//  p2p_addr_delete(msg->hdr.src);
//  p2p_addr_delete(msg->hdr.dst);
//  if (msg->payload != NULL)
//    free(msg->payload);
//  free(msg);
} 


p2p_msg 
p2p_msg_duplicate(const p2p_msg msg)
{
  p2p_msg msgDuplicata;
  
  if ((msgDuplicata = p2p_msg_create()) == NULL)
    return NULL;

  p2p_msg_set_version(msgDuplicata,p2p_msg_get_version(msg));
  p2p_msg_set_type   (msgDuplicata,p2p_msg_get_type   (msg));
  p2p_msg_set_ttl    (msgDuplicata,p2p_msg_get_ttl    (msg)); 
	p2p_msg_set_length (msgDuplicata,p2p_msg_get_length	(msg)); // [!] 
  msgDuplicata->hdr.src  = p2p_addr_duplicate(msg->hdr.src);
  msgDuplicata->hdr.dst  = p2p_addr_duplicate(msg->hdr.dst);

  assert(msgDuplicata->payload = (unsigned char*)malloc(sizeof(char)*p2p_msg_get_length(msg))); 
  memcpy(msgDuplicata->payload, msg->payload, p2p_msg_get_length(msg));
  return msgDuplicata;
} 


int 
p2p_msg_init(p2p_msg msg, 
	     unsigned int type, unsigned int ttl,
	     const p2p_addr src, const p2p_addr dst)
{
  p2p_msg_set_version(msg,P2P_VERSION);
  p2p_msg_set_type   (msg,type);
  p2p_msg_set_ttl    (msg,ttl); 
	p2p_addr_copy      (msg->hdr.src,src);
  p2p_addr_copy      (msg->hdr.dst,dst);

  return P2P_OK;
} 


unsigned char* 
p2p_msg_get_payload(p2p_msg msg) {
	return msg->payload;
}

int 
p2p_msg_init_payload(p2p_msg msg,
		     const unsigned short int length,
		     unsigned char* payload)
{
	printf("INIT PAYLOAD : length of payload = %d\n",length);
  p2p_msg_set_length(msg,length); 
  assert(msg->payload = (unsigned char*)malloc(sizeof(unsigned char)*length));
  memcpy(msg->payload,payload,length);
  return P2P_OK;
} 

/*********************************************************/
/* Header                                                */
/*********************************************************/

/********************/
/* VERSION | TYPE   */
/********************/
unsigned char 
p2p_msg_get_version(const p2p_msg msg) 
{
  return (msg->hdr.version_type & 0xF0) >> 4;
}

void 
p2p_msg_set_version(p2p_msg msg, unsigned char version) 
{
  msg->hdr.version_type = ((version & 0x0F) << 4) | p2p_msg_get_type(msg);
}

unsigned char 
p2p_msg_get_type(const p2p_msg msg) 
{
  return msg->hdr.version_type & 0x0F;
}

char *p2p_msg_get_type_str(const p2p_msg msg)
{
	unsigned char type = p2p_msg_get_type(msg);
	switch (type) {
		case 4:  return "P2P_MSG_JOIN_REQ"; 			break;
		case 5:	 return "P2P_MSG_JOIN_ACK"; 			break;
		case 6:	 return "P2P_MSG_LINK_UPDATE"; 		break;
		case 8:	 return "P2P_MSG_SEARCH"; 				break;
		case 9:	 return "P2P_MSG_REPLY"; 					break;
		case 10: return "P2P_MSG_NEIGHBORS_REQ"; 	break;
		case 11: return "P2P_MSG_NEIGHBORS_LIST"; break;
		case 12: return "P2P_MSG_GET"; 						break;
		case 13: return "P2P_MSG_DATA"; 					break;

		default: return "P2P_MSG_UNDEFINED";
	}
}

void 
p2p_msg_set_type(p2p_msg msg, unsigned char type) 
{
  msg->hdr.version_type = (p2p_msg_get_version(msg) << 4) | (type & 0x0F);
}



/********************/
/* A COMPLETER      */
/********************/


unsigned char 
p2p_msg_get_ttl(const p2p_msg msg) {
  return (unsigned char)msg->hdr.ttl;
}

void 
p2p_msg_set_ttl(p2p_msg msg, unsigned char ttl) {
	msg->hdr.ttl = ttl;
}

unsigned short 
p2p_msg_get_length(const p2p_msg msg) {
	return ntohs(msg->hdr.length);
}

void 
p2p_msg_set_length(p2p_msg msg, unsigned short length) {
	msg->hdr.length = htons(length); 
}

p2p_addr 
p2p_msg_get_src(const p2p_msg msg) {
	return msg->hdr.src;	
}

void 
p2p_msg_set_src(p2p_msg msg, p2p_addr src) {
	p2p_addr_copy(msg->hdr.src,src);
}

p2p_addr 
p2p_msg_get_dst(const p2p_msg msg) {
	return msg->hdr.dst;
}

void 
p2p_msg_set_dst(p2p_msg msg, p2p_addr dst) {
	p2p_addr_copy(msg->hdr.dst,dst);
}

/*** debug ***/
int 
p2p_msg_dumpfile(server_params* sp, const p2p_msg msg, const FILE* fd, int print_payload) {
	// on a le FILE pointeur => file déjà ouvri, 
	// pas de possibilité de vérifier si le mode de l'ouvreture est write
	// précond: file: ouvri, # NULL
	
	fprintf((FILE*)fd,"%s\t** ===========================================================\n", sp->server_name);
	fprintf((FILE*)fd,"%s\t** version=%d, type=%s (%d), TTL=%d, data length=%d\n", sp->server_name, p2p_msg_get_version(msg), p2p_msg_get_type_str(msg), p2p_msg_get_type(msg), p2p_msg_get_ttl(msg), p2p_msg_get_length(msg));
	fprintf((FILE*)fd,"%s\t** src : %s\n", sp->server_name, p2p_addr_get_str(p2p_msg_get_src(msg)));
	fprintf((FILE*)fd,"%s\t** dst : %s\n", sp->server_name, p2p_addr_get_str(p2p_msg_get_dst(msg)));
	
	if (print_payload != 0) { // imprimmer le payload
		unsigned char *payload = p2p_msg_get_payload(msg);
		if (payload == NULL) 
			fprintf((FILE*)fd,"%s\t** payload : (null)\n", sp->server_name);
		else {
			unsigned char type_msg = p2p_msg_get_type(msg);
			
			switch (type_msg) {
				case P2P_MSG_NEIGHBORS_LIST: {
					char *node_name;
					int l;
					unsigned char Nb_neighbors;
					p2p_addr addr_neighbor = p2p_addr_create();
					
					memcpy(&Nb_neighbors, payload, sizeof(Nb_neighbors));
					memcpy((char*)addr_neighbor, payload + 4, P2P_ADDR_SIZE);
					
					fprintf((FILE*)fd,"%s\t** number of neighbors: %d\n", sp->server_name, Nb_neighbors);	
					fprintf((FILE*)fd,"%s\t** padding (3 bytes)\n", sp->server_name);	
					fprintf((FILE*)fd,"%s\t** neighbor(0) %s\n", sp->server_name, p2p_addr_get_str(addr_neighbor));	
					if (Nb_neighbors == 2) {
						memcpy((char*)addr_neighbor, payload + 4 + P2P_ADDR_SIZE, P2P_ADDR_SIZE);
						fprintf((FILE*)fd,"%s\t** neighbor(1) %s\n", sp->server_name, p2p_addr_get_str(addr_neighbor));	
					}	
					l = p2p_msg_get_length(msg) - 4 - (Nb_neighbors * P2P_ADDR_SIZE);
					node_name = malloc(l + 1);
					memcpy(node_name, payload + 4 + (Nb_neighbors * P2P_ADDR_SIZE), l);
					node_name[l] = '\0';
					fprintf((FILE*)fd,"%s\t** node name : %s\n", sp->server_name, node_name);	
					free(node_name);
					break;
				}
				
				case P2P_MSG_NEIGHBORS_REQ: {
					p2p_addr noeud_init = p2p_addr_create();
					memcpy(noeud_init, payload, P2P_ADDR_SIZE);
					fprintf((FILE*)fd,"%s\t** initiator: %s\n", sp->server_name, p2p_addr_get_str(noeud_init));	
					p2p_addr_delete(noeud_init);
					break;
				}

				case P2P_MSG_GET: {
				int l = p2p_msg_get_length(msg) - 8;
				int boffset, eoffset;
				char *filename = malloc(l + 1);
				memcpy(&boffset, payload, 4);
				memcpy(&eoffset, payload + 4, 4);
				memcpy(filename, payload + 8, l);
				boffset = ntohl(boffset);
				eoffset = ntohl(eoffset);
				filename[l] = '\0';
				
				fprintf((FILE*)fd,"%s\t** begin offset : %d\n", sp->server_name, boffset);	
				fprintf((FILE*)fd,"%s\t** end   offset : %d\n", sp->server_name, eoffset);	
				fprintf((FILE*)fd,"%s\t** filename     : %s\n", sp->server_name, filename);	
				free(filename);	
					break;
				}
				
				case P2P_MSG_DATA: {
				unsigned char status_code;
				int size; 
				memcpy(&status_code, payload, sizeof(status_code));
				memcpy(&size, payload + 4, 4);
				size = ntohl(size);

				if (status_code == P2P_DATA_OK) {
					int l = p2p_msg_get_length(msg) - 8; // longueur du data
					char *data = malloc(l + 1);
					memcpy(data, payload + 8, l);
					data[l] ='\0';
					fprintf((FILE*)fd,"%s\t** status    : %d P2P_DATA_OK\n", sp->server_name, status_code);	
					fprintf((FILE*)fd,"%s\t** file size : %d\n", sp->server_name, size);	
					free(data);
				}
				else { // le cas d'erreur
					fprintf((FILE*)fd,"%s\t** status     : %d P2P_DATA_ERROR\n", sp->server_name, status_code);	
					switch (size) {
						
						case 400: 
							fprintf((FILE*)fd,"%s\t** error code : %d P2P_DATA_BAD_REQUEST\n", sp->server_name, size);	
							break;
						case 401: 
							fprintf((FILE*)fd,"%s\t** error code : %d P2P_DATA_UNAUTHORIZED\n", sp->server_name, size);	
							break;
						case 402: 
							fprintf((FILE*)fd,"%s\t** error code : %d P2P_DATA_FORBIDDEN\n", sp->server_name, size);	
							break;
						case 403: 
							fprintf((FILE*)fd,"%s\t** error code : %d P2P_DATA_NOT_FOUND\n", sp->server_name, size);	
							break;
						case 410: 
							fprintf((FILE*)fd,"%s\t** error code : %d P2P_BAD_REQUEST\n", sp->server_name, size);	
							break;
						case 414: 
							fprintf((FILE*)fd,"%s\t** error code : %d P2P_REQUEST_URI_TOO_LARGE\n", sp->server_name, size);	
							break;
						case 500: 
							fprintf((FILE*)fd,"%s\t** error code : %d P2P_INTERNAL_SERVER_ERROR\n", sp->server_name, size);	
							break;
						case 501: 
							fprintf((FILE*)fd,"%s\t** error code : %d P2P_NOT_IMPLEMENTED\n", sp->server_name, size);	
							break;
						case 502: 
							fprintf((FILE*)fd,"%s\t** error code : %d P2P_SERVICE_UNAVAILABLE\n", sp->server_name, size);	
							break;
					}
				}
					break;
				}

				default:
					fprintf((FILE*)fd,"%s\t** (no description available)\n", sp->server_name);	
			}
		}
	}

	fprintf((FILE*)fd,"%s\t** ===========================================================\n", sp->server_name);
	return 0;
}

// precond: file ouvert en mode write
// 					msg est sous forme d'une chaine de caratere 
// 					=> ajouter cette fonction lors de l'emission ou reception 
// 					car on utilise un buffer (char*) pour stocker msg
//					size: la taille du msg en octets
int 
p2p_msg_hexdumpheader(server_params *sp, unsigned char *msg, int size, const FILE* fs){
	return 0;
}


int 
p2p_socket_create	(char* mode, int port) {
	
	struct sockaddr_in server_addr;
	int sockfd; // descripteur de socket

	if (strncmp(mode,"tcp",4) == 0) {
		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0))==-1) {
				perror("Erreur de creation\n"); 
				return -1;
		}
	}
	else if ((strncmp(mode,"udp",4)) == 0) {
		if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0))==-1) { 
				perror("Erreur de creation\n"); 
				return -1;
		}
	}

	/* preparation de l'adresse d'attachement */
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY; /* rempli automatiquement */
	
	/* reouverture de socket en TIMEWAIT */
	/* SOL_SOCKET: manipuler au niveau socket */
	int yes = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void*)&yes, sizeof(yes))== -1){	
			perror("setsockopt erreur\n");
			return -1;
	}
	
	if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof (server_addr)) == -1) {
			perror("bind erreur\n");
			close(sockfd);
			return -1;
	}
	return sockfd;
}


int p2p_socket_non_blocking_enable(int fd) {
	int bloc = 1;
	if (ioctl(fd, FIONBIO, &bloc) < 0) {
		perror("ioctl error\n");
		return P2P_ERROR;
	}
	return P2P_OK;
}

/*********************************************************/
/***************        Partie TCP        ****************/
/*********************************************************/
int 
p2p_tcp_listen_create(server_params* sp) {
	int sockfd;
	VERBOSE(sp, VSYSCL," Create socket tcp listening on port %d :\n",sp->port_p2p_tcp);
	if ((sockfd = p2p_socket_create("tcp", sp->port_p2p_tcp)) == -1) {
		VERBOSE(sp, VSYSCL,"  [Fail]\n ERROR: socket create\n");
		return -1;
	}
	
	if (listen(sockfd, BACKLOG) == -1) {
		VERBOSE(sp, VSYSCL,"  [Fail]\n ERROR: socket listen\n");
		return -1;
	}
	
	if (p2p_socket_non_blocking_enable(sockfd) == -1) {
		return -1;
	}

	VERBOSE(sp, VSYSCL,"  [OK]\n");
	return sockfd;
}	

/***************    TCP Create Socket    ****************/
int 
p2p_tcp_socket_create(server_params* sp, p2p_addr dst) {
	struct sockaddr_in dst_addr;
	int sockfd;
	int port = 0;
	int lg = sizeof(dst_addr);
	struct hostent *hp;
	char *ip = p2p_addr_get_ip_str(dst);

	VERBOSE(sp,VSYSCL," creating socket tcp:\n");
	if ((sockfd = p2p_socket_create("tcp", port)) == -1) {
		VERBOSE(sp,VSYSCL,"  [Fail]\n ERROR: socket create\n");
		return -1;
	} 
	else VERBOSE(sp,VSYSCL,"  [OK]\n");
	
	VERBOSE(sp,VSYSCL," gethostbyname:\n");
	if ((hp = gethostbyname(ip)) == NULL) 
		VERBOSE(sp,VSYSCL,"  [Fail]\n ERROR: host not found\n");
	else VERBOSE(sp,VSYSCL,"  [OK]\n");
	
	/* preparaton de l'addresse destinatrice */
	dst_addr.sin_family = AF_INET;
	dst_addr.sin_port = htons(p2p_addr_get_tcp_port(dst));
	memcpy(&(dst_addr.sin_addr.s_addr), hp->h_addr, hp->h_length);

	/* demande de connexion au serveur */
	VERBOSE(sp,VSYSCL," connecting:\n");
	if (connect(sockfd, (struct sockaddr*)&dst_addr, lg) == -1) {
		VERBOSE(sp,VSYSCL,"  [Fail]\n ERROR: socket connect\n");
		return -1;
	}
	else VERBOSE(sp, VSYSCL," Connect [OK]\n");
	return sockfd;
}

/******************   TCP Close Socket    *****************/
int 
p2p_tcp_socket_close(server_params* sp, int fd) {
	VERBOSE(sp,VSYSCL," closing socket tcp:\n");
	if (close(fd) == 0) {
		VERBOSE(sp,VSYSCL,"  [OK]\n");
		return 0;
	}
	else {
		VERBOSE(sp,VSYSCL,"  [Fail]\n ERROR: socket close\n");
		return -1;
	}
}

/***************        TCP SendFD        ****************/
int 
p2p_tcp_msg_sendfd(server_params* sp, p2p_msg msg, int fd) {
	unsigned char *buffer = (unsigned char *)malloc(P2P_HDR_SIZE + p2p_msg_get_length(msg));

// copier le contenu de message dans le buffer
	memcpy(buffer, (char*)&(msg->hdr), P2P_HDR_BITFIELD_SIZE);
  memcpy(buffer + P2P_HDR_BITFIELD_SIZE, (char*)(msg->hdr.src), P2P_ADDR_SIZE);
	memcpy(buffer + P2P_HDR_BITFIELD_SIZE + P2P_ADDR_SIZE, (char*)(msg->hdr.dst), P2P_ADDR_SIZE);
	memcpy(buffer + P2P_HDR_BITFIELD_SIZE + 2 * P2P_ADDR_SIZE, p2p_msg_get_payload(msg) , p2p_msg_get_length(msg));

	VERBOSE(sp, VSYSCL," SENDING MSG :\n");
	VERBOSE(sp, VSYSCL," length = %d\n",p2p_msg_get_length(msg));
	
	if (write(fd, buffer, (P2P_HDR_SIZE + p2p_msg_get_length(msg))) == -1) {
		VERBOSE(sp,VSYSCL,"  [Fail]\n ERROR: socket write\n");
		perror("erreur write\n");
		free(buffer);
		return -1;
	}
	
	VERBOSE(sp,VSYSCL,"  [OK]\n");
// afficher msg sur l'ecran
	p2p_msg_hexdumpheader(sp, buffer, P2P_HDR_SIZE + p2p_msg_get_length(msg), stderr);
	p2p_msg_dumpfile(sp, msg, stderr, 1);
	free(buffer);
	return 0;
}

/***************        TCP RecvFD      ****************/

int p2p_tcp_msg_recvfd     (server_params* sp, p2p_msg msg, int fd)
{
  unsigned char* buffer_hdr;
  unsigned char* buffer_payload;
	unsigned char *newbuffer = NULL;
    //Allocation
  buffer_hdr=(unsigned char*)malloc(20*sizeof(char));
  if (read(fd,buffer_hdr,20) > 0) {
  
    //copier l'entête dans le msg.
		memcpy((char*)&(msg->hdr), buffer_hdr, P2P_HDR_BITFIELD_SIZE);
		memcpy((char*)(msg->hdr.src), buffer_hdr + P2P_HDR_BITFIELD_SIZE, P2P_ADDR_SIZE);
		memcpy((char*)(msg->hdr.dst), buffer_hdr + P2P_HDR_BITFIELD_SIZE + P2P_ADDR_SIZE, P2P_ADDR_SIZE);	
  if (p2p_msg_get_length(msg) > 0) {  //Si le payload n'est pas vide 
    buffer_payload=(unsigned char*)malloc(p2p_msg_get_length(msg)*sizeof(char));

/* Important: Dans le cas ou la taille du payload est très grand, le message arrive en "morceaux"
il faut alors faire une boucle pour récupérer tout le message !!!!!!!!!!*/

    int data_downloaded, data_remained, temp;
    data_downloaded = 0;
    data_remained = p2p_msg_get_length(msg);
    while (data_remained > 0) {
      temp = read(fd,buffer_payload+data_downloaded,data_remained);
      if (temp < 1) {
				VERBOSE(sp,VSYSCL,"  [Fail]\n ERROR: read payload\n");
        free(buffer_hdr);
        free(buffer_payload);
        return P2P_ERROR;
      }
      data_remained -= temp;
      data_downloaded += temp;
    }
      //copier le payload dans le message
    p2p_msg_init_payload(msg, p2p_msg_get_length(msg), buffer_payload);
		newbuffer = (unsigned char*)malloc(P2P_HDR_SIZE + p2p_msg_get_length(msg));
		memcpy(newbuffer, buffer_hdr, P2P_HDR_SIZE);
		memcpy(newbuffer + P2P_HDR_SIZE, buffer_payload, p2p_msg_get_length(msg));
		free(buffer_hdr);
		buffer_hdr=NULL;
  }
	VERBOSE(sp,VSYSCL,"  [OK]\n");
	if (buffer_hdr != NULL) {
			p2p_msg_hexdumpheader(sp, buffer_hdr, P2P_HDR_SIZE, stderr);
			VERBOSE(sp,VSYSCL," total %d bytes received\n", P2P_HDR_SIZE);
			free(buffer_hdr);
		}
		else {
			p2p_msg_hexdumpheader(sp, newbuffer, P2P_HDR_SIZE + p2p_msg_get_length(msg), stderr);
			VERBOSE(sp,VSYSCL," total %d bytes received\n", P2P_HDR_SIZE + p2p_msg_get_length(msg));
			free(newbuffer);
		}
	p2p_msg_dumpfile(sp, msg, stderr, 1);
  }
  return P2P_OK;
}

/***************         TCP Send         ****************/

int 
p2p_tcp_msg_send(server_params* sp, const p2p_msg msg) {
	int sockfd;
	p2p_addr dst = p2p_msg_get_dst(msg);
	if ((sockfd = p2p_tcp_socket_create(sp, dst)) == P2P_ERROR) 		
		return P2P_ERROR;
	if ((p2p_tcp_msg_sendfd(sp, msg, sockfd)) == P2P_OK) {
		p2p_tcp_socket_close(sp,sockfd);
		return P2P_OK;
	}
	else return P2P_ERROR;
}

/*********************************************************/
/***************        Partie UDP        ****************/
/*********************************************************/


/***************     UDP Listen Create     ***************/
int 
p2p_udp_listen_create(server_params* sp) {
	int sockfd;
	VERBOSE(sp,VSYSCL, " Create socket udp listening on port %d :\n", sp->port_p2p_udp);
	if ((sockfd = p2p_socket_create("udp", sp->port_p2p_udp)) == -1) {
		VERBOSE(sp,VSYSCL,"  [Fail]\n ERROR: socket create\n");
		return -1;
	}
	
	if (p2p_socket_non_blocking_enable(sockfd) == -1) {
		return -1;
	}
	VERBOSE(sp,VSYSCL,"  [OK] \n");
	return sockfd;
}

/***************     UDP Socket Create    ***************/
int 
p2p_udp_socket_create(server_params* sp, p2p_addr dst) {
	int sockfd;
	int port = 0;
	
	VERBOSE(sp,VSYSCL," creating socket udp:\n");
	if ((sockfd = p2p_socket_create("udp", port)) == -1) {
		VERBOSE(sp,VSYSCL,"  [Fail]\n ERROR: socket create\n");
		return -1;
	} 
	else {
		VERBOSE(sp,VSYSCL,"  [OK]\n");
		return sockfd;
	}
}

/***************    UDP Socket Close    ***************/
int 
p2p_udp_socket_close(server_params* sp, int fd) {
	VERBOSE(sp, VSYSCL," closing socket udp:\n");
	if (close(fd) == 0) {
		VERBOSE(sp, VSYSCL,"  [OK]\n");
		return 0; 
	}
	else { 
		VERBOSE(sp, VSYSCL,"  [Fail]\n ERROR: socket close\n");
		return -1;
	}
}
/***************        UDP SendFD        ****************/
int 
p2p_udp_msg_sendfd(server_params* sp, p2p_msg msg, int fd, p2p_addr dst) {
	unsigned char* buffer = (unsigned char *)malloc(P2P_HDR_SIZE + p2p_msg_get_length(msg));
	char *ip = p2p_addr_get_ip_str(dst);
	struct hostent *hp;
	struct sockaddr_in dst_addr;

	// copier le contenu de message dans le buffer
	memcpy(buffer, (char*)&(msg->hdr), P2P_HDR_BITFIELD_SIZE);
  memcpy(buffer + P2P_HDR_BITFIELD_SIZE, (char*)(msg->hdr.src), P2P_ADDR_SIZE);
	memcpy(buffer + P2P_HDR_BITFIELD_SIZE + P2P_ADDR_SIZE, (char*)(msg->hdr.dst), P2P_ADDR_SIZE);
	memcpy(buffer + P2P_HDR_BITFIELD_SIZE + 2 * P2P_ADDR_SIZE, p2p_msg_get_payload(msg) , p2p_msg_get_length(msg));

	VERBOSE(sp,VSYSCL," gethostbyname:\n");
	if ((hp = gethostbyname(ip)) == NULL)
		VERBOSE(sp,VSYSCL,"  [Fail]\n ERROR: host not found\n");
	else VERBOSE(sp,VSYSCL,"  [OK]\n");

	dst_addr.sin_family = AF_INET;
	dst_addr.sin_port = htons(p2p_addr_get_udp_port(dst));
	memcpy(&(dst_addr.sin_addr.s_addr), hp->h_addr, hp->h_length);
	
	VERBOSE(sp,VSYSCL," sending  msg:\n");
	if (sendto(fd, buffer, P2P_HDR_SIZE + p2p_msg_get_length(msg), 0, (struct sockaddr*)&dst_addr, sizeof(dst_addr)) == -1) {
		VERBOSE(sp, VSYSCL,"  [Fail]\n ERROR: socket sendto\n");
		free(buffer);
		return -1;
	} 	
	VERBOSE(sp, VSYSCL,"  [OK]\n");

	// afficher msg sur l'ecran
	p2p_msg_dumpfile(sp, msg, stderr, 1);
	p2p_msg_hexdumpheader(sp, buffer, P2P_HDR_SIZE + p2p_msg_get_length(msg), stderr);
	free(buffer);

	return 0;
}

/***************        UDP RecvFD        ****************/
int 
p2p_udp_msg_recvfd(server_params* sp, p2p_msg msg, int fd) {
	unsigned char *buffer = (unsigned char *)malloc(P2P_HDR_SIZE);
	struct sockaddr_in scr_addr;
	unsigned int lg = sizeof(scr_addr);
	int size; // juste pour la fonction p2p_msg_hexdumpheader()

	if (recvfrom(fd, buffer, P2P_HDR_SIZE, MSG_PEEK, (struct sockaddr*)&scr_addr, &lg) > 0) {
		VERBOSE(sp,VSYSCL," receiving msg:\n");
	// copier l'entete du message dans le buffer
		memcpy((char*)&(msg->hdr), buffer, P2P_HDR_BITFIELD_SIZE);
		memcpy((char*)(msg->hdr.src), buffer + P2P_HDR_BITFIELD_SIZE, P2P_ADDR_SIZE);
		memcpy((char*)(msg->hdr.dst), buffer + P2P_HDR_BITFIELD_SIZE + P2P_ADDR_SIZE, P2P_ADDR_SIZE);				
		size = P2P_ADDR_SIZE;

		unsigned short l = p2p_msg_get_length(msg);
		if (l > 0) {
			free(buffer);
			buffer = (unsigned char *)malloc(P2P_HDR_SIZE + l);
			
			if (recvfrom(fd, buffer, P2P_HDR_SIZE + l, 0, (struct sockaddr*)&scr_addr, &lg) == -1) {
				perror("erreur recvfrom\n");
				free(buffer);			
				return -1;
			}
			else { 
				unsigned char *payload = (unsigned char *)malloc(l);			
				memcpy((char*)payload,(char*)buffer+P2P_HDR_SIZE,l);
				p2p_msg_init_payload(msg, l, payload);			
				free(payload);
				size = size + l;
			}
		}	
		VERBOSE(sp,VSYSCL,"  [OK]\n");
		p2p_msg_hexdumpheader(sp, buffer, size, stderr);
		p2p_msg_dumpfile(sp, msg, stderr, 1);
		VERBOSE(sp,VSYSCL," total %d bytes received\n", size );
		
		return 0;
	} 
	else {
		free(buffer);
		return -1;
	}
}

/*******************    UDP Send     *******************/
int 
p2p_udp_msg_send(server_params* sp, p2p_msg msg) {	
	p2p_addr dst = p2p_msg_get_dst(msg);
	int sockfd;
	if ((sockfd = p2p_udp_socket_create(sp, dst)) == -1) return -1;	
	p2p_udp_msg_sendfd(sp, msg, sockfd, dst);
	p2p_udp_socket_close(sp,sockfd);
	return 0;
}


/***************      UDP Rebroadcasr     **************/
// @ dans msg doit etre broadcast
int 
p2p_udp_msg_rebroadcast(server_params* sp, p2p_msg msg) {
	if (p2p_udp_msg_send(sp,msg) == 0) return 0;
	else return -1;
}

/******************************************************/
/*************          Partie UI         *************/
/******************************************************/

int 
p2p_ui_socket_create(server_params* sp) {
	int sockfd;
	
	VERBOSE(sp,VSYSCL, " Create socket UI  listening on port %d :\n", sp->port_ui);
	if ((sockfd = p2p_socket_create("tcp",sp->port_ui)) == -1) {
		VERBOSE(sp,VSYSCL, "  [Fail]\n ERROR: socket create\n");
		return -1;
	} 
	
	if (listen(sockfd, BACKLOG) == -1) {
		VERBOSE(sp,VSYSCL, "  [Fail]\n ERROR: socket listen\n");
		return -1;
	}

	if (p2p_socket_non_blocking_enable(sockfd) == -1) {
		return -1;
	}
	
	VERBOSE(sp,VSYSCL, "  [OK]\n");
	return sockfd;
}

int
p2p_ui_socket_close(server_params* sp, int fd) {
	VERBOSE(sp,VSYSCL, " closing socket ui:\n");
	if (sp->client_ui != -1) {
		close(sp->client_ui);
		sp->client_ui = -1;
		VERBOSE(sp,VSYSCL, "  [OK]\n");
		return 0;
	}
	else {
		VERBOSE(sp,VSYSCL," No ui socket available\n");
		return -1;
	}
}
