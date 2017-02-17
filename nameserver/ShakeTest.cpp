//GIT HW COMMENT
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "fifo.h"

using namespace std;
string receive_fifo = "Wordreply";
string send_fifo = "Wordrequest";

int main() {
  string message, name;
  string reply;
  char stem, next;
  
  next = 'q';

  while (next != 'y') {
	  //ask for the search term
	cout << "Enter a word:";
	cin >>  name;
	transform(name.begin(), name.end(), name.begin(), ::toupper);
	
	// create the FIFOs for communication
	Fifo recfifo(receive_fifo);
	Fifo sendfifo(send_fifo);

	//Ask if they want the word stemmed or not
	cout << "Would you like to search for that word stemmed? Y/N ";
	cin >> stem;
	
	//If they want it stemmed, add the $STEMMED* tag, if they don't, add the $SPECIFIC* tag
	if (stem == 'N' || stem == 'n')
	{
	message = "$SPECIFIC*" + name;
	cout << "Send:" << message << endl;
	sendfifo.openwrite();
	sendfifo.send(message);
	sendfifo.fifoclose();
	}
	else
	{
	message = "$STEMMED*" + name;
	cout << "Send:" << message << endl;
	sendfifo.openwrite();
	sendfifo.send(message);
	sendfifo.fifoclose();
	}
	
	do{
	/* Get a message from a server */
	/* Repeat until the $END signal is received */
	recfifo.openread();
	reply = recfifo.recv();
	recfifo.fifoclose();
	if (reply != "$END")
		cout << "Server sent: " << reply << endl;
	} while (reply.find("$END") == string::npos);
  }

}
