/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Host.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/01/05 13:21:53 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <set>

#include "webserv.hpp"

#ifndef HOST_HPP
# define HOST_HPP

// Forward declaration
class	Address;
class	Server;
class	Request;
class	Response;
class	Configuration;

//https://www.ibm.com/docs/en/ztpf/2020?topic=overview-blocking-nonblocking
class	Host
{
	private:

		int				                        _max_clients;
		size_t				                    _client_max_body_size;
		size_t				                    _client_body_buffer_size;

		bool				                    _parser_error;

		int				                        _sk_ready;
		int				                        _max_sk;		    // Max of all fd
		fd_set              		            _master_read_set;	// Set of all read fd
		fd_set              		            _master_write_set;	// Set of all read fd
		fd_set              		            _read_set;		    // Set of active read fd
		fd_set              		            _write_set;		    // Set of active write fd
		fd_set              		            _listen_set;
		std::map<std::string, Address*>		    _str_address;
		std::map<int, Address*>		            _sk_address;
		std::map<int, Request*>	                _sk_request;
		std::map<int, std::string>  		    _status_message;
		std::map<std::string, std::string>	    _mimes;
		std::set<std::string>	                _set_mimes;

		bool				                    select_available_sk(void);
		void  				                    add_sk_2_master_read_set(int, Address*);
		void    			                    start_server(void);
		void    			                    check_sk_ready(void);
		bool				                    check_servers_conf(void);
		void				                    mimes(void);
		void				                    status_message(void);

		Host(const Host&);
		Host &operator=(const Host& op);

	public:
		Host();
		virtual ~Host();

		void    		                    start(void);
		void			                    new_request_sk(int, Address*);
		void			                    new_response_sk(int);
		void			                    close_client_sk(int);

		int				                    get_max_clients(void) const;
		std::map<int, Server*>		        get_sk_server(void) const;
		std::map<int, Request*>	            get_sk_request(void) const;
		std::map<int, Response*>	        get_sk_response(void) const;
		size_t				                get_client_max_body_size(void) const;
		size_t				                get_client_body_buffer_size(void) const;
        std::map<std::string, std::string>*	get_mimes(void);
        std::set<std::string>*	            get_set_mimes(void);
        std::map<int, std::string>*  		get_status_message(void);

		void	set_client_max_body_size(size_t);
		void	set_client_body_buffer_size(size_t);
		void	set_parser_error(bool);
        void    set_servers(std::vector<Server*>);
        void    set_str_address(std::map<std::string, Address*>);
};

#endif
