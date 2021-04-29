
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <mad.h>
#include <pulse/simple.h>
#include <pulse/error.h>

#include <unistd.h>

#include "http-client.h" // http connection

pa_simple *device = NULL;
int ret = 1;
int error;
struct mad_stream mad_stream;
struct mad_frame mad_frame;
struct mad_synth mad_synth;

void output(struct mad_header const *header, struct mad_pcm *pcm);

int main(int argc, char **argv) {
    // Parse command-line arguments
   /* if (argc != 2) {
        fprintf(stderr, "Usage: %s [filename.mp3]", argv[0]);
        return 255;
    }*/

    // Set up PulseAudio 16-bit 44.1kHz stereo output
    static const pa_sample_spec ss = { .format = PA_SAMPLE_S16LE, .rate = 48000, .channels = 2 };
    if (!(device = pa_simple_new(NULL, "MP3 player", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error))) {
        printf("pa_simple_new() failed\n");
        return 255;
    }

    // Initialize MAD library
    mad_stream_init(&mad_stream);
    mad_synth_init(&mad_synth);
    mad_frame_init(&mad_frame);

    /*// Filename pointer
    char *filename = argv[1];

    // File pointer
    FILE *fp = fopen(filename, "r");
    int fd = fileno(fp);

    // Fetch file size, etc
    struct stat metadata;
    if (fstat(fd, &metadata) >= 0) {
        printf("File size %d bytes\n", (int)metadata.st_size);
    } else {
        printf("Failed to stat %s\n", filename);
        fclose(fp);
        return 254;
    }

    // Let kernel do all the dirty job of buffering etc, map file contents to memory
    char *input_stream = (char*)mmap(0, metadata.st_size, PROT_READ, MAP_SHARED, fd, 0);

    // Copy pointer and length to mad_stream struct
    mad_stream_buffer(&mad_stream, input_stream, metadata.st_size);

	*/
	
	
	
	struct http_handle *handle = init_connection("https://kanliveicy.media.kan.org.il/icy/kanbet_mp3", NULL);
	struct chunk chunk = { 0 };
	
	unsigned char *last = NULL;
    // Decode frame and synthesize loop
    chunk = read_chunk(handle->sock);
    fprintf(stderr, "chunk size: %ld\n", chunk.size);
    mad_stream_buffer(&mad_stream, chunk.data, chunk.size);
    
    while (1) {
		#define FRAME_SIZE (384)
		size_t unused = mad_stream.bufend - mad_stream.this_frame;
		fprintf(stderr, "loop: unused: %lu\n", unused);
		if(unused <= (FRAME_SIZE * 2))
		{
			fprintf(stderr, "SMALLER: unused: %lu\n", unused);
			unsigned char tmp[FRAME_SIZE * 2] = { 0 };
			//printf("tmp: %x %x %x %x\n", tmp[0], tmp[1], tmp[2], tmp[3]);
			if(unused > 0)
			{
				//printf("Copying to tmp: %p\n", mad_stream.this_frame);
				memcpy(tmp, mad_stream.this_frame, unused);
				
			//	printf("tmp: %x %x %x %x\n", tmp[0], tmp[1], tmp[2], tmp[3]);
			}
			fprintf(stderr, "last in tmp - tmp[unused]: %x\n", tmp[unused - 1]);
			
			free(chunk.data);
			chunk = read_chunk(handle->sock);	
			chunk.data = realloc(chunk.data, chunk.size + unused);
			fprintf(stderr, "chunk.data[0]: %x\n", ((unsigned char*)chunk.data)[0]);
			if(NULL == chunk.data)
			{
				perror("realloc");
			}
			
			if(unused > 0)
			{
				//printf("before move - chunk.data: %x %x %x %X\n", chunk.data[0], chunk.data[1], chunk.data[2], chunk.data[3]);
				memmove(chunk.data + unused, chunk.data, chunk.size);
				
				memcpy(chunk.data, tmp, unused);
				//printf("after copy - chunk.data: %x %x %x %x\n", ((unsigned char*)chunk.data)[0], chunk.data[1], chunk.data[2], chunk.data[3]);
			}
			fprintf(stderr, "new buffer: chunk.data[unused - 1]: %x, chunk.data[unused]: %x\n", ((unsigned char*)chunk.data)[unused - 1], ((unsigned char*)chunk.data)[unused]);
			mad_stream_buffer(&mad_stream, chunk.data, chunk.size + unused);
			
		}
		//printf("this_frame: %p\n", mad_stream.this_frame);
		//printf("bufend: %p\n", mad_stream.bufend);
        // Decode frame from the stream
        int written_bytes = 0;
        while(written_bytes < FRAME_SIZE)
        {
        	written_bytes = write(1, mad_stream.this_frame + written_bytes, FRAME_SIZE - written_bytes);
        }
        if (mad_frame_decode(&mad_frame, &mad_stream)) {
        	fprintf(stderr, "*********ERROR***********\n");
            fprintf(stderr, "%s\n", mad_stream_errorstr(&mad_stream));
            if (MAD_RECOVERABLE(mad_stream.error)) {
                continue;
            } else if (mad_stream.error == MAD_ERROR_BUFLEN) {
            	//printf("MAD_ERROR_BUFLEN\n");
                continue;
            } else {
                break;
            }
        }
        // Synthesize PCM data of frame
        mad_synth_frame(&mad_synth, &mad_frame);
        output(&mad_frame.header, &mad_synth.pcm);
    }

    // Close
   // fclose(fp);

    // Free MAD structs
    mad_synth_finish(&mad_synth);
    mad_frame_finish(&mad_frame);
    mad_stream_finish(&mad_stream);

    // Close PulseAudio output
    if (device)
        pa_simple_free(device);

    return EXIT_SUCCESS;
}

// Some helper functions, to be cleaned up in the future
int scale(mad_fixed_t sample) {
     /* round */
     sample += (1L << (MAD_F_FRACBITS - 16));
     /* clip */
     if (sample >= MAD_F_ONE)
         sample = MAD_F_ONE - 1;
     else if (sample < -MAD_F_ONE)
         sample = -MAD_F_ONE;
     /* quantize */
     return sample >> (MAD_F_FRACBITS + 1 - 16);
}
void output(struct mad_header const *header, struct mad_pcm *pcm) {
    register int nsamples = pcm->length;
    mad_fixed_t const *left_ch = pcm->samples[0], *right_ch = pcm->samples[1];
    static char stream[1152*4];
    if (pcm->channels == 2) {
        while (nsamples--) {
            signed int sample;
            sample = scale(*left_ch++);
            stream[(pcm->length-nsamples)*4 ] = ((sample >> 0) & 0xff);
            stream[(pcm->length-nsamples)*4 +1] = ((sample >> 8) & 0xff);
            sample = scale(*right_ch++);
            stream[(pcm->length-nsamples)*4+2 ] = ((sample >> 0) & 0xff);
            stream[(pcm->length-nsamples)*4 +3] = ((sample >> 8) & 0xff);
        }
        if (pa_simple_write(device, stream, (size_t)1152*4, &error) < 0) {
            fprintf(stderr, "pa_simple_write() failed: %s\n", pa_strerror(error));
            return;
        }
    } else {
        printf("Mono not supported!");
    }
}
