#include <stdio.h>
#include <stdlib.h>

#define HEADER_SIZE (4)

int main(int argc, char ** argv) {
  FILE * file;
  unsigned char header[4];
  size_t bytes_read = 0;
  int bitrates[2][16] = {
  { 0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0},  // MPEG 2 Layer 3
  { 0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0}}; // MPEG 1 Layer 3
  int samplerates[4][3] = { {11025, 12000, 8000}, {0, 0, 0}, {22050, 24000, 16000}, {44100, 48000, 32000} }; 
  int resync = 0;
  int after_tag = 0;
  int version0, layer0, bitrate0, sample0;
  int first_frame = 0;
  int last_valid_header = -1;

  file = fopen(argv[1], "r");
  if (!file)
    exit(1);
  if (argc == 3)
    fseek(file, atoi(argv[2]),SEEK_SET);

while (1) {
	printf("DEBUG: Last valid header: %d\n", last_valid_header);
    bytes_read = fread(&header, 4 , 1, file);
	
    if (bytes_read == 1) { /* this is actually not bytes but members */
      if ( header[0] == 0xFF && (header[1] & 0xE0) == 0xE0) {
        printf("DEBUG: Found header at %ld\n", ftell(file) - HEADER_SIZE);
        int version, layer, bitrate, sample, padding, flen;
     
        version = (header[1] & 0x18) >> 3;
        layer   = (header[1] & 0x06) >> 1;
        bitrate = bitrates[version & 0x1][(header[2] & 0xF0) >> 4];
        sample  = samplerates[version][(header[2] & 0x0C) >> 2];
        
        printf("DEBUG: Version: %d, layer: %d, bitrate: %d, sample: %d\n",
         2 - (version&0x1), 4 - layer, bitrate, sample);
        if ((! first_frame && version != 1 && layer != 0 && bitrate != 0 && sample != 0) || (first_frame && 
              version == version0 && layer == layer0 && bitrate == bitrate0 && sample == sample0)) {
          printf("DEBUG: In the if, first_frame: %d, currently at: %ld\n", first_frame, ftell(file)); 
          padding = (header[2] & 0x02) >> 1;
          flen    = (version % 2 == 0 ? 72 : 144) * (1000*bitrate)/sample + padding;
          
          if (resync) {
            printf(" (length = %ld bytes)\nFound header at pos %ld\n",
                ftell(file) - 4 - (last_valid_header > -1 ? last_valid_header : 0), ftell(file) - 4);
            resync = 0;
          }
          
          if (!first_frame) {
            printf("MPEG%d layer %d, %d kbps, %d kHz, %d B/frame\n",2-(version&0x1),4-layer,bitrate,sample,flen-padding);
            version0 = version;
            layer0 = layer;
            bitrate0 = bitrate;
            sample0 = sample;
            first_frame = 1;
          }
          if (after_tag) {
            printf("Next piece starts at %ld\n", ftell(file) - 4);
            after_tag = 0;
          }
          last_valid_header = ftell(file) - HEADER_SIZE;
          fseek(file, flen - 4, SEEK_CUR);
        }
      }
      else if (header[0] == 'T' && header[1] == 'A' && header[2] == 'G') {
          printf("Found ID3v1 tag at pos %ld\n", ftell(file) -4);
          fseek(file, 124, SEEK_CUR);
          after_tag = 1;
        }
      else if (header[0] == 'I' && header[1] == 'D' && header[2] == '3') {
        int length = 0;
        char v[2];
        char s[4];
        fread(&v, 1, 2, file);
        fread(&s, 1, 4, file);
        length |= s[3];
        length |= (s[2] <<  7);
        length |= (s[1] << 15);
        length |= (s[0] << 23);
        printf("Found ID3v2.%d.%d tag at pos %ld (%d bytes)\n", header[3], v[0], ftell(file) - 10, length + 10);
        fseek(file, length, SEEK_CUR);
      }
      else {
      	printf("DEBUG: Didn't find header in: %ld!\n", ftell(file) - HEADER_SIZE);
        if (!resync) {
          if (!after_tag) {
            printf("Broken frame starting at %d\n", last_valid_header > -1 ? last_valid_header : 0);
            if (last_valid_header == -1)
              // Beginning of file
              fseek(file, last_valid_header + 1, SEEK_SET);
            else
              // Not at the beginning of the file
              fseek(file, last_valid_header + 4, SEEK_SET);
          }
          else {
            printf("Next piece starts at %ld with a broken frame", ftell(file) - 4);
            last_valid_header = ftell(file) - 4;
          }
          
          printf("DEBUG: Setting resync to 1\n");
          resync = 1;
        }
        
        printf("DEBUG: fseek\n");
        fseek(file, -3, SEEK_CUR);
      }
    }
    else {
      if (feof(file)) {
        if (after_tag)
          printf("File ends with a TAG at %ld\n", ftell(file));
        else
          printf("File ends NOT with a TAG\n");

        exit(0);
      }
      else {
        printf("bytes_read = %lu\n", bytes_read);
        exit(1);
      }
    }
  }
  return 0;
}
