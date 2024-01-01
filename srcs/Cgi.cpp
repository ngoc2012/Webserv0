/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2023/12/31 09:51:22 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstring>
#include <unistd.h>

#include "Host.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "webserv.hpp"

#include "Cgi.hpp"

#define BUFFER_SIZE 65792

Cgi::Cgi()
{
	std::cout << "Cgi Default constructor" << std::endl;
    _envs = 0;
    _pid = -1;
}
Cgi::Cgi(const Cgi& src) { *this = src; }
Cgi&	Cgi::operator=( Cgi const & src )
{
	(void) src;
	return (*this);
}
Cgi::~Cgi()
{
    if (_envs)
        delete[] _envs;
}

void    Cgi::execute()
{
    Response*       response;

    response = _request->get_response();
    if (!get_envs())
    {
        response->set_status_code(500);
        return ;
    }

    std::cout << "Cgi execute" << std::endl;

    int pipe_in[2];
    int pipe_out[2];
    
    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1) {
        std::cerr << "Error: pipe" << std::endl;
        perror("pipe");
        return ;
    }

    _pid = fork();

    if (_pid == -1)
    {
        std::cerr << "Error: fork" << std::endl;
        return ;
    }
    else if (!_pid)
    {
        close(pipe_in[1]);
        close(pipe_out[0]);
        dup2(pipe_in[0], STDIN_FILENO);
        dup2(pipe_out[1], STDOUT_FILENO);
        close(pipe_in[0]);
        close(pipe_out[1]);
        char*   argv[3];
        argv[0] = (char*) _pass.c_str();
        argv[1] = (char*) _file.c_str();
        argv[2] = 0;
        execve(argv[0], argv, _envs);
    }
    else
    {
        close(pipe_in[0]);
        close(pipe_out[1]);
        int     fd_in = _request->get_fd_in();
        if (lseek(fd_in, 0, SEEK_SET) == -1) {
            std::cerr << "Error: using lseek" << std::endl;
            return ;
        }
        char    buffer[BUFFER_SIZE];
        size_t  bytesRead;
        while ((bytesRead = read(fd_in, buffer, BUFFER_SIZE)) > 0)
            write(pipe_in[1], buffer, bytesRead);
        _request->get_response()->set_fd_out(pipe_out[0]);
    }
}

bool    Cgi::get_envs()
{
    std::vector<std::string>  envs;

    if (_request->get_method() == POST) {
        envs.push_back("CONTENT_TYPE=" + _request->get_content_type());
        envs.push_back("CONTENT_LENGTH=" + ft::itos((int) _request->get_content_length()));
    }
    envs.push_back("GATEWAY_INTERFACE=CGI/1.1");
    envs.push_back("PATH_INFO=" + _file);
    envs.push_back("PATH_TRANSLATED=" + _file);
    envs.push_back("QUERY_STRING=");
    envs.push_back("REMOTE_ADDR=");
    //envs["QUERY_STRING" + config_.getQuery();
    //envs["REMOTE_ADDR" + config_.getClient().getAddr();

    //if (config_.getAuth() != "off") {
        envs.push_back("AUTH_TYPE=Basic");
        envs.push_back("REMOTE_IDENT=");
        envs.push_back("REMOTE_USER=");
        //envs["REMOTE_IDENT" + config_.getAuth().substr(0, config_.getAuth().find(':'));
        //envs["REMOTE_USER" + config_.getAuth().substr(0, config_.getAuth().find(':'));
    //}

    envs.push_back("REQUEST_METHOD=" + Location::get_method_str(_request->get_method()));
    envs.push_back("REQUEST_URI=" + _request->get_url());

    Server*   server = _request->get_server();
    envs.push_back("SCRIPT_NAME=" + _file);
    envs.push_back("SERVER_NAME=" + server->get_server_name());
    envs.push_back("SERVER_PROTOCOL=");
    envs.push_back("SERVER_PORT=" + ft::itos((int) server->get_port()));
    envs.push_back("SERVER_SOFTWARE=WEBSERV/1.0");

    std::string extension = ft::file_extension(_request->get_full_file_name());
    if (extension == "php")
        envs.push_back("REDIRECT_STATUS=200");

    std::vector<std::string> header_lines = ft::split_string(_request->get_header(), "\n");
    size_t          i;
    for (std::vector<std::string>::iterator it = header_lines.begin();
            it != header_lines.end(); it++)
    {
        i = it->find(":");
        if (i != NPOS)
            envs.push_back(ft::str_replace(
                    "HTTP_" + ft::to_upper(it->substr(0, i)), "-", "_")
                    + "=" + it->substr(i + 2));
    }

    if (!(_envs = new char*[envs.size() + 1]))
        return false;

    i = 0;
    for (std::vector<std::string>::iterator it = envs.begin();
            it != envs.end(); it++)
    {
        std::cout << *it << std::endl;
        _envs[i++] = (char*) it->c_str();
    }
    _envs[i] = 0;
    return (true);
}

int         Cgi::get_pid(void) const {return (_pid);}

void        Cgi::set_request(Request* r) {_request = r;}
