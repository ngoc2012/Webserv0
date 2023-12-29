/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Host.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2023/12/26 17:03:09 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Host.hpp"
#include "Server.hpp"
#include "Response.hpp"
#include "Request.hpp"
#include "Configuration.hpp"

Host::Host(const Host& src) { *this = src; }

Host::Host(const char* conf) {
	_max_clients = 128;
	_client_max_body_size = 1;
	_client_body_buffer_size = 128;
	_parser_error = false;

	_max_sk = -1;
    mimes();
    status_message();
	Configuration	parser(_servers, this, conf);
}

Host&	Host::operator=( Host const & src )
{
	(void) src;
	return (*this);
}

Host::~Host()
{
	std::cout << "Host Destructor" << std::endl;
	for (std::vector<Server*>::iterator it = _servers.begin();
		it != _servers.end(); ++it)
		delete (*it);
	for (std::map<int, Request*>::iterator it = _sk_request.begin();
		it != _sk_request.end(); ++it)
		delete (it->second);
	std::cout << "End server" << std::endl;
}

void	Host::start(void)
{
	if (_parser_error || !check_servers_conf())
		return ;
	FD_ZERO(&_master_read_set);
	FD_ZERO(&_master_write_set);
	FD_ZERO(&_server_set);
	start_server();
	if (!_sk_server.size())
		return ;
	do
	{
		memcpy(&_read_set, &_master_read_set, sizeof(_master_read_set));
		memcpy(&_write_set, &_master_write_set, sizeof(_master_write_set));
		if (select_available_sk() == false)
			break;
		check_sk_ready();
	} while (true);
}

bool	Host::check_servers_conf(void)
{
	for (std::vector<Server*>::iterator it = _servers.begin(); it != _servers.end(); ++it)
		if ((*it)->get_root() == "")
		{
			std::cerr << "Error: root null" << std::endl;
			return (false);
		}
	return (true);
}

void	Host::start_server(void)
{
	int	listen_sk;
	for (std::vector<Server*>::iterator it = _servers.begin(); it != _servers.end();)
	{
		(*it)->set_host(this);
		listen_sk = (*it)->server_socket();
		if (listen_sk > 0)
		{
			add_sk_2_master_read_set(listen_sk, *it);
			FD_SET(listen_sk, &_server_set);
			++it;
		}
		else
		{
			delete (*it);
			_servers.erase(it);
		}
	}
}

bool	Host::select_available_sk(void)
{
	std::cout << "Waiting on select() ..." << std::endl;
	_sk_ready = select(_max_sk + 1, &_read_set, &_write_set, NULL, NULL);// No timeout
	//std::cout << "_sk_ready = " << _sk_ready << std::endl;
	if (_sk_ready < 0)
	{
		perror("select() failed");
		return (false);
	}
	return (true);
}

void	Host::check_sk_ready(void)
{
    for (int i = 0; i <= _max_sk && _sk_ready > 0; ++i)
    {
        if (FD_ISSET(i, &_read_set))
        {
            //std::cout << "Read set sk = " << i << std::endl;
            _sk_ready--;
            if (FD_ISSET(i, &_server_set))
                _sk_server[i]->accept_client_sk();
            else
                _sk_request[i]->read();
        }
        if (FD_ISSET(i, &_write_set))
        {
            _sk_ready--;
            std::cout << "Write set sk = " << i << std::endl;
            _sk_request[i]->get_response()->write();
        }
    }
}

void  	Host::add_sk_2_master_read_set(int new_sk, Server* s)
{
	if (new_sk > _max_sk)
		_max_sk = new_sk;
	FD_SET(new_sk, &_master_read_set);
	_sk_server[new_sk] = s;
}

void	Host::new_request_sk(int new_sk, Server* s)
{
	add_sk_2_master_read_set(new_sk, s);
	_sk_request[new_sk] = new Request(new_sk, this, s);
}

void	Host::new_response_sk(int new_sk)
{
	FD_CLR(new_sk, &_master_read_set);
	FD_SET(new_sk, &_master_write_set);
}

void	Host::close_client_sk(int i)
{
    std::cout << "close_client_sk " << i << std::endl;
	FD_CLR(i, &_master_write_set);
	delete (_sk_request[i]);
	_sk_request.erase(i);
	// If i is max_sk -> find another max_sk
	if (i == _max_sk)
		while (!FD_ISSET(_max_sk, &_master_read_set))
			_max_sk -= 1;
}

void    Host::status_message(void)
{
	_status_message[100] = "Continue";
	_status_message[200] = "OK";
	_status_message[201] = "Created";
	_status_message[204] = "No Content";
	_status_message[400] = "Bad Request";
	_status_message[403] = "Forbidden";
	_status_message[404] = "Not Found";
	_status_message[405] = "Method Not Allowed";
	_status_message[413] = "Payload Too Large";
	_status_message[500] = "Internal Server Error";
}

void	Host::mimes(void)
{
    _mimes["html"] = "text/html";
    _mimes["htm"] = "text/html";
    _mimes["shtml"] = "text/html";
    _mimes["css"] = "text/css";
    _mimes["xml"] = "text/xml";
    _mimes["gif"] = "image/gif";
    _mimes["jpeg"] = "image/jpeg";
    _mimes["jpg"] = "image/jpeg";
    _mimes["js"] = "application/javascript";
    _mimes["atom"] = "application/atom+xml";
    _mimes["rss"] = "application/rss+xml";

    _mimes["mml"] = "text/mathml";
    _mimes["txt"] = "text/plain";
    _mimes["jad"] = "text/vnd.sun.j2me.app-descriptor";
    _mimes["wml"] = "text/vnd.wap.wml";
    _mimes["htc"] = "text/x-component";

    _mimes["png"] = "image/png";
    _mimes["svg"] = "image/svg+xml";
    _mimes["svgz"] = "image/svg+xml";
    _mimes["tif"] = "image/tiff";
    _mimes["tiff"] = "image/tiff";
    _mimes["wbmp"] = "image/vnd.wap.wbmp";
    _mimes["webp"] = "image/webp";
    _mimes["ico"] = "image/x-icon";
    _mimes["jng"] = "image/x-jng";
    _mimes["bmp"] = "image/x-ms-bmp";

    _mimes["woff"] = "font/woff";
    _mimes["woff2"] = "font/woff2";

    _mimes["jar"] = "application/java-archive";
    _mimes["war"] = "application/java-archive";
    _mimes["ear"] = "application/java-archive";
    _mimes["json"] = "application/json";
    _mimes["hqx"] = "application/mac-binhex40";
    _mimes["doc"] = "application/msword";
    _mimes["pdf"] = "application/pdf";
    _mimes["ps"] = "application/postscript";
    _mimes["eps"] = "application/postscript";
    _mimes["ai"] = "application/postscript";
    _mimes["rtf"] = "application/rtf";
    _mimes["m3u8"] = "application/vnd.apple.mpegurl";
    _mimes["kml"] = "application/vnd.google-earth.kml+xml";
    _mimes["kmz"] = "application/vnd.google-earth.kmz";
    _mimes["xls"] = "application/vnd.ms-excel";
    _mimes["eot"] = "application/vnd.ms-fontobject";
    _mimes["ppt"] = "application/vnd.ms-powerpoint";
    _mimes["odg"] = "application/vnd.oasis.opendocument.graphics";
    _mimes["odp"] = "application/vnd.oasis.opendocument.presentation";
    _mimes["ods"] = "application/vnd.oasis.opendocument.spreadsheet";
    _mimes["odt"] = "application/vnd.oasis.opendocument.text";

    _mimes["pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
    _mimes["xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    _mimes["docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";

    _mimes["wmlc"] = "application/vnd.wap.wmlc";
    _mimes["7z"] = "application/x-7z-compressed";
    _mimes["cco"] = "application/x-cocoa";
    _mimes["jardiff"] = "application/x-java-archive-diff";
    _mimes["jnlp"] = "application/x-java-jnlp-file";
    _mimes["run"] = "application/x-makeself";
    _mimes["pl"] = "application/x-perl";
    _mimes["pm"] = "application/x-perl";
    _mimes["prc"] = "application/x-pilot";
    _mimes["pdb"] = "application/x-pilot";
    _mimes["rar"] = "application/x-rar-compressed";
    _mimes["rpm"] = "application/x-redhat-package-manager";
    _mimes["sea"] = "application/x-sea";
    _mimes["swf"] = "application/x-shockwave-flash";
    _mimes["sit"] = "application/x-stuffit";
    _mimes["tcl"] = "application/x-tcl";
    _mimes["tk"] = "application/x-tcl";
    _mimes["der"] = "application/x-x509-ca-cert";
    _mimes["pem"] = "application/x-x509-ca-cert";
    _mimes["crt"] = "application/x-x509-ca-cert";
    _mimes["xpi"] = "application/x-xpinstall";
    _mimes["xhtml"] = "application/xhtml+xml";
    _mimes["xspf"] = "application/xspf+xml";
    _mimes["zip"] = "application/zip";

    _mimes["bin"] = "application/octet-stream";
    _mimes["exe"] = "application/octet-stream";
    _mimes["dll"] = "application/octet-stream";
    _mimes["deb"] = "application/octet-stream";
    _mimes["dmg"] = "application/octet-stream";
    _mimes["iso"] = "application/octet-stream";
    _mimes["img"] = "application/octet-stream";
    _mimes["msi"] = "application/octet-stream";
    _mimes["msp"] = "application/octet-stream";
    _mimes["msm"] = "application/octet-stream";

    _mimes["mid"] = "audio/midi";
    _mimes["midi"] = "audio/midi";
    _mimes["kar"] = "audio/midi";
    _mimes["mp3"] = "audio/mpeg";
    _mimes["ogg"] = "audio/ogg";
    _mimes["m4a"] = "audio/x-m4a";
    _mimes["ra"] = "audio/x-realaudio";

    _mimes["3gpp"] = "video/3gpp";
    _mimes["3gp"] = "video/3gpp";
    _mimes["ts"] = "video/mp2t";
    _mimes["mp4"] = "video/mp4";
    _mimes["mpeg"] = "video/mpeg";
    _mimes["mpg"] = "video/mpeg";
    _mimes["mov"] = "video/quicktime";
    _mimes["webm"] = "video/webm";
    _mimes["flv"] = "video/x-flv";
    _mimes["m4v"] = "video/x-m4v";
    _mimes["mng"] = "video/x-mng";
    _mimes["asx"] = "video/x-ms-asf";
    _mimes["asf"] = "video/x-ms-asf";
    _mimes["wmv"] = "video/x-ms-wmv";
    _mimes["avi"] = "video/x-msvideo";
}

int			                        Host::get_max_clients(void) const {return (_max_clients);}
std::map<int, Server*>	            Host::get_sk_server(void) const {return (_sk_server);}
std::map<int, Request*>	            Host::get_sk_request(void) const {return (_sk_request);}
//std::map<int, Response*>	Host::get_sk_response(void) const {return (_sk_response);}
size_t			                    Host::get_client_max_body_size(void) const {return (_client_max_body_size);}
size_t			                    Host::get_client_body_buffer_size(void) const {return (_client_body_buffer_size);}
std::map<std::string, std::string>*	Host::get_mimes(void) {return (&_mimes);}
std::map<int, std::string>*  		Host::get_status_message(void) {return (&_status_message);}

void			Host::set_client_max_body_size(size_t n) {_client_max_body_size = n;}
void			Host::set_client_body_buffer_size(size_t n) {_client_body_buffer_size = n;}
void			Host::set_parser_error(bool e) {_parser_error = e;}
