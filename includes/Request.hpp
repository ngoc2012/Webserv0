/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2023/12/28 11:13:12 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>

#include "webserv.hpp"
#include "Response.hpp"
#include "Cgi.hpp"

#ifndef CLIENTREQUEST_HPP
# define CLIENTREQUEST_HPP
 
# define MEGABYTE 1048576
# define KILOBYTE 1024

class	Host;
enum    e_method;

class	Request
{
	private:
		int		        _socket;
		Host*		    _host;
		Server*		    _server;
		Response	    _response;
		Location*	    _location;	
        Cgi             _cgi;


		std::string	    _header;
		std::string	    _url;
		e_method	    _method;
		std::string	    _content_type;
		size_t		    _content_length;
		size_t		    _body_size;
		size_t		    _body_position;

		int		        _fd_in;
		std::string	    _full_file_name;
		size_t		    _body_buffer;
		size_t		    _body_max;
        char*   	    _buffer;
        bool            _read_queue;
        std::string     _tmp_file;

		int		        _status_code;

		void		    read_header(void);
		bool		    parser_header(void);
		bool		    read_method_url(void);
		bool		    read_content_type(void);
		bool		    read_content_length(void);
        bool	        check_location(void);

		void		    process_fd_in(void);
        void	        read_body();

		int             end_read(void);
        void            envs(void);
        void            exec(void);

		Request();
		Request(const Request&);
		Request &operator=(const Request& op);
	public:
		Request(int, Host*, Server*);
		virtual ~Request();

		int             read(void);

        Host*		    get_host(void) const;
        Server*		    get_server(void) const;
		e_method	    get_method(void) const;
		std::string	    get_url(void) const;
		Response*	    get_response(void);
        const Cgi*      get_cgi(void) const;
		int		        get_status_code(void) const;
        std::string	    get_content_type(void) const;
        size_t		    get_content_length(void) const;
        size_t		    get_body_size(void) const;
        std::string	    get_header(void) const;
        std::string	    get_full_file_name(void) const;
        Location*	    get_location(void) const;	
        int		        get_fd_in(void) const;

        void		    set_fd_in(int);
};

#endif
