#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

static const char *user_agent = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *accept = "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";
static const char *accept_encoding = "Accept-Encoding: gzip, deflate\r\n";

/* local structs */
typedef struct
{
    char method[10];
    char hostname[200];
    char path[1000];
} http_request;

typedef struct
{
    char key[200];
    char value[1000];
    http_header* next;
} http_header;

/* local functions */
void help_message();
void handle_request(int fd);
http_request* parse_request(char* line);
http_header* parse_header(char* line);
void free_http_metadata(http_request* request_ptr, http_header* header_head);

/* display usage message */
void help_message()
{
    const char* help_message_content = "Usage: ./proxy <port-number>\n";
    printf("%s", help_message_content);
    return;
}

/* main loop and dispatcher */
int main(int argc, char **argv)
{
    /* parse input message */
    int port_number;
    if (argc != 2)
    {
        help_message();
        return 0;
    }
    port_number = atoi(argv[1]);

    /* open listen port */
    int listenfd, connfd, clientlen;
    struct sockaddr_in clientaddr;
    listenfd = Open_listenfd(port_number);

    /* wait for client requests */
    while (1)
    {
	      clientlen = sizeof(clientaddr);
	      connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
	      handle_request(connfd);
	      Close(connfd);
    }

    return 0;
}

/* handle incoming request */
void handle_request(int fd)
{
    /* start parsing request */
    /* pointers for metadata */
    http_header* header_root = NULL;
    http_header* header_curr = NULL;
    http_header* header_temp = NULL;
    http_request* request_info = NULL;
    /* rio init */
    char buf[MAXLINE];
    rio_t rio;
    Rio_readinitb(&rio, fd);
    /* parse http request */
    Rio_readlineb(&rio, buf, MAXLINE);
    request_info = parse_request(buf);
    if (!request_info)
    {
        /* TODO: implement error handling here */
    }
    /* parse request header */
    Rio_readlineb(&rio, buf, MAXLINE);
    header_temp = parse_header(buf, http_header* header_curr);
    if (!header_temp)
    {
        /* TODO: implement error handling here */
    }
    header_root = header_temp;
    header_curr = header_temp;
    while(strcmp(buf, "\r\n"))
    {
        Rio_readlineb(&rio, buf, MAXLINE);
        header_temp = parse_header(buf, http_header* header_curr);
        if (!header_temp)
        {
            /* TODO: implement error handling here. Should exit rather then break */
        }
        header_curr = header_temp;
    }


    /* TODO: call free_http_metadata() here to free metadata tables */
}

/*
 parse HTTP request line (first line)
 return value: 0 for failed, pointer for success
 */
http_request* parse_request(char* line)
{
    char raw_method[10];
    char raw_url[1500];
    http_request* storage = malloc(sizeof(http_request));
    /* parse input line, we only need first two parameters */
    if (!(sscanf(line, "%s %s", raw_method, raw_url) == 2))
    {
        return 0;
    }

    /* parse method */
    if (strcmp("GET", raw_method) != 0)
    {
        /* if method is not GET, ignore this request */
        return 0;
    }
    else
    {
        strcpy(storage -> method, raw_method);
    }

    /* parse url */
    char* url_starting;
    char* path_starting;
    url_starting = strstr(raw_url, "http://");
    if (!url_starting)
    {
        /* can't find http scheme */
        return 0;
    }
    /* point to the start of hostname */
    url_starting += 7;
    path_starting = strstr(url_starting, '/');
    if (!path_starting)
    {
        /* no path specified */
        strcpy(storage -> hostname, url_starting);
        strcpy(storage -> path, "/");
    }
    else
    {
        memcpy(storage -> hostname, url_starting, path_starting - url_starting);
        *(storage -> hostname + path_starting - url_starting) = '\0';
        strcpy(storage -> path, path_starting);
    }
    return storage;
}

/* parse a header line, return NULL when error */
http_header* parse_header(char* line, http_header* last_node)
{
    char* sperater = strstr(line, ': ');
    char* terminator = strstr(line, '\r\n');
    if ((!sperater) || (!terminator))
    {
        /* bad header format */
        return NULL;
    }
    /* allocate space for new node */
    http_header* current = malloc(sizeof(http_header));
    /* fill in the struct */
    current -> next = NULL;
    memcpy(current -> key, line, sperater - line);
    *(current -> key + sperater - line) = '\0';
    memcpy(current -> value, sperator + 2, terminator - sperater - 2);
    *(current -> value + terminator - sperater - 2) = '\0';
    /* link the last node to the current node */
    if (last_node)
    {
        last_node -> next = current;
    }
    return current;
}

void free_http_metadata(http_request* request_ptr, http_header* header_head)
{
    free request_ptr;
    http_header* temp;
    while (header_head != NULL)
    {
        temp = header_head -> next;
        free header_head;
        header_head = temp;
    }
    return;
}
