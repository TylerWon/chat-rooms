#define BACKLOG_LIMIT 10

/**
 * Gets the address info of the server for the given port. The IP address will be the wildcard address so connections
 * can be accpeted on any of the host's network addresses.
 * 
 * On success, returns 1 and stores the address info in *res which is a linked list of struct addrinfos. Otherwise, 
 * returns a non-zero error code (same codes as getaddrinfo()).
 */
int get_server_addr_info(char *port, struct addrinfo **res); 

/**
 * Creates a socket for listening to incoming connections to the address provided in res (a linked list of struct 
 * addrinfos).
 * 
 * On success, returns the socket file descriptor. Otherwise, returns -1. 
 */
int create_listener_socket(struct addrinfo *res); 

/**
 * Application entry point.
 */
int main();