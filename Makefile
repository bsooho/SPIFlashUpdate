main : main.c IS25LP256.c IS25LP256.h
	cc -o main main.c IS25LP256.c -lwiringPi -lgpiod
