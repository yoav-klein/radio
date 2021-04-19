
CC=gcc
CXX=g++

mp3player.out: mp3player.cpp
	$(CXX) $^ -lmpg123 -lao -o $@

http-client.out: client.c
	$(CC) $^ -o $@ 
