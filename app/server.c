#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

/**
 * Important things I did not know:
 * 
 * 1. What is a file descriptor (fd)?
 * 
 * 		According to Dave (ChatGPT):
 * 		"In Unix-like operating systems, including Linux,
 * 		file descriptors are integer values that the operating
 * 		system uses to uniquely identify an open file or a
 * 		communication channel. These channels can include
 * 		regular files, sockets, pipes, character devices, and
 * 		others."
 * 
 * 		It is essentially an identifier for operations such
 * 		as binding, listening, accepting connections, etc.
 * 
 * 2. What is big-endian and little-endian?
 * 	
 * 		Some processors store the least significant bytes first,
 * 		`Little-endian`, while others store the most significant bytes first,
 * 		`Big-endian`. Due to this, sometimes sending data over the network turns
 * 		into a complicated mess. To account for this `Network Byte Order` was
 * 		established. Network Byte Order is almost always `Big-endian`.
 * 
*/

int main()
{
	
	/**
	 * Disable output buffering.
	 * This means that the program will not wait to
	 * output to the terminal
	 * 
	 * For example, when doing
	 * printf("a");
	 * printf("b");
	 * printf("c");
	 * The program will output a,b, and c immediately after
	 * each print instead of waiting for a newline ('\n')
	*/
	setbuf(stdout, NULL);


	// printf for debugging
	printf("Starting program :)!\n");

	/**
	 * Creates a variable srv_addr that stores an IPv4 socket address.
	 * .sin_family = AF_INET -> Indicates that the socket address is an IPv4 address
	 * .sin_port = htons(PORT) -> Converts the port number from `Host Byte Order` to
	 * 								`Network Byte Order`
	 * .sin_addr = { htonl(INADDR_ANY) } -> "Sets the IP address to INADDR_ANY, which
	 * 										means the socket will be bound to all available
	 * 										network interfaces on the machine. INADDR_ANY is
	 * 										a constant that represents "any" network interface."
	 * 										- Dave (ChatGPT)
	 * 
	 * "The htons() function converts the unsigned short integer hostshort from host byte order to network byte order. "
	 * "The htonl() function converts the unsigned integer hostlong from host byte order to network byte order. "
	 * 		- https://linux.die.net/man/3/htons
	 * 
	 * struct sockaddr_in is a struct from in.h that has the following data:
	 * 			sin_port -> Port number
	 * 			sin_addr -> Internet address
	 * 			sin_zero -> Padding to make the structure the same size as sockaddr
	*/
	struct sockaddr_in serv_addr = { .sin_family = AF_INET ,			// IPv4
									 .sin_port = htons(4221),			// `Host Byte Order` to `Network Byte Order` short
									 .sin_addr = { htonl(INADDR_ANY) },	// `Host Byte Order` to `Network Byte Order` long
									};
	
	/**
	 * Creates a socket connection using IPv4 (AF_INET), and TCP (SOCK_STREAM)
	 * Other options include AF_INET6 for IPv6 and SOCK_DGRAM for UDP.
	 * 
	 * The last parameter is the protocol, which according to the documentation:
	 * 
	 * 		"If the protocol parameter is set to 0, the system selects the default
	 * 		protocol number for the domain and socket type requested."
	 * 
	 * I was a bit confused about why is it necessary to specify the protocol twice.
	 * For instance, SOCK_STREAM is already selecting TCP, but the function also
	 * expects IPPROTO_TCP as the third parameter. I couldn't really find an answer
	 * so I assume it is for safety reasons or if you know what you are doing.
	 * 
	 * socket() returns a number greater or equal than 0 if the connection was successful
	 * or -1 if an error occurred
	*/
	int server_fd = socket(AF_INET, SOCK_STREAM, 0); // Variable to represent the file descriptor (fd) of the server socket
	if (server_fd == -1)
	{
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}

	/**
	 * Since the tester restarts your program quite often, setting REUSE_PORT
	 * ensures that we don't run into 'Address already in use' errors
	*/
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0)
	{
		printf("SO_REUSEPORT failed: %s \n", strerror(errno));
		return 1;
	}
	
	/**
	 * According to IBM:
	 * 
	 * 		"The bind() function binds a unique local name to the socket with descriptor socket.
	 * 		After calling socket(), a descriptor does not have a name associated with it.
	 * 		However, it does belong to a particular address family as specified when socket()
	 * 		is called. The exact format of a name depends on the address family."
	 * 
	 * 	- https://www.ibm.com/docs/en/zos/2.3.0?topic=functions-bind-bind-name-socket
	 * 
	 * In other words: when you first create a socket using `socket()` function, the socket is
	 * not yet associated with a specific address or port.
	 * 
	 * Params:
	 * -> server_fd is the socket identifier that we created before.
	 * -> serv_addr is the struct that we specified. serv_addr is casted from
	 * sockaddr_in to sockaddr since sockaddr is the generic struct for socket addresses.
	 * -> The size of the socket address structure.
	 * 
	 * The function should return 0 if the binding was successful
	*/
	if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0)
	{
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}
	
	int connection_backlog = 5; // Maximum length of the queue of pending connections

	/**
	 * `listen()` indicates that the server socket is ready to accept incoming connections
	 * 
	 * returns 0 if connection was successful
	*/
	if (listen(server_fd, connection_backlog) != 0)
	{
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}
	
	struct sockaddr_in client_addr; // Variable of type struct sockaddr_in to store the client address
	int client_addr_len = sizeof(client_addr); // Variable to store the length of the struct client_addr
	
	printf("Waiting for a client to connect...\n");
	

	int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len); // Variable to represent the file descriptor (fd) of the client socket
	printf("Client connected\n");

	/**
	 * \r\n\r\n represents the CRLF:
	 * 		CR = Carriage Return (\r, 0x0D in hexadecimal, 13 in decimal) — moves the cursor to the beginning
	 * 		of the line without advancing to the next line.
	 * 
	 * 		LF = Line Feed (\n, 0x0A in hexadecimal, 10 in decimal) — moves the cursor down to the next line
	 * 		without returning to the beginning of the line.
	 * 
	 * - https://developer.mozilla.org/en-US/docs/Glossary/CRLF
	*/
	char* res = "HTTP/1.1 200 OK\r\n\r\n"; // HTTP response

	/**
	 * `recv()` receives data on the client_fd socket and stores it in the res buffer.
	 * If successful, returns the length of the message or datagram in bytes, otherwise
	 * returns -1.
	*/
	// int bytesReceived = recv(client_fd, res, strlen(res), 0);
	// if (bytesReceived == -1)
	// {
	// 	printf("Receive failed!\n");
	// 	return 1;
	// }

	/**
	 * `send()` sends data on the client_fd socket.
	 * If successful, returns 0 or greater indicating the number of bytes sent, otherwise
	 * returns -1.
	*/
	int bytesSent = send(client_fd, res, strlen(res), 0);
	if (bytesSent < 0)
	{
		printf("Send failed\n");
		return 1;
	}
	
	close(server_fd);

	return 0;
}
