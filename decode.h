#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types

/*
 * Structure to store information required for
 * decoding secret file from steged Image
 * Info about output and intermediate data is
 * also stored
 */

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 5

typedef struct _DecodeInfo
{

	/* Decoded output secret file info */
	char *output_fname;
	FILE *fptr_output;
	char extn_output_file[MAX_FILE_SUFFIX];
	char decoded_data[MAX_SECRET_BUF_SIZE];

	/*stego image info */
	char *stego_image_fname;
	FILE *fptr_stego_image;
	uint size_image_data;
	char image_data[MAX_IMAGE_BUF_SIZE];
	char magic_string[3];
} DecodeInfo;

/* Decoding function prototype */

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the decoding*/
Status do_decoding(DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Status open_decode_files(DecodeInfo *decInfo);

/* Decode Magic String */
Status decode_magic_string(DecodeInfo *decInfo);

/* Decode secret file extenstion size */
Status decode_file_extn_size(DecodeInfo *decInfo);

/* Decode secret file extenstion */
Status decode_file_extn(uint size, DecodeInfo *decInfo);

/* Decode secret file size */
Status decode_file_size(DecodeInfo *decInfo);

/* Decode secret file data */
Status decode_file_data(DecodeInfo *decInfo);

/* Decode byte from LSBs of image data array */
Status decode_byte_from_lsb(char *decode_data, char *image_data);

/* Decode size from LSBs of image data array */
Status decode_size_from_lsb(char *str, uint *size);

#endif
