#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <wiringPiSPI.h>
#include "packets.h"


struct PacketBluetooth pBluetooth = { 0 };
struct PacketSPI_OUT pOut = { 0 };
struct PacketSPI_IN pIN = { 0 };

// modes
#define MASTER 1
#define SLAVE 0

// bluetooth devices
const char PI_ONE[18]  = "B8:27:EB:EA:F3:FD";
const char PI_TWO[18] = "B8:27:EB:55:6B:64";

int main(int argc, char** argv) {
	int mode = MASTER;
	if( argc > 2 && !strcmp( argv[1], "SLAVE" ) ) {
		mode = SLAVE;
	}
	
	// setup SPI
	wiringPiSPISetup( 0, 1000000 );
	unsigned char bytes[4];
	

	// setup Bluetooth
	int sock, client;
	int bytesRead, totalRead;
	struct sockaddr_rc con = { 0 }, client_con = { 0 };
	char dest[18], clientName[18], buffer[32];
	socklen_t opt = sizeof( client_con );

	if( mode == MASTER ) {
		printf( "[*] Starting as Master\n" );
		
		// create server-socket
		sock = socket( AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM );
		
		// bind socket to port 1
		con.rc_family = AF_BLUETOOTH;
		con.rc_bdaddr = *BDADDR_ANY;
		con.rc_channel = (uint8_t) 1;
		bind( sock, (struct sockaddr*) &con, sizeof(con) );
		
		// listen to port 1
		listen( sock, 1 );
		
		// Wait for slave to connect
		do {
			printf( "[*] Waiting for slave to connect...\n" );
			client = accept( sock, (struct sockaddr*) &client_con, &opt );
			ba2str( &client_con.rc_bdaddr, clientName );
			printf( "[*] %s connected\n", clientName );
			
			if( strcmp(clientName, PI_ONE) && strcmp(clientName, PI_TWO) ) {
				close( client );
			}
		} while( strcmp(clientName, PI_ONE) && strcmp(clientName, PI_TWO) );
		printf( "[*] Slave connected" );
	}
	
	if( mode == SLAVE ) {
		printf( "[*] Starting as Slave\n" );
		
		if( argc < 3 ) {
			printf( "[*] Error: Destination not given\n" );
			return 0;
		}
		
		if( !strcmp( argv[2], "PI_ONE" ) )
			memcpy( dest, PI_ONE, 18 );
		else if( !strcmp( argv[2], "PI_TWO" ) )
			memcpy( dest, PI_TWO, 18 );
		else {
			printf( "[*] Error: Unknown Destination\n" );
			return 0;
		}
		
		printf( "[*] Master is %s\n", dest );
		
		// Create socket
		sock = socket( AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM );
		
		// Set connection
		con.rc_family = AF_BLUETOOTH;
		con.rc_channel = (uint8_t) 1;
		str2ba( dest, &con.rc_bdaddr );
		
		// Connect
		printf( "[*] Connect to master\n" );
		int status = connect( sock, (struct sockaddr*) &con, sizeof(con) );
		if( status < 0 ) {
			printf( "[*] Error: Failed to connect. %d\n" );
			return 0;
		} else if( status == 0 ) {
			// Connected
			printf( "[*] Connected to master\n" );
		}
	}
	
	while( 1 ) {
		
		// Get value from camera
		
		
		// Exchange data with controller
		printf( "[*] Send data to controler\n" );
		
		memcpy( bytes, pOut, 4 );
		wiringPiSPIDataRW( 0, bytes, 4 );
		memcpy( pIn, bytes, 4 );
		
		
		// Exchange data with other robot
		printf( "[*] Send data to other robot\n" );
		if( mode == MASTER ) {
			memcpy( buffer, pBluetooth, 3 );
			write( client,  buffer, 3 );
			
			totalRead = 0;
			do {
				bytesRead = read( client, buf, 32 );
				totalRead += bytesRead;
			} while( totalRead < 3 && bytesRead >= 0 );
			
			if( bytesRead < 0 ) {
				printf( "[*] Error: Read data from bluetooth failed\n" );
			} else {
				memcpy( pBluetooth, buffer, 3 );
			}
		}
		
		if( mode == SLAVE ) {
			totalRead = 0;
			do {
				bytesRead = read( sock, buf, 32 );
				totalRead += bytesRead;
			} while( totalRead < 3 && bytesRead >= 0 );
			
			if( bytesRead < 0 ) {
				printf( "[*] Error: Read data from bluetooth failed\n" );
			} else {
				memcpy( pBluetooth, buffer, 3 );
			}
			
			memcpy( buffer, pBluetooth, 3 );
			write( sock, buffer, 3 );
		}
		
	}
	


}
