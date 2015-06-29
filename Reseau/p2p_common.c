/* NGUYEN LE Duc Tan
   MIF39 Lyon 1 Informatique
 ***/

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include "p2p_common.h"
#include "p2p_msg.h"
#include "p2p_file.h"

// decouper d'une chaine en elements

int
get_tokens(const char *str, char tok[MAX_TOK][MAX_TOKLEN], int (*test_delim)(char)) {
    int i = 0;
    int token = 0;
    int index = 0;

    while (str[i] && token < MAX_TOK) {
        while (str[i] && test_delim(str[i])) {
            i++;
        }

        index = 0;
        while (str[i] && !test_delim(str[i]) && index < MAX_TOKLEN) {
            tok[token][index++] = str[i++];
        }

        tok[token++][index] = '\0';
    }
    return token;
}

/****************************************************/
/****************************************************/

#define MAX_VNSPRINTF_BUF_LENGTH 300

void
VERBOSE(server_params* sp, int level, char* fmt, ...) {
    FILE* out = stderr;
    FILE *ofp;
    char outputFilename[] = "log.txt";
    ofp = fopen(outputFilename, "a");
    if (ofp == NULL) {
        fprintf(stderr, "Can't open output file %s!\n", outputFilename);
        exit(1);
    }
    
    int length;
    char buf[MAX_VNSPRINTF_BUF_LENGTH + 1];
    va_list ap;

    va_start(ap, fmt);
    length = vsnprintf(buf, MAX_VNSPRINTF_BUF_LENGTH, fmt, ap);
    if (length >= MAX_VNSPRINTF_BUF_LENGTH)
        length = MAX_VNSPRINTF_BUF_LENGTH;
    va_end(ap);

    if (level == CLIENT) {
        if (buf[length - 1] == '\n') {
            buf[length - 1] = '\r';
            buf[length ] = '\n';
            buf[length + 1] = '\0';
            length++;
        }
        write(sp->client_ui, buf, length);
        return;
    }


    if (ofp == NULL) {
        fprintf(stderr, "Can't open output file %s!\n",
                outputFilename);
        exit(1);
    }

    if (sp->verbosity >= level) {
        int i;
        fprintf(out, sp->server_name);
        for (i = 0; i < level; i++)
            fprintf(out, "  ");
        fprintf(out, "** ");
        fprintf(out, buf);
        
        fprintf(ofp, sp->server_name);
        for (i = 0; i < level; i++)
            fprintf(ofp, "  ");
        fprintf(ofp, "** ");
        fprintf(ofp, buf);
        
        fclose(ofp);
        
        return;
    }
}

p2p_node p2p_node_init(p2p_addr addr, char* s) {
    p2p_node a;

    if ((a = (p2p_node) malloc(sizeof (struct p2p_node_t))) == NULL)
        return NULL;

    a->addr = p2p_addr_duplicate(addr);
    if (s != NULL) {
        a->name = (char*) malloc(P2P_MAX_PATH);
        strcpy(a->name, s);
    }
    else a->name = NULL;

    return a;
}

int p2p_node_search(p2p_node s[16], p2p_addr a) {
    int j;

    for (j = 0; j < 16; j++) {
        if (s[j] != NULL) {
            if (p2p_addr_is_equal(s[j]->addr, a))
                return 0;
        }
    }

    return -1;
}

void p2p_node_delete(p2p_node a) {
    if (a != NULL) {
        if (a->name != NULL) free(a->name);
        p2p_addr_delete(a->addr);
        free(a);
        a = NULL;
    }
}

/* Gestion de synchronisation avec mutex */
void p2p_mutex_lock(server_params *sp) {
    errno = pthread_mutex_lock(&(sp->log_mutex));
    if (errno) {
        perror(" ERROR: pthread mutex lock cant continue\n");
        exit(1);
    }
}

void p2p_mutex_unlock(server_params *sp) {
    errno = pthread_mutex_unlock(&(sp->log_mutex));
    if (errno) {
        perror(" ERROR: pthread mutex unlock cant continue\n");
        exit(1);
    }
}

void networkUpdate(server_params *sp) {
    // emettre LINK_UPDATE a deux voisins
    VERBOSE(sp, VSYSCL, " Starting NETWORK UPDATE\n");
    p2p_msg msg_update = p2p_msg_create();
    unsigned char *payload = (unsigned char*) malloc(P2P_ADDR_SIZE + 4);
    unsigned int type_right = htonl(P2P_NEIGNBOR_RIGHT);
    p2p_msg_init((p2p_msg) msg_update, P2P_MSG_LINK_UPDATE, P2P_MSG_TTL_ONE_HOP, sp->p2pMyId, sp->p2pLeft);
    memcpy(payload, sp->p2pMyId, P2P_ADDR_SIZE);
    memcpy(payload + P2P_ADDR_SIZE, &type_right, 4);
    p2p_msg_init_payload((p2p_msg) msg_update, P2P_ADDR_SIZE + 4, payload);
    VERBOSE(sp, VSYSCL, " Check left neighbor %s\n", p2p_addr_get_str(sp->p2pLeft));

    int left = p2p_tcp_msg_send(sp, (p2p_msg) msg_update);

    if (left == P2P_ERROR) {
        // le voisin a gauche meurt
        p2p_mutex_lock(sp);
        p2p_node_delete(sp->p2pTopology.Left[0]);
        sp->p2pTopology.Left[0] = NULL;
        p2p_mutex_unlock(sp);
        VERBOSE(sp, CLIENT, "\n [WARNING]\n");
        VERBOSE(sp, CLIENT, " Left neighbor disappears\n");
        VERBOSE(sp, CLIENT, " To repair network:\n");
        if (p2p_addr_is_equal(sp->p2pLeft, sp->p2pRight) == 1)
            VERBOSE(sp, CLIENT, " JOIN another node\n");
        else
            VERBOSE(sp, CLIENT, " First, do DISCOVER then join another node by repair command\n\n");
        VERBOSE(sp, CLIENT, "p2p:: ");
    }

    p2p_msg_init((p2p_msg) msg_update, P2P_MSG_LINK_UPDATE, P2P_MSG_TTL_ONE_HOP, sp->p2pMyId, sp->p2pRight);
    unsigned int type_left = htonl(P2P_NEIGNBOR_LEFT);
    memcpy(payload, sp->p2pMyId, P2P_ADDR_SIZE);
    memcpy(payload + P2P_ADDR_SIZE, &type_left, 4);
    p2p_msg_init_payload((p2p_msg) msg_update, P2P_ADDR_SIZE + 4, payload);
    VERBOSE(sp, VSYSCL, " Check right neighbor %s\n", p2p_addr_get_str(sp->p2pRight));
    int right = p2p_tcp_msg_send(sp, (p2p_msg) msg_update);
    if (right == P2P_ERROR) {
        // le voisin a droite meurt
        p2p_mutex_lock(sp);
        p2p_node_delete(sp->p2pTopology.Right[0]);
        sp->p2pTopology.Right[0] = NULL;
        p2p_mutex_unlock(sp);
        VERBOSE(sp, CLIENT, "\n [WARNING]\n");
        VERBOSE(sp, CLIENT, " Right neighbor disappears\n");
        VERBOSE(sp, CLIENT, " To repair network:\n");
        if (p2p_addr_is_equal(sp->p2pLeft, sp->p2pRight) == 1)
            VERBOSE(sp, CLIENT, " JOIN another node\n");
        else
            VERBOSE(sp, CLIENT, " First, do DISCOVER then join another node by repair command\n\n");
        VERBOSE(sp, CLIENT, "p2p:: ");
    }

    if (left + right == 0)
        VERBOSE(sp, VSYSCL, " UPDATE completed\n");
    else {
        p2p_mutex_lock(sp);
        sp->checkNetwork = -1;
        sp->repairNetwork = 0; //on peut reparer le reseau
        p2p_mutex_unlock(sp);
    }
    p2p_msg_delete(msg_update);
    free(payload);
}

int networkRepair(server_params *sp) {
    if (p2p_addr_is_equal(sp->p2pMyId, sp->p2pLeft) == 1) {
        VERBOSE(sp, CLIENT, " ** we are alone, do JOIN first\n");
        return P2P_OK;
    } else {
        VERBOSE(sp, VSYSCL, " Starting NETWORK REPAIR\n");
        int i = 0;

        if (sp->p2pTopology.Left[0] == NULL) {
            while (sp->p2pTopology.Right[i] != NULL)
                i++;
            i--;
            p2p_mutex_lock(sp);
            p2p_node_delete(sp->p2pTopology.Right[i]);
            p2p_mutex_unlock(sp);
            i--;
            p2p_addr new_left = p2p_addr_duplicate(sp->p2pTopology.Right[i]->addr);
            p2p_msg msg_link = p2p_msg_create();

            VERBOSE(sp, VSYSCL, " SEND msg link update to new left neighbor %s\n", p2p_addr_get_str(new_left));
            p2p_msg_init((p2p_msg) msg_link, P2P_MSG_LINK_UPDATE, P2P_MSG_TTL_ONE_HOP, sp->p2pMyId, new_left);
            unsigned char *payload = malloc(P2P_ADDR_SIZE + 4);

            unsigned int type_right = htonl(P2P_NEIGNBOR_RIGHT);
            memcpy(payload, sp->p2pMyId, P2P_ADDR_SIZE);
            memcpy(payload + P2P_ADDR_SIZE, &type_right, 4);
            p2p_msg_init_payload((p2p_msg) msg_link, P2P_ADDR_SIZE + 4, payload);
            int code = p2p_tcp_msg_send(sp, (p2p_msg) msg_link);
            p2p_msg_delete(msg_link);
            free(payload);

            if (code == P2P_OK) {
                p2p_mutex_lock(sp);
                p2p_addr_copy(sp->p2pLeft, new_left);
                sp->p2pTopology.Left[0] = p2p_node_init(sp->p2pLeft, NULL);
                p2p_mutex_unlock(sp);
                p2p_addr_delete(new_left);
                return P2P_OK;
            }
            p2p_addr_delete(new_left);
            return P2P_ERROR;
        } else {
            while (sp->p2pTopology.Left[i] != NULL)
                i++;
            i--;
            p2p_mutex_lock(sp);
            p2p_node_delete(sp->p2pTopology.Left[i]);
            p2p_mutex_unlock(sp);
            i--;
            p2p_addr new_right = p2p_addr_duplicate(sp->p2pTopology.Left[i]->addr);
            p2p_msg msg_link = p2p_msg_create();

            VERBOSE(sp, VSYSCL, " SEND msg link update to new right neighbor %s\n", p2p_addr_get_str(new_right));
            p2p_msg_init((p2p_msg) msg_link, P2P_MSG_LINK_UPDATE, P2P_MSG_TTL_ONE_HOP, sp->p2pMyId, new_right);
            unsigned char *payload = malloc(P2P_ADDR_SIZE + 4);

            unsigned int type_left = htonl(P2P_NEIGNBOR_LEFT);
            memcpy(payload, sp->p2pMyId, P2P_ADDR_SIZE);
            memcpy(payload + P2P_ADDR_SIZE, &type_left, 4);
            p2p_msg_init_payload((p2p_msg) msg_link, P2P_ADDR_SIZE + 4, payload);


            int code = p2p_tcp_msg_send(sp, (p2p_msg) msg_link);
            p2p_msg_delete(msg_link);
            free(payload);
            if (code == P2P_OK) { // mise a jour la table
                p2p_mutex_lock(sp);
                p2p_addr_copy(sp->p2pRight, new_right);
                sp->p2pTopology.Right[0] = p2p_node_init(sp->p2pRight, NULL);
                p2p_mutex_unlock(sp);
                p2p_addr_delete(new_right);
                return P2P_OK;
            }

            p2p_addr_delete(new_right);
            return P2P_ERROR;
        }
    }
}

void* p2pthreadget(void *threadId) {
    int Id = (int) threadId;
    int searchId, size, nbThread, lastSize, partSize;
    struct file_search file_demande;
    char *file_result;


    for (;;) {
        char *file_get = malloc(32);
        sprintf(file_get, "%d", Id);

        pthread_mutex_lock(&get_mutex);
        pthread_cond_wait(&get_cond, &get_mutex);
        pthread_mutex_unlock(&get_mutex);

        searchId = sp->fileId;
        file_demande = sp->p2pSearchList.search[searchId];
        file_result = file_demande.file_name;

        strcat(file_get, file_result);

        size = file_demande.file_size[0];
        nbThread = file_demande.nb_result * 2;
        // si le nombre de thread demande > (Id + 1) => participer au GET
        // si non remettre en attente
        if (nbThread > Id) {

            partSize = size / nbThread;
            lastSize = size - (partSize * (nbThread - 1));

            if (Id + 1 < nbThread) size = partSize;
            else if (Id + 1 == nbThread) size = lastSize;

            int max_data = 65535 - 8;
            int nbMsg = ((size - 1) / (max_data)) + 1;
            VERBOSE(sp, VSYSCL, " thread: pid = %ld \n", (long int) syscall(224));
            VERBOSE(sp, VSYSCL, " get multi source from search [%d]\n", searchId);
            VERBOSE(sp, VSYSCL, " Address Source : %s\n", file_demande.result[Id / 2]);
            VERBOSE(sp, VSYSCL, "ui: Envoyer %d messages pour recuperer data. Size = %d\n", nbMsg, size);

            p2p_file_create_file(sp, file_get, size);

            int i;
            // on demande de recuperer (nbMsg-1) message en meme taille de 64Kb.
            unsigned short length = 8 + strlen(file_result);
            p2p_addr dst = p2p_addr_create();
            p2p_addr_setstr(dst, file_demande.result[Id / 2]);
            p2p_msg new_msg = p2p_msg_create(); // message a envoyer
            p2p_msg_init(new_msg, P2P_MSG_GET, P2P_MSG_TTL_ONE_HOP, sp->p2pMyId, dst);

            unsigned char *payload = (unsigned char *) malloc(length + 1);
            memcpy(payload + 8, (char*) (file_result), strlen(file_result));
            payload[length] = '\0';
            int boffset, eoffset, begin, end;
            int beginfile = Id * size;

            unsigned char *part_file = (unsigned char*) malloc(size);

            p2p_msg msg_data = p2p_msg_create();
            unsigned char *payload_data;
            unsigned char type_msg = p2p_msg_get_type((p2p_msg) msg_data);
            unsigned char status_code;
            int value;
            unsigned short length_data;
            unsigned char *data;
            int sock;
            for (i = 1; i < nbMsg; i++) {
                begin = beginfile + (i - 1) * max_data;
                end = beginfile + (i * max_data) - 1;
                boffset = htonl(begin);
                eoffset = htonl(end);
                VERBOSE(sp, VSYSCL, "ui: DATA begin : %d     DATA end : %d\n", begin, end);
                memcpy(payload, (char*) &boffset, 4);
                memcpy(payload + 4, (char*) &eoffset, 4);
                p2p_msg_init_payload(new_msg, length + 1, payload);
                sock = p2p_tcp_socket_create(sp, dst);
                if (p2p_tcp_msg_sendfd(sp, new_msg, sock) != P2P_OK) return NULL; //envoyer msg GET

                // recuperer le message DATA
                p2p_tcp_msg_recvfd(sp, msg_data, sock);
                p2p_tcp_socket_close(sp, sock);
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
                        p2p_file_set_chunck(sp, file_get, begin - beginfile, end - beginfile, data);
                        VERBOSE(sp, VSYSCL, "WROTE DATA from %d to %d\n", begin, end);
                        free(data);
                    } else {
                        VERBOSE(sp, VSYSCL, "ui: Error with reason : %d\n", value);
                        return NULL;
                    }
                } else {
                    VERBOSE(sp, VSYSCL, "ui: wrong data\n");
                    return NULL;
                }
            }
            // le dernier message	
            begin = beginfile + (nbMsg - 1) * max_data;
            end = beginfile + size - 1;
            boffset = htonl(begin);
            eoffset = htonl(end);
            memcpy(payload, (char*) &boffset, 4);
            memcpy(payload + 4, (char*) &eoffset, 4);
            VERBOSE(sp, VSYSCL, "ui: DATA begin : %d     DATA end : %d\n", begin, end);
            p2p_msg_init_payload(new_msg, length + 1, payload);
            sock = p2p_tcp_socket_create(sp, dst);
            if (p2p_tcp_msg_sendfd(sp, new_msg, sock) != P2P_OK) return NULL;

            // recuperer le message DATA
            p2p_tcp_msg_recvfd(sp, msg_data, sock);
            p2p_tcp_socket_close(sp, sock);
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
                    p2p_file_set_chunck(sp, file_get, begin - beginfile, end - beginfile, data);
                    p2p_file_get_chunck(sp, file_get, 0, size - 1, &part_file);
                    p2p_mutex_lock(sp);
                    p2p_file_set_chunck(sp, file_result, beginfile, beginfile + size - 1, part_file);
                    VERBOSE(sp, VSYSCL, "WROTE DATA from %d to %d\n", begin, end);
                    // si le thread est le dernier:
                    ftime(sp->eDownload);
                    int difftime = (int) (sp->eDownload->time - sp->bDownload->time);
                    p2p_mutex_unlock(sp);
                    VERBOSE(sp, VSYSCL, " TIME to GET file: %d seconds\n", difftime);
                    free(file_get);
                    char *file_delete = malloc(32);
                    sprintf(file_delete, "%s/%d%s", sp->dir_name, Id, file_result);
                    VERBOSE(sp, VSYSCL, " delete file %s\n", file_delete);

                    remove(file_delete);
                    free(file_delete);

                }
                free(data);
                free(part_file);
            } else {
                VERBOSE(sp, VSYSCL, "ui: Error with reason : %d\n", value);
                return NULL;
            }

            p2p_msg_delete(msg_data);
            p2p_msg_delete(new_msg);
            p2p_addr_delete(dst);
            free(payload);
        }
        else VERBOSE(sp, VSYSCL, " Waiting..\n");
    }
    return NULL;
}
