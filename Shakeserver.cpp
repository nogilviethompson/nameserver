/*YanqiXu added this comment.*/
/***************************************************************************
* nameservercpp  -  Program to serve of last name statistics
 *
* copyright : (C) 2009 by Jim Skon
*
* This program runs as a background server to a CGI program, providinging US Census
* Data on the frequency of names in response to requestes.  The rational for this 
* is to demonstrait how a background process can serve up information to a web program,
* saving the web program from the work of intiallizing large data structures for every
* call.
* 
* 
*
***************************************************************************/
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>
#include "english_stem.h"
#include "meta.h"
#include "stemming.h"
#include "string_util.h"
#include "utilities.h"
#include "fifo.h"

using namespace std;

/* Fifo names */
string receive_fifo = "Wordrequest";
string send_fifo = "Wordreply";

void stemmer (string& word);
//Precondition: "word" is a single English word
//Postcondition: the stemmed version of "word" is returned
int build (ifstream& infile, map<string, vector<int> >& refs, map<string, vector<int> >& stems);
//Precondition: "infile" is an opened ifstream and "refs" and "stems" are empty maps
//Postcondition: The maps have been filled with each individual word in "infile" and a vector of the positions of the lines they are on, "refs" containing all words and "stems" containing only the stems
vector<int> indexSearch (map<string, vector<int> > refs, string word);
//Precondition: "refs" has been filled and "word" is a single word
//Postcondition: returns a vector of all the positions in the file of "word"
vector <int> stemmedIndexSearch (map<string, vector<int> > refs, string word);
//Precondition: "refs" has been filled and "word" is a single word
//Postcondition: returns a vector of all the positions in the file of the stemmed version of "word"

/* Server main line,create name MAP, wait for and serve requests */
int main() {
  
  map<string, vector<int> > refs, stems, currentMap;
  ifstream infile;
  vector<int> positions;
  string type;
  
  infile.open("Shakespeare.txt");
  
  //Build two maps, one containing every individual word and the other containing every word stem
  int good = build(infile, refs, stems);
  
  if (good == 0)
	  cout << "Failed to build map" << endl;
  else
	  cout << "Map data loaded" << endl;

  // create the FIFOs for communication
  Fifo recfifo(receive_fifo);
  Fifo sendfifo(send_fifo);
  
  while (1) {

    /* Get a message from a client */
    recfifo.openread();
    string inMessage = recfifo.recv();
	recfifo.fifoclose();
	
	/* Parse the incoming message */
	/* Form:  $type*name  */
	int pos = inMessage.find_first_of("*");
	type = inMessage.substr(0,pos);
	pos = pos + 1;
	
	
	//Create a string that is the search term
	string search = inMessage.substr(pos,2000);
	cout << "Message: " << type << " : " << search << endl;
	
	//If the user wants to search for stems, we search the stem map, if they want to search for specific words we search the refs map
	if (type == "$STEMMED") {
	  positions = stemmedIndexSearch(stems, search);
	} else  if (type == "$SPECIFIC") {
	  positions = indexSearch(refs, search);
	}

	//Clear the position in the infile
	infile.clear();
	string outMessage = "";
	
	//Iterate through the positions vector, sending each line out as a message
	for (unsigned int i = 0; i < positions.size(); i++)
    {
    int pointer = positions[i];
	infile.seekg (pointer, infile.beg);
    getline(infile, outMessage);
	cout << outMessage << endl;
	sendfifo.openwrite();
	sendfifo.send(outMessage);
	sendfifo.fifoclose();
	}
	
	//After all the messages have been sent, send out the $END signal
	outMessage = "$END";
	sendfifo.openwrite();
	sendfifo.send(outMessage);
	sendfifo.fifoclose();
  }
  
  return 0;
}

void stemmer(string& word) 
{
	transform(word.begin(), word.end(), word.begin(), ::toupper);
    
    stemming::english_stem<char, std::char_traits<char> > StemEnglish;    
    
    StemEnglish(word);
}

int build (ifstream& infile, map<string, vector<int> >& refs, map<string, vector<int> >& stems)
{
    string word, line, stemmed;
        
    if (infile.good())
    {
        while(!infile.eof())
        {
//Find the position of the line in the file then get the line
			int pos = infile.tellg();
            getline(infile, line);
            size_t found2 = 0;
            size_t found = line.find(" ");
            while (found != string::npos)
            {
//Break the line down to individual words, create a stemmed version of the word and then input the word and the stem with their position into the respective maps
                word = line.substr(found2, found-found2);
                transform(word.begin(), word.end(), word.begin(), ::toupper);
                refs[word].push_back(pos);
				stemmer(word);
				stems[word].push_back(pos);
				found2 = found + 1;
                found = line.find(" ", found2);
            }
		}
		return 1;
    }
    else
    {
        cout << "File failed to open" << endl;
		return 0;
    }
}

vector <int> indexSearch (map<string, vector<int> > refs, string word)
{
	size_t space = word.find(" ");
	word = word.substr(0, space);
	
    map<string, vector<int> >::iterator it;  // iterator for find                                                                
    vector<int> blank; // return for no matches                                                                                  
    /* find the word and get the vector of references */
    /* First use find, so as to NOT create a new entry */
//The words are stored uppercase, so we must transform our search term to uppercase
	transform(word.begin(), word.end(), word.begin(), ::toupper);
    it = refs.find(word);
 //If the search term is not in the map, tell them it's not there, and if it is, return the vector of the positions of the lines it's in
	if (it == refs.end())
    {
        return(blank);
    } 
    else
    {
        return (refs[word]);
    }
}

vector <int> stemmedIndexSearch (map<string, vector<int> > stems, string word)
{
    size_t space = word.find(" ");
	word = word.substr(0, space);
	
	map<string, vector<int> >::iterator it;  // iterator for find                                                                
    vector<int> blank; // return for no matches                                                                                  
    string save, search;
    /* find the word and get the vector of references */
    /* First use find, so as to NOT create a new entry */
    
    save = word;
    stemmer(word);
    
    cout << save << " was stemmed down to " << word << endl;
    it = stems.find(word);
  
//If the stemmed search term is not in the map, tell them it's not there, and if it is, return the vector of the positions of the lines it's in
	if (it == stems.end())
    {
        return(blank);
    } 
    else
    {
        return (stems[word]);
    }
}