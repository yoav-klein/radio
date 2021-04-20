
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "http-client-c.h"
#include <stdio.h>




int main(int argc, char** argv)
{
	struct http_response* resp;
	if(argv[1] == NULL)
	{
		printf("Enter URL\n");
		exit(0);
	}
	resp = http_get(argv[1], NULL);
	
	fprintf(stderr, "Response headers: \n%s\n\n", resp->response_headers);
	fprintf(stderr, "---- print reponse body--------\n");
	printf("%s", resp->body);
	
	return 0;
}
