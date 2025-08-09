#define PORT "4000"

#define SEND_FLAGS 0
#define RECV_FLAGS 0

/** 
 * Returns a pointer to an IPv4/IPv6 address from a struct sockaddr. Since struct sockaddr is protocol agnostic, its 
 * sa_family field is used to determine if the struct represents a IPv4 or IPv6 address so it can be cast to a struct 
 * sockaddr_in or sockaddr_in6 for easier access.
 */ 
void *get_ip_address(struct sockaddr *sa);

/** 
 * Returns the port number from a struct sockaddr. Since struct sockaddr is protocol agnostic, its sa_family field is 
 * used to determine if the struct represents a IPv4 or IPv6 address so it can be cast to a struct sockaddr_in or 
 * sockaddr_in6 for easier access.
 */ 
uint16_t get_port(struct sockaddr *sa);

/**
 * Sends a message stored in buf on sockfd, handling partial sends so the entire messsage is delivered. 
 * 
 * On success, returns the number of bytes sent. -1 is returned on error and errno is set to indicate the error.
 */
ssize_t sendall(int sockfd, char *buf, size_t len);

/**
 * Receives a message on sockfd, handling partial receives so the entire message is obtained. Since messages have
 * variable lengths, *buf will be dynamically allocated based on the incoming message's size. 
 * 
 * On success, returns the number of bytes received and *buf will contain the message. -1 is returned on error and errno
 * is set to indicate the error. 0 is returned if the connection is closed.
 */
ssize_t recvall(int sockfd, char **buf);
