#ifndef __server_h__
#define __server_h__


#include <unistd.h>

#include <memory>
#include <thread>
#include <netinet/in.h>
#include "io_tcp.h"


class server;
struct connection;


typedef std::unique_ptr<io_tcp> 			io_tcp_ptr;
typedef std::function<void(io_tcp_ptr)>		tcp_server_handler;

/*
 * the server class opens TCP port and listens for incoing connections
 * after a connection established it calls connection_handler, passing to it
 * socket and client address
 */
class tcp_server
{
private:
	int		s;
	int 	port;
	bool	exit_flag;

	void loop();
	void accept_connection();

	tcp_server_handler	cb;

public:	
	tcp_server(tcp_server const&) = delete;
	tcp_server(int port, tcp_server_handler cb);
	~tcp_server();

	// blocking
	void run();
	// async
	void stop();
};


#endif // __server_h__


