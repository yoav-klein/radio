default: build

build: clean
	gcc -Wall -o curl  main.c util.c  -lcurl

clean:
	rm -rf curl 

test: build
	./curl https://freegeoip.app/json/ 
