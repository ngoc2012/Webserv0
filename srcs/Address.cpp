/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Address.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/01/08 14:16:20 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Host.hpp"
#include "Server.hpp"
//#include "Request.hpp"
//#include "Response.hpp"
#include "webserv.hpp"

#include "Address.hpp"

Address::Address()
{
	std::cout << "Address Default constructor" << std::endl;
}
Address::Address(const Address& src) { *this = src; }
Address&	Address::operator=( Address const & src )
{
	(void) src;
	return (*this);
}
Address::~Address()
{
	std::cout << "Address Destructor" << std::endl;
	for (std::vector<Server*>::iterator it = _servers.begin();
		it != _servers.end(); ++it)
		delete (*it);
	if (_listen_socket > 0)
	{
		std::cout << "Close listen socket: " << _listen_socket << std::endl;
		close(_listen_socket);
	}
}
Address::Address(Host* host, std::string ip, short unsigned int p):
    _host(host),
    _ip_address(ip),
    _port(p)
{}

void    Address::push(Server* s) { _servers.push_back(s); }

int     Address::listen_socket(void)
{
	_listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_listen_socket < 0)
	{
		perror("listen socket: socket() failed");
		return (-1);
	}
	int    on = 1;
	if (setsockopt(_listen_socket, SOL_SOCKET,  SO_REUSEADDR,
                   (char *)&on, sizeof(on)) < 0)
	{
		perror("reusable socket: setsockopt() failed");
		return (-1);
	}
	fcntl(_listen_socket, F_SETFL, O_NONBLOCK);	// ioctl not allowed
	return (bind_addr());
}

int	    Address::bind_addr()
{
	struct sockaddr_in	addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(_port);
	addr.sin_addr.s_addr = inet_addr(_ip_address.c_str());
    //std::cout << _ip_address << ":" << _port << std::endl;
	if (bind(_listen_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		perror("bind() failed");
		return (-1);
	}
	if (listen(_listen_socket, _host->get_max_clients()) < 0)
	{
		perror("listen() failed");
		return (-1);
	}
	std::cout << "Listening at " << _ip_address << ":" << _port
		<< " (socket : " << _listen_socket << ")" << std::endl;
	return (_listen_socket);
}

//Accept all the new connections, create a new socket and add to the master set
void	Address::accept_client_sk(void)
{
	std::cout << "Listening socket is readable " << _listen_socket << std::endl;
	int	new_sk;
	do
	{
		new_sk = accept(_listen_socket, NULL, NULL);
		if (new_sk < 0)
		{
			//if (errno != EWOULDBLOCK)
			//	perror("accept() failed");
			break;
		}
		fcntl(new_sk, F_SETFL, O_NONBLOCK);
		std::cout << "  New incoming connection - " << new_sk << std::endl;
		_host->new_request_sk(new_sk, this);
	} while (new_sk != -1);
}

std::vector<Server*>    Address::get_servers(void) {return (_servers);}
//std::string		    Address::get_ip_address(void) const {return (_ip_address);}
//short unsigned int	Address::get_port(void) const {return (_port);}
