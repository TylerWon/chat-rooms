#define DEFAULT_NAME "anonymous"
#define COMMAND_SIZE_LIMIT 5

/**
 * Clears previous line from terminal.
 */
void clear_previous_line(); 

/**
 * Executes the command in str if it is a valid command.
 * 
 * Commands:
 * - /name [name] - Sets the user's name to [name]
 * - /exit - Exits the application
 */
void execute_command(char *str, char *name); 

/**
 * Gets the address info of the server for the given port. The server runs on the same host as the client so the IP 
 * address will be the loopback address (i.e. 127.0.0.1 or ::1).
 * 
 * On success, returns 1 and stores the address info in *res which is a linked list of struct addrinfos. Otherwise, 
 * returns a non-zero error code (same codes as getaddrinfo()).
 */
int get_server_addr_info(char *port, struct addrinfo **res);

/**
 * Creates a socket which is connected to the address provided in res (a linked list of struct addrinfos).
 * 
 * On success, returns the socket file descriptor. Otherwise, returns -1.
 */
int create_socket(struct addrinfo *res);

/**
 * Application entry point.
 */
int main();