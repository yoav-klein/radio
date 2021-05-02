#!/bin/bash


############################################
#	
#	read_stream.sh
#	--------------
#
#	DESCRIPTION: This script reads MP3 stream from a remote server
#			   and outputs it to a file or stdout
#
#	SYNOPSIS:
#		$ read_stream.sh [options] URL
#
#	OPTIONS:
#		--raw - print raw output
#		
#		-o,--outfile <file path> - print output to file
#
#		-v,--verbose - verbose
#
#		-h,--headers <file path> - print response headers to file
#
####################################

while [ -n "$1" ]; do
case "$1" in
--headers | -h) 
	if ! [[ $2 =~ ^[a-zA-Z]+[a-zA-Z0-9]*\.*[a-zA-Z]+[a-zA-Z0-9]*$ ]]; then
		echo "--headers <file path>"
		exit
	fi
	headers_file_path=$2
	opts+="-D $headers_file_path"
	shift
	shift
	;;
--verbose | -v)
	opts+=" -v "
	shift
	;;
--outfile | -o)
	if ! [[ $2 =~ ^[a-zA-Z]+[a-zA-Z0-9]*\.*[a-zA-Z]+[a-zA-Z0-9]*$ ]]; then
		echo "--outfile <file path>"
		exit
	fi
	outfile=$2
	shift
	shift
	;;
--raw)
	opts+="--raw"
	shift
	;;

*)
	URL=$1
	shift
	;;
esac
done

if [ -z $URL ]; then
	echo "Usage: read_stream.sh <URL>"
	exit
fi

if [ -z $outfile ]; then
	outfile=/dev/tty
fi

curl $opts http://kanliveicy.media.kan.org.il/icy/kanbet_mp3 > $outfile


