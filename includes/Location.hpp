/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2023/12/27 12:47:57 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <vector>

#include "webserv.hpp"

#ifndef LOCATION_HPP
# define LOCATION_HPP

class	Location
{
	private:
		std::string			        _url;
		std::vector<e_method>	    _methods;
		std::string			        _alias;
		std::string			        _cgi_pass;


		Location();
		Location(const Location&);
		Location			&operator=(const Location& op);
	public:
		Location(std::string);
		virtual ~Location();

        static Location*	        find_location(std::string, std::vector<Location*>, e_method, int &);
		bool	                    compare_url(std::string, std::string);
		bool		                find_method(e_method);
        std::string	                get_full_file_name(std::string, std::string);
		std::string                 get_methods_str(void);
        static std::string	        get_method_str(e_method);

		std::vector<e_method>		get_methods(void) const;
		std::string			        get_alias(void) const;
		std::string			        get_url(void) const;
        std::string			        get_cgi_pass(void) const;

		void				        insert_methods(e_method);
		void				        set_alias(std::string);
		void				        set_url(std::string);
        void			            set_cgi_pass(std::string);
};

#endif
