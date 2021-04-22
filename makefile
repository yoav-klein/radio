
CC=gcc
CXX=g++

all: http-client.out mp3player.out parse.out

mp3player.out: mp3player.cpp
	$(CXX) $^ -lmpg123 -lao -o $@

http-client.out: client.c http-client-c.h
	$(CC) $^ -o $@ 
	
	
parse.out: parse.c
	$(CC) -o $@ $^


.PHONY: clean
clean:
	rm *.out *.mp3 *.hex
	
.PHONY: clean-streams
clean-streams:
	rm *.mp3
