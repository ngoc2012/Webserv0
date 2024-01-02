/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Configuration.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/01/02 11:03:25 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Configuration.hpp"
#include "Host.hpp"
#include "Server.hpp"
#include "Location.hpp"

Configuration::Configuration() {}
Configuration::Configuration(const Configuration& src) { *this = src; }
Configuration&	Configuration::operator=( Configuration const & src )
{
	(void) src;
	return (*this);
}
Configuration::~Configuration() {}
std::string	Configuration::remove_comments(std::string& s)
{
	size_t	hash_pos = s.find("#");
	if (hash_pos == std::string::npos)
		return (s);
	return (s.substr(0, hash_pos));
}
std::string	Configuration::remove_spaces_end(std::string& s)
{
	size_t	n = s.length();
	while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '	'))
		n--;
	return (s.substr(0, n));
}

bool	Configuration::host_parser(std::string cmd, Host* host, std::vector<std::string>& words)
{
	int	n;

	if (cmd[0] != '	')
		return (true);
	if (words[0] == "client_max_body_size")
	{
		n = std::atoi(words[1].c_str());
		if (!ft::is_digit(words[1]) || n < 0 || n > 100)
			return (true);
		host->set_client_max_body_size(n);
	}
	else if (words[0] == "client_body_buffer_size")
	{
		n = std::atoi(words[1].c_str());
		if (!ft::is_digit(words[1]) || n < 0 || n > 1024)
			return (true);
		host->set_client_body_buffer_size(n);
	}
	else
		return (true);
	return (false);
}

bool	Configuration::server_parser(std::string cmd, Server* server, std::vector<std::string>&words)
{
	if (cmd[0] != '	')
		return (true);
	if (words[0] == "listen")
	{
		if (!listen(server, words))
			return (true);
	}
	else if (words[0] == "server_name")
		server->set_server_name(words[1]);
	else if (words[0] == "root")
	{
		struct stat	info;
		if (!(stat(words[1].c_str(), &info) == 0 && S_ISDIR(info.st_mode)))
		{
			std::cerr << "Error: folder not found" << std::endl;
			return (true);
		}
		server->set_root(words[1]);
	}
	else
		return (true);
	return (false);
}

bool	Configuration::location_parser(std::string cmd, Location* loc, std::vector<std::string>& words)
{
	if (cmd.substr(0, 2) != "		")
    {
        std::cerr << "Error: Indentation error" << std::endl;
		return (true);
    }
	if (words[0] == "methods")
		for (std::vector<std::string>::iterator it = words.begin() + 1;
				it != words.end(); ++it)
		{
			if (*it == "GET")
				loc->insert_methods(GET);
			else if (*it == "PUT")
				loc->insert_methods(PUT);
			else if (*it == "POST")
				loc->insert_methods(POST);
			else if (*it == "DELETE")
				loc->insert_methods(DELETE);
			else
			{
				std::cerr << "Error: Method unknown : " << *it << std::endl;
				return (true);
			}
		}
	else if (words[0] == "index")
	{
		for (std::vector<std::string>::iterator it = words.begin() + 1;
				it != words.end(); ++it)
            loc->push_back_index(*it);
    }
	else if (words[0] == "autoindex")
	{
        if (words.size() != 2 || (words[1] != "on" && words[1] != "off"))
        {
			std::cerr << "Error: alias folder not found" << std::endl;
			return (true);
        }
        if (words[1] == "on")
            loc->set_autoindex(true);
    }
	else if (words[0] == "alias")
	{
		struct stat	info;
		if (!(stat(words[1].c_str(), &info) == 0 && S_ISDIR(info.st_mode)))
		{
			std::cerr << "Error: alias folder not found" << std::endl;
			return (true);
		}
		loc->set_alias(words[1]);
	}
	else if (words[0] == "cgi_pass")
	{
		struct stat	info;
		if (stat(words[1].c_str(), &info) == -1 || S_ISDIR(info.st_mode))
		{
			std::cerr << "Error: cgi pass not found" << std::endl;
			return (true);
		}
		loc->set_cgi_pass(words[1]);
	}
	else
		return (true);
	return (false);
}

Configuration::Configuration(std::vector<Server*>& servers, Host* host, const char* conf)
{
	//const char*	keys_server[] = {"listen", "server_name", "location"};
	//const char*	keys_location[] = {"methods", "client_max_body_size", "client_body_buffer_size", "fastcgi_pass", "fastcgi_param", "include"};
	std::ifstream	conf_file(conf);
	if (!conf_file.is_open()) {
		std::cerr << "Error opening the file." << std::endl;
		return ;
	}
	enum e_part {LOCATION, HOST, SERVER, P_NONE};
	e_part	part = P_NONE;
	Server		*new_server = 0;
	Location	*new_location = 0;
	int		i = 0;
	std::string	line;
	bool	err = false;
	while (std::getline(conf_file, line))
	{
		std::string		s = remove_comments(line);
		//s = remove_spaces_end(s);
		//std::cout << "|" << s << "|" << std::endl;
		std::vector<std::string>	words = ft::split_string(s, std::string(" 	"));
		i++;
		if (words.size() == 0)
			;
		else if (s[0] == 'h' && words[0] == "host")
			part = HOST;
		else if (s[0] == 's' && words[0] == "server")
		{
			part = SERVER;
			new_server = new Server();
			servers.push_back(new_server);
		}
		else if (s[0] == '	' && words[0] == "location")
		{
			if (part != SERVER && part != LOCATION)
				err = true;
			else
			{
				part = LOCATION;
				new_location = new Location(words[1]);
				new_server->insert_location(new_location);
			}
		}
		else
			switch (part)
			{
				case P_NONE:
					err = true;
					break ;
				case LOCATION:
					err = location_parser(s, new_location, words);
					break;
				case SERVER:
					err = server_parser(s, new_server, words);
					break;
				case HOST:
					err = host_parser(s, host, words);
					break;
			}
		if (err)
		{
			conf_file_error(s, i);
			break ;
		}
	}
	host->set_parser_error(err);
	conf_file.close();
}

bool	Configuration::listen(Server* s, std::vector<std::string> words)
{
	if (words.size() != 2)
		return (false);
	std::vector<std::string>	address = ft::split_string(words[1], std::string(":"));
	if (address.size() != 2)
		return (false);
	std::vector<std::string>	ip = ft::split_string(address[0], std::string("."));
	if (ip.size() != 4)
		return (false);
	int	n;
	for (int j = 0; j < 4; j++)
	{
		n = std::atoi(ip[j].c_str());
		if (!ft::is_digit(ip[j]) || n < 0 || n > 255)
			return (false);
	}
	n = std::atoi(address[1].c_str());
	if (!ft::is_digit(address[1]) || n < 0 || n > 65535)
		return (false);
	s->set_ip_address(address[0]);
	s->set_port(std::atoi(address[1].c_str()));
	//std::cout << s->get_ip_address() << ":" << s->get_port() << std::endl;
	return (true);
}

void	Configuration::conf_file_error(std::string line, int i)
{
	std::cerr << "Configuration file error at line "
		<< i << " :" << line << std::endl;
}
