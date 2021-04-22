/*
	http-client-c
	Copyright (C) 2012-2013  Swen Kooij

	This file is part of http-client-c.

    http-client-c is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    http-client-c is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with http-client-c. If not, see <http://www.gnu.org/licenses/>.

	Warning:
	This library does not tend to work that stable nor does it fully implent the
	standards described by IETF. For more information on the precise implentation of the
	Hyper Text Transfer Protocol:

	http://www.ietf.org/rfc/rfc2616.txt
*/

#pragma GCC diagnostic ignored "-Wwrite-strings"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <stdio.h>
	#pragma comment(lib, "Ws2_32.lib")
#elif _LINUX
	#include <sys/socket.h>
#elif __FreeBSD__
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <arpa/inet.h>
#else
/*error Platform not suppoted.*/
#endif

#include <errno.h>
#include "stringx.h"
#include "urlparser.h"

/*
	Prototype functions
*/
struct http_response* http_req(char *http_headers, struct parsed_url *purl);
struct http_response* http_get(char *url, char *custom_headers);
struct http_response* http_head(char *url, char *custom_headers);
struct http_response* http_post(char *url, char *custom_headers, char *post_data);


void http_response_free(struct http_response *hresp);

/*
	Represents an HTTP html response
*/
struct http_response
{
	struct parsed_url *request_uri;
	char *body;
	char *status_code;
	int status_code_int;
	char *status_text;
	char *request_headers;
	char *response_headers;
};

/*
	Handles redirect if needed for get requests
*/
struct http_response* handle_redirect_get(struct http_response* hresp, char* custom_headers)
{
	if(hresp->status_code_int > 300 && hresp->status_code_int < 399)
	{
		char *token = strtok(hresp->response_headers, "\r\n");
		while(token != NULL)
		{
			if(str_contains(token, "Location:"))
			{
				/* Extract url */
				char *location = str_replace("Location: ", "", token);
				return http_get(location, custom_headers);
			}
			token = strtok(NULL, "\r\n");
		}
	}
	else
	{
		/* We're not dealing with a redirect, just return the same structure */
		return hresp;
	}
}


int create_connection(struct parsed_url *purl)
{
	int sock = 0;
	int tmpres = 0;
	struct sockaddr_in *remote;
	
	/* Create TCP socket */
	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
	    printf("Can't create TCP socket");
		return 0;
	}

	/* Set remote->sin_addr.s_addr */
	remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
	remote->sin_family = AF_INET;
  	tmpres = inet_pton(AF_INET, purl->ip, (void *)(&(remote->sin_addr.s_addr)));
  	if( tmpres < 0)
  	{
	    	printf("Can't set remote->sin_addr.s_addr");
	    	return 0;
  	}
	else if(tmpres == 0)
  	{
		printf("Not a valid IP");
    		return 0;
  	}
	remote->sin_port = htons(atoi(purl->port));

	/* Connect */
	if(connect(sock, (struct sockaddr *)remote, sizeof(struct sockaddr)) < 0)
	{
		printf("Could not connect");
		return 0;
	}
	return sock;
}

char* get_header_value(char* headers, char* key)
{
	char* key_pos = strstr(headers, key);
	if(NULL == key_pos)
	{
		return NULL;
	}
	char *val_pos = index(key_pos, ':') + 2; /* 2 for the [: ]*/
	
	return val_pos;
}

struct http_response* process_http_headers(char* response)
{
	struct http_response *hresp = (struct http_response*)malloc(sizeof(struct http_response));
	/* Allocate memeory for response from server */
	
	if(hresp == NULL)
	{
		printf("Unable to allocate memory for htmlcontent.");
		return NULL;
	}
	
	hresp->body = NULL;
	hresp->request_headers = NULL;
	hresp->response_headers = NULL;
	hresp->status_code = NULL;
	hresp->status_text = NULL;
	
	/* Parse status code and text */
	char *status_line = get_until(response, "\r\n");
	status_line = str_replace("HTTP/1.1 ", "", status_line);
	char *status_code = str_ndup(status_line, 4);
	status_code = str_replace(" ", "", status_code);
	char *status_text = str_replace(status_code, "", status_line);
	status_text = str_replace(" ", "", status_text);
	hresp->status_code = status_code;
	hresp->status_code_int = atoi(status_code);
	hresp->status_text = status_text;

	/* Parse response headers */
	char *headers = get_until(response, "\r\n\r\n");
	hresp->response_headers = headers;

	/* Assign request url */

	return hresp;
}

void process_buffer(char* buffer, long int recived_len, int is_first_buffer)
{
	fprintf(stderr, "process buffer: is_first_buffer: %d\n", is_first_buffer);
	struct http_response* hresp = NULL;
	int is_chunked = 0;
		
	int written_bytes = 0;
	buffer[recived_len] = '\0';
	
	if(is_first_buffer) /* process HTTP headers */ 
	{
		hresp = process_http_headers(buffer);
		char* transfer_encoding = NULL;
		if(transfer_encoding = get_header_value(buffer, "Transfer-Encoding"))
		{
			fprintf(stderr, "DEBUG: Transfer-Encoding exists !!");
			char* end = index(transfer_encoding, '\r');
			fprintf(stderr, "DEBUG: Length of value: %d\n", (int)(end - transfer_encoding));
		}
	}	
	while(written_bytes < recived_len)
	{
		/*written_bytes += write(1, (buffer + written_bytes), (recived_len - written_bytes));*/
	}
	fprintf(stderr, "Read: %li\n", recived_len);
}


struct http_response* http_req(char *http_headers, struct parsed_url *purl)
{
	int tmpres = 0;
	int sock = 0;
	/* DEBUG: PRINT HEADERS */
	fprintf(stderr, "HEADERS: %s\n", http_headers);
	fprintf(stderr, "---------\n");
	/* END DEBUG */
	
	/* Parse url */
	if(purl == NULL)
	{
		printf("Unable to parse url");
		return NULL;
	}
	

	if(!(sock = create_connection(purl))) 
	{
		printf("Creating connection failed !\n");
		return NULL;
	}
	
	/* Send headers to server */
	int sent = 0;
	while(sent < strlen(http_headers))
	{
	     tmpres = send(sock, http_headers + sent, strlen(http_headers) - sent, 0);
		if(tmpres == -1)
		{
			printf("Can't send headers");
			return NULL;
		}
		sent += tmpres;
	 }
	
	
	/* Recieve into response*/
	char *response = (char*)malloc(0);
	char buffer[BUFSIZ];
	size_t recived_len = 0;
	int is_first_buffer = 1;
	
	while((recived_len = recv(sock, buffer, BUFSIZ-1, 0)) > 0)
	{	
		process_buffer(buffer, recived_len, is_first_buffer);
		is_first_buffer = 0;
	}
	if (recived_len < 0)
    	{
		free(http_headers);
		#ifdef _WIN32
			closesocket(sock);
		#else
			close(sock);
		#endif
	   printf("Unabel to recieve");
		return NULL;
    	}

	/* Reallocate response */
	response = (char*)realloc(response, strlen(response) + 1);

	/* Close socket */
	#ifdef _WIN32
		closesocket(sock);
	#else
		close(sock);
	#endif

	

	/* Return response */

}

/*
	Makes a HTTP GET request to the given url
*/
struct http_response* http_get(char *url, char *custom_headers)
{
	/* Parse url */
	struct parsed_url *purl = parse_url(url);
	
	/* DEBUG */
	
	fprintf(stderr, "host: %s\nip: %s\nport: %s\npath: %s\n", purl->host, purl->ip, purl->port, purl->path);
	
	/* END DEBUG*/
	if(purl == NULL)
	{
		printf("Unable to parse url");
		return NULL;
	}

	/* Declare variable */
	char *http_headers = (char*)malloc(1024);

	/* Build query/headers */
	if(purl->path != NULL)
	{
		if(purl->query != NULL)
		{
			sprintf(http_headers, "GET /%s?%s HTTP/1.1\r\nHost:%s\r\nConnection:keep-alive\r\n", purl->path, purl->query, purl->host);
		}
		else
		{
			sprintf(http_headers, "GET /%s HTTP/1.1\r\nHost:%s\r\nConnection:keep-alive\r\n", purl->path, purl->host);
		}
	}
	else
	{
		if(purl->query != NULL)
		{
			sprintf(http_headers, "GET /?%s HTTP/1.1\r\nHost:%s\r\nConnection:keep-alive\r\n", purl->query, purl->host);
		}
		else
		{
			sprintf(http_headers, "GET / HTTP/1.1\r\nHost:%s\r\nConnection:keep-alive\r\n", purl->host); /*Connection:close\r\n*/
		}
	}


	/* Add custom headers, and close */
	if(custom_headers != NULL)
	{
		sprintf(http_headers, "%s%s\r\n", http_headers, custom_headers);
	}
	else
	{
		sprintf(http_headers, "%s\r\n", http_headers);
	}
	http_headers = (char*)realloc(http_headers, strlen(http_headers) + 1);

	/* Make request and return response */
	struct http_response *hresp = http_req(http_headers, purl);

	/* Handle redirect */
	return handle_redirect_get(hresp, custom_headers);
}


/*
	Free memory of http_response
*/
void http_response_free(struct http_response *hresp)
{
	if(hresp != NULL)
	{
		if(hresp->request_uri != NULL) parsed_url_free(hresp->request_uri);
		if(hresp->body != NULL) free(hresp->body);
		if(hresp->status_code != NULL) free(hresp->status_code);
		if(hresp->status_text != NULL) free(hresp->status_text);
		if(hresp->request_headers != NULL) free(hresp->request_headers);
		if(hresp->response_headers != NULL) free(hresp->response_headers);
		free(hresp);
	}
}
