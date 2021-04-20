#!/bin/bash

FILE=$1

if [ ! -f $FILE ] || [ -z $FILE ]; then
	echo "Enter name of existing mp3 file"
	exit
fi

PATTERN="(.*)\.mp3"
if [[ $FILE =~ $PATTERN ]]; then
	echo "Writing to ${BASH_REMATCH[1]}.hex"
	hexdump -C $FILE > "${BASH_REMATCH[1]}.hex"
fi

