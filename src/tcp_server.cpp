#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdexcept>
#include <iostream>
#include <string>

#include <memory.h>

#include "err.h"
#include "trace.h"
#include "tcp_server.h"
#include "common.h"


static const int max_connections = 1024;
static const int check_exit_flag_interval = 15;

/*
 * imp of tcp_server
 */
tcp_server::tcp_server(int port, tcp_server_handler cb) : port(port), cb(cb)
{
	sockaddr_in6	name;

	s = socket(AF_INET6, SOCK_STREAM, 0);
	throw_if(s < 0, std::runtime_error("open socket failed"));

	memset(&name, 0, sizeof(name));
	name.sin6_family = AF_INET6;
	name.sin6_port = htons(port);
	name.sin6_addr = in6addr_any;

	if (bind(s, reinterpret_cast<sockaddr*>(&name), sizeof(name)) < 0)
	{
		close(s);
		throw std::runtime_error("bind socket failed");
	}
}

tcp_server::~tcp_server()
{
	stop();
	close(s);
}

void tcp_server::run()
{
	exit_flag = false;
	loop();
}

void tcp_server::accept_connection()
{
	sockaddr_in6 	client_name;
	socklen_t 		size = sizeof(client_name);

	int client_sock = accept(s, reinterpret_cast<sockaddr*>(&client_name), &size);
	throw_if(client_sock < 0, std::runtime_error("accept socket failed"))

	char addr[256];
	trace(std::string("new connection from host ") + 
		inet_ntop(client_name.sin6_family, &client_name.sin6_addr, addr, sizeof(addr)) + 
		":" + std::to_string(ntohs(client_name.sin6_port)) + " established\n");

	cb(std::unique_ptr<io_tcp>(new io_tcp(client_sock)));
}

void tcp_server::loop()
{
	try
	{
	  	int res;

	  	// open the port
	  	res = listen(s, max_connections);
	  	throw_if(res < 0, std::runtime_error("listen socket failed"))

		// wait for new connections and check each
		// 'check_exit_flag_interval' sec if exit_flag is set
		while (true)
		{
			fd_set 	rfds;
			timeval timeout = {check_exit_flag_interval, 0};

			FD_ZERO(&rfds);
			FD_SET(s, &rfds);

			res = select(s + 1, &rfds, nullptr, nullptr, &timeout);

			if (exit_flag)
			{
				trace("server stopped\n");
				break;
			}

			throw_if(res < 0, std::runtime_error("select socket failed"))

			// 5 sec elapsed
			if (!res)
			{
				trace("there are no messages in " + std::to_string(check_exit_flag_interval) + " sec\n");
				continue;
			}

			if (!FD_ISSET(s, &rfds))
			{
				trace("there are no ready sockets\n");
				continue;
			}

			trace("processing new connection\n");
			accept_connection();
	    }
	}
	catch (std::exception const& e)
	{
		std::cout << std::string("[tcp_server failed] ") + e.what() + "\n";
	}
}

void tcp_server::stop()
{
	exit_flag = true;
}
