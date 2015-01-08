#include <string>


/*
 * socket wrapper
 * 
 * the class is used like a iostream 
 * as a tamplate parameter
 */

class io_tcp
{
private:
    int 	sock;
    bool 	err;
    bool 	end;
    int 	last_read;

public:
	io_tcp(io_tcp const&) = delete;

    io_tcp(int sock);
    io_tcp();
    ~io_tcp();

    void connect(std::string const& server, int port);
    void close();
    void read(char* buf, int len);
    void write(char const* buf, int len);

    inline int gcount() const
    {
    	return last_read;
    }

    inline bool good() const
    {
    	return !err;
    }

    inline bool eof() const
    {
    	return end;
    }

};
