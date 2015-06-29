/* NGUYEN LE Duc Tan
   MIF39 Lyon 1 Informatique
 ***/


#include <stdio.h>
#include <errno.h> // pour la fonction perror affichant le message erreur 
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h> // pour la fonction memcpy
#include <unistd.h> // pour la fonction close
#include <sys/timeb.h>  // pour le temps
#include <pthread.h>	// pour mutex
#include <sys/syscall.h> // pour la fonction syscall

#include "p2p_common.h"
#include "p2p_options.h"
#include "p2p_msg.h"
#include "p2p_addr.h"
#include "p2p_file.h"
#include "p2p_ui.h"

#define P2P_FILE_NAME_SIZE 200 // longueur maximal du nom du fichier


struct sockaddr_in sock_client_tcp, sock_client_ui;
unsigned int lg = sizeof (sock_client_tcp);


/*************************************************************************/
/*                           Initialisation 			 							 				 */

/*************************************************************************/

void init(int verbosity, int port_ui, int port_p2p_tcp, int port_p2p_udp, const char* ip, char* server_name, char* dir_name) {
    if ((sp = (server_params*) malloc(sizeof (server_params))) == NULL)
        perror("Imposible d'allocation memoire\n");

    sp->verbosity = verbosity;
    sp->port_ui = port_ui;
    sp->port_p2p_tcp = port_p2p_tcp;
    sp->port_p2p_udp = port_p2p_udp;

    sp->p2pMyId = p2p_addr_create();
    p2p_addr_set(sp->p2pMyId, ip, port_p2p_tcp, port_p2p_udp);

    sp->p2pLeft = p2p_addr_create();
    sp->p2pRight = p2p_addr_create();
    p2p_addr_set(sp->p2pLeft, ip, port_p2p_tcp, port_p2p_udp);
    p2p_addr_set(sp->p2pRight, ip, port_p2p_tcp, port_p2p_udp);


    sp->server_name = (char*) malloc(P2P_MAX_PATH);
    sp->dir_name = (char*) malloc(P2P_MAX_PATH);
    strncpy(sp->server_name, server_name, P2P_MAX_PATH);
    strncpy(sp->dir_name, dir_name, P2P_MAX_PATH);

    sp->p2pSearchList.nbsearch = 0;

    int i = 0;
    for (i = 0; i < 16; i++) {
        sp->p2pTopology.Left[i] = NULL;
        sp->p2pTopology.Right[i] = NULL;
    }

    sp->checkNetwork = -1;
    sp->repairNetwork = -1;
    sp->nextSock = 0;
    sp->lastDone = 0;
    sp->bDownload = (struct timeb*) malloc(sizeof (struct timeb));
    sp->eDownload = (struct timeb*) malloc(sizeof (struct timeb));

    sp->bDownload->time = (time_t) malloc(sizeof (time_t));
    sp->eDownload->time = (time_t) malloc(sizeof (time_t));

    // initialisation le mutex et variable conditionnelle
    errno = pthread_mutex_init(&(sp->log_mutex), NULL);
    if (errno) perror(" ERROR pthread_mutex_init\n");

    errno = pthread_mutex_init(&sock_mutex, NULL);
    if (errno) perror(" ERROR pthread_mutex_init\n");

    errno = pthread_mutex_init(&get_mutex, NULL);
    if (errno) perror(" ERROR pthread_mutex_init\n");

    errno = pthread_cond_init(&get_cond, NULL);
    if (errno) perror(" ERROR pthread_cond_init\n");

    errno = pthread_cond_init(&var_cond, NULL);
    if (errno) perror(" ERROR pthread_cond_init\n");
}

/*************************************************************************/
/*                           CREATION SOCKET	                           */

/*************************************************************************/

void socket_create() {
    VERBOSE(sp, VSYSCL, " Creating Socket ..\n");
    sp->sock_ui = p2p_ui_socket_create(sp);
    sp->sock_tcp = p2p_tcp_listen_create(sp);
    sp->sock_udp = p2p_udp_listen_create(sp);
}


/*************************************************************************/
/*                           	TCP 			            	                   */

/*************************************************************************/

void *msg_tcp_handle(void *a) {

    while (1) {
        // le thread prend le mutex pour verifier le variable conditionnelle
        pthread_mutex_lock(&sock_mutex);
        // si qu'il n'y a pas de socket d'attente dans la queue, relache le mutex
        // et mettre en attente 
        while (sp->nextSock == sp->lastDone)
            pthread_cond_wait(&var_cond, &sock_mutex);

        // quand il y a le socket a servir => recoit signal depuis main()
        // bloque le mutex pour prend la valeur du socket e servir

        p2p_msg msg_tcp = p2p_msg_create();
        int sockfd = sp->p2pSocket[sp->lastDone];
        sp->lastDone++;
        if (sp->lastDone == P2P_MAX_SOCKET) sp->lastDone = 0;
        // si fini, relacher le mutex
        pthread_mutex_unlock(&sock_mutex);

        p2p_tcp_msg_recvfd(sp, msg_tcp, sockfd);
        unsigned char type_msg = p2p_msg_get_type((p2p_msg) msg_tcp);

        switch (type_msg) {
                p2p_addr node_addr;

            case P2P_MSG_JOIN_REQ:
            { // Verifie que: 
                // 		pay_load length = 0
                // 		le node courant est bien le destinataire
                if ((p2p_msg_get_length((p2p_msg) msg_tcp) == 0)
                        && (p2p_addr_is_equal(sp->p2pMyId, p2p_msg_get_dst((p2p_msg) msg_tcp)))) {

                    // @node demande JOIN
                    node_addr = p2p_addr_duplicate(p2p_msg_get_src((p2p_msg) msg_tcp));
                    p2p_msg new_msg = p2p_msg_create();
                    VERBOSE(sp, VSYSCL, " RECV msg join request from %s\n", p2p_addr_get_str(node_addr));

                    VERBOSE(sp, VSYSCL, " msg join ack create\n");
                    p2p_msg_init((p2p_msg) new_msg, P2P_MSG_JOIN_ACK, P2P_MSG_TTL_ONE_HOP, sp->p2pMyId, node_addr);
                    unsigned char *payload = (unsigned char*) malloc(2 * P2P_ADDR_SIZE);
                    memcpy(payload, sp->p2pMyId, P2P_ADDR_SIZE);
                    memcpy(payload + P2P_ADDR_SIZE, sp->p2pRight, P2P_ADDR_SIZE);
                    p2p_msg_init_payload((p2p_msg) new_msg, 2 * P2P_ADDR_SIZE, payload);

                    VERBOSE(sp, VSYSCL, " SEND msg join ack to %s\n", p2p_addr_get_str(node_addr));

                    // envoie succes => mise a jour de la table de voisinage
                    // (dans LINK_UPDATE)
                    p2p_tcp_msg_sendfd(sp, (p2p_msg) new_msg, sockfd);
                    p2p_tcp_socket_close(sp, sockfd);
                    p2p_msg_delete(new_msg);
                    free(payload);
                    VERBOSE(sp, VSYSCL, " ay ay ay a ay ay\n");
                } else VERBOSE(sp, VSYSCL, " ERROR: wrong dst addr or wrong data length\n");
                break;
            }

            case P2P_MSG_LINK_UPDATE:
            {
                if ((p2p_msg_get_length((p2p_msg) msg_tcp) == P2P_ADDR_SIZE + 4)
                        && (p2p_addr_is_equal(sp->p2pMyId, p2p_msg_get_dst((p2p_msg) msg_tcp)))) {
                    VERBOSE(sp, VSYSCL, " RECV msg link update from %s\n", p2p_addr_get_str(p2p_msg_get_src((p2p_msg) msg_tcp)));
                    p2p_addr voisin_addr = p2p_addr_create();
                    unsigned int type_neighbor;
                    memcpy(voisin_addr, p2p_msg_get_payload((p2p_msg) msg_tcp), P2P_ADDR_SIZE);
                    memcpy(&type_neighbor, p2p_msg_get_payload((p2p_msg) msg_tcp) + P2P_ADDR_SIZE, 4);
                    type_neighbor = htonl(type_neighbor);

                    // mutex: modifier sp->p2pLeft, sp->p2pRight, sp->p2pTopology 
                    p2p_mutex_lock(sp);
                    switch (type_neighbor) {
                        case P2P_NEIGNBOR_LEFT:
                        {
                            VERBOSE(sp, VSYSCL, " new LEFT neighbor %s\n", p2p_addr_get_str(voisin_addr));
                            p2p_addr_copy(sp->p2pLeft, voisin_addr);
                            p2p_node_delete(sp->p2pTopology.Left[0]);
                            sp->p2pTopology.Left[0] = p2p_node_init(sp->p2pLeft, NULL);
                            break;
                        }

                        case P2P_NEIGNBOR_RIGHT:
                        {
                            VERBOSE(sp, VSYSCL, " new RIGHT neighbor %s\n", p2p_addr_get_str(voisin_addr));
                            p2p_addr_copy(sp->p2pRight, voisin_addr);
                            p2p_node_delete(sp->p2pTopology.Right[0]);
                            sp->p2pTopology.Right[0] = p2p_node_init(sp->p2pRight, NULL);
                            break;
                        }

                        default:
                            VERBOSE(sp, VSYSCL, " ERROR: msg undefined\n");
                    }
                    p2p_mutex_unlock(sp);
                    VERBOSE(sp, VSYSCL, " UPDATE the neighbors\n");
                    p2p_addr_delete(voisin_addr);
                } else VERBOSE(sp, VSYSCL, " ERROR: wrong dst addr or wrong data length\n");

                p2p_tcp_socket_close(sp, sockfd);
                break;
            }


            case P2P_MSG_GET: // verifie le node courant est bien le destinataire
                if (p2p_addr_is_equal(sp->p2pMyId, p2p_msg_get_dst((p2p_msg) msg_tcp))) {

                    node_addr = p2p_msg_get_src((p2p_msg) msg_tcp);
                    VERBOSE(sp, VSYSCL, " RECV msg GET from %s\n", p2p_addr_get_str(node_addr));
                    unsigned char code; // code = P2P_OK ou P2P_ERROR
                    // recupere le payload
                    VERBOSE(sp, VSYSCL, "data length = %d \n", p2p_msg_get_length(msg_tcp));
                    unsigned short length = p2p_msg_get_length((p2p_msg) msg_tcp) - 8;
                    char* filename = (char *) malloc(length + 1);
                    int filesize;

                    unsigned char *getpayload = p2p_msg_get_payload((p2p_msg) msg_tcp);
                    int boffset, eoffset; // begin, end offset
                    memcpy(&boffset, getpayload, 4);
                    memcpy(&eoffset, getpayload + 4, 4);
                    boffset = ntohl(boffset);
                    eoffset = ntohl(eoffset);
                    memcpy(filename, getpayload + 8, length);
                    filename[length] = '\0';
                    VERBOSE(sp, VSYSCL, " Searching file: %s   from %d to %d\n", filename, boffset, eoffset);

                    p2p_msg new_msg = p2p_msg_create();
                    p2p_msg_init((p2p_msg) new_msg, P2P_MSG_DATA, P2P_MSG_TTL_ONE_HOP, sp->p2pMyId, node_addr);
                    // verifie si le node detient bien le fichier demande
                    if (p2p_file_is_available(sp, filename, &filesize) == P2P_OK) {
                        VERBOSE(sp, VSYSCL, " File found with size: %d\n", filesize);
                        unsigned char *data;
                        if (p2p_file_get_chunck(sp, filename, boffset, eoffset, &data) == P2P_OK) {
                            int size = eoffset - boffset + 1;
                            VERBOSE(sp, VSYSCL, " SEND file to %s\n", p2p_addr_get_str(node_addr));
                            unsigned char* payload = (unsigned char *) malloc(size + 8);
                            code = P2P_DATA_OK;
                            memcpy(payload, &code, 1);
                            VERBOSE(sp, VSYSCL, " size = %d \n", size);
                            int hsize = htonl(size);
                            memcpy(payload + 4, &hsize, 4);
                            memcpy(payload + 8, data, size);
                            p2p_msg_init_payload((p2p_msg) new_msg, size + 8, payload);

                            p2p_tcp_msg_sendfd(sp, (p2p_msg) new_msg, sockfd);
                            p2p_tcp_socket_close(sp, sockfd);
                            free(data);
                            free(payload);
                        } else {
                            VERBOSE(sp, VSYSCL, " ERROR: p2p_file_get_chunck\n");
                            VERBOSE(sp, VSYSCL, " SEND data with error code 500\n");
                            unsigned char* payload = (unsigned char *) malloc(8);
                            code = P2P_DATA_ERROR;
                            memcpy(payload, &code, 1);
                            int code_err = P2P_INTERNAL_SERVER_ERROR;
                            memcpy(payload + 4, &code_err, 4);
                            p2p_msg_init_payload((p2p_msg) new_msg, 8, payload);
                            p2p_tcp_msg_sendfd(sp, (p2p_msg) new_msg, sockfd);
                            p2p_tcp_socket_close(sp, sockfd);
                            free(payload);
                        }
                    } else {
                        VERBOSE(sp, VSYSCL, " FILE NOT FOUND! SEND data with error code\n");
                        unsigned char* payload = (unsigned char *) malloc(8);
                        code = P2P_ERROR;
                        int code_err = P2P_BAD_REQUEST;
                        memcpy(payload, &code, 1);
                        memcpy(payload + 4, &code_err, 4);
                        p2p_msg_init_payload((p2p_msg) new_msg, 8, payload);
                        p2p_tcp_msg_sendfd(sp, (p2p_msg) new_msg, sockfd);
                        p2p_tcp_socket_close(sp, sockfd);
                        free(payload);
                    }
                    free(filename);
                    p2p_msg_delete(new_msg);
                } else VERBOSE(sp, VSYSCL, " ERROR: wrong dst addr\n");
                break;

            default:
                break;
        }
        p2p_msg_delete(msg_tcp);
    } // fin de while
}


/*************************************************************************/
/*                             	    UDP 		         	                   */

/*************************************************************************/

void *msg_udp_handle(void *msg_udp) {
    msg_udp = (p2p_msg) msg_udp;
    unsigned char type_msg = p2p_msg_get_type(msg_udp);
    p2p_msg new_msg;
    switch (type_msg) {

        case P2P_MSG_SEARCH:
        {
            if (p2p_addr_is_broadcast(p2p_msg_get_dst((p2p_msg) msg_udp))) {
                VERBOSE(sp, VSYSCL, " RECV msg search from %s\n", p2p_addr_get_str(p2p_msg_get_src(msg_udp)));

                p2p_msg new_msg = p2p_msg_duplicate(msg_udp);
                int length = p2p_msg_get_length(msg_udp);
                unsigned char *payload = (unsigned char*) malloc(length);
                memcpy(payload, p2p_msg_get_payload(msg_udp), length);
                p2p_addr noeud_init = p2p_addr_create();
                unsigned int reg_id;
                p2p_addr dst;
                p2p_addr addr_broadcast = p2p_addr_create();
                p2p_addr_set_broadcast(addr_broadcast);

                memcpy((char*) noeud_init, payload, P2P_ADDR_SIZE);
                memcpy((char*) &reg_id, payload + P2P_ADDR_SIZE, 4);
                reg_id = ntohl(reg_id);
                unsigned int filename_size = length - 4 - P2P_ADDR_SIZE;
                char *filename = (char*) malloc(filename_size + 1);
                memcpy(filename, payload + P2P_ADDR_SIZE + 4, filename_size);
                filename[filename_size] = '\0';

                VERBOSE(sp, VSYSCL, " original source : %s\n", p2p_addr_get_str(noeud_init));
                VERBOSE(sp, VSYSCL, " request id      : %d\n", reg_id);
                VERBOSE(sp, VSYSCL, " request string  : %s\n", filename);
                VERBOSE(sp, VSYSCL, " length          : %d\n", p2p_msg_get_length(new_msg));
                if (p2p_addr_is_equal(noeud_init, sp->p2pMyId)) {
                    //jeter le msg , faire rien
                } else { //retransmission	
                    int sockfd;
                    if (p2p_msg_get_ttl(msg_udp) > 1) {
                        p2p_msg_set_src(new_msg, sp->p2pMyId);
                        p2p_msg_set_dst(new_msg, addr_broadcast);
                        p2p_msg_set_ttl(new_msg, (p2p_msg_get_ttl(msg_udp)) - 1);

                        if ((p2p_addr_is_equal(p2p_msg_get_src(msg_udp), sp->p2pLeft) != 1) && (p2p_addr_is_equal(noeud_init, sp->p2pLeft) != 1)) {
                            VERBOSE(sp, VSYSCL, "********** RETRANSMISSION **********\n");
                            dst = sp->p2pLeft;
                            VERBOSE(sp, VSYSCL, " SEND msg search to LEFT neighbor: %s\n", p2p_addr_get_str(dst));
                            // envoyer msg search vers le voisin gauche
                            sockfd = p2p_udp_socket_create(sp, dst);
                            p2p_udp_msg_sendfd(sp, new_msg, sockfd, dst);
                            p2p_udp_socket_close(sp, sockfd);
                        } else
                            if ((p2p_addr_is_equal(p2p_msg_get_src(msg_udp), sp->p2pRight) != 1) && (p2p_addr_is_equal(noeud_init, sp->p2pRight) != 1)) {
                            VERBOSE(sp, VSYSCL, "********** RETRANSMISSION **********\n");
                            dst = sp->p2pRight;
                            VERBOSE(sp, VSYSCL, " SEND msg search to RIGHT neighbor: %s\n", p2p_addr_get_str(dst));
                            // envoyer vers le voisin droite
                            sockfd = p2p_udp_socket_create(sp, dst);
                            p2p_udp_msg_sendfd(sp, new_msg, sockfd, dst);
                            p2p_udp_socket_close(sp, sockfd);
                        }
                    }
                }

                //verifier si on a le fichier qu'il cherche et envoyer le msg REPLY		
                unsigned int filesize;
                if (p2p_file_is_available(sp, filename, (int*) &filesize) == P2P_OK) {
                    VERBOSE(sp, VSYSCL, "  [OK]\n");
                    VERBOSE(sp, VSYSCL, " filename  : %s\n", filename);
                    VERBOSE(sp, VSYSCL, " size      : %d\n", filesize);

                    VERBOSE(sp, VSYSCL, " SEND msg reply to initiator %s\n", p2p_addr_get_str(noeud_init));
                    p2p_msg_init(new_msg, P2P_MSG_REPLY, P2P_MSG_TTL_ONE_HOP, sp->p2pMyId, noeud_init);

                    unsigned char* payload_reply = (unsigned char*) malloc(4 + 4);
                    reg_id = htonl(reg_id);
                    filesize = htonl(filesize);

                    memcpy(payload_reply, (char*) &reg_id, 4);
                    memcpy(payload_reply + 4, (char*) &filesize, 4);

                    p2p_msg_init_payload(new_msg, 8, payload_reply);
                    p2p_udp_msg_send(sp, new_msg); // envoyer new_msg
                    free(payload_reply);
                } else VERBOSE(sp, VSYSCL, " File not Found\n");

                p2p_addr_delete(noeud_init);
                free(filename);
                p2p_msg_delete(new_msg);
            } else VERBOSE(sp, VSYSCL, " ERROR: wrong dst addr\n");
            break;
        }

        case P2P_MSG_REPLY:
        {
            if (p2p_addr_is_equal(sp->p2pMyId, p2p_msg_get_dst(msg_udp))) {
                char *src_str = p2p_addr_get_str((p2p_addr) p2p_msg_get_src(msg_udp));

                VERBOSE(sp, VSYSCL, " RECV msg reply from %s\n", src_str);

                int id, filesize;
                char *payload_reply = (char*) malloc(8);

                memcpy((char*) payload_reply, (char*) p2p_msg_get_payload(msg_udp), 8);
                memcpy((char*) &id, payload_reply, 4);
                memcpy((char*) &filesize, payload_reply + 4, 4);

                id = ntohl(id);
                filesize = ntohl(filesize);
                VERBOSE(sp, VSYSCL, " id            : %d\n", id);
                VERBOSE(sp, VSYSCL, " filesize      : %d\n", filesize);
                VERBOSE(sp, VSYSCL, " Reply done\n");

                if (id >= sp->p2pSearchList.nbsearch) {
                    VERBOSE(sp, VSYSCL, " ERROR: wrong id search\n");
                    break;
                }
                // mutex: modifier sp->p2pSearchList..
                int i, trouve = 0;
                p2p_mutex_lock(sp);
                for (i = 0; i < sp->p2pSearchList.search[id].nb_result; i++) {
                    if ((strcmp(sp->p2pSearchList.search[id].result[i], src_str) == 0)&&(sp->p2pSearchList.search[id].file_size[i] == filesize)) {
                        trouve = 1;
                        break;
                    }
                }

                if (trouve == 0) {
                    sp->p2pSearchList.search[id].nb_result++;
                    sp->p2pSearchList.search[id].result[sp->p2pSearchList.search[id].nb_result - 1] = malloc(strlen((char*) src_str));
                    strcpy((char*) sp->p2pSearchList.search[id].result[sp->p2pSearchList.search[id].nb_result - 1], (char*) src_str);
                    sp->p2pSearchList.search[id].file_size[sp->p2pSearchList.search[id].nb_result - 1] = filesize;
                }

                p2p_mutex_unlock(sp);
            } else VERBOSE(sp, VSYSCL, " ERROR: wrong dst addr\n");
            break;
        }

        case P2P_MSG_NEIGHBORS_REQ:
        {
            // verifie si dst est bien un addr broadcast 
            // et data length = P2P_ADDR_SIZE
            if (p2p_addr_is_broadcast(p2p_msg_get_dst((p2p_msg) msg_udp)) && (p2p_msg_get_length(msg_udp) == P2P_ADDR_SIZE)) {
                new_msg = p2p_msg_duplicate(msg_udp);
                p2p_addr node_addr;
                int sockfd;
                p2p_addr addr_broadcast = p2p_addr_create();
                p2p_addr_set_broadcast(addr_broadcast);
                p2p_addr noeud_init = p2p_addr_create();
                memcpy((char*) noeud_init, (char*) p2p_msg_get_payload(new_msg), P2P_ADDR_SIZE);

                VERBOSE(sp, VSYSCL, " RECV msg neighbor request from %s\n", p2p_addr_get_str(p2p_msg_get_src(new_msg)));

                // envoie msg neighbr request e voisin A si 
                // A # noeud init et
                // A # src de new_msg
                if ((p2p_addr_is_equal(sp->p2pRight, p2p_msg_get_src(new_msg)) != 1)
                        && (p2p_addr_is_equal(sp->p2pRight, (p2p_addr) noeud_init) != 1)) {
                    node_addr = p2p_addr_duplicate(sp->p2pRight);
                    VERBOSE(sp, VSYSCL, " rebroadcast msg neighbor request to RIGHT neighbor %s\n", p2p_addr_get_str(sp->p2pRight));

                    // rebroadcast le msg neighbor request, ttl diminue par 1
                    p2p_msg_init(new_msg, P2P_MSG_NEIGHBORS_REQ, p2p_msg_get_ttl(msg_udp) - 1, sp->p2pMyId, addr_broadcast);
                    p2p_msg_set_length(new_msg, P2P_ADDR_SIZE);
                    // il ne faut pas utiliser send() mais sendfd() car dst = @broadcast
                    sockfd = p2p_udp_socket_create(sp, node_addr);
                    p2p_udp_msg_sendfd(sp, new_msg, sockfd, node_addr);
                    p2p_udp_socket_close(sp, sockfd);
                    p2p_addr_delete(node_addr);
                    p2p_msg_delete(new_msg);
                } else
                    if ((p2p_addr_is_equal(sp->p2pLeft, p2p_msg_get_src(new_msg)) != 1)
                        && (p2p_addr_is_equal(sp->p2pLeft, (p2p_addr) noeud_init) != 1)) {
                    node_addr = p2p_addr_duplicate(sp->p2pLeft);
                    VERBOSE(sp, VSYSCL, " rebroadcast msg neighbor request to LEFT neighbor %s\n", p2p_addr_get_str(sp->p2pLeft));

                    // rebroadcast le msg neighbor request, ttl diminue par 1
                    p2p_msg_init(new_msg, P2P_MSG_NEIGHBORS_REQ, p2p_msg_get_ttl(msg_udp) - 1, sp->p2pMyId, addr_broadcast);
                    p2p_msg_set_length(new_msg, P2P_ADDR_SIZE); //set length si non pb
                    // il ne faut pas utiliser send() mais sendfd() car dst = @broadcast
                    sockfd = p2p_udp_socket_create(sp, node_addr);
                    p2p_udp_msg_sendfd(sp, new_msg, sockfd, node_addr);
                    p2p_udp_socket_close(sp, sockfd);
                    p2p_addr_delete(node_addr);
                    p2p_msg_delete(new_msg);
                }

                VERBOSE(sp, VSYSCL, " SEND msg neighbor list to initiator %s\n", p2p_addr_get_str(noeud_init));
                new_msg = p2p_msg_create();
                p2p_msg_init(new_msg, P2P_MSG_NEIGHBORS_LIST, P2P_MSG_TTL_ONE_HOP, sp->p2pMyId, noeud_init);

                unsigned char Nb_neighbors;
                if (p2p_addr_is_equal(sp->p2pLeft, sp->p2pRight))
                    Nb_neighbors = 1;
                else Nb_neighbors = 2;
                int l = 4 + (Nb_neighbors * P2P_ADDR_SIZE) + strlen(sp->server_name);
                unsigned char* payload_neighbors = (unsigned char*) malloc(l);

                p2p_msg_set_length(new_msg, l);
                memcpy(payload_neighbors, &Nb_neighbors, sizeof (Nb_neighbors));
                memcpy(payload_neighbors + 4, sp->p2pLeft, P2P_ADDR_SIZE);

                if (Nb_neighbors == 2)
                    memcpy(payload_neighbors + 4 + P2P_ADDR_SIZE, sp->p2pRight, P2P_ADDR_SIZE);
                memcpy(payload_neighbors + 4 + Nb_neighbors * P2P_ADDR_SIZE, sp->server_name, strlen(sp->server_name));

                p2p_msg_init_payload(new_msg, l, payload_neighbors);
                p2p_udp_msg_send(sp, new_msg);
                p2p_msg_delete(new_msg);
            } else
                VERBOSE(sp, VSYSCL, " ERROR: wrong dst addr or wrong data length\n");
            break;
        }


        case P2P_MSG_NEIGHBORS_LIST:
        {
            // mise e jour la liste	de topologie du reseau
            // si on a une seul voisin, on update pas
            if (p2p_addr_is_equal(sp->p2pMyId, p2p_msg_get_dst(msg_udp))) {
                p2p_addr node_addr = p2p_msg_get_src(msg_udp);
                VERBOSE(sp, VSYSCL, " RECV msg neighbor list from %s\n", p2p_addr_get_str(node_addr));

                if (p2p_addr_is_equal(sp->p2pLeft, sp->p2pRight) != 1) {
                    p2p_msg new_msg = p2p_msg_duplicate(msg_udp);
                    unsigned char *payload = p2p_msg_get_payload(new_msg);
                    // les 2 voisins de la source: e gauche: left_node, 
                    // e droite: right_node
                    p2p_addr left_node = p2p_addr_create();
                    p2p_addr right_node = p2p_addr_create();
                    memcpy((char*) left_node, payload + 4, P2P_ADDR_SIZE);
                    memcpy((char*) right_node, payload + 4 + P2P_ADDR_SIZE, P2P_ADDR_SIZE);
                    int l = p2p_msg_get_length(new_msg) - 4 - 2 * P2P_ADDR_SIZE;
                    char *node_name = (char *) malloc(l + 1);
                    memcpy(node_name, payload + 4 + 2 * P2P_ADDR_SIZE, l);
                    node_name[l] = '\0';
                    int i = 0;
                    int continuer = 0;

                    // mutex: modifier sp->p2pTopology
                    p2p_mutex_lock(sp);

                    while ((sp->p2pTopology.Left[i] != NULL) && (i < 16) && (continuer == 0)) {
                        if (p2p_addr_is_equal(sp->p2pTopology.Left[i]->addr, node_addr)) {
                            continuer = -1;
                            p2p_node_delete(sp->p2pTopology.Left[i]);
                            sp->p2pTopology.Left[i] = p2p_node_init(node_addr, node_name);
                            if ((p2p_node_search(sp->p2pTopology.Right, left_node) == -1) && (p2p_addr_is_equal(node_addr, left_node) != 1))
                                sp->p2pTopology.Left[i + 1] = p2p_node_init(left_node, NULL);
                        }
                        i++;
                    }

                    i = 0;
                    while ((sp->p2pTopology.Right[i] != NULL) && (i < 16) && (continuer == 0)) {
                        if (p2p_addr_is_equal(sp->p2pTopology.Right[i]->addr, node_addr)) {
                            continuer = -1;
                            p2p_node_delete(sp->p2pTopology.Right[i]);
                            sp->p2pTopology.Right[i] = p2p_node_init(node_addr, node_name);
                            if ((p2p_node_search(sp->p2pTopology.Left, right_node) == -1) && (p2p_addr_is_equal(node_addr, right_node) != 1))
                                sp->p2pTopology.Right[i + 1] = p2p_node_init(right_node, NULL);
                        }
                        i++;
                    }

                    p2p_mutex_unlock(sp);

                    p2p_msg_delete(new_msg);
                    p2p_addr_delete(left_node);
                    p2p_addr_delete(right_node);
                    free(node_name);
                } else { // on a un seul voisin, on update le nom du voisin
                    unsigned char *payload = p2p_msg_get_payload(msg_udp);

                    int l = p2p_msg_get_length(msg_udp) - 4 - P2P_ADDR_SIZE;
                    char *node_name = (char *) malloc(l + 1);
                    memcpy(node_name, payload + 4 + P2P_ADDR_SIZE, l);
                    node_name[l] = '\0';

                    p2p_mutex_lock(sp);
                    p2p_node_delete(sp->p2pTopology.Left[0]);
                    p2p_node_delete(sp->p2pTopology.Right[0]);
                    sp->p2pTopology.Left[0] = p2p_node_init(node_addr, node_name);
                    sp->p2pTopology.Right[0] = p2p_node_init(node_addr, node_name);
                    p2p_mutex_lock(sp);

                    free(node_name);
                }
            } else
                VERBOSE(sp, VSYSCL, " ERROR: wrong dst addr\n");
            break;
        }
    }

    p2p_msg_delete(msg_udp);
    VERBOSE(sp, VSYSCL, " thread UDP %ld terminates\n", (long int) syscall(224));
    p2p_mutex_unlock(sp);
    return NULL;
}

void *ui_handle(void *a) {

    while (sp->client_ui != -1) {
        VERBOSE(sp, CLIENT, "\np2p:: ");
        ui_command(sp);
    }
    return NULL;
}


/*************************************************************************/
/*                           MAIN				 			                           */

/*************************************************************************/

void
main(int argc, char* argv[]) {
    int p_ui, p_tcp, p_udp;
    char *addr;
    char *server_name = (char*) malloc(P2P_MAX_PATH);
    char *dir_name = (char*) malloc(P2P_MAX_PATH);
    p2p_msg newmsg_udp = p2p_msg_create();
    int newsock;
    pthread_t ui_thread, udp_thread;

    struct timeb *tp1 = (struct timeb*) malloc(sizeof (struct timeb));
    struct timeb *tp2 = (struct timeb*) malloc(sizeof (struct timeb));

    tp1->time = (time_t) malloc(sizeof (time_t));
    tp2->time = (time_t) malloc(sizeof (time_t));

    ftime(tp1);

    //Si on veut connecter au noeud hors 
    //reseau local, changer l'adresse IP suivant
    addr = "127.0.0.1";
    int i = 1;
    while (i < argc) {
        if (strcmp(argv[i], "-ui") == 0) p_ui = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-tcp") == 0) p_tcp = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-udp") == 0) p_udp = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-name") == 0) strcpy(server_name, argv[i + 1]);
        else if (strcmp(argv[i], "-dir") == 0) strcpy(dir_name, argv[i + 1]);
        i = i + 2;
    }

    printf("#   NGUYEN LE Duc Tan    #\n");
    printf("#   MIF39 Lyon 1 Informatique  #\n\n");
    printf("PEER-2-PEER Network starting..............\n\n");


    //INITIALISATION
    init(6, p_ui, p_tcp, p_udp, addr, server_name, dir_name); //Niveau VERBOSITY=6

    printf("INFORMATIONS sur le noeud: \n");
    printf("        pid                = %d\n", getpid());
    printf("        dir_name           = %s\n", dir_name);
    printf("        server_name        = %s\n", server_name);
    printf("        listening ip       = %s\n", p2p_addr_get_ip_str(sp->p2pMyId));
    printf("        ui tcp             = %d\n", sp->port_ui);
    printf("        p2p tcp            = %d\n", sp->port_p2p_tcp);
    printf("        p2p udp            = %d\n", sp->port_p2p_udp);
    printf("\n");

    //CREATION des SOCKET
    socket_create();

    printf("\n");

    for (i = 0; i < P2P_MAX_THREAD; i++) {
        pthread_create(&(sp->p2pThread[i]), NULL, msg_tcp_handle, NULL);
        pthread_detach(sp->p2pThread[i]);
    }

    for (i = 0; i < P2P_MAX_THREAD; i++) {
        pthread_create(&(sp->p2pGetThread[i]), NULL, p2pthreadget, (void*) i);
        pthread_detach(sp->p2pGetThread[i]);
    }



    /************************************************************************/
    /*                                                                      */
    /*                        LA BOUCLE PRINCIPALE                          */
    /*                                                                      */
    /************************************************************************/


    while (1) {

        // si on n'a au moins un voisin
        // -> commence e emettre LINK_UPDATE chaque 30s
        ftime(tp2);
        if ((p2p_addr_is_equal(sp->p2pMyId, sp->p2pLeft) != 1)
                && ((tp2->time - tp1->time) > 30)
                && (sp->p2pTopology.Left[0] != NULL)
                && (sp->p2pTopology.Right[0] != NULL)
                && (sp->checkNetwork == 0)) {
            networkUpdate(sp);
            tp1->time = tp1->time + 30;
        }



        /* Socket TCP */
        if ((newsock = accept(sp->sock_tcp, (struct sockaddr *) &sock_client_tcp, &lg)) > 0) {
            VERBOSE(sp, VSYSCL, " incoming traffic on port tcp %d\n", sp->port_p2p_tcp);
            // reeoit une nouvelle connexion => bloquer le mutex, mettre le socket
            // dans la queue, ensuite reveiller un thread pour traiter la connexion
            pthread_mutex_lock(&sock_mutex);
            sp->p2pSocket[sp->nextSock] = newsock;
            sp->nextSock++;
            if (sp->nextSock == P2P_MAX_SOCKET) sp->nextSock = 0;
            pthread_cond_signal(&var_cond); // reveiller un thread
            pthread_mutex_unlock(&sock_mutex);
        }



        /* Socket UI */
        if ((newsock = accept(sp->sock_ui, (struct sockaddr *) &sock_client_ui, &lg)) > 0) {
            VERBOSE(sp, VSYSCL, " incoming traffic on port ui %d\n", sp->port_ui);
            // creer le thread qui va traiter la connexion ui utilisant la fonction ui_command(sp)			
            sp->client_ui = newsock;
            errno = pthread_create(&ui_thread, NULL, ui_handle, NULL);
            if (errno) perror(" ERROR: pthread ui create\n");

            errno = pthread_detach(ui_thread);
            if (errno) perror(" ERROR: pthread ui detach\n");
        }


        /* Socket UDP */
        if (p2p_udp_msg_recvfd(sp, newmsg_udp, sp->sock_udp) == 0) {
            p2p_msg msg_dupli = p2p_msg_duplicate(newmsg_udp);
            errno = pthread_create(&udp_thread, NULL, msg_udp_handle, (void*) msg_dupli);
            if (errno) perror(" ERROR: pthread udp create\n");

            errno = pthread_detach(udp_thread);
            if (errno) perror(" ERROR: pthread udp detach\n");
        }

    }
}
