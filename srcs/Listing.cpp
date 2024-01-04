/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listing.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/01/02 16:44:08 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include "Response.hpp"
#include "webserv.hpp"
#include <dirent.h>

#include "Listing.hpp"

Listing::Listing() {
	std::cout << "Listing Default constructor" << std::endl;
}

Listing::Listing(const Listing& src) { 
	*this = src;
}

Listing&	Listing::operator=( Listing const & src ){
	(void) src;
	return (*this);
}

Listing::~Listing() {

}

std::string Listing::get_listing(const std::string& directory_name) {
    std::string listing_html = "<!DOCTYPE html>\n<html>\n    <head>\n        <title>Listing</title>\n    </head>\n    <body>\n";
    const char* directory_path = directory_name.c_str();
    DIR* directory = opendir(directory_path);
    if (directory != nullptr) {
        listing_html += "        <h2>Contenu du repertoire : " + directory_name + "</h2>\n";
        listing_html += "        <ul>\n";
        struct dirent* entry;
        while ((entry = readdir(directory)) != nullptr) {
            if (entry->d_type == DT_REG) {
                // Fichier régulier
                listing_html += "<li>";
                listing_html += "        <a href='/index_files/";
                listing_html += entry->d_name;
                listing_html += "'>";
                listing_html += entry->d_name;
                listing_html += "</a>";
                listing_html += "</li>\n";
            } else if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                // Dossier (à l'exception des dossiers '.' et '..')
                listing_html += "<li>";
                listing_html += "        <a href='/index_files/";
                listing_html += entry->d_name;
                listing_html += "'>";
                listing_html += entry->d_name;
                listing_html += "/</a>";
                listing_html += "</li>\n";
            }
        }
        listing_html += "        </ul>\n";
        closedir(directory);
    } else {
        listing_html += "Impossible d'ouvrir le répertoire.";
    }
    listing_html += "    </body>\n</html>";
    return listing_html;
}

std::string Listing::get_html(Response* response) {
	(void)response;
	Request* request = response->get_request();
	std::string folder_name = request->get_full_file_name();
	std::string listing = get_listing(folder_name);
	return listing;
}

