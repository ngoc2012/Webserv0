/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/05 21:21:18 by ngoc              #+#    #+#             */
/*   Updated: 2023/11/26 16:19:25 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <signal.h>

#include "Host.hpp"
#include "Server.hpp"
#include "Configuration.hpp"

Host	*g_host;

void	main_signal_handler(int sig)
{
	if (sig == SIGINT)
	{
		write(STDOUT_FILENO, "", 0);
		//g_host->end();
	}
	if (sig == SIGPIPE)
	{
		/*
		std::map<int, Response*>	sk_response = g_host->get_sk_response();
		std::map<int, Request*>	sk_request = g_host->get_sk_request();
		for (std::map<int, Response*>::iterator	it = sk_response.begin();
			it != sk_response.end(); ++it)
			g_host->delete_response(it->first);
		for (std::map<int, Request*>::iterator	it = sk_request.begin();
			it != sk_request.end(); ++it)
			g_host->close_client_sk(it->first);
			*/
	}
}

int	main()
{
	struct sigaction	act;
	act.sa_flags = SA_RESTART;
	act.sa_handler = main_signal_handler;
	sigemptyset(&act.sa_mask);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGPIPE, &act, NULL);
	Host			host(".conf");
	g_host = &host;
	host.start();
	return (0);
}
