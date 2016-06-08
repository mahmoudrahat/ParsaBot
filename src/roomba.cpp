/////////////////////////////////////////////////
// Serial port interface program 			   //
// Class to control Roomba					   //
// This class is responsible to send control   //
// commands to roomba						   //
/////////////////////////////////////////////////

#include <stdio.h> // standard input / output functions
#include <string.h> // string function definitions
#include <unistd.h> // UNIX standard function definitions
#include <fcntl.h> // File control definitions
#include <errno.h> // Error number definitions
#include <termios.h> // POSIX terminal control definitionss
#include <time.h>   // time calls


// roomba.cpp
#include "roomba.h"

roomba::roomba()
{
	move_left[0] = 145;
	move_left[1] = 0;
	move_left[2] = 30;
	move_left[3] = 255;
	move_left[4] = 226;

	move_right[0] = 145;
	move_right[1] = 255;
	move_right[2] = 226;
	move_right[3] = 0;
	move_right[4] = 30;

	send_beep[0] = 140;
	send_beep[1] = 3;
	send_beep[2] = 1;
	send_beep[3] = 64;
	send_beep[4] = 16;
	send_beep[5] = 141;
	send_beep[6] = 3;

	move_forward[0] = 145;
	move_forward[1] = 0;
	move_forward[2] = 100;
	move_forward[3] = 0;
	move_forward[4] = 100;

	move_back[0] = 145;
	move_back[1] = 255;
	move_back[2] = 206;
	move_back[3] = 255;
	move_back[4] = 206;

	send_stop[0] = 145;
	send_stop[1] = 0;
	send_stop[2] = 0;
	send_stop[3] = 0;
	send_stop[4] = 0;

	mode_pause[0] = 128;

	mode_pause[0] = 131;
}

// open serial port
int roomba::open_port(void)
{
	fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
	
	if(fd == -1) // if open is unsucessful
	{
		//perror("open_port: Unable to open /dev/ttyS0 - ");
		printf("open_port: Unable to open /dev/ttyS0. \n");
	}
	else
	{
		fcntl(fd, F_SETFL, 0);
		printf("port is open.\n");
	}
} //open_port

int roomba::configure_port()      // configure the port
{
	struct termios port_settings;      // structure to store the port settings in

	cfsetispeed(&port_settings, B115200);    // set baud rates
	cfsetospeed(&port_settings, B115200);

	port_settings.c_cflag &= ~PARENB;    // set no parity, stop bits, data bits
	port_settings.c_cflag &= ~CSTOPB;
	port_settings.c_cflag &= ~CSIZE;
	port_settings.c_cflag |= CS8;
	
	tcsetattr(fd, TCSANOW, &port_settings);    // apply the settings to the port
	return(fd);

} //configure_port

int roomba::send_roomba(commands direction)   // 0:stop 1:left 2:right 3:forward 4:backward 5:beep 6:mode_pause 7:mode_safe
{
	//char n;
	//fd_set rdfs;
	//struct timeval timeout;
	
	// initialise the timeout structure
	//timeout.tv_sec = 10; // ten second timeout
	//timeout.tv_usec = 0;
	
	//Create byte array
	
	if(direction == stop)
		write(fd, send_stop, 5);  //Send data
	else if(direction == left)
		write(fd, move_left, 5);  //Send data
	else if(direction == right)
		write(fd, move_right, 5);  //Send data
	else if(direction == forward)
		write(fd, move_forward, 5);  //Send data
	else if(direction == backward)
		write(fd, move_back, 5);  //Send data
	else if(direction == beep)
		write(fd, send_beep, 7);  //Send data
	else if(direction == pause)
		write(fd, mode_pause, 1);  //Send data
	else if(direction == safe)
		write(fd, mode_safe, 1);  //Send data
	
	//printf("Wrote the bytes. \n");
	
	// do the select
	//n = select(fd + 1, &rdfs, NULL, NULL, &timeout);
	
	// check if an error has occured
	//if(n < 0)
	//{
	// perror("select failed\n");
	//}
	//else if (n == 0)
	//{
	// puts("Timeout!");
	//}
	//else
	//{
	// printf("\nBytes detected on the port!\n");
	//}

	return 0;
	
} //query_modem

void roomba::test_roomba()
{
	send_roomba(pause);
	usleep(200000);	
	send_roomba(safe);
	usleep(200000);	
	send_roomba(left);
	usleep(200000);	
	send_roomba(right);
	usleep(200000);	
	send_roomba(forward);
	usleep(200000);	
	send_roomba(backward);
	usleep(200000);	
	send_roomba(stop);
	usleep(200000);	
	send_roomba(beep);
	usleep(200000);	
}

