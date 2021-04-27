#include <ao/ao.h>
#include <stdio.h>
#include <mpg123.h>

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

    if(argc < 2)
        exit(0);

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
	
	
    /* decode and play */
    while (1)
    {
    		char *fake = NULL;
    	  fprintf(stderr, "LOOP\n");
    	  chunk = read_chunk(handle->sock);
	  fprintf(stderr, "chunk size: %ld\n", chunk.size);
	  
	  int status = mpg123_feed(mh, (const unsigned char*)chunk.data, chunk.size);
	  if(status != MPG123_OK)
	  {
	  	printf("ERROR in mpg123_feed\n");
	  	exit(1);
	  }
    	  if(mpg123_read(mh, (unsigned char*)buffer, buffer_size, &done) == MPG123_OK)
    	  {
         printf("buffer size: %li\n", buffer_size);
         ao_play(dev, buffer, done);
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
