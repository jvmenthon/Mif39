/* NGUYEN LE Duc Tan
   MIF39 Lyon 1 Informatique
***/



#ifndef __P2P_FILE
#define __P2P_FILE

#define P2P_FILE_NB_ELT 255

int p2p_file_is_available(server_params* sp, const char* file, int* filesize);
int p2p_file_get_chunck  (server_params* sp, const char* file, int boffset, int eoffset, unsigned char** data);
int p2p_file_create_file (server_params* sp, const char* file, int size);
int p2p_file_set_chunck  (server_params* sp, const char* file, int boffset, int eoffset, unsigned char* data);

#endif
