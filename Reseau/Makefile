#  NGUYEN LE Duc Tan
#  MIF39 Lyon 1 Informatique


CC=gcc
CFLAGS=-Wall -g 

SRC=			\
	p2p_common.c	\
	p2p_file.c	\
	p2p_addr.c	\
	p2p_msg.c	\
	p2p_ui.c	\
	p2p_main.c

OBJ=$(SRC:.c=.o)
BIN=p2p_node

$(BIN): $(OBJ)
	$(CC) -D_REENTRANT $(OBJ) -o $@ -lpthread

%.o:%.c
	$(CC) -c $(CFLAGS) $< 

tar: clean
	(cd .. ; tar --exclude=src-dist/.svn -zcvf 3TC-P2P-sources.tgz src-dist ) 
clean:
	rm -Rf $(OBJ) $(BIN) log.txt repertoire_node_* *~

p2p_main.o: p2p_main.c p2p_common.h
p2p_addr.o: p2p_addr.c p2p_addr.h p2p_common.h
p2p_msg.o:  p2p_msg.c p2p_msg.h p2p_addr.h p2p_common.h
p2p_common.o: p2p_common.c p2p_common.h


