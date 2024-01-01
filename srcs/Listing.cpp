/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listing.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2023/12/31 15:25:29 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "webserv.hpp"

#include "Listing.hpp"

Listing::Listing()
{
	std::cout << "Listing Default constructor" << std::endl;
}
Listing::Listing(const Listing& src) { *this = src; }
Listing&	Listing::operator=( Listing const & src )
{
	(void) src;
	return (*this);
}
Listing::~Listing() {}

std::string Listing::get_html(Response* response)
{
    (void)response;
    return "Listing";
}
