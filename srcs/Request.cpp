/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2023/12/29 17:18:22 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Host.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include "Location.hpp"

Request::Request()
{
	//std::cout << "Request Default constructor" << std::endl;
}
Request::Request(const Request& src) { *this = src; }
Request&	Request::operator=( Request const & src )
{
	(void) src;
	return (*this);
}
Request::Request(int sk, Host* h, Server* s) : _socket(sk), _host(h), _server(s)
{
	//std::cout << "Request Constructor sk: " << sk << std::endl;

	_response.set_socket(sk);
	_response.set_host(h);
	_response.set_server(s);
	_response.set_request(this);

	_header = "";
	_url = "";
	_method = NONE;
	_content_type = "";
	_content_length = 0;
	_body_size = 0;

	_fd_in = -1;
	_full_file_name = "";
    _body_max = _host->get_client_max_body_size() * MEGABYTE;
    _body_buffer = _host->get_client_body_buffer_size() * KILOBYTE;
    _buffer = new char[_body_buffer + 1];
	_read_queue = true;
	_tmp_file = "";

	_status_code = 200;
    _cgi.set_request(this);
}

Request::~Request()
{
    delete[] _buffer;
	if (_socket > 0)
		close(_socket);
	if (_tmp_file != "" && std::remove(_tmp_file.c_str()))
        std::cerr << "Error: Can not delete file " << _tmp_file << std::endl;
}

int     Request::read(void)
{
    if (!_read_queue)
        return (0);
    if (_header == "")
    {
        read_header();
        if (_status_code != 200)
            return (end_read());
        process_fd_in();
        if (_status_code != 200)
            return (end_read());
    }
    else
        read_body();
    return (0);
}

void	Request::read_header()
{
    int ret = 1;

    _body_position = NPOS;
    while (_body_position == NPOS && ret > 0)
    {
        ret = recv(_socket, _buffer, _body_buffer, 0);
        if (ret < 0)
        {
            _status_code = 500;
            return ;
        }
        if (ret > 0)
        {
            _buffer[ret] = 0;
            _header += _buffer;
            _body_position = _header.find("\r\n\r\n");
        }
    }
    std::cout << "Request header: " << _header.size() << std::endl << _header << std::endl;
    if (_body_position == NPOS)
    {
        std::cerr << "Error: No end header found.\n" << std::endl;
        _status_code = 400;	// Bad Request
        return ;
    }
    _body_position += 4;
    _body_size = ret - _body_position;
    //std::cout << "read_header _body_size: " << _body_size << std::endl;
    if (!parser_header())
    {
        std::cerr << "Error: request header invalid.\n" << std::endl;
        _status_code = 400;	// Bad Request
    }
    if (_status_code != 200 || !_body_size)
        end_read();
}

bool	Request::parser_header(void)
{
    if (!read_method_url())
        return (false);
    check_location();
    //if (!check_location())
    //    return (false);
    if (_method != GET && !read_content_type())
        return (false);
    if (_method != GET && !read_content_length())
        return (false);
    return (true);
}

bool	Request::read_method_url()
{
    size_t  newline = _header.find("\n");
    if (newline == NPOS)
        return (false);

    std::vector<std::string>	line0;
    line0 = ft::split_string(_header.substr(0, newline), "     ");
    if (line0.size() != 3)
    {
        std::cerr << "Error: First line header" << std::endl;
        return (false);
    }
    _url = line0[1];
    if (line0[0] == "GET")
        _method = GET;
    else if (line0[0] == "POST")
        _method = POST;
    else if (line0[0] == "PUT")
        _method = PUT;
    else
    {
        std::cerr << "Error: Method unknown : " << line0[0] << std::endl;
        return (false);
    }
    return (true);
}

bool	Request::read_content_type()
{
	const char*	types[] = {
		"text/plain",
		"text/html",
		"text/css",
		"text/javascript",
		"text/xml",
		"application/json",
		"application/xml",
		"application/pdf",
		"application/zip",
		"application/octet-stream",
		"image/jpeg",
		"image/png",
		"image/gif",
		"image/svg+xml",
		"audio/mpeg",
		"video/mp4",
		"multipart/form-data",
		"application/x-www-form-urlencoded"
	};
	size_t	pos = _header.find("Content-Type:");
	if (pos == NPOS)
	{
        std::cerr << "Error: Content type not found." << std::endl;
        return (false);
	}
    for (int i = 0; i < 18; i++)
        if (_header.substr(pos + 14, 50).find(types[i]) != NPOS)
        {
            _content_type = types[i];
            return (true);
        }
    std::cerr << "Error: Content type not found." << std::endl;
    return (false);
}

bool	Request::read_content_length()
{
	size_t	pos1;
	size_t	pos = _header.find("Content-Length: ");
	if (pos != NPOS)
        pos1 = _header.substr(pos).find("\n");
	if (pos == NPOS || pos1 == NPOS)
	{
        std::cerr << "Error: Content length not found." << std::endl;
        return (false);
	}
    pos += 16;
    _content_length = std::atoi(_header.substr(pos, pos1).c_str());
    return (true);
}

void	Request::read_body()
{
    char	buffer[_body_buffer];
    int     ret;

	//std::cout << "chunk_size: " << chunk_size << std::endl;
    ret = recv(_socket, buffer, _body_buffer, 0);
    _body_size += ret;
	std::cout << "read_body: " << ret << std::endl;
	std::cout << "_body_size: " << _body_size << std::endl;
    if (ret < 0)
    {
        std::cerr << "Error: recv error" << std::endl;
        _status_code = 400;
        end_read();
        return ;
    }
    if (ret > 0 && _fd_in > 0)
        write(_fd_in, buffer, ret);
    //if (ret < (int) _body_buffer)
    if (ret == 0 || _body_size >= _content_length)
        end_read();
}

bool	Request::check_location()
{
    //std::cout << "============================" << std::endl;
    //std::cout << "Header:" << _header.size() << std::endl  << _header << std::endl;
    //std::cout << "============================" << std::endl;
    _location = Location::find_location(_url,
            _server->get_locations(), _method, _status_code);

    if (!_location || _status_code != 200)
        return (false);

    _full_file_name = _location->get_full_file_name(_url,
            _server->get_root());

    std::cout << "check_location " << _full_file_name << std::endl;

	struct stat buffer;
	if (_method != PUT
            && stat(_full_file_name.c_str(), &buffer) != 0)
    {
		_status_code = 404; // Not found
        return (false);             
    }
    return (true);
}

void	Request::process_fd_in()
{
    int i = 0;
    switch (_method)
    {
        case GET:
            break;
        case PUT:
            _fd_in = open(_full_file_name.c_str(),
                    O_CREAT | O_WRONLY | O_TRUNC, 0664);
            if (_fd_in == -1)
                _status_code = 500;
            break;
        case POST:
            _tmp_file = "/tmp/0";
            struct stat buffer;
            while (stat(_tmp_file.c_str(), &buffer) != 0)
                _tmp_file = "/tmp/" + ft::itos(++i);
            _fd_in = open(_tmp_file.c_str(),
                    O_CREAT | O_WRONLY | O_TRUNC, 0664);
            if (_fd_in == -1)
                _status_code = 500;
            break;
        case NONE:
            break;
    }
    if (_body_size > 0 && _fd_in != -1)
        write(_fd_in, &_buffer[_body_position], _body_size);
}

int     Request::end_read(void)
{
    std::cout << "end_read" << std::endl;

    if (_fd_in > 0)
        close(_fd_in);
    _read_queue = false;
    _host->new_response_sk(_socket);
    _response.set_status_code(_status_code);
    if (_status_code == 200 && _method == POST)
        _cgi.execute();
    _response.set_write_queue(true);
    return (0);
}

Host*		Request::get_host(void) const {return (_host);}
Server*		Request::get_server(void) const {return (_server);}
e_method	Request::get_method(void) const {return (_method);}
std::string	Request::get_url(void) const {return (_url);}
Response*	Request::get_response(void) {return (&_response);}
const Cgi*  Request::get_cgi(void) const {return (&_cgi);}
int		    Request::get_status_code(void) const {return (_status_code);}
std::string	Request::get_content_type(void) const {return (_content_type);}
size_t		Request::get_content_length(void) const {return (_content_length);}
size_t		Request::get_body_size(void) const {return (_body_size);}
std::string	Request::get_header(void) const {return (_header);}
std::string	Request::get_full_file_name(void) const {return (_full_file_name);}
Location*	Request::get_location(void) const {return (_location);}
int		    Request::get_fd_in(void) const {return (_fd_in);}

void		Request::set_fd_in(int f) {_fd_in = f;}
