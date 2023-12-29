/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/14 22:38:19 by ngoc              #+#    #+#             */
/*   Updated: 2023/12/27 15:30:40 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstdlib>
#include <sys/stat.h>	// stat, S_ISDIR
#include <sys/select.h>	// select

#include <iostream>
#include <vector>
#include <map>
#include <fstream>

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# define NPOS std::string::npos

enum	e_method {GET, POST, PUT, NONE};

namespace ft {

std::vector<std::string>	split_string(const std::string& input, std::string delimiters);
bool				        is_digit(std::string& str);
std::string			        itos(int n);
bool                        match_wildcard(const char* str, const char* pattern);
std::string	                file_extension(std::string);
size_t                      atoi_base(const char *str, const char *base);
std::string                 itoa_base(int nbr, const char *base);
std::string                 str_replace(std::string src, std::string find, std::string rep);
std::string                 to_upper(std::string s);

}

#endif
