#include <iostream>
#include <fstream>
#include <stdexcept>
#include <array>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <memory.h>

#include <signal.h>

#include <vector>
#include <memory>

#include "io_tcp.h"
#include "thread_pool.h"
#include "common.h"
#include "config.h"
#include "file_serializer.h"


std::unique_ptr<thread_pool>	threads;

/*
 * external interrupt signal hook
 */

void signal_callback_handler(int signum)
{
	std::cout << "stopping client...\n";
	if (threads)
		threads->stop();
}

/*
 * main program
 */

void parse_args(int argc, char** argv, std::vector<std::string>& names)
{
	if (argc == 1)
	{
		std::cout << "syntax usage:" << std::endl <<
			"send filename1 [filenam2 ...]" << std::endl;
		throw std::invalid_argument("incorrect arguments");
	}

	for (int i = 1; i < argc; ++ i)
	{
		std::string path = trim(argv[i]);
		throw_if(!check_file_for_reading(path), 
			std::runtime_error("can't access file " + path + " for reading"));
		names.push_back(path);
	}
}

int main(int argc, char** argv)
{
	try
	{
		signal(SIGINT, signal_callback_handler);

		// init file sender
		std::vector<std::string> files;
		parse_args(argc, argv, files);

		// connect to remote server
		config cfg("client_config.txt");

		std::string const server = cfg.gets("server");
		int const port = cfg.geti("port", 1, 0xFFFF);

		// multithreading
		int const nthreads = std::min<size_t>(files.size(), 8);
		threads.reset(new thread_pool(nthreads));

		// send task
		auto task = [server,port](std::string const& filepath)
		{
			std::cout << "sending file '" + filepath + "'\n";

			file_serializer serializer(filepath);
			io_tcp sock;
			sock.connect(server, port);
			serializer >> sock;

			std::cout << "file '" + filepath + "' sent\n";
		};

		for (auto& filepath: files)
		{
			auto t = std::bind(task, filepath);
			threads->post(t);
		}

		threads->run();
	}
	catch (std::exception const& e)
	{
		std::cout << e.what() << std::endl;
		return -1;
	}

	return 0;
}
