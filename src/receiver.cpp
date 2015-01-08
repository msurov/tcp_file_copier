#include <iostream>
#include <fstream>
#include <stdexcept>
#include <array>
#include <memory>

#include <sys/socket.h>
#include <signal.h>

#include "err.h"
#include "config.h"
#include "thread_pool.h"
#include "tcp_server.h"
#include "file_serializer.h"


/*
 * external interrupt signal hook
 */
std::unique_ptr<tcp_server>			server;
std::unique_ptr<thread_pool>		threads;
std::unique_ptr<file_deserializer>	deserializer;


void signal_callback_handler(int signum)
{
	trace("closing server...\n");

	if (server)
		server->stop();
}

/* we can't pass a unique pointer here 
 * because std::bind does not support 
 * move-only arguments. so, we pass a
 * raw pointer: before the call we release
 * unique poineter, and inside the function
 * we catch the raw pointer back
 * maybe it is better to use here 
 * shared_ptr<io_tcp> but i guess it's not necessary
 */
void srv_deserialyze_handler(io_tcp* ptcp)
{
	std::unique_ptr<io_tcp> tcp(ptcp);
	*deserializer << *tcp;
}

void srv_handler(std::unique_ptr<io_tcp> tcp)
{
	auto task = std::bind(srv_deserialyze_handler, tcp.release());
	threads->post(task);
};

/*
 * main program
 */
int main()
{
	try
	{
		signal(SIGINT, signal_callback_handler);

		config cfg("server_config.txt");

		int port = cfg.geti("port", 1, 0xFFFF);
		std::string const& folder = cfg.gets("storage");
		throw_if(!check_dir_write_access(folder), 
			std::invalid_argument("can't access directory '" + folder + "' for write. create it or change privileges."));

		threads.reset(new thread_pool());
		threads->run_async(true);

		deserializer.reset(new file_deserializer(folder));

		server.reset(new tcp_server(port, srv_handler));
		server->run();
	}
	catch (std::exception const& e)
	{
		std::cout << e.what() << std::endl;
	}

	trace("main thread closed\n");
	return 0;
}
