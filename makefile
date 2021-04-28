
CC=gcc
CXX=g++
INCLUDE=http-client/include
HTTP-OBJS=http-client/objs/http-client.o http-client/objs/utils.o


all: http-client mp3player.out parse.out


mp3player.out: mp3player.c
	$(CC) -g -I$(INCLUDE) $^ $(HTTP-OBJS) -lmad -lpulse -lpulse-simple -o $@
	
mp3fileplayer.out: mp3fileplayer.c
	$(CC) -g -I$(INCLUDE) $^ $(HTTP-OBJS) -lmad -lpulse -lpulse-simple -o $@

.PHONY: http-client

http-client:
	@echo "------- Building http-client ------"
	@cd http-client; make
	@echo "------- SUCCESS: built http-client --------"
		
parse.out: parse.c
	$(CC) -o $@ $^




.PHONY: clean
clean:
	@cd http-client; make clean
	@rm *.out *.hex
	
.PHONY: clean-streams
clean-streams:
	rm *.mp3
