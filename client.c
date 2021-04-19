
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "http-client-c.h"
#include <stdio.h>




int main()
{

	printf("Here\n");
	struct http_response* resp;
	
	resp = http_get("http://www.google.com", NULL);
	printf("headers: %s\n", resp->response_headers);
	printf("body: %s\n", resp->body);
	
	return 0;
}
