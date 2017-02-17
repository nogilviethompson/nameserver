#MakeFile to build and deploy the Sample US CENSUS Name Data using ajax
# For CSC3004 Software Development

# Put your user name below:
USER= ogilviethompsonh

CC= g++

#For Optimization
#CFLAGS= -O2
#For debugging
CFLAGS= -g

RM= /bin/rm -f

all: Shakeserver ShakeTest Shakefetchajax PutCGI PutHTML
#all: Shakeserver ShakeTest

ShakeTest.o: ShakeTest.cpp fifo.h
	$(CC) -c $(CFLAGS) ShakeTest.cpp

Shakeserver.o: Shakeserver.cpp fifo.h
	$(CC) -c $(CFLAGS) Shakeserver.cpp

Shakefetchajax.o: Shakefetchajax.cpp fifo.h
	$(CC) -c $(CFLAGS) Shakefetchajax.cpp

ShakeTest: ShakeTest.o fifo.o
	$(CC) ShakeTest.o fifo.o -o ShakeTest

Shakeserver: Shakeserver.o fifo.o
	$(CC) Shakeserver.o  fifo.o -o Shakeserver

fifo.o:		fifo.cpp fifo.h
		g++ -c fifo.cpp
Shakefetchajax: Shakefetchajax.o  fifo.h
	$(CC) Shakefetchajax.o  fifo.o -o Shakefetchajax -L/usr/local/lib -lcgicc

PutCGI: Shakefetchajax
	chmod 757 Shakefetchajax
	cp Shakefetchajax /usr/lib/cgi-bin/$(USER)_Shakefetchajax.cgi 

	echo "Current contents of your cgi-bin directory: "
	ls -l /usr/lib/cgi-bin/

PutHTML:
	cp shakespeare_ajax.html /var/www/html/class/softdev/$(USER)
	cp shakespeare_ajax.css /var/www/html/class/softdev/$(USER)

	echo "Current contents of your HTML directory: "
	ls -l /var/www/html/class/softdev/$(USER)

clean:
	rm -f *.o shakespeare_ajax Shakeserver ShakeTest
