#CSE 5462 Project 1

Authors: James Baker, Eric Olson

##TABLE OF CONTENTS

1. About

2. Installation

3. Usage

4. Help

-----------------------------------------------------------------------------

##1. ABOUT

This application will send a file using TCP sockets implmented with UDP
sockets from a client process to a server process. The client will send
the file in packets to a daemon process on the local machine. The daemon will
forward the packets to a utility called troll running on the local machine.
Troll will then forward the packets to a daemon running on the remote
machine. Finally the remote daemon will send the packets to the remote
server process.

###Checkpoint 1 Updates:

The checksum of packetes sent/received on the tcpd daemon on calculated. If 
they do not match the tcpd server prints an error.

The timer and driver processes work as required.

-----------------------------------------------------------------------------

##2. INSTALLATION

###File transfer

Ensure that 'ftpc.c', 'ftps.c', 'tcpd.c' and 'Makefile' are located in
your desired installation directory. 

Navigate to this directory in your terminal. 

Type 'make' to compile the application. There will now be an executable
client (ftpc), an executable server (ftps), and executable daemon (tcpd) in
the installation directory. 

Ensure that the Troll package is installed in the installation directory.

###Timer and driver

Type 'make timer' to compile the timer and driver programs.

-----------------------------------------------------------------------------

##3. USAGE

###File Transer
Make note of the location of the file you wish to transfer. In the terminal,
navigate to the location of that the program was installed above. 

First start the troll process on the local machine.

~$ Troll 'local troll port'

Here, you need to designate what port troll will run with 'local troll port'.


Next start the tcpd process on the local machine.

~$ tcpd 1 'local host' 'local troll port' 'remote host' 'remote port'

Here, the first argument is 1 to indicate the daemon is run on the local
machine. 'local host' is the host of the local machine. 'local troll port'
is the port from above the troll is running on. 'remote host' is the host
name of the remote machine. 'remote_port' is the port you designate for
communication between troll and the remote machine.


Next start the tcpd process on the remote machine.

~$ tcpd 0 'remote port'

Here, the first argument is 0 to indicate the daemon is run on the remote
machine. 'remote port' is the port specified above.


Next start the server on the remote machine. 

~$ ftps


Lastly, start the client process on the local machine.

~ftpc 'local-file-to-transfer'

Here, 'local-file-to-transfer' is the name (and path if applicable) of 
the local file on the client machine you wish to send to the server.

The file will be sent from the client to the local tcpd, local tcpd to troll,
troll to remote tcpd, and finally remote tcpd to the server.

If any packages are garbled, a checksum error will be displayed.

###Timer and driver

The driver program sends start timer and cancel timer requests to the timer program.
The timer program infinitely listens for requests from the driver program.
The timer program maintains a delta timer, and upon receiving a start timer request
from the driver, inserts a new node into the appropriate location in the delta timer
list. Upon receiving a cancel timer request from the driver, the timer searches for the
node to delete in the delta timer list and removes it if it is found or prints an error
message if it is not. The timer waits a certain amount of time to receive a request from
the driver. If that wait time times out, the timer reduces the amount of time in the head
of the delta timer list by the amount of elapsed time. If the time for the head becomes
less than or equal to 0, the head is removed from the list.

-----------------------------------------------------------------------------

##4. HELP

Below are common errors encountered in the server program.

"There are not enough arguments. Please be sure to include the local port 
number."
 - Please ensure the server execution command includes the correct argument
   as listed above in the Usage section.

"Error opening socket"
 - Ensure that the port arguemnt is correct. Avoid the use of 
   "Well-known TCP Ports"
 - Ensure host is online and accessible.


"Error binding stream socket"
 - Ensure that the port arguemnt is correct. Avoid the use of 
   "Well-known TCP Ports"
 - Ensure host is online and accessible.

"Error connecting stream socket"
 - Ensure that the port arguemnt is correct. Avoid the use of 
   "Well-known TCP Ports"
 - Ensure host is online and accessible.

"Error: The size read returned less than 4"
 - The file size was sent incorrectly or data was lost in transmission. 
   Please ensure file is valid and try again.

"Error: The name read returned less than 20"
 - The file name was sent incorrectly or data was lost in transmission. 
   Please ensure file is valid and try again.

"Error opening the output file"
 - Please ensure that the file sent is valid.
 - Ensure directory application is running from is accessible.

"Error reading from the connection stream. Server terminating"
 - There was an error within the connection stream and data was lost.
   Please try again.

Below are common errors encountered in the client program. Ensure that
the server was executed first or else unexpected behavior may occur.

"Error: Include host in arguments, port, and local file to transfer in 
arguments"
 - Please ensure the client execution command includes the correct arguments
   as listed above in the Usage section.

"Error opening socket"
 - Ensure that the host and port arguemnts are correct. Avoid the use of 
   "Well-known TCP Ports"
 - Ensure host is online and accessible.

"<Host>: unknown host"
 - Ensure that the host and port arguemnts are correct. Avoid the use of 
   "Well-known TCP Ports"
 - Ensure host is online and accessible.

"Error connecting stream socket."
 - Ensure that the host and port arguemnts are correct. Avoid the use of 
   "Well-known TCP Ports"
 - Ensure host is online and accessible.

"Error opening file"
 - Please ensure the file exists, the name was entered correctly, and if 
   applicable that the path was entered correctly.

"Error reading file"
 - Please ensure the file exists, the name was entered correctly, and if 
   applicable that the path was entered correctly.

-----------------------------------------------------------------------------

This README was written on Mon Sep 19 12:39 2016.

-----------------------------------------------------------------------------

