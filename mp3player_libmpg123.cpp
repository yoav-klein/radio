#include <ao/ao.h>
#include <stdio.h>
#include <mpg123.h>


#include <unistd.h> // read
#include <sys/types.h> /* open */
#include <sys/stat.h> /* open */
#include <fcntl.h> /* open */


extern "C"
{
#include "http-client.h" /* open_connection */
}
#define BITS 8





int main(int argc, char *argv[])
{
    mpg123_handle *mh;
    char *buffer;
    size_t buffer_size;
    size_t done;
    int err;

    int driver;
    ao_device *dev;

    ao_sample_format format;
    int channels, encoding;
    long rate;
    
    /* initializations */
    ao_initialize();
    driver = ao_default_driver_id();
    mpg123_init();
    mh = mpg123_new(NULL, &err);
    buffer_size = mpg123_outblock(mh);
    buffer = (char*) malloc(buffer_size * sizeof(unsigned char));

    /* open the file and get the decoding format */
    /*mpg123_open(mh, argv[1]);*/
    mpg123_open_feed(mh);
    
    mpg123_getformat(mh, &rate, &channels, &encoding);

    /* set the output format and open the output device */
    format.bits = mpg123_encsize(encoding) * BITS;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;
    dev = ao_open_live(driver, &format, NULL);

	
	struct http_handle *handle = init_connection("https://kanliveicy.media.kan.org.il/icy/kanbet_mp3", NULL);
	struct chunk chunk = { 0 };
	
	
	/************************/
	// read from file into buffer
	/**************************/
	/*int fd = open("/home/yoav/playground/radio/curl_stream.mp3", 0, O_RDONLY);
	if(-1 == fd)
	{
		perror("open");
		
		exit(1);
	}
	#define BUFSIZE 800000
	unsigned char* buf = (unsigned char*)malloc(BUFSIZE);
	if(NULL == buf)
	{
		perror("malloc");
		exit(1);
	}
	int read_bytes = 0;
	while(read_bytes < BUFSIZE)
	{
		int n = read(fd, buf + read_bytes, BUFSIZE - read_bytes);
		read_bytes += n;
	}
	int status = mpg123_feed(mh, (const unsigned char*)buf/*chunk.data*//*, BUFSIZE);
  	if(status != MPG123_OK)
  	{
 	 	printf("ERROR in mpg123_feed\n");
 	 	exit(1);
  	}
  	*/
  	/*********************************/
  	
  	
  	printf("START LOOP\n");
    /* decode and play */
    std::size_t count = 0;
    while (1)
    {
    		 
	    	chunk = read_chunk(handle->sock); // chunk has a member data that contains MP3 data
	   	int status = mpg123_feed(mh, (const unsigned char*)chunk.data, chunk.size); 
	   	if(MPG123_OK != status)
		{
		  mpg123_strerror(mh);
		}
		status = mpg123_read(mh, (unsigned char*)buffer, buffer_size, &done);
		if(MPG123_OK == status)
		{
		  printf("Playing.. %lu buffer: %p\n", count++, buffer);
		 ao_play(dev, buffer, done);
		}
		else
		{
		  mpg123_strerror(mh);
		}
		
	}
    /* clean up */
    free(buffer);
    ao_close(dev);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    ao_shutdown();

    return 0;
}
