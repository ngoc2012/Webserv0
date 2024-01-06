/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/01/06 09:26:03 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Host.hpp"
#include "Address.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "Request.hpp"
#include "Header.hpp"
#include "Cgi.hpp"

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
Request::Request(int sk, Host* h, Address* a) : _socket(sk), _host(h), _address(a)
{
	//std::cout << "Request Constructor sk: " << sk << std::endl;

    _server = 0;
	_response.set_socket(sk);
	_response.set_host(h);
	_response.set_server(_server);
	_response.set_request(this);
    _cgi = 0;

	_str_header = "";
	_url = "";
	_method = NONE;
	_content_type = "";
	_content_length = 0;
	_body_size = 0;
    _header.set_host(h);
    _header.set_str(&_str_header);

	_fd_in = -1;
	_full_file_name = "";
    _body_max = _host->get_client_max_body_size() * MEGABYTE;
    _body_buffer = _host->get_client_body_buffer_size() * KILOBYTE;
    _buffer = new char[_body_buffer + 1];
	_read_queue = true;
	_tmp_file = "";

	_status_code = 200;
}

Request::~Request()
{
    delete[] _buffer;
    if (_cgi)
        delete (_cgi);
    if (_socket > 0)
        close(_socket);
    if (_tmp_file != "" && std::remove(_tmp_file.c_str()))
        std::cerr << "Error: Can not delete file " << _tmp_file << std::endl;
}

int     Request::read(void)
{
    if (!_read_queue)
        return (0);
    if (_str_header == "")
        read_header();
    else
        read_body();
    return (0);
}

int     Request::read_header()
{
    //std::cout << "read_header _body_size: " << _body_size << std::endl;
    if (!receive_header())
        return (end_read());
    if (!parse_header())
        return (end_read());
    process_fd_in();
    if (_status_code != 200 || !_body_size)
        return (end_read());
    return (0);
}

bool	Request::receive_header(void)
{
    int ret = 1;

    _body_position = NPOS;
    while (_body_position == NPOS && ret > 0)
    {
        ret = recv(_socket, _buffer, _body_buffer, 0);
        if (ret < 0)
        {
            _status_code = 500;
            return (false);
        }
        if (ret > 0)
        {
            _buffer[ret] = 0;
            _str_header += _buffer;
            _body_position = _str_header.find("\r\n\r\n");
        }
    }
    std::cout << "Request header: " << _str_header.size() << std::endl << _str_header << std::endl;
    if (_body_position == NPOS)
    {
        std::cerr << "Error: No end header found.\n" << std::endl;
        _status_code = 400;	// Bad Request
        return (false);
    }
    _body_position += 4;
    _body_size = ret - _body_position;
    return (true);
}

bool	Request::parse_header(void)
{
    if (!_header.parse_method_url(_url, _method))
    {
        _status_code = 400;	// Bad Request
        return (false);
    }
    _host_name = _header.parse_host_name();
    if (_host_name == "")
    {
        _status_code = 400;	// Bad Request
        return (false);
    }
    std::vector<Server*>        servers = _address->get_servers();
    std::vector<std::string>    server_names;
    _server = servers[0];
    for (std::vector<Server*>::iterator sv = servers.begin() + 1;
            sv != servers.end(); ++sv)
    {
        server_names = (*sv)->get_server_names();
        for (std::vector<std::string>::iterator it = server_names.begin() + 1;
                it != server_names.end(); ++it)
            if (_host_name == *it)
                _server = *sv;
    }
    if (!check_location())
        return (false);
    if (_method == POST)
        _cgi = new Cgi(this);
    if (_method == GET)
        return (true);
    _content_type = _header.parse_content_type();
    std::cout << "Content-Type: " << _content_type << std::endl;
    _content_length = _header.parse_content_length();
    std::cout << "Content-Length: " << _content_length << std::endl;
    if (_content_type == "" || _content_length == NPOS)
    {
        _status_code = 400;	// Bad Request
        return (false);
    }
    if (_content_length > _body_max)
	{
        _status_code = 400;	// Bad Request
        std::cerr << "Error: Content length bigger than " << _body_max << std::endl;
        return (false);
	}
    return (true);
}

int     Request::read_body()
{
    char	buffer[_body_buffer];
    int     ret;

    ret = recv(_socket, buffer, _body_buffer, 0);
    if (ret < 0)
    {
        std::cerr << "Error: recv error" << std::endl;
        _status_code = 400;
        return (end_read());
    }
    _body_size += ret;
	//std::cout << "read_body: " << ret << std::endl;
	//std::cout << "_body_size: " << _body_size << std::endl;
    if (ret > 0 && _fd_in > 0
        && write(_fd_in, buffer, ret) == -1)
        return (end_read());
    if (ret == 0 || _body_size >= _content_length)
        return (end_read());
    return (0);
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
            _server->get_root(), _method);

    std::cout << "check_location " << _socket << " " << _full_file_name << std::endl;

	struct stat info;
	if (_method != PUT
            && stat(_full_file_name.c_str(), &info) != 0)
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
        case DELETE:
            if (std::remove(_full_file_name.c_str()))
                std::cerr << "Error: Can not delete file " << _tmp_file << std::endl;
            break;
        case NONE:
            break;
    }
    if (_body_size > 0 && _fd_in != -1 && _status_code == 200
        && write(_fd_in, &_buffer[_body_position], _body_size) == -1)
        _status_code = 500;
}

int     Request::end_read(void)
{
    std::cout << "end_read " << _socket << " " << _full_file_name << std::endl;

    if (_fd_in > 0)
        close(_fd_in);
    _read_queue = false;
    _host->new_response_sk(_socket);
    _response.set_status_code(_status_code);
    if (_status_code == 200 && _method == POST)
        _cgi->execute();
    _response.set_write_queue(true);
    return (0);
}

Host*		Request::get_host(void) const {return (_host);}
Server*		Request::get_server(void) const {return (_server);}
e_method	Request::get_method(void) const {return (_method);}
std::string	Request::get_url(void) const {return (_url);}
Response*	Request::get_response(void) {return (&_response);}
Cgi*        Request::get_cgi(void) const {return (_cgi);}
int		    Request::get_status_code(void) const {return (_status_code);}
std::string	Request::get_content_type(void) const {return (_content_type);}
size_t		Request::get_content_length(void) const {return (_content_length);}
size_t		Request::get_body_size(void) const {return (_body_size);}
std::string	Request::get_str_header(void) const {return (_str_header);}
std::string	Request::get_full_file_name(void) const {return (_full_file_name);}
Location*	Request::get_location(void) const {return (_location);}
int		    Request::get_fd_in(void) const {return (_fd_in);}

void		Request::set_fd_in(int f) {_fd_in = f;}
