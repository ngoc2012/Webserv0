/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2023/12/16 11:54:19 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>	// htons
#include <unistd.h>	// close
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>	// fcntl
#include <cstdio>	// perror
#include <cstring>	// memcpy

#include <iostream>
#include <vector>

#ifndef SERVER_HPP
# define SERVER_HPP

// Forward declaration
class	Host;
class	Location;

class	Server
{
	private:
		Host*			        _host;
		std::string		        _ip_address;
		short unsigned int	    _port;
		int			            _socket;
		std::string		        _server_name;
		std::string	        	_root;
		std::vector<Location*>	_locations;	

		int			            bind_addr(void);

		Server(const Server&);
		Server			&operator=(const Server& op);

	public:
		Server();
		virtual ~Server();

		void			accept_client_sk(void);
		int			server_socket(void);
		void			insert_location(Location* l);

		const char*		get_ip_address(void) const;
		short unsigned int	get_port(void) const;
		int			get_socket(void) const;
		Host*			get_host(void) const;
		std::string		get_root(void) const;
		std::string		get_server_name(void) const;
		std::vector<Location*>	get_locations(void) const;	

		void			set_socket(int);
		void			set_ip_address(std::string);
		void			set_port(short unsigned int);
		void			set_host(Host*);
		void			set_root(std::string);
		void			set_server_name(std::string);
};

#endif
