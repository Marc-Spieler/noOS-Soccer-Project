#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "camera.h"
#include "com.h"
#include <wiringPiSPI.h>
#include <wiringPi.h>

struct PacketSPI_OUT pOut = { 0 };
struct PacketSPI_IN pIn = { 0 };

struct Info infoBall;
struct Info infoGoal;
struct Info infoGeneral;

int comBallReady;
int comGoalReady;


void *comTask(void *arguments)
{		
	int index = *((int *)arguments);

	// setup SPI
	wiringPiSPISetup( 0, 1000000 );
	unsigned char bytes[4];

	// 'Setup pin listener
	wiringPiSetup() ;
	//pinMode( 1, INPUT );

	while (1)
	{
		if((comBallReady==1)&&(comGoalReady==1))
		{
#if 0			// Get value from camera
			unsigned short horizontal = *((unsigned short*)(shared+1));
			unsigned short vertical = *((unsigned short*)(shared+1+sos));
			
			if( horizontal == 0xFFFF ) {
				long long elapsed = (t_now.tv_sec * 1000000 + t_now.tv_usec) - (t_ball.tv_sec * 1000000 + t_ball.tv_usec);
				if( elapsed < 250000 ) {
					horizontal = lastBall;
				}
			} else {
				lastBall = horizontal;
				gettimeofday(&t_ball, NULL );	
			}
			
			info.ball1.horizontal = horizontal;
			info.ball1.vertical = vertical;
			info.status.have1 = *((unsigned char*) (shared+1+sos+sos+1));
	
			printf("BallPos %d %d", horizontal, vertical);
#endif			
			//pOut.roboPos.x = infoBall.roboPos2.x;
			//pOut.roboPos.y = infoBall.roboPos2.y;
			//pOut.ballPos.x = infoBall.ballPos.x;
			//pOut.ballPos.y = infoBall.ballPos.y;
			pOut.bits.ball = infoBall.ball1.horizontal;
			//pOut.bits.have = (info.ball1.vertical <= 12) ? 1 : 0;
			pOut.bits.seeBall = infoBall.status.see;
			pOut.bits.have = infoBall.status.have1;
			pOut.bits.rsvd = 0;

			pOut.bits.goal = infoGoal.ball1.horizontal;			
			pOut.bits.seeGoal = infoGoal.status.see;
			
			//if( !infoBall.go ) pOut.bits.ball = 0x0;
			
			memcpy( bytes, &pOut, sizeof(pOut) );
			
			//uint8_t tmp = bytes[0];
			//for( int i = 0; i < sizeof(pOut) - 1; i++ ) {
			//	bytes[i] = bytes[i+1];
			//}
			//bytes[sizeof(pOut) - 1] = tmp;
			
			wiringPiSPIDataRW( 0, bytes, sizeof(pOut) );
			memcpy( &pIn, bytes, sizeof(pIn) );

			//printf("0x%02x 0x%02x 0x%02x 0x%02x\r\n", bytes[0], bytes[1], bytes[2], bytes[3]);
			
//			infoGeneral.roboPos1.x = pIn.roboPos.x;
//			infoGeneral.roboPos1.y = pIn.roboPos.y;
//			infoGeneral.status.s1 = pIn.bits.onField;
//			infoGeneral.wlanOld = infoGeneral.wlanNew;
//			infoGeneral.wlanNew = pIn.bits.WLAN;
//			infoGeneral.status.valid1 = pIn.bits.validX && pIn.bits.validY;
#if 0			
			int compass = pIn.info.compass - 180;
			if( compass < 0 ) compass = 360 - compass;
			
			//info.ball1.horizontal = compass + (info.ball1.horizontal - 32 );
			//if( info.ball1.horizontal >= 360 ) info.ball1.horizontal = info.ball1.horizontal - 360;
			//if( info.ball1.horizontal < 0 ) info.ball1.horizontal = info.ball1.horizontal + 360;
			
			// Exchange data with other robot
			if( mode == MASTER && sendDelay > 3 ) {
				char tm[16];
				sendDelay = 0;
				
				if( client > 0 ) {
				
					
					pBluetooth.roboPos.x = infoGeneral.roboPos1.x;
					pBluetooth.roboPos.y = infoGeneral.roboPos2.y;
					pBluetooth.bits.horizontal = info.ball1.horizontal;
					pBluetooth.bits.vertical = info.ball1.vertical;
					pBluetooth.bits.onField = infoGeneral.status.s1;
					pBluetooth.bits.valid = infoGeneral.status.valid1;
					pBluetooth.bits.have = info.status.have1;
					pBluetooth.id = sendID;
					sendID++;
					
					
					//printf( "[*] Writing data to client..." );
					//scanf( "%s", tm );
					memcpy( buffer, &pBluetooth, sizeof(pBluetooth) );
					write( client,  buffer, sizeof(pBluetooth) );
					sendCounter++;
					
					/* printf( "-------- SEND --------\n" );
					printf( "%x %x\n", pBluetooth.roboPos.x, pBluetooth.roboPos.y );
					printf( "%x\n", pBluetooth.bits.horizontal );
					printf( "%x\n", pBluetooth.bits.vertical );
					printf( "%x\n", pBluetooth.bits.onField );
					printf( "-------- SEND --------\n" ); */
					//scanf( "%s", tm );
					
					
					//printf( "[*] Reading data from client..." );
					usleep( 2000 );
					totalRead = 0;
					do {
						bytesRead = read( client, buffer+totalRead, 32 );
						totalRead += bytesRead;
						
					} while( totalRead < sizeof(pBluetooth) && bytesRead >= 0 );
					
					
					
					if( bytesRead < 0 ) {
						//printf( "[*] Error: Read data from bluetooth failed\n" );				
						readError++;
						sprintf(line9, "[*] Error: Read data from bluetooth failed.\n" );					
						liveCounter = 20;
						if( readError > 3 ) {
							info.status.s2 = 0;
						}
					} else {
						readError = 0;
						readCounter++;
						memcpy( &pBluetooth, buffer, sizeof(pBluetooth) );
						info.roboPos2.x = pBluetooth.roboPos.x;
						info.roboPos2.y = pBluetooth.roboPos.y;
						info.ball2.horizontal = pBluetooth.bits.horizontal;
						info.ball2.vertical = pBluetooth.bits.vertical;
						info.status.s2 = pBluetooth.bits.onField;
						info.status.valid2 = pBluetooth.bits.valid;
						info.status.have2 = pBluetooth.bits.have;
						recvID = pBluetooth.id;
					
						/* printf( "-------- RECV --------\n");
						printf( "%x %x\n", pBluetooth.roboPos.x, pBluetooth.roboPos.y );
						printf( "%x\n", pBluetooth.bits.horizontal );
						printf( "%x\n", pBluetooth.bits.vertical );
						printf( "%x\n", pBluetooth.bits.onField );
						printf( "-------- RECV --------\n"); */
						//scanf( "%s", tm );
					}
				}
				
			}
			
			if( mode == SLAVE && stateConnected && sendDelay > 3 ) {
				sendDelay = 0;
				usleep( 1000 );
				
				// Reading bluetooth data
				int nloop = 0;
				totalRead = 0;
				do {
					fcntl( sock, F_SETFL, O_NONBLOCK );
					nloop++;
					bytesRead = read( sock, buffer+totalRead, 32 );
					totalRead += bytesRead;
				} while( totalRead < sizeof(pBluetooth) && bytesRead >= 0 );
				
				if( bytesRead < 0 ) {
					//printf( "[*] Error: Read data from bluetooth failed\n" );
					readError++;
					sprintf(line9, "[*] Error: Read data from bluetooth failed[%d]\n", readError );
					liveCounter = 50;
					if( readError > 3 ) {
						info.status.s2 = 0;
					}
					
					if( readError >= 100 ) {
						//stateConnected = false;
						//readError = 0;
					}
				} else {
					readError = 0;
					readCounter++;
					memcpy( &pBluetooth, buffer, sizeof(pBluetooth) );
					info.roboPos2.x = pBluetooth.roboPos.x;
					info.roboPos2.y = pBluetooth.roboPos.y;
					info.ball2.horizontal = pBluetooth.bits.horizontal;
					info.ball2.vertical = pBluetooth.bits.vertical;
					info.status.s2 = pBluetooth.bits.onField;
					info.status.valid2 = pBluetooth.bits.valid;
					info.status.have2 = pBluetooth.bits.have; 
					recvID = pBluetooth.id;
					
					/* printf( "-------- RECEIVED--------\n");
					printf( "%x %x\n", pBluetooth.roboPos.x, pBluetooth.roboPos.y );
					printf( "%x\n", pBluetooth.bits.horizontal );
					printf( "%x\n", pBluetooth.bits.vertical );
					printf( "%x\n", pBluetooth.bits.onField );
					printf( "-------- RECEIVED--------\n"); */
					//scanf( "%s", ts );
				}
				
				
				pBluetooth.roboPos.x = info.roboPos1.x;
				pBluetooth.roboPos.y = info.roboPos2.y;
				pBluetooth.bits.horizontal = info.ball1.horizontal;
				pBluetooth.bits.vertical = info.ball1.vertical;
				pBluetooth.bits.onField = info.status.s1;
				pBluetooth.bits.valid = infoGeneral.status.valid1;
				pBluetooth.bits.have = info.status.have1;
				pBluetooth.id = sendID;
				sendID++;
	
				memcpy( buffer, &pBluetooth, sizeof(pBluetooth) );
				write( sock, buffer, sizeof(pBluetooth) );
				sendCounter++;
				
				/* printf( "-------- SEND --------\n");
				printf( "%x %x\n", pBluetooth.roboPos.x, pBluetooth.roboPos.y );
				printf( "%x\n", pBluetooth.bits.horizontal );
				printf( "%x\n", pBluetooth.bits.vertical );
				printf( "%x\n", pBluetooth.bits.onField );
				printf( "-------- SEND --------\n"); */
				//scanf( "%s", ts );
			}
			

			if( mDebug ) {
				clear();
				mvprintw( 0, 0, "Mode: %s", mode == MASTER ? "MASTER" : "SLAVE" );
				mvprintw( 1, 0, "My Position: (%d/%d)", info.roboPos1.x, info.roboPos1.y );
				mvprintw( 2, 0, "Other Position: (%d/%d)", info.roboPos2.x, info.roboPos2.y );
				mvprintw( 3, 0, "My Ball: %d:%d", info.ball1.horizontal, info.ball1.vertical );
				mvprintw( 4, 0, "Other Ball %d:%d", info.ball2.horizontal, info.ball2.vertical );
				mvprintw( 5, 0, "On Field: %d %d", info.status.s1, info.status.s2 );
				mvprintw( 6, 0, "Having ball: %d - %d", info.status.have1, info.status.have2 );
				//mvprintw( 7, 0, "WIFI: %s", pIn.bits.WLAN ? "ON" : "OFF" );
				mvprintw( 7, 0, "FPS: %d", *((unsigned long*) (shared+1+sos+sos+1)) );
				mvprintw( 8, 0, line8 );
				mvprintw( 9, 0, line9 );
				mvprintw( 12, 0, "BLUETOOTH SendID: %d", sendID - 1 );
				mvprintw( 13, 0, "BLUETOOTH RecvID: %d", recvID );
				refresh();
				
				
				if( liveCounter == 0 ) {
					sprintf(line8, " " );
					sprintf(line9, " " );
				} else {
					liveCounter--;
				}
			} 
				
			/*
			printf( "Mode: %s\n", mode == MASTER ? "MASTER" : "SLAVE" );
			printf( "My Position (%d/%d)\n", info.roboPos1.x, info.roboPos1.y );
			printf( "Other Position: (%d/%d)\n", info.roboPos2.x, info.roboPos2.y );
			printf( "My Ball horizontal: %d\n", info.ball1.horizontal );
			printf( "My Ball vertical: %d\n", info.ball1.vertical );
			printf( "Other Ball horizontal: %d\n", info.ball2.horizontal );
			printf( "Other Ball vertical: %d\n", info.ball2.vertical );
			printf( "On Field: %d %d\n", info.status.s1, info.status.s2 );
			printf( "Having ball: %d\n", pOut.bits.have );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			*/
			
			// Interpretation
			/* if( info.wlanOld != info.wlanNew ) {
				if( info.wlanNew ) system("sudo ifconfig wlan0 up");
				if( !info.wlanNew ) system("sudo ifconfig wlan0 down");
			} */
			
		
			if( info.status.have2 ) info.go = false;
			if( info.ball2.vertical < info.ball1.vertical ) info.go = false;
			
			if( info.status.have1 ) info.go = true;
			if( !info.status.s2 ) info.go = true;
			if( info.ball1.vertical <= info.ball2.vertical ) info.go = true;
#endif				
			comBallReady=0;
			comGoalReady=0;			
		}
			
		usleep(10*1000); 
	}
}
