all:
	gcc -o ftps ftps.c capitalFunctions.c
	gcc -o ftpc ftpc.c capitalFunctions.c
	gcc -o tcpd tcpd.c

timer:
	gcc -o timer timer.c
	gcc -o driver driver.c

clean_timer:
	rm driver
	rm timer

clean:
	rm ftps
	rm ftpc
	rm tcpd
	
