/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2023/12/30 11:54:20 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <sys/types.h>
# include <sys/wait.h>

#include "Host.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "Request.hpp"
#include "Header.hpp"

#include "Response.hpp"

Response::Response()
{
    //std::cout << "Response Default constructor" << std::endl;
    _header = "";
    _status_code = 200;
    _content_length = 0;
    _body_size = 0;

    _full_file_name = "";
    _write_queue = false;
    _fd_out = -1;
}
Response::Response(const Response& src) { *this = src; }
Response&	Response::operator=( Response const & src )
{
	(void) src;
	return (*this);
}
Response::~Response() {}

int     Response::write()
{
    if (!_write_queue)
        return (0);
    if (_header == "")
        write_header();
    else
        write_body();
    return (0);
}

void     Response::write_header()
{
    //std::cout << "write_header" << std::endl;

    _full_file_name = _request->get_full_file_name();
    //std::cout << "write_header " << _full_file_name << std::endl;
    Header	header(this, ft::file_extension(_full_file_name));
    if (_status_code == 200)
    {
        header.set_allow(_request->get_location()->get_methods_str());
        if (_request->get_method() == GET)
            get_file_size();
    }
    _header = header.generate();
    std::cout << "Response Header:\n" << _header << std::endl;
    if (send(_socket, _header.c_str(), _header.length(), 0) < 0)
        end_connection();
    if (_status_code != 200)
        end_connection();
}

void     Response::get_file_size()
{
    const char*   fn = _full_file_name.c_str();
    _fd_out = open(fn, O_RDONLY);
    if (_fd_out == -1)
    {
        _status_code = 500;
        return ;
    }
    struct stat fileStat;
    if (stat(fn, &fileStat) == 0)
        _content_length = fileStat.st_size;
    else
    {
        std::cerr << "Error: Get file size." << std::endl;
        _status_code = 500;
        return ;
    }
    //std::cout << "get_file_size _content_length: " << _content_length << std::endl;
}

int     Response::write_body()
{
    _write_queue = true;

    char	buffer[RESPONSE_BUFFER * 1028];
    int ret = read(_fd_out, buffer, RESPONSE_BUFFER * 1028);

    if (ret <= 0)
        return (end_connection());

    _body_size += ret;
    //std::cout << "write_body read |" << _full_file_name << "| " << _socket << " " << _body_size << "/" << _content_length << std::endl;

    if (send(_socket, buffer, ret, 0) < 0)
        return (end_connection());
    //std::cout << "write_body send |" << _full_file_name << "| " << _socket << " " << _body_size << "/" << _content_length << std::endl;
    return (0);
}

int     Response::end_connection(void)
{
    if (_request->get_method() == POST)
        waitpid(_request->get_cgi()->get_pid(), NULL, 0);
    //std::cout << "end_connection read |" << _full_file_name << "| " << _socket << " " << _body_size << "/" << _content_length << std::endl;
    if (_fd_out > 0)
        close(_fd_out);
    _write_queue = false;
    _host->close_client_sk(_socket);
    return (0);
}

int		    Response::get_status_code(void) const {return (_status_code);}
size_t		Response::get_content_length(void) const {return (_content_length);}
int         Response::get_fd_out(void) const {return (_fd_out);}
Host*		Response::get_host(void) const {return (_host);}

void		Response::set_socket(int s) {_socket = s;}
void		Response::set_host(Host* h) {_host = h;}
void		Response::set_server(Server* s) {_server = s;}
void		Response::set_request(Request* r) {_request = r;}
void		Response::set_status_code(int e) {_status_code = e;}
void        Response::set_write_queue(bool b) {_write_queue = b;}
void        Response::set_fd_out(int f) {_fd_out = f;}
