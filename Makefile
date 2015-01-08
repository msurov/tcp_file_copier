SRCDIR = ./src
OBJDIR = ./obj
BINDIR = ./bin

SERVER = receive_srv
CLIENT = send_files

SERVER_SRC = 						\
	$(SRCDIR)/config.cpp			\
	$(SRCDIR)/file_serializer.cpp	\
	$(SRCDIR)/receiver.cpp			\
	$(SRCDIR)/tcp_server.cpp		\
	$(SRCDIR)/thread_pool.cpp		\
	$(SRCDIR)/io_tcp.cpp			\

CLIENT_SRC = 						\
	$(SRCDIR)/config.cpp			\
	$(SRCDIR)/file_serializer.cpp	\
	$(SRCDIR)/sender.cpp			\
	$(SRCDIR)/io_tcp.cpp			\
	$(SRCDIR)/thread_pool.cpp		\


SERVER_OBJ = $(addprefix $(OBJDIR)/,$(notdir $(SERVER_SRC:.cpp=.o)))
CLIENT_OBJ = $(addprefix $(OBJDIR)/,$(notdir $(CLIENT_SRC:.cpp=.o)))

CC 			= g++
CFLAGS		= -Wall -std=c++11 -c
LDFLAGS		= -lm -lpthread
DEBUG 		= -DDEBUG -g2
RELEASE 	= -O2 -g0

.PHONY: all
all: release

debug: CFLAGS += $(DEBUG)
debug: link_server link_client

release: CFLAGS += $(RELEASE)
release: link_server link_client

.PHONY:
client: link_client

.PHONY:
server: link_server

.PHONY:
compile_server: $(SERVER_OBJ)

.PHONY:
compile_client: $(CLIENT_OBJ)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) $< -o $@

.PHONY:
link_server: compile_server
	$(CC) -o $(SERVER) $(LDFLAGS) $(SERVER_OBJ)

.PHONY:
link_client: compile_client
	$(CC) -o $(CLIENT) $(LDFLAGS) $(CLIENT_OBJ)

.PHONY:
test:
	./test.sh

.PHNOY: clean
clean:
	find -type f -name "$(SERVER)" -delete
	find -type f -name "$(CLIENT)" -delete
	find -type f -name "*.o" -delete
	find -type f -name "*~" -delete
