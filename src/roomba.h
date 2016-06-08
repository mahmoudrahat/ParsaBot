// ROOMBA.h
#ifndef ROOMBA_H
#define ROOMBA_H

class roomba
{
int fd;  // file description for the serial port

public:

  enum commands {stop, left, right, forward, backward, beep, pause, safe};

  unsigned char move_right[5];  	// bytes to send move right command
  unsigned char move_left[5];		// bytes to send move left command
  unsigned char move_forward[5];
  unsigned char move_back[5];
  unsigned char send_stop[5];
  unsigned char send_beep[7];
  unsigned char mode_pause[1];
  unsigned char mode_safe[1];


  roomba();							// class constructor
  int open_port();					// open serial port
  int configure_port();				// configure serial port
  int send_roomba(commands );		// send control commands by writing on serial port
  void test_roomba();				// test roomba movemnents
};

#endif