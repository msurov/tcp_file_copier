#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>

#include <stdexcept>
#include <string>

#include "err.h"
#include "trace.h"
#include "common.h"
#include "io_tcp.h"


io_tcp::io_tcp(int sock) : sock(sock), err(false), end(false)
{
}

io_tcp::io_tcp() : sock(-1), err(false), end(true)
{
}

io_tcp::~io_tcp()
{
	shutdown(sock, SHUT_RDWR);
	close();
}

void io_tcp::close()
{
	if (sock > 0)
		::close(sock);    	
}

void io_tcp::connect(std::string const& server, int port)
{
	addrinfo 	hints, *rp;

	// query ip address, family
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_CANONNAME;

	int res = getaddrinfo(server.c_str(), NULL, &hints, &rp);
	throw_if(res, std::runtime_error("can't require address of " + server + ": " + gai_strerror(res)));

	// create socket 
	sock = socket(rp->ai_family, rp->ai_socktype, 0);
	if (sock < 0)
	{
		freeaddrinfo(rp);
		throw std::runtime_error("open socket failed");
	}

	// IPv4
	if (rp->ai_family == AF_INET)
	{
		sockaddr_in		name;

		memset(&name, 0, sizeof(name));
		name.sin_family = AF_INET;
		name.sin_port = htons(port);
		memcpy(&name.sin_addr, &reinterpret_cast<sockaddr_in*>(rp->ai_addr)->sin_addr, sizeof(sockaddr_in::sin_addr));

		freeaddrinfo(rp);

		if (::connect(sock, reinterpret_cast<sockaddr*>(&name), sizeof(name)) < 0)
		{
			::close(sock);
			throw std::runtime_error("can't connect to " + server + ":" + std::to_string(port));
		}
	}
	// IPv6
	else if (rp->ai_family == AF_INET6)
	{
		sockaddr_in6	name;

		memset(&name, 0, sizeof(name));
		name.sin6_family = AF_INET6;
		name.sin6_port = htons(port);
		memcpy(&name.sin6_addr, &reinterpret_cast<sockaddr_in6*>(rp->ai_addr)->sin6_addr, sizeof(sockaddr_in6::sin6_addr));

		freeaddrinfo(rp);

		if (::connect(sock, reinterpret_cast<sockaddr*>(&name), sizeof(name)) < 0)
		{
			::close(sock);
			throw std::runtime_error("can't connect to " + server + ":" + std::to_string(port));
		}
	}
	else
	{
		freeaddrinfo(rp);
		::close(sock);
		throw std::runtime_error("unsupported address family");
	}
}

void io_tcp::read(char* buf, int len)
{
	if (end)
	{
		err = true;
		last_read = 0;
		return;
	}

	int total = 0;

	while (total < len)
	{
		int res = recv(sock, buf + total, len - total, 0);
		if (res < 0)
		{
			err = true;
			break;
		}
		if (res == 0)
		{
			end = true;
			break;
		}

		total += res;
	}

	last_read = total;
}

void io_tcp::write(char const* buf, int len)
{
	int total = 0;

	while (total < len)
	{
		int res = send(sock, buf + total, len - total, 0);
		if (res < 0)
		{
			err = true;
			break;
		}
    	total += res;
	}
}
