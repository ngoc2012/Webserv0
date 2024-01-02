# webserv

## Compile project 
```console
clear && make re && valgrind --track-fds=yes --leak-check=full --show-leak-kinds=all ./server
clear && make re && valgrind --track-fds=yes --leak-check=full --show-leak-kinds=all ./server && make clean
```
## README.md

[Example](https://github.com/github-linguist/linguist)

## Naming conventions

[Google rules](https://google.github.io/styleguide/cppguide.html)

- Class names, struct:  ThisIsMyClass
- Function names:       this\_is\_a\_function
- Constants, macros:    CONSTANTS\_MACRO
- Class attributes:     \_attributes\_of\_class

### Header files
Related header, C system headers, C++ standard library headers, other libraries' headers, your project's headers

Example header of a `fooserver.c`
```
#include "foo/server/fooserver.h"

#include <sys/types.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "foo/server/bar.h"
#include "third_party/absl/flags/flag.h"
```
### Vim (personal)

`qg` getter function declaration to function definition
`qs` setter function declaration to function definition
`qc` getter function definition to setter function definition
`qv` getter function declaration to setter function declaration
`qt` function definition to forward declaration
`qr` function definition to forward declaration
`qh` attribute declaration to getter declaration

So to save a macro `'x'`, you can do:

    From normal mode: `qx`
    enter whatever commands
    From normal mode: `q`
    open `.vimrc`
    insert a line `let @x = '...'` (see the following)
    For the above `...` pattern, you can use `"xp` or `Ctrl+r Ctrl+r registerName` just at the place where the pattern should be placed. But this is not essential, you can just type in the macro definition.

## Basic knowledge

[rfc9110](https://www.rfc-editor.org/rfc/rfc9110.html)

## Basics tests

### Autoindex:
```console
http://127.0.0.1:4141/index_files
```

### Send a file:
```console
curl -i -X PUT -H "Content-Type: image/jpeg" --data-binary "@test/port6.jpg" 127.0.0.1:4141/put_test/port6.jpg
```

### GET:
```console
curl -i -X GET -F "file=@.conf" 127.0.4.1:80
```

### How the netcat command `nc` works:

[How to Use The Netcat (nc) Command](https://nooblinux.com/how-to-use-netcat/)

[Example of using netcqt to send a file](https://stackoverflow.com/questions/4238809/example-of-multipart-form-data)

### Upload a file to test the webserv:

```console
curl -X POST -F "file=@yourfile.txt" http://localhost/upload
```

`-X`, `--request <method>`

(HTTP) Specifies a custom request method to use when communicating with the HTTP server.  The specified request method will be used instead of the method  otherwise  used  (which defaults  to  GET). Read the HTTP 1.1 specification for details and explanations. Common additional HTTP requests include PUT and DELETE, but related  technologies  like  WebDAV offers PROPFIND, COPY, MOVE and more.

Normally  you do not need this option. All sorts of GET, HEAD, POST and PUT requests are rather invoked by using dedicated command line options.  This option only changes the actual word used in the HTTP request, it does not alter the way  curl  behaves.  So  for example if you want to make a proper HEAD request, using -X HEAD will not suffice. You need to use the -I, --head option.

The method string you set with -X, --request will be used for all requests, which if you for  example  use  -L,  --location  may cause unintended side-effects when curl does not change request method according to the HTTP 30x response codes - and similar.

(FTP) Specifies a custom FTP command to use instead of LIST when doing file  lists  with FTP.

(POP3) Specifies a custom POP3 command to use instead of LIST or RETR.

(IMAP) Specifies a custom IMAP command to use instead of LIST. (Added in 7.30.0)

(SMTP) Specifies a custom SMTP command to use instead of HELP or VRFY. (Added in 7.34.0)

If this option is used several times, the last one will be used.

Examples:
 ``` console
 curl -X "DELETE" https://example.com
 curl -X NLST ftp://example.com/
 ```


## Some C++ notions

[Static keyword](https://stackoverflow.com/questions/15235526/the-static-keyword-and-its-various-uses-in-c)

## HTTP requests

HTTP requests typically include a Content-Type header that indicates the type of data in the request body. Here are some common Content-Type values:

* `application/json`: Used for JSON data.
* `application/xml`: Used for XML data.
* `application/x-www-form-urlencoded`: Used for form data submitted in the body of the request.
* `multipart/form-data`: Used when submitting forms that include binary data, like file uploads.
* `text/plain`: Used for plain text data.
* `application/octet-stream`: Used for binary data when the specific type is not known.

### multipart/form-data (not supported)

`multipart/form-data` is a MIME type used in HTTP requests to submit binary data, especially files, as part of a form submission. It is commonly used when uploading files through HTML forms. In a `multipart/form-data` request, the data is divided into multiple parts, and each part is identified by a boundary.

```css
POST /upload HTTP/1.1
Host: example.com
Content-Type: multipart/form-data; boundary=boundary123

--boundary123
Content-Disposition: form-data; name="field1"

value1
--boundary123
Content-Disposition: form-data; name="field2"

value2
--boundary123
Content-Disposition: form-data; name="file"; filename="example.txt"
Content-Type: text/plain

This is the content of the file.
It can span multiple lines.
--boundary123--
```

* Boundary:
    The boundary is a string that is used to separate different parts of the request.
    It is specified in the `Content-Type` header, for example: `Content-Type: multipart/form-data; boundary=boundary123`.
    The boundary should be unique and not appear in the data.

* Parts:
    Each part starts with the boundary preceded by two hyphens (`--boundary`).
    After each part, there is a set of headers (such as `Content-Disposition` and `Content-Type`) followed by the actual data.

* End of Parts:
    The entire request is terminated with a final boundary followed by two hyphens and no additional content (`--boundary--`).

* Content-Disposition:
        This header is used to describe the nature of the part. It typically includes information about the name of the form field and, in the case of file uploads, the filename.

## CGI

Non blocking CGI with fork:

```cpp
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    // ... other server initialization code ...

    while (true) {
        // ... handle incoming requests ...

        pid_t childPid = fork();

        if (childPid == 0) {
            // Child process: execute CGI script
            // ... execute CGI script code ...
            exit(EXIT_SUCCESS);
        } else if (childPid < 0) {
            // Handle fork error
            std::cerr << "Fork failed." << std::endl;
            exit(EXIT_FAILURE);
        } else {
            // Parent process: continue handling requests
            // ... handle other server tasks ...

            // Check for terminated child processes without blocking
            int status;
            pid_t terminatedChild = waitpid(-1, &status, WNOHANG);

            while (terminatedChild > 0) {
                // Process information about the terminated child
                // ... handle termination information ...

                // Check for more terminated child processes
                terminatedChild = waitpid(-1, &status, WNOHANG);
            }

            // ... continue handling requests ...
        }
    }

    return 0;
}
```

Handling Terminated Child Processes:
* If `waitpid` returns a positive value (process ID of the terminated child), the parent process enters the loop to handle the termination.
* The loop can process information about the terminated child, such as exit status or other details.
* The loop continues to check for more terminated child processes using `waitpid` with `WNOHANG` until no more terminated child processes are found.

### Configuration file

```nginx
location /cgi-bin/ {
    fastcgi_pass unix:/var/run/php-fpm.sock;
    include fastcgi_params;
    fastcgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;
}
location ~ \.cgi$ {
    # Set the CGI script execution handler
    fastcgi_pass unix:/var/run/php-fpm.sock; # Adjust this line based on your setup
    include fastcgi_params;
    fastcgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;
}
```
* The `location /cgi-bin/` block specifies that any request with a URI starting with "/cgi-bin/" should be processed as a CGI script.
* The `fastcgi_pass` directive points to the FastCGI server that will handle the execution of CGI scripts. In this case, it assumes a PHP-FPM server running on a Unix socket. Adjust it based on your CGI script interpreter (e.g., Python, Perl, etc.).
* The `include fastcgi_params` line includes common FastCGI parameters.
* The `fastcgi_param SCRIPT_FILENAME` line sets the script filename for the FastCGI server.
* The `location ~ \.cgi$` block uses a regular expression (`~`) to match URIs ending with ".cgi".
* `~`: Indicates that the following string is a case-sensitive regular expression.
* `\.`: Escapes the dot (.) character. In regular expressions, a dot is a special character that matches any character. By escaping it with a backslash, we specifically match a literal dot.
* `cgi`: Matches the characters "cgi" literally.
* `$`: Anchors the regex to the end of the URI. This ensures that the regex matches only if the URI ends with ".cgi."


### Methods

#### GET

* Must contain no body
According to the HTTP/1.1 specification (RFC 7231), a GET request is not expected to have a message body. The GET method is defined as a safe and idempotent method, and the use of a message body in a GET request is generally considered non-conforming.

The HTTP/1.1 specification (RFC 7231, Section 4.3.1) states:

    A payload within a GET request message has no defined semantics; sending a payload body on a GET request might cause some existing implementations to reject the request.
