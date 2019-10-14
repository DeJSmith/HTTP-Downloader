#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "http.h"

#define BUF_SIZE 1024

Buffer *new_buffer(size_t size) {

  Buffer *new_buff = malloc(sizeof(Buffer));
  
  new_buff->data = malloc(size);
  new_buff->length = 0;
  memset(new_buff->data, 0, size);
  
  return new_buff;
  
}

int append_buffer(Buffer *buffer, char *data, size_t length, size_t reserved) {

	while (buffer->length + length > reserved){
		reserved += reserved;
		buffer->data = realloc(buffer->data, reserved);
		memset(buffer->data + buffer->length, 0, reserved - buffer->length);
	}	
  
    char* buffer_end = buffer->data + buffer->length;
    
    memcpy(buffer_end, data, length);
    buffer->length += length;

	return reserved;
}

void free_buffer(Buffer *buffer) {
    free(buffer->data);
    free(buffer);
}

Buffer* http_query(char *host, char *page, int port) {
/*
 *	
 *	
 * */

	struct hostent *server;
	struct sockaddr_in serv_addr;
	int sockfd, conn, recvd;
	Buffer *request_buffer = new_buffer(BUF_SIZE); 	//store get request 
    Buffer *content_buffer = new_buffer(BUF_SIZE); 	//store individual packet content
    Buffer *download = new_buffer(BUF_SIZE);		//store entrie request content

	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	
	sprintf(request_buffer->data, "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: getter\r\n\r\n", page, host);

	//create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) perror("could no create socket");

	//get server info
	server = gethostbyname(host);
    if(server == NULL) {
        perror("bad host");
        printf("unable to connect to host: %s, and collect page: %s", host, page);
        exit(EXIT_FAILURE);
    }
    
    memcpy(&serv_addr.sin_addr, server->h_addr_list[0], server->h_length);
	
    //connect to host
    conn = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if(conn < 0) perror("unable to connect to host");

    write(sockfd, request_buffer->data, strlen(request_buffer->data));

    int total_bytes = 0;

    size_t reserved = BUF_SIZE;
	
    while (1) {
		
        recvd = read(sockfd, content_buffer->data, BUF_SIZE);

        total_bytes += recvd;

        if(recvd < 0) perror("failed to read from socket");

        if(recvd == 0) break;

        reserved = append_buffer(download, content_buffer->data, recvd, reserved);
        memset(content_buffer->data, 0, BUF_SIZE);
    } 

    free_buffer(content_buffer);
    free_buffer(request_buffer);
    close(sockfd);
    return download;
    
}

// split http content from the response string
char* http_get_content(Buffer *response) {

    char* header_end = strstr(response->data, "\r\n\r\n");

    if (header_end) {
        return header_end + 4;
    }
    else {
        return response->data;
    }
}


Buffer *http_url(const char *url) {
    char host[BUF_SIZE];
    strncpy(host, url, BUF_SIZE);

    char *page = strstr(host, "/");
    if (page) {
        page[0] = '\0';

        ++page;
        return http_query(host, page, 80);
    }
    else {

        fprintf(stderr, "could not split url into host/page %s\n", url);
        return NULL;
    }
}
