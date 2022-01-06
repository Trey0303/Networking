#include "serverClient.h"

//Winsock
#include <WinSock2.h>
//include <stdio.h> for things like printf
#include <stdio.h>

//entry point of server program
int main() {
	//server

	//We need to pass a WORD as the version we want
	WORD winsock_version = 0x202;//according to the documentation the current version is 2.2, and is supported from Windows 98 onwards
	//need a pointer to a WSADATA struct which is populated by the call
	WSADATA winsock_data;
	//It will return 0 on success (yay Windows!), and if there is a problem we can get the error code with WSAGetLastError.
	if (WSAStartup(winsock_version, &winsock_data)) {
		printf("WSAStartup failed: %d", WSAGetLastError());
		//return;
	}

	//create a socket, this can be done using the socket function
	int address_family = AF_INET;
	int type = SOCK_DGRAM;
	int protocol = IPPROTO_UDP;
	SOCKET sock = socket(address_family, type, protocol);
	if (sock == INVALID_SOCKET) {
		printf("socket failed %d", WSAGetLastError());
		//return;
	}

	//When a packet is sent, not only is it sent to a particular machine, but also on a particular port. This allows a program to only receive the packets that it was actually meant to see.
	SOCKADDR_IN local_address;
	local_address.sin_family = AF_INET;
	local_address.sin_port = htons(9999);//We need to pick a port to use - anything below 1024 is reserved
	local_address.sin_addr.s_addr = INADDR_ANY;
	if (bind(sock, (SOCKADDR*)&local_address, sizeof(local_address)) == SOCKET_ERROR)
	{
		printf("bind failed: %d", WSAGetLastError());
		//return;
	}

	//Now to start receiving packets
	//We'll be limiting the size of the packets we send, not just because we cannot exceed our MTU, but mainly because we'll need to do everything we can to limit bandwidth. The smaller our state packets, the more frequently we can get away with sending them, and/or the more clients we can have connected to the same instance.
	//The call to recvfrom also tells us who sent the packet to us. This is one of the main differences between TCP and UDP - we would need a TCP socket per client, whereas with UDP we can have all the clients send packets to a single socket. In future when we receive a packet, we'll use this to figure out which player it came from.
	
	char buffer[SOCKET_BUFFER_SIZE];
	int player_x = 0;
	int player_y = 0;
	
	bool is_running = 1;
	while (is_running)
	{
		// get input packet from player
		int flags = 0;
		SOCKADDR_IN from;
		int from_size = sizeof(from);
		int bytes_received = recvfrom(sock, buffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&from, &from_size);

		if (bytes_received == SOCKET_ERROR)
		{
			printf("recvfrom returned SOCKET_ERROR, WSAGetLastError() %d", WSAGetLastError());
			break;
		}

		// process input
		char client_input = buffer[0];
		printf("%d.%d.%d.%d:%d - %c\n", from.sin_addr.S_un.S_un_b.s_b1, from.sin_addr.S_un.S_un_b.s_b2, from.sin_addr.S_un.S_un_b.s_b3, from.sin_addr.S_un.S_un_b.s_b4, from.sin_port, client_input);

		switch (client_input)
		{
		case 'w':
			++player_y;
			break;

		case 'a':
			--player_x;
			break;

		case 's':
			--player_y;
			break;

		case 'd':
			++player_x;
			break;

		case 'q':
			is_running = 0;
			break;

		default:
			printf("unhandled input %c\n", client_input);
			break;
		}

		// create state packet
		int write_index = 0;
		memcpy(&buffer[write_index], &player_x, sizeof(player_x));
		write_index += sizeof(player_x);

		memcpy(&buffer[write_index], &player_y, sizeof(player_y));
		write_index += sizeof(player_y);

		memcpy(&buffer[write_index], &is_running, sizeof(is_running));

		// send back to client
		int buffer_length = sizeof(player_x) + sizeof(player_y) + sizeof(is_running);
		flags = 0;
		SOCKADDR* to = (SOCKADDR*)&from;
		int to_length = sizeof(from);
		if (sendto(sock, buffer, buffer_length, flags, to, to_length) == SOCKET_ERROR)
		{
			printf("sendto failed: %d", WSAGetLastError());
			WSACleanup();
			//return;
		}
	}
	printf("done");

	WSACleanup();

	return 0;
}

	//old loop
	/*char buffer[SOCKET_BUFFER_SIZE];
	int flags = 0;
	SOCKADDR_IN from;
	int from_size = sizeof(from);
	int bytes_received = recvfrom(sock, buffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&from, &from_size);

	if (bytes_received == SOCKET_ERROR)
	{
		printf("recvfrom returned SOCKET_ERROR, WSAGetLastError() %d", WSAGetLastError());
	}
	else
	{
		buffer[bytes_received] = 0;
		printf("%d.%d.%d.%d:%d - %s",
			from.sin_addr.S_un.S_un_b.s_b1,
			from.sin_addr.S_un.S_un_b.s_b2,
			from.sin_addr.S_un.S_un_b.s_b3,
			from.sin_addr.S_un.S_un_b.s_b4,
			from.sin_port,
			buffer);
	}*/

//all commented code below is already definded by just including winsock. only need to call them
////need to first call WSAStartup before we can use any Winsock functions
////https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsastartup
//int WSAStartup(
//	[in]  WORD      wVersionRequested,
//	[out] LPWSADATA lpWSAData
//);
//
////create a socket, this can be done using the socket function
//SOCKET WSAAPI socket(
//	_In_ int af,      //af stands for address family(passing AF_INET for IPv4?)
//	_In_ int type,    //for a UDP socket this needs to be SOCK_DGRAM
//	_In_ int protocal
//);