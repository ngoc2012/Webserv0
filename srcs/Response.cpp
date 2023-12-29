/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2023/12/27 23:33:32 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
    {
        write_header();
        if (_status_code != 200)
            return (end_connection());
        process_fd_out();
    }
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
        header.set_allow(_request->get_location()->get_methods_str());
    if (_status_code == 200 && _request->get_method() == GET)
        get_file_size();
    _header = header.generate();
    std::cout << "Response Header:\n" << _header << std::endl;
    if (send(_socket, _header.c_str(), _header.length(), 0) < 0)
        end_connection();
}

void     Response::get_file_size()
{
    struct stat fileStat;
    if (stat(_full_file_name.c_str(), &fileStat) == 0)
        _content_length = fileStat.st_size;
    else
    {
        std::cerr << "Error: Get file size." << std::endl;
        _status_code = 500;
    }
    std::cout << "get_file_size _content_length: " << _content_length << std::endl;
}

void     Response::process_fd_out()
{
    switch (_request->get_method())
    {
        case GET:
            _fd_out = open(_full_file_name.c_str(), O_RDONLY);
            std::cout << "get_fd_out _full_file_name " << _fd_out << " " << _full_file_name << " " << _socket << std::endl;
            std::cout << _full_file_name << _fd_out << std::endl;
            if (_fd_out == -1)
            {
                _status_code = 500;
                end_connection();
            }
            break;
        default:
            end_connection();
            break;
    }
}

int     Response::write_body()
{
    _write_queue = true;

    char	buffer[RESPONSE_BUFFER * 1028];
    int ret = read(_fd_out, buffer, RESPONSE_BUFFER * 1028);

    if (ret <= 0)
        return (end_connection());

    _body_size += ret;
    std::cout << "write_body read |" << _full_file_name << "| " << _socket << " " << _body_size << "/" << _content_length << std::endl;

    if (send(_socket, buffer, ret, 0) < 0)
        return (end_connection());
    std::cout << "write_body send |" << _full_file_name << "| " << _socket << " " << _body_size << "/" << _content_length << std::endl;
    return (0);
}

int     Response::end_connection(void)
{
    std::cout << "end_connection read |" << _full_file_name << "| " << _socket << " " << _body_size << "/" << _content_length << std::endl;
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
