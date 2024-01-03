/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Header.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/01/01 15:50:24 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Host.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "webserv.hpp"

#include "Header.hpp"

Header::Header()
{
	std::cout << "Header Default constructor" << std::endl;
}
Header::Header(const Header& src) { *this = src; }
Header&	Header::operator=( Header const & src )
{
	(void) src;
	return (*this);
}
Header::Header(Response* r, std::string ext) :
	_response(r),
	_extension(ext)
{
    _host = _response->get_host();
    _mimes = _host->get_mimes();
    _status_message = _host->get_status_message();
	init();
	std::cout << "Header Constructor" << std::endl;
}
Header::~Header()
{
	std::cout << "Header Destruction" << std::endl;
}


std::string	Header::generate(void)
{
	std::string	str;

    _status_code = _response->get_status_code();
	str = "HTTP/1.1 ";
	str += ft::itos(_status_code) + " ";
	if (_status_message->find(_status_code) == _status_message->end())
		str += "Unknown error code";
	else
		str += (*_status_message)[_status_code];
	str += "\r\n";
    if (_status_code == 200)
    {
        str += "Allow: " + _allow + "\r\n";
        str += std::string("Content-Language: en") + "\r\n";
        if (_mimes->find(_extension) == _mimes->end())
            str += "Content-Type: text/html\r\n";
        else
            str += "Content-Type: " + (*_mimes)[_extension] + "\r\n";
    }
    else
        str += "Content-Type: text/html\r\n";
    str += "Content-Length: " + ft::itos(_response->get_content_length()) + "\r\n";
    str += "Date: " + get_current_time() + "\r\n\r\n";
	//std::cout << str << std::endl;
	return (str);
}

void	Header::init(void)
{
}
/*
std::string     Header::date() {
    struct timeval tv;
    char buf[32];
    gettimeofday(&tv, NULL);

    struct tm	*tm;
    tm = gmtime(&tv.tv_sec);
    int ret = strftime(buf, 32, "%a, %d %b %Y %T GMT", tm);
    return std::string(buf, ret);
}
*/
std::string	Header::get_current_time(void)
{
	std::time_t currentTime = std::time(0);
	std::tm* timeInfo = std::gmtime(&currentTime);
	char buffer[80];
	std::strftime(buffer, 80, "%a, %d %b %Y %H:%M:%S GMT", timeInfo);
	return (std::string(buffer));
}

std::string	Header::file_last_modified_time(std::string file_name)
{
	struct stat	file_info;
	if (stat(file_name.c_str(), &file_info) != 0) {
		std::cerr << "Error getting file information." << std::endl;
		return ("");
	}
	std::tm*	time_info = std::gmtime(&file_info.st_mtime);
	char		buffer[80];
	std::strftime(buffer, 80, "%a, %d %b %Y %H:%M:%S GMT", time_info);
	return (std::string(buffer));
}

bool	Header::parse_method_url(std::string& s, std::string& url, e_method& m)
{
    size_t  newline = s.find("\n");
    if (newline == NPOS)
        return (false);

    std::vector<std::string>	line0;
    line0 = ft::split_string(s.substr(0, newline), "     ");
    if (line0.size() != 3)
    {
        std::cerr << "Error: First line header invalid" << std::endl;
        return (false);
    }
    url = line0[1];
    if (line0[0] == "GET")
        m = GET;
    else if (line0[0] == "POST")
        m = POST;
    else if (line0[0] == "PUT")
        m = PUT;
    else if (line0[0] == "DELETE")
        m = DELETE;
    else
    {
        std::cerr << "Error: Method unknown : " << line0[0] << std::endl;
        return (false);
    }
    return (true);
}

bool	    Header::parse_content_type(Host* host, std::string &s, std::string& ct)
{
    size_t	pos = s.find("Content-Type:");
    if (pos == NPOS)
    {
        std::cerr << "Error: Content type not found." << std::endl;
        return (false);
    }
    std::string type = s.substr(pos + 14, 50);
    std::map<std::string, std::string>*	mimes = host->get_mimes();
    for (std::map<std::string, std::string>::iterator it = mimes->begin();
            it != mimes->end(); ++it)
        if (type.find(it->second) != NPOS)
        {
            ct = it->second;
            return (true);
        }
    std::cerr << "Error: Content type not found." << std::endl;
    return (false);
}

bool	Header::parse_content_length(std::string& s, size_t& cl)
{
	size_t	pos1;
	size_t	pos = s.find("Content-Length: ");
	if (pos != NPOS)
        pos1 = s.substr(pos).find("\n");
	if (pos == NPOS || pos1 == NPOS)
	{
        std::cerr << "Error: Content length not found." << std::endl;
        return (false);
	}
    pos += 16;
    cl = std::atoi(s.substr(pos, pos1).c_str());
    return (true);
}

void		Header::set_status_code(int s) {_status_code = s;}
void		Header::set_allow(std::string a) {_allow = a;}
