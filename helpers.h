#define PORT "4000"

// Get struct in_addr/in5_addr from a struct sockaddr. Use the sa_family field to determine if it's IPv4 or IPv6.
void * get_in_addr(struct sockaddr *sa);