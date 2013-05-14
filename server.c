#include <pthread.h>
#include <arpa/inet.h> // inet_ntoa - ip to string
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "config.h"

unsigned int port = HTTP_PORT;

/*
 * Construct HTTP Headers
 */
char *constructHTTPHeader(char *status, char *type, int len) {
	if (status == NULL)
		status = "200 OK";
	if (type == NULL)
		type = "html";
	if (len < 0)
		len = 0;

	char *e = malloc(512);
	memset(e, 0, 512); // zero memory

	sprintf(e, "HTTP/1.1 %s\r\n", status);
	sprintf(e + strlen(e), "Content-Type: text/%s\r\n", type);
	sprintf(e + strlen(e), "Content-Length: %d\r\n\r\n", len); // send blank line to mark end of headers

	return e;
}

/*
 * Sends a packet back to the client
 */
void sendPacket(int sockfd, char *message, int packetLen) {
	int bytes_sent;
	if ((bytes_sent = send(sockfd, message, packetLen, 0)) == -1) {
		perror("send");
	}
	printf("\tSent packet [%d bytes]\n", bytes_sent);
}

/*
 * Send a given file complete with HTTP headers
 */
int sendFile(int c_sock, char *filename, char *httpCode) {
	FILE *fp = fopen(filename, "r");
	if (fp != NULL) {
		if (fseek(fp, 0L, SEEK_END) == 0) {
			long fsize = ftell(fp);
			fseek(fp, 0L, SEEK_SET);

			char *header = constructHTTPHeader(httpCode, get_filename_ext(filename), fsize);
			sendPacket(c_sock, header, strlen(header));

    	    		char *buf = malloc(sizeof(char) * 512); // 512
			size_t len = 512;
			while (len == 512) {
				len = fread(buf, sizeof(char), 512, fp);
				sendPacket(c_sock, buf, strlen(buf));
			}
			free(header); free(buf);
			fclose(fp);
			return 0;
		}
		fclose(fp);
		return 1;
	}
	else {
		return 2;
	}
}

/*
 * Send the requested file back to the client
 */
int getFile(int c_sock, char client_msg[MAX_LEN]) {
	char *pch = strtok(client_msg, " ");
	int i = 0; while (i++ != 1) pch = strtok(NULL, " ");
	char *filename = pch;

	if (filename[0] != '/') {
		sendFile(c_sock, "400.html", "400 Bad Request");
		return -1;
	}

	if (strcmp(filename, "/") == 0) 
		filename = "index.html";
	if (filename[0] == '/') // remove the / from the filename
		memmove(filename, filename+1, strlen(filename));

	int error;
	if ((error = sendFile(c_sock, filename, "200 OK")) == 2) { 
		// File Not Found
		sendFile(c_sock, "404.html", "404 Not Found");
		return 2;
	}

	return 0;
}

int putFile(int c_sock, char client_msg[MAX_LEN]) {
	// get the filename
	char *msg = strdup(client_msg);
	char *pch = strtok(msg, " ");
	int i = 0; while (i++ != 1) pch = strtok(NULL, " ");
	char *filename = pch;
	free(msg);
	// get the POST data
	pch = strtok(client_msg, "\n");
	char *post_data = malloc(strlen(client_msg)); // more than enough..
	while (pch != NULL) {
		post_data = pch; 
		pch = strtok(NULL, "\n");
	}
	post_data = url_decode(post_data);

	printf("Post data: [%s]\n", post_data);

	if (filename[0] != '/') {
		sendFile(c_sock, "400.html", "400 Bad Request");
		return -1;
	}
	
	if (strcmp(filename, "/") == 0) 
		filename = "index.html";
	if (filename[0] == '/') // remove the / from the filename
		memmove(filename, filename+1, strlen(filename));

	// create child process
	char *buffer = malloc(1024); // starting amount - str[1024]
	char str[1024];
 	int pipefd[2];
 	pid_t pid;

  	pipe (pipefd);
  	switch(pid = fork()) {
  		case -1: 
  			perror("fork");
  			break; // send 503 server error

   		case 0 : // this is the code the child runs 
    		dup2(pipefd[1], STDOUT_FILENO);
        	close(pipefd[0]);

			if (access( filename, F_OK ) != -1 ) { 
				char *argv[] = { filename, post_data, 0 };
				char *q = malloc(strlen(post_data) + 14);
					sprintf(q, "QUERY_STRING=%s", post_data);
				char *l = malloc(strlen(post_data) + 14);
					sprintf(l, "CONTENT_LENGTH=%d", strlen(post_data) -1 );
				char *envp[] = { 
		    		"REQUEST_METHOD=POST",
		    		l,
		    		q,
		    		0
		    	};

				if ((i = execve (argv[0], argv, envp)) == -1) {
        			perror("execle");
        		}
        		fflush(stdout);
			}

   		default: // this is the code the parent runs 
   			memset(buffer, 0, 1024 * sizeof(char)); // zero the buffer
   			int res;
   			struct timeval timeout;
			fd_set fd;
			timeout.tv_sec = 5;
			timeout.tv_usec = 0;

    		do {
    			FD_ZERO(&fd);
				FD_SET(pipefd[0], &fd);

				res = select(pipefd[0] + 1, &fd, NULL, NULL, &timeout);
				if (res <= 0) {
					perror("select");
					break;
				}
				else {
					memset(&(str), '\0', 1023);
	    			if (read(pipefd[0], str, 1023) > 0) {
	    				if ((buffer = realloc(buffer, strlen(buffer) + strlen(str) + 1)) == NULL) {
	    					perror("realloc");
	    				} else {
	    					strncat(buffer, str, strlen(str) + 1);
	    				}
	    			}
		        }
			} while (1);

			if (buffer == NULL || strlen(buffer) == 0) {
				sendFile(c_sock, "404.html", "404 Not Found");
    			return -1;
			} else {
				// Send header
    			char *header = constructHTTPHeader("200 OK", "application/x-www-form-urlencoded", strlen(buffer));
        	    	sendPacket(c_sock, header, strlen(header)); 
            		// Send data
    			sendPacket(c_sock, buffer, strlen(buffer));
			}

   		}

	return 0;
}

int main(int argc, char* argv[]) {
	int server_sock, client_sock, client_len;
	struct sockaddr_in server, client;

	if (getArgv(argc, argv) == -1)
		return -1;

	if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Could not create server socket.");
		return 1;
	}
	printf("Initialising server socket.\n");

	server.sin_family = AF_INET; 			// host byte order
	server.sin_addr.s_addr = INADDR_ANY;	// automatically fill with my IP
	server.sin_port = htons(port);			// short, network byte order
	memset(&(server.sin_zero), '\0', 8);	// zero the rest of the struct

	if (bind(server_sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
		perror("Unable to bind to port.");
		return 1;
	}
	printf("Successfully binded to port: %d\n", port);

	if (listen(server_sock, MAX_CONN) == -1) {
		perror("listen");
		return 1;
	}
	printf("Listening for incoming connections.\n");

	while (1) { // connection listener
		client_len = sizeof(struct sockaddr_in);
		if ((client_sock = accept(server_sock, (struct sockaddr*)&client, (socklen_t*)&client_len)) < 0) {
			perror("accept");
			continue; // block for new client
		}
		printf("Connection accepted from: %s:%d\n", inet_ntoa((struct in_addr) client.sin_addr), client.sin_port);

		// start child process
		if (!fork()) {
			close(server_sock);

			int read_size;
			char client_msg[MAX_LEN];
				memset(&(client_msg), '\0', MAX_LEN);
			int headerCount = 0;
			int res = 0;
			char fileHeader[MAX_LEN];

			struct timeval timeout;
			fd_set fd;
			timeout.tv_sec = 10;
			timeout.tv_usec = 0;

			do {
				FD_ZERO(&fd);
				FD_SET(client_sock, &fd);

				read_size = select(client_sock + 1, &fd, NULL, NULL, &timeout);
				if (read_size < 0) {
					perror("select");
					break;
				}
				else if (read_size == 0) {
					printf("Client timed out.\n");
					break;
				}
				else {
					read_size = recv(client_sock, client_msg, MAX_LEN, 0);
					printf("\tReceived packet from: %s:%d [%s]\n", inet_ntoa((struct in_addr) client.sin_addr), client.sin_port, client_msg);

					if (headerCount == 0 && !strstr(client_msg, "HTTP/1.1") && !strstr(client_msg, "HTTP/1.0") ) {
		                res = 400; 
		            }
	                if (headerCount == 0 && strncmp(client_msg, "GET", 3) == 0 ) {
	                	res = 1;
	                	strcpy(fileHeader, client_msg);
	                }           	
	            	if (headerCount == 0 && strncmp(client_msg, "POST", 4) == 0) {
	            		res = 2;
	            		strcpy(fileHeader, client_msg);
	            	}
	            	if (strcmp(client_msg, "\r\n") == 0 || strcmp(client_msg, "\n") == 0 || 
	            			strstr(client_msg, "\r\n\r\n") || strstr(client_msg, "\n\n") ||
	            			(client_msg[strlen(client_msg) - 1] == '\n' && (client_msg[strlen(client_msg) - 3] == '\n' || client_msg[strlen(client_msg) - 2] == '\n'))) {
						break; // end of headers
					}

					memset(&(client_msg), '\0', MAX_LEN);
	            	headerCount++;
				}
			} while (1);

			switch (res) {
				case 1:
					getFile(client_sock, fileHeader);
					break;
				case 2:
					putFile(client_sock, fileHeader);
					break;
				case 400:
					sendFile(client_sock, "400.html", "400 Bad Request");
					break;
				default:
					sendFile(client_sock, "500.html", "500 Internal Server Error");
					break;
			}

			close(client_sock);
			exit(0);
		}

		close(client_sock); // parent process doesn't need anymore
	}

	return 0;
}
