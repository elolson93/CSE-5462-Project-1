all:
	gcc -o ftps ftps.c capitalFunctions.c
	gcc -o ftpc ftpc.c capitalFunctions.c
	gcc -o tcpd tcpd.c

clean:
	rm ftps
	rm ftpc
	rm tcpd
