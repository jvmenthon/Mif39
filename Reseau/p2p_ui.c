/* NGUYEN LE Duc Tan
   MIF39 Lyon 1 Informatique
 ***/


#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/timeb.h>

#include "p2p_common.h"
#include "p2p_addr.h"
#include "p2p_msg.h"
#include "p2p_ui.h"
#include "p2p_file.h"
#include "p2p_options.h"

#define MAX_PATH 1024
#define MAX_REQ 1000
#define MAX_OPT 4
#define P2P_NEIGNBOR_LEFT 0xFFFF0000
#define P2P_NEIGNBOR_RIGHT 0x0000FFFF

/****************************************************/

/****************************************************/

struct params_t {
    server_params *sp;
    char *options[MAX_OPT];
};
typedef struct params_t params;

int help(params*);
int quit(params*);
int status(params*);
int file_list(params*);
int p2pjoin(params*);
int p2pleave(params*);
int p2phalt(params*);

int p2psearch(params*);
int p2plist_search(params*);
int p2plist_result(params*);
int p2pget(params*);

int p2pdiscover(params*);
int p2ptopology(params*);
int p2pnetworkstatus(params*);
int p2pnetworkrepair(params*);
int p2pmultiget(params*);

/****************************************************/

/****************************************************/

struct cmd_t {
    char* name;
    int options;
    char* text;
    int (*fun)(params*);
};

static struct cmd_t commands[] = {
    { "help", 0, "print this message", help},
    { "state", 0, "print node state", status},
    { "list", 0, "list available files", file_list},
    { "", 0, "", NULL},
    { "join", 1, "connect to node [p2p_Id]", p2pjoin},
    { "leave", 0, "leave the p2p network", p2pleave},
    { "quit", 0, "detach ui from the node", quit},
    { "halt", 0, "leave the p2p and stop the server", p2phalt},
    { "", 0, "", NULL},
    { "search", 1, "search the [file]", p2psearch},
    { "list_search", 0, "list searches", p2plist_search},
    { "list_result", 1, "list the results of search [n]", p2plist_result},
    { "get", 2, "get [result] [search]", p2pget},
    { "", 0, "", NULL},
    { "discover", 0, "topology discovery", p2pdiscover},
    { "topology", 0, "network topology", p2ptopology},
    { "watch", 0, "verify status of neighbors", p2pnetworkstatus},
    { "repair", 0, "repair network", p2pnetworkrepair},
    { "multiget", 1, "get multi-source from search [n]", p2pmultiget},
    { NULL, 0, NULL, NULL}
};

/****************************************************/

/****************************************************/

int help(params *p) {
    int i;
    VERBOSE(p->sp, CLIENT, "\n");
    for (i = 0; commands[i].name; i++) {
        VERBOSE(p->sp, CLIENT, "%11s : %s\n", commands[i].name, commands[i].text);
    }
    VERBOSE(p->sp, CLIENT, "\n");
    return P2P_UI_OK;
}

/****************************************************/

/****************************************************/

int quit(params *p) {
    p2p_ui_socket_close(p->sp, p->sp->client_ui);
    return P2P_UI_QUIT;
}

/****************************************************/

/****************************************************/

int status(params *p) {
    VERBOSE(p->sp, CLIENT, "\n");
    VERBOSE(p->sp, CLIENT, "  server_name = \"%s\"\n", p->sp->server_name);
    VERBOSE(p->sp, CLIENT, "  dir_name    = \"%s\"\n", p->sp->dir_name);
    VERBOSE(p->sp, CLIENT, "  ui tcp      = %d\n", p->sp->port_ui);
    VERBOSE(p->sp, CLIENT, "  p2p tcp     = %d\n", p->sp->port_p2p_tcp);
    VERBOSE(p->sp, CLIENT, "  p2p udp     = %d\n", p->sp->port_p2p_udp);
    VERBOSE(p->sp, CLIENT, "  p2p Id      = %s\n", p2p_addr_get_str(p->sp->p2pMyId));
    VERBOSE(p->sp, CLIENT, "  status      = \n");
    VERBOSE(p->sp, CLIENT, "  neighbors   = [ip:tcp:udp]\n");
    VERBOSE(p->sp, CLIENT, "    %s\n", p2p_addr_get_str(p->sp->p2pLeft));
    VERBOSE(p->sp, CLIENT, "    %s\n", p2p_addr_get_str(p->sp->p2pRight));
    VERBOSE(p->sp, CLIENT, "\n");
    return P2P_UI_OK;
}

/****************************************************/

/****************************************************/

int file_list(params *p) {
    DIR *dir;
    struct dirent* file;
    struct stat state;
    char dirname[MAX_PATH];
    char filename[MAX_PATH];

    strncpy(dirname, p->sp->dir_name, MAX_PATH);
    if (strlen(dirname) == 0)
        strncat(dirname, ".", MAX_PATH);
    if (dirname[strlen(dirname) - 1] != '/')
        strncat(dirname, "/", MAX_PATH - strlen(dirname));

    if ((dir = opendir(dirname)) == NULL) {
        VERBOSE(p->sp, VSYSCL, "\nCannot open the shared directory \"%s\"\n", dirname);
        VERBOSE(p->sp, CLIENT, "\n\n  ** cannot open the shared directory on server ** \n\n");
        return P2P_UI_OK;
    }
    VERBOSE(p->sp, VSYSCL, "ui: getting list file for %s\n", dirname);
    VERBOSE(p->sp, CLIENT, "\nFile list\n");
    while ((file = readdir(dir)) != NULL) {
        strncpy(filename, dirname, MAX_PATH);
        strncat(filename, file->d_name, MAX_PATH - strlen(filename));
        if (stat(filename, &state) == 0) {
            if (S_ISREG(state.st_mode)) {
                VERBOSE(p->sp, CLIENT, "  %20s  (%d bytes)\n", file->d_name, state.st_size);
            } else if (S_ISDIR(state.st_mode)) {
                VERBOSE(p->sp, CLIENT, "  [dir] %14s\n", file->d_name);
            }
        }
    }
    VERBOSE(p->sp, CLIENT, "\n");
    closedir(dir);
    return P2P_UI_OK;
}

/****************************************************/

/****************************************************/

int p2pjoin(params *p) {
    VERBOSE(p->sp, VSYSCL, " ui: Starting JOIN\n");
    // si l'une de deux voisins # MyId => effectuer le LEAVE avant JOIN
    if (p2p_addr_is_equal(p->sp->p2pMyId, p->sp->p2pLeft) != 1) {
        VERBOSE(p->sp, VSYSCL, " we have at least 1 neighbor, do LEAVE first\n");
        p2pleave(p);
        VERBOSE(p->sp, VSYSCL, " LEAVE done, begin JOIN\n");
    }

    p2p_addr dst = p2p_addr_create();
    if (p2p_addr_setstr(dst, p->options[0]) != P2P_OK) {
        VERBOSE(p->sp, CLIENT, "ui: could not parse p2p address\n");
        return P2P_UI_ERROR;
    }

    if (p2p_addr_is_equal(p->sp->p2pMyId, dst) == 1) {
        VERBOSE(p->sp, CLIENT, " Cannot join to myself\n");
        return P2P_ERROR;
    }

    VERBOSE(p->sp, VSYSCL, " sending p2p join msg to %s\n", p2p_addr_get_str(dst));

    p2p_msg msg_join = p2p_msg_create();
    p2p_msg_init((p2p_msg) msg_join, P2P_MSG_JOIN_REQ, P2P_MSG_TTL_ONE_HOP, (p2p_addr) p->sp->p2pMyId, (p2p_addr) dst);
    p2p_msg_init_payload(msg_join, 0, NULL);

    int sock = p2p_tcp_socket_create(p->sp, dst);
    // connect OK
    if (sock != -1) {
        if (p2p_tcp_msg_sendfd(p->sp, (p2p_msg) msg_join, sock) != P2P_OK)
            return P2P_UI_ERROR;

        p2p_msg msg_tcp = p2p_msg_create();
        p2p_tcp_msg_recvfd(p->sp, msg_tcp, sock);
        // après avoir reçu join ack, ferme la socket sock
        // envoie link update par autre socket
        p2p_tcp_socket_close(p->sp, sock);
        unsigned char type_msg = p2p_msg_get_type((p2p_msg) msg_tcp);
        if (type_msg == P2P_MSG_JOIN_ACK) {
            // 	pay_load length = 2 x 8 Octets = 2 x P2P_ADDR_SIZE
            // 	le node courant est bien le destinataire
            if ((p2p_msg_get_length((p2p_msg) msg_tcp) == (2 * P2P_ADDR_SIZE))
                    && (p2p_addr_is_equal(p->sp->p2pMyId, p2p_msg_get_dst((p2p_msg) msg_tcp)))) {

                p2p_addr node_addr = p2p_addr_duplicate(p2p_msg_get_src((p2p_msg) msg_tcp));
                VERBOSE(p->sp, VSYSCL, " RECV msg join ack from %s\n", p2p_addr_get_str(node_addr));
                p2p_addr Left = p2p_addr_create();
                p2p_addr Right = p2p_addr_create();
                memcpy(Left, p2p_msg_get_payload((p2p_msg) msg_tcp), P2P_ADDR_SIZE);
                memcpy(Right, p2p_msg_get_payload((p2p_msg) msg_tcp) + P2P_ADDR_SIZE, P2P_ADDR_SIZE);

                VERBOSE(p->sp, VSYSCL, " SEND msg link update to left neighbor %s\n", p2p_addr_get_str(Left));
                p2p_msg_init((p2p_msg) msg_tcp, P2P_MSG_LINK_UPDATE, P2P_MSG_TTL_ONE_HOP, p->sp->p2pMyId, Left);
                unsigned char *payload = malloc(P2P_ADDR_SIZE + 4);

                unsigned int type_right = htonl(P2P_NEIGNBOR_RIGHT);
                memcpy(payload, p->sp->p2pMyId, P2P_ADDR_SIZE);
                memcpy(payload + P2P_ADDR_SIZE, &type_right, 4);
                p2p_msg_init_payload((p2p_msg) msg_tcp, P2P_ADDR_SIZE + 4, payload);
                sock = p2p_tcp_socket_create(p->sp, Left);
                int code1 = p2p_tcp_msg_sendfd(p->sp, (p2p_msg) msg_tcp, sock);
                p2p_tcp_socket_close(p->sp, sock);

                VERBOSE(p->sp, VSYSCL, " SEND msg link update to right neighbor %s\n", p2p_addr_get_str(Right));
                p2p_msg_init((p2p_msg) msg_tcp, P2P_MSG_LINK_UPDATE, P2P_MSG_TTL_ONE_HOP, p->sp->p2pMyId, Right);

                unsigned int type_left = htonl(P2P_NEIGNBOR_LEFT);
                memcpy(payload, p->sp->p2pMyId, P2P_ADDR_SIZE);
                memcpy(payload + P2P_ADDR_SIZE, &type_left, 4);
                p2p_msg_init_payload((p2p_msg) msg_tcp, P2P_ADDR_SIZE + 4, payload);
                sock = p2p_tcp_socket_create(p->sp, Right);
                int code2 = p2p_tcp_msg_sendfd(p->sp, (p2p_msg) msg_tcp, sock);
                p2p_tcp_socket_close(p->sp, sock);

                // verifie si l'envoie de link_update reussit
                // si oui: mise a jour la table de voisinage
                if (code1 + code2 == 0) {
                    VERBOSE(p->sp, VSYSCL, " UPDATE the neighbors\n");
                    p2p_mutex_lock(p->sp); // mutex
                    p2p_addr_copy(p->sp->p2pLeft, Left);
                    p2p_addr_copy(p->sp->p2pRight, Right);
                    p->sp->p2pTopology.Left[0] = p2p_node_init(p->sp->p2pLeft, NULL);
                    p->sp->p2pTopology.Right[0] = p2p_node_init(p->sp->p2pRight, NULL);
                    p2p_mutex_unlock(p->sp);
                }
                free(payload);
                p2p_addr_delete(Left);
                p2p_addr_delete(Right);
                return P2P_UI_OK;
            } else {
                VERBOSE(p->sp, VSYSCL, " ERROR: wrong data length\n");
                return P2P_UI_ERROR;
            }
        } else {
            VERBOSE(p->sp, VSYSCL, " ERROR: msg undefined\n");
            return P2P_UI_ERROR;
        }
    } else {
        VERBOSE(p->sp, VSYSCL, " Cannot connect to host, JOIN fail\n");
        return P2P_UI_ERROR;
    }
    p2p_addr_delete(dst);
    p2p_msg_delete(msg_join);
//    VERBOSE(p->sp, VSYSCL, "Oh come on this is good\n");
}

/****************************************************/

/****************************************************/

int p2pleave(params *p) {
    // si on est seul, fait pas LEAVE
    if (p2p_addr_is_equal(p->sp->p2pMyId, p->sp->p2pLeft) == 1) {
        // ne pas rien
        return P2P_UI_OK;
    } else {
        p2p_msg msg_link_upd = p2p_msg_create();
        ;
        unsigned char *payload = malloc(P2P_ADDR_SIZE + 4);

        VERBOSE(p->sp, VSYSCL, " ui: Starting LEAVE\n");
        p2p_addr Left = p2p_addr_duplicate(p->sp->p2pLeft);
        p2p_addr Right = p2p_addr_duplicate(p->sp->p2pRight);

        p2p_msg_init((p2p_msg) msg_link_upd, P2P_MSG_LINK_UPDATE, P2P_MSG_TTL_ONE_HOP, (p2p_addr) p->sp->p2pMyId, Left);

        unsigned int type_right = P2P_NEIGNBOR_RIGHT;
        type_right = htonl(type_right);
        memcpy(payload, Right, P2P_ADDR_SIZE);
        memcpy(payload + P2P_ADDR_SIZE, &type_right, 4);
        p2p_msg_init_payload(msg_link_upd, P2P_ADDR_SIZE + 4, payload);
        int code1 = p2p_tcp_msg_send(p->sp, (p2p_msg) msg_link_upd);
        VERBOSE(p->sp, VSYSCL, " sending msg link update to LEFT neighbor %s:\n", p2p_addr_get_str(Left));
        if (code1 == P2P_OK) VERBOSE(p->sp, VSYSCL, "  [OK]\n");
        else VERBOSE(p->sp, VSYSCL, "  [Fail]\n");

        p2p_msg_init((p2p_msg) msg_link_upd, P2P_MSG_LINK_UPDATE, P2P_MSG_TTL_ONE_HOP, (p2p_addr) p->sp->p2pMyId, Right);

        unsigned int type_left = P2P_NEIGNBOR_LEFT;
        type_left = htonl(type_left);
        memcpy(payload, Left, P2P_ADDR_SIZE);
        memcpy(payload + P2P_ADDR_SIZE, &type_left, 4);
        p2p_msg_init_payload(msg_link_upd, P2P_ADDR_SIZE + 4, payload);
        int code2 = p2p_tcp_msg_send(p->sp, (p2p_msg) msg_link_upd);
        VERBOSE(p->sp, VSYSCL, " sending p2p link update msg to RIGHT neighbor %s\n", p2p_addr_get_str(p->sp->p2pRight));
        if (code2 == P2P_OK) VERBOSE(p->sp, VSYSCL, "  [OK]\n");
        else VERBOSE(p->sp, VSYSCL, "  [Fail]\n");

        free(payload);
        p2p_msg_delete(msg_link_upd);
        p2p_addr_delete(Left);
        p2p_addr_delete(Right);

        if ((code1 + code2) == P2P_OK) {
            p2p_mutex_lock(p->sp);
            p2p_addr_copy(p->sp->p2pLeft, p->sp->p2pMyId);
            p2p_addr_copy(p->sp->p2pRight, p->sp->p2pMyId);
            p2p_node_delete(p->sp->p2pTopology.Left[0]);
            p2p_node_delete(p->sp->p2pTopology.Right[0]);
            p2p_mutex_unlock(p->sp);
            return P2P_UI_OK;
        } else
            return P2P_UI_ERROR;
    }
}

/****************************************************/

/****************************************************/

int p2phalt(params *p) {
    VERBOSE(p->sp, VSYSCL, " ui: stoping node\n");
    if (p2pleave(p) == P2P_UI_OK) {
        VERBOSE(p->sp, VSYSCL, " leave Ring completed\n");
        p2p_addr_delete(p->sp->p2pRight);
        p2p_addr_delete(p->sp->p2pMyId);
        p2p_addr_delete(p->sp->p2pLeft);
        free(p->sp->dir_name);

        int i = 0;
        for (i = 1; i < 16; i++) {
            p2p_node_delete(p->sp->p2pTopology.Left[i]);
            p2p_node_delete(p->sp->p2pTopology.Right[i]);
        }

        VERBOSE(p->sp, VSYSCL, " closing sockets\n");
        if ((p2p_ui_socket_close(p->sp, p->sp->client_ui) +
                p2p_tcp_socket_close(p->sp, p->sp->sock_ui) +
                p2p_tcp_socket_close(p->sp, p->sp->sock_tcp) +
                p2p_udp_socket_close(p->sp, p->sp->sock_udp))
                == P2P_OK) {
            VERBOSE(p->sp, VSYSCL, " killing completed\n");
            free(p->sp->server_name);
            free(p->sp);
            exit(P2P_UI_OK);
        } else exit(P2P_UI_ERROR);
    } else exit(P2P_UI_ERROR);
}

/****************************************************/

/****************************************************/

int p2psearch(params* p) {
    // effectue le SEARCH si on a au moins 1 voisin # p2pMyId
    if (p2p_addr_is_equal(p->sp->p2pMyId, p->sp->p2pLeft) == 1) {
        VERBOSE(p->sp, CLIENT, " ** we are alone, do JOIN first\n");
        return P2P_UI_OK;
    } else {
        char *filename = malloc(strlen(p->options[0]) + 1);
        strcpy(filename, p->options[0]);
        filename[strlen(p->options[0])] = '\0';

        // @ broadcast pour SEARCH
        p2p_addr addr_broadcast = p2p_addr_create();
        p2p_addr_set_broadcast(addr_broadcast);

        VERBOSE(p->sp, VSYSCL, " ui: Starting SEARCH for %s\n", filename);
        VERBOSE(p->sp, VSYSCL, " original source : %s\n", p2p_addr_get_str(p->sp->p2pMyId));
        VERBOSE(p->sp, VSYSCL, " id              : %d\n", p->sp->p2pSearchList.nbsearch);
        VERBOSE(p->sp, VSYSCL, " txt             : %s\n", filename);

        p2p_mutex_lock(p->sp); // mutex
        p->sp->p2pSearchList.search[p->sp->p2pSearchList.nbsearch].file_name = filename;
        p->sp->p2pSearchList.search[p->sp->p2pSearchList.nbsearch].nb_result = 0;

        p2p_msg msg_search = p2p_msg_create();
        p2p_msg_init((p2p_msg) msg_search, P2P_MSG_SEARCH, P2P_MSG_TTL_MAX, (p2p_addr) p->sp->p2pMyId, (p2p_addr) addr_broadcast);

        int l = P2P_ADDR_SIZE + 4 + strlen(p->options[0]);
        unsigned char *payload_search = malloc(l);
        int id_search = htonl(p->sp->p2pSearchList.nbsearch);
        memcpy(payload_search, p->sp->p2pMyId, P2P_ADDR_SIZE);
        memcpy(payload_search + P2P_ADDR_SIZE, &id_search, 4);
        memcpy(payload_search + P2P_ADDR_SIZE + 4, filename, strlen(filename));
        p2p_msg_init_payload(msg_search, l, payload_search);
        p->sp->p2pSearchList.nbsearch++;
        p2p_mutex_unlock(p->sp);
        int sockfd;

        VERBOSE(p->sp, VSYSCL, " SEND msg search to LEFT neighbor %s\n", p2p_addr_get_str(p->sp->p2pLeft));
        sockfd = p2p_udp_socket_create(p->sp, p->sp->p2pLeft);
        int code1 = p2p_udp_msg_sendfd(p->sp, (p2p_msg) msg_search, sockfd, p->sp->p2pLeft);
        p2p_udp_socket_close(p->sp, sockfd);


        VERBOSE(p->sp, VSYSCL, " SEND msg search to RIGHT neighbor %s\n", p2p_addr_get_str(p->sp->p2pRight));
        sockfd = p2p_udp_socket_create(p->sp, p->sp->p2pRight);
        int code2 = p2p_udp_msg_sendfd(p->sp, (p2p_msg) msg_search, sockfd, p->sp->p2pRight);
        p2p_udp_socket_close(p->sp, sockfd);

        p2p_addr_delete(addr_broadcast);
        p2p_msg_delete(msg_search);
        free(payload_search);
        if (code1 + code2 == P2P_OK) return P2P_UI_OK;
        else return P2P_UI_ERROR;
    }
}

/****************************************************/

/****************************************************/

int p2plist_search(params* p) {
    int i;
    for (i = 0; i < (p->sp->p2pSearchList.nbsearch); i++) {
        VERBOSE(p->sp, CLIENT, " %d: %s (%d results)\n", i, p->sp->p2pSearchList.search[i].file_name, p->sp->p2pSearchList.search[i].nb_result);
    }
    return P2P_UI_OK;
}

/****************************************************/

/****************************************************/

int p2plist_result(params* p) {
    int nb = atoi(p->options[0]);
    VERBOSE(p->sp, CLIENT, " %d results for search %d (%s)\n", p->sp->p2pSearchList.search[nb].nb_result, nb, p->sp->p2pSearchList.search[nb].file_name);
    int i;
    for (i = 0; i < (p->sp->p2pSearchList.search[nb].nb_result); i++)
        VERBOSE(p->sp, CLIENT, " %d: %s (%d bytes)\n", i, p->sp->p2pSearchList.search[nb].result[i], p->sp->p2pSearchList.search[nb].file_size[i]);
    return P2P_UI_OK;
}

/****************************************************/

/****************************************************/

int p2pmultiget(params* p) {
    int searchId, i;
    searchId = atoi(p->options[0]);
    p->sp->fileId = searchId;
    VERBOSE(p->sp, VSYSCL, "ui: starting GET MULTI SOURCE from search [%d]\n", searchId);

    struct file_search file_demande = p->sp->p2pSearchList.search[searchId];
    if (searchId >= p->sp->p2pSearchList.nbsearch) {
        VERBOSE(p->sp, VSYSCL, "ui: Wrong data input \n");
        return P2P_UI_ERROR;
    }

    char *file_result = file_demande.file_name;
    int size = file_demande.file_size[0];
    int nbSource = file_demande.nb_result;
    VERBOSE(p->sp, VSYSCL, " Number of sources: %d\n", nbSource);
    for (i = 0; i < nbSource; i++)
        VERBOSE(p->sp, VSYSCL, " Source %d : %s\n", i, file_demande.result[i]);

    p2p_file_create_file(p->sp, file_result, size);
    ftime(p->sp->bDownload);
    for (i = 0; i < nbSource * 2; i++) {
        pthread_mutex_lock(&get_mutex);
        pthread_cond_broadcast(&get_cond);
        pthread_mutex_unlock(&get_mutex);
    }

    return P2P_UI_OK;
}

int
p2pget(params* p) {
    int searchId, resultId;
    resultId = atoi(p->options[0]);
    searchId = atoi(p->options[1]);
    VERBOSE(p->sp, VSYSCL, "ui: starting get result [%d] from search [%d]\n", resultId, searchId);

    struct file_search file_demande = p->sp->p2pSearchList.search[searchId];
    if ((searchId >= p->sp->p2pSearchList.nbsearch) || (resultId >= file_demande.nb_result)) {
        VERBOSE(p->sp, VSYSCL, "ui: Wrong data input \n");
        return P2P_UI_ERROR;
    }

    char *file_result = file_demande.file_name;
    int size = file_demande.file_size[resultId];
    int max_data = 65535 - 8;
    int nbMsg = ((size - 1) / (max_data)) + 1;
    VERBOSE(p->sp, VSYSCL, "ui: Envoyer %d messages pour recuperer data. Size = %d\n", nbMsg, size);
    p2p_file_create_file(p->sp, file_result, size);

    int i;
    // on demande de récupérer (nbMsg-1) message en même taille de 64Kb.
    unsigned short length = 8 + strlen(file_result);
    p2p_addr dst = p2p_addr_create();
    p2p_addr_setstr(dst, file_demande.result[resultId]);
    p2p_msg new_msg = p2p_msg_create(); // message à envoyer
    p2p_msg_init(new_msg, P2P_MSG_GET, P2P_MSG_TTL_ONE_HOP, p->sp->p2pMyId, dst);

    unsigned char *payload = (unsigned char *) malloc(length + 1);
    memcpy(payload + 8, (char*) (file_result), strlen(file_result));
    payload[length] = '\0';
    int boffset, eoffset, begin, end;

    p2p_msg msg_data = p2p_msg_create();
    unsigned char *payload_data;
    unsigned char type_msg = p2p_msg_get_type((p2p_msg) msg_data);
    unsigned char status_code;
    int value;
    unsigned short length_data;
    unsigned char *data;
    int sock;
    ftime(p->sp->bDownload);
    for (i = 1; i < nbMsg; i++) {
        begin = (i - 1) * max_data;
        end = (i * max_data) - 1;
        boffset = htonl(begin);
        eoffset = htonl(end);
        VERBOSE(p->sp, VSYSCL, "ui: DATA begin : %d     DATA end : %d\n", begin, end);
        memcpy(payload, (char*) &boffset, 4);
        memcpy(payload + 4, (char*) &eoffset, 4);
        p2p_msg_init_payload(new_msg, length + 1, payload);
        sock = p2p_tcp_socket_create(p->sp, dst);
        if (p2p_tcp_msg_sendfd(p->sp, new_msg, sock) != P2P_OK) return P2P_UI_ERROR; //envoyer msg GET

        // récupérer le message DATA
        p2p_tcp_msg_recvfd(p->sp, msg_data, sock);
        p2p_tcp_socket_close(p->sp, sock);
        type_msg = p2p_msg_get_type((p2p_msg) msg_data);
        if (type_msg == P2P_MSG_DATA) {
            length_data = p2p_msg_get_length(msg_data) - 8;
            payload_data = p2p_msg_get_payload(msg_data);
            memcpy(&status_code, payload_data, 1);
            memcpy(&value, payload_data + 4, 4);
            if (status_code == P2P_DATA_OK) {
                value = ntohl(value);
                data = (unsigned char *) malloc(length_data);
                memcpy(data, payload_data + 8, length_data);
                p2p_file_set_chunck(p->sp, file_result, begin, end, data);
                VERBOSE(p->sp, VSYSCL, "WROTE DATA from %d to %d\n", begin, end);
                free(data);
            } else {
                VERBOSE(p->sp, VSYSCL, "ui: Error with reason : %d\n", value);
                return P2P_UI_ERROR;
            }
        } else {
            VERBOSE(p->sp, VSYSCL, "ui: wrong data\n");
            return P2P_UI_ERROR;
        }
    }

    // le dernier message	
    begin = (nbMsg - 1) * max_data;
    end = size - 1;
    boffset = htonl(begin);
    eoffset = htonl(end);
    memcpy(payload, (char*) &boffset, 4);
    memcpy(payload + 4, (char*) &eoffset, 4);
    VERBOSE(p->sp, VSYSCL, "ui: DATA begin : %d     DATA end : %d\n", begin, end);
    p2p_msg_init_payload(new_msg, length + 1, payload);
    sock = p2p_tcp_socket_create(p->sp, dst);
    if (p2p_tcp_msg_sendfd(p->sp, new_msg, sock) != P2P_OK) return P2P_UI_ERROR;

    // récupérer le message DATA
    p2p_tcp_msg_recvfd(p->sp, msg_data, sock);
    p2p_tcp_socket_close(p->sp, sock);
    type_msg = p2p_msg_get_type((p2p_msg) msg_data);
    if (type_msg == P2P_MSG_DATA) {
        length_data = p2p_msg_get_length(msg_data) - 8;
        payload_data = p2p_msg_get_payload(msg_data);
        memcpy(&status_code, payload_data, 1);
        memcpy(&value, payload_data + 4, 4);
        if (status_code == P2P_DATA_OK) {
            value = ntohl(value);
            data = (unsigned char *) malloc(length_data);
            memcpy(data, payload_data + 8, length_data);
            p2p_file_set_chunck(p->sp, file_result, begin, end, data);
            VERBOSE(p->sp, VSYSCL, "WROTE DATA from %d to %d\n", begin, end);
            ftime(p->sp->eDownload);
            int difftime = (int) (p->sp->eDownload->time - p->sp->bDownload->time);
            VERBOSE(p->sp, VSYSCL, " TIME to GET file: %d seconds\n", difftime);
            free(data);
        } else {
            VERBOSE(p->sp, VSYSCL, "ui: Error with reason : %d\n", value);
            return P2P_UI_ERROR;
        }
    } else {
        VERBOSE(p->sp, VSYSCL, "ui: wrong data\n");
        return P2P_UI_ERROR;
    }
    p2p_msg_delete(msg_data);
    free(payload);
    p2p_addr_delete(dst);
    return P2P_UI_OK;
}


/****************************************************/

/****************************************************/

int p2pdiscover(params* p) {
    // si on est seul, fait pas de DISCOVER
    if (p2p_addr_is_equal(p->sp->p2pMyId, p->sp->p2pLeft) == 1) {
        VERBOSE(p->sp, CLIENT, " ** we are alone, do JOIN first\n");
        return P2P_UI_OK;
    } else {
        p2p_msg new_msg = p2p_msg_create();
        int sockfd;
        p2p_addr addr_broadcast = p2p_addr_create();
        p2p_addr_set_broadcast(addr_broadcast);

        VERBOSE(p->sp, VSYSCL, " ui: sending p2p broadcast topology discovery\n");
        VERBOSE(p->sp, VSYSCL, " Starting TOPO DISCOVERY\n");

        p2p_msg_init(new_msg, P2P_MSG_NEIGHBORS_REQ, P2P_MSG_TTL_MAX, p->sp->p2pMyId, addr_broadcast);
        unsigned char *payload = malloc(P2P_ADDR_SIZE);
        memcpy(payload, (char*) p->sp->p2pMyId, P2P_ADDR_SIZE);
        p2p_msg_init_payload(new_msg, P2P_ADDR_SIZE, payload);

        VERBOSE(p->sp, VSYSCL, " SEND msg neighbor request to LEFT neighbor %s\n", p2p_addr_get_str(p->sp->p2pLeft));
        sockfd = p2p_udp_socket_create(p->sp, p->sp->p2pLeft);
        int code1 = p2p_udp_msg_sendfd(p->sp, new_msg, sockfd, p->sp->p2pLeft);
        p2p_udp_socket_close(p->sp, sockfd);

        VERBOSE(p->sp, VSYSCL, " SEND msg neighbor request to RIGHT neighbor %s\n", p2p_addr_get_str(p->sp->p2pRight));
        sockfd = p2p_udp_socket_create(p->sp, p->sp->p2pRight);
        int code2 = p2p_udp_msg_sendfd(p->sp, new_msg, sockfd, p->sp->p2pRight);
        p2p_udp_socket_close(p->sp, sockfd);

        p2p_msg_delete(new_msg);
        p2p_addr_delete(addr_broadcast);
        free(payload);

        if (code1 + code2 == P2P_OK)
            return P2P_UI_OK;
        else
            return P2P_UI_ERROR;
    }
}

int p2ptopology(params* p) {
    if (p2p_addr_is_equal(p->sp->p2pMyId, p->sp->p2pLeft) == 1) {
        VERBOSE(p->sp, CLIENT, " ** we are alone, do JOIN first\n");
        return P2P_UI_OK;
    } else {

        if ((p->sp->p2pTopology.Left[0] == NULL) || (p->sp->p2pTopology.Right[0] == NULL)) {
            VERBOSE(p->sp, CLIENT, " Network Problem\n");
            return P2P_UI_OK;
        }

        if (p2p_addr_is_equal(p->sp->p2pLeft, p->sp->p2pRight)) {
            VERBOSE(p->sp, CLIENT, "\n\t MyId: %s [%s]\n", p->sp->server_name, p2p_addr_get_str(p->sp->p2pMyId));
            VERBOSE(p->sp, CLIENT, "\t\t ||\n\t\t ||\n");
            VERBOSE(p->sp, CLIENT, "\t Left, Right: [%s]\n", p2p_addr_get_str(p->sp->p2pLeft));

            return P2P_UI_OK;
        } else {
            int i = 0;
            VERBOSE(p->sp, CLIENT, "\t MyId: %s [%s]\n", p->sp->server_name, p2p_addr_get_str(p->sp->p2pMyId));
            while ((i < 16) && (p->sp->p2pTopology.Left[i] != NULL)) {
                VERBOSE(p->sp, CLIENT, "\t\t ||\n\t\t ||\n");
                VERBOSE(p->sp, CLIENT, "\t node %d (Left): [%s]\n", i, p2p_addr_get_str(p->sp->p2pTopology.Left[i]->addr));
                i++;
            }

            i = 0;
            while ((i < 16) && (p->sp->p2pTopology.Right[i] != NULL)) i++;
            while (i > 0) {
                i--;
                VERBOSE(p->sp, CLIENT, "\t\t ||\n\t\t ||\n");
                VERBOSE(p->sp, CLIENT, "\t node %d (Right): [%s]\n", i, p2p_addr_get_str(p->sp->p2pTopology.Right[i]->addr));
            }

            VERBOSE(p->sp, CLIENT, "\t\t ||\n\t\t ||\n");
            VERBOSE(p->sp, CLIENT, "\t MyId: %s [%s]\n", p->sp->server_name, p2p_addr_get_str(p->sp->p2pMyId));

            return P2P_UI_OK;
        }
    }
}

int p2pnetworkstatus(params *p) {
    if (p2p_addr_is_equal(p->sp->p2pMyId, p->sp->p2pLeft) == 1) {
        VERBOSE(p->sp, CLIENT, " ** we are alone, do JOIN first\n");
        return P2P_UI_OK;
    }
    p2p_mutex_lock(p->sp);
    p->sp->checkNetwork = 0;
    networkUpdate(p->sp);
    p2p_mutex_unlock(p->sp);
    return P2P_UI_OK;
}

int p2pnetworkrepair(params *p) {
    if (p->sp->repairNetwork == -1) {
        VERBOSE(p->sp, VSYSCL, " No network problem, no need to repair\n");
        return P2P_UI_OK;
    } else {
        p2p_mutex_lock(p->sp);
        if (networkRepair(p->sp) == P2P_OK)
            p->sp->repairNetwork = -1;
        p2p_mutex_unlock(p->sp);
        VERBOSE(p->sp, VSYSCL, " Network REPAIR completed\n");
        return P2P_OK;
    }
}

int test_ui_delim(char c) {
    return (c == ' ' || c == '\t' || c == 10 || c == 13);
}

static int read_command(char* buf, int maxsize, int sock) {
    int eol = 0;
    char c;
    int length = 0;

    /* telnet sends '1310' == '\r\n' for each newline          */
    /* eol == 2 at the end of a line after it receives '13'10' */
    while (eol < 2 && length < maxsize) {
        if (read(sock, &c, sizeof (char)) == -1) {
            return P2P_UI_QUIT;
        } else {
            if (c == '\n' || c == '\r') {
                eol++;
            } else {
                buf[length] = c;
                length++;
            }
        }
    }
    buf[length] = '\0';

    return length;
}

/****************************************************/

/****************************************************/

int ui_command(server_params *sp) {
    int i, o;
    int ntokens;
    char req[MAX_REQ];
    char tokens[MAX_TOK][MAX_TOKLEN];
    params p = {sp,
        { NULL, NULL, NULL, NULL}};

    if (read_command(req, sizeof (req), sp->client_ui) == -1) {
        return P2P_UI_QUIT;
    }

    VERBOSE(sp, VSYSCL, "ui: request=-%s-\n", req);

    if ((ntokens = get_tokens(req, tokens, test_ui_delim)) == 0) {
        return help(&p);
    }

    for (i = 0; i < ntokens; i++)
        VERBOSE(sp, VSYSCL, "   token %d: -%s-\n", i, tokens[i]);

    for (i = 0; commands[i].name != NULL; i++) {
        if (strcasecmp(commands[i].name, tokens[0]) == 0) {
            if (commands[i].options != ntokens - 1) {
                VERBOSE(sp, VSYSCL, "ui: incorrect number of arguments %s\n", tokens[0]);
                VERBOSE(p.sp, CLIENT, " ** incorrect number of arguments for %s\n", tokens[0]);
                return P2P_UI_OK;
            }
            for (o = 0; o < ntokens; o++) {
                p.options[o] = tokens[o + 1];
            }
            return commands[i].fun(&p);
        }
    }

    VERBOSE(p.sp, CLIENT, "\n %s command unknown\n\n", tokens[0]);

    return P2P_UI_OK;
}
