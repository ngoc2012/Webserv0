/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/01/04 21:59:44 by ngoc             ###   ########.fr       */
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
		Host*			            _host;
		std::string		            _ip_address;
		short unsigned int	        _port;
		std::string		            _address;
		int			                _socket;
        std::vector<std::string>	_server_names;
		std::string	        	    _root;
		std::vector<Location*>	    _locations;	

		Server(const Server&);
		Server			&operator=(const Server& op);
		Server();
	public:
		Server(Host*);
		virtual ~Server();

		void			        insert_location(Location* l);

		const char*		            get_ip_address(void) const;
		short unsigned int	        get_port(void) const;
		int			                get_socket(void) const;
		Host*			            get_host(void) const;
		std::string		            get_root(void) const;
        std::vector<std::string>    get_server_names(void) const;
		std::vector<Location*>	    get_locations(void) const;	
		std::string		            get_address(void) const;

		void			set_socket(int);
		void			set_ip_address(std::string);
		void			set_address(std::string);
		void			set_port(short unsigned int);
		void			set_host(Host*);
		void			set_root(std::string);
		void			set_server_name(std::string);
};

#endif
