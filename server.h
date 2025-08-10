#define BACKLOG_LIMIT 10

extern int listener;

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
 * Accepts an incoming connection on the listening socket.
 * 
 * On success, returns 0. Otherwise, returns -1.
 */
int accept_connection();

/**
 * Accepts a new connection and appends the resulting socket file descriptor to pollfds so it can be monitored.
 * 
 * On success, returns 0. Otherwise, returns -1.
 */
int create_connection();

/** 
 * Receives data from the socket sender and sends it to all other sockets (except for the listener socket).
 * 
 * On success, returns 1. If the connection to the client is closed, returns 0. Returns -1 if there is an error.
 */
int handle_data(int sender);

/**
 * Closes the connection to socket sockfd and removes it from pollfds at index i.
 */
void close_connection(int sockfd, int i);

/**
 * Application entry point.
 */
int main();
