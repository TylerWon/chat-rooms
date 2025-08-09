#include <stdint.h>

/** 
 * Gets the IP address from a struct sockaddr and stores it an array buf that has a size of n. Since struct sockaddr is 
 * protocol agnostic, its sa_family field is used to determine if the struct represents a IPv4 or IPv6 address so it can
 * be cast to a struct sockaddr_in or sockaddr_in6 for easier access.
 */ 
void get_ip_address(struct sockaddr *sa, char *buf, size_t n);

/** 
 * Returns the port number from a struct sockaddr. Since struct sockaddr is protocol agnostic, its sa_family field is 
 * used to determine if the struct represents a IPv4 or IPv6 address so it can be cast to a struct sockaddr_in or 
 * sockaddr_in6 for easier access.
 */ 
uint16_t get_port(struct sockaddr *sa);
