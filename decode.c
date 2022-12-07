#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Read and validate decode arguments
 * Input: Command line arguments and decInfo
 * Output: File names are stored in decInfo
 * Return: e_success or e_failure
 */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    // Checks if 2nd argument passed is a .bmp file
    if (strstr(argv[2], ".") != NULL && (strcmp(strstr(argv[2], "."), ".bmp") == 0))
    {
        // stores it in decInfo
        decInfo->stego_image_fname = argv[2];
    }
    else
    {
        printf("INFO : Please mention steged image file correctly Eg:stego.bmp\n");
        // returns failure
        return e_failure;
    }
    // checks if output filename with extension is provided or not
    if (argv[3] != NULL && strstr(argv[3], ".") != NULL && ((strcmp(strstr(argv[3], "."), ".txt") == 0) || (strcmp(strstr(argv[3], "."), ".sh") == 0) || (strcmp(strstr(argv[3], "."), ".c") == 0)))
    {
        // stores it in decInfo
        decInfo->output_fname = argv[3];
    }
    else
    {
        // stores NULL in output_fname to store default filename later after decoding the file extension from steged image
        decInfo->output_fname = NULL;
    }
    return e_success;
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: decInfo
 * Output: FILE pointer for image and output files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_decode_files(DecodeInfo *decInfo)
{
    // Stego Image file
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");
    // Do Error handling
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen ");
        fprintf(stderr, "ERROR : Unable to open file %s\n", decInfo->stego_image_fname);
        return e_failure;
    }
    // if output file name is mentioned
    if (decInfo->output_fname != NULL)
    {
        // output file
        decInfo->fptr_output = fopen(decInfo->output_fname, "w");
    }

    // No failure return e_success
    return e_success;
}

/* Decode magic string from stego image and check
 * Inputs: decInfo
 * Output: Magic string is decoded and checked
 * Return Value: e_success or e_failure
 */
Status decode_magic_string(DecodeInfo *decInfo)
{
    // variable i declared
    uint i;
    // Seek 54 bytes from start to skip bmp header bytes
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);
    // loop runs till size of magic string
    for (i = 0; i < 2; i++)
    {
        // read image data
        fread(decInfo->image_data, sizeof(char), MAX_IMAGE_BUF_SIZE, decInfo->fptr_stego_image);
        // calls decode byte from lsb function
        if (decode_byte_from_lsb(decInfo->decoded_data, decInfo->image_data) == e_success)
        {
            // stores decoded magic string in decInfo
            decInfo->magic_string[i] = decInfo->decoded_data[0];
        }
        else
            return e_failure;
    }
    // null terminates magic string
    decInfo->magic_string[i] = 0;
    // compares it with MAGIC_STRING macro
    if (strcmp(decInfo->magic_string, MAGIC_STRING) == 0)
        return e_success;
    else
        return e_failure;
}

/* Decode data from LSB bits of stego image
 * Input: Image data and destination array decoded data
 * Output: Decodes and stores 1 byte data in decoded_data from 8 bytes image data
 * Return: e_success or e_failure
 */
Status decode_byte_from_lsb(char *decoded_data, char *image_data)
{
    decoded_data[0] = 0;
    // 1 byte data is decoded from LSB of 8 byte image data
    for (int i = 0; i < MAX_IMAGE_BUF_SIZE; i++)
    {
        decoded_data[0] <<= 1;
        decoded_data[0] |= (image_data[i] & 1);
    }
    return e_success;
}

/* Decode file extenstion size from stego image
 * Input: decInfo
 * Output: Decodes the file extenstion size
 * Return: e_success or e_failure
 */
Status decode_file_extn_size(DecodeInfo *decInfo)
{
    char str[32];
    // reads stego image data to str
    fread(str, sizeof(char), 32, decInfo->fptr_stego_image);
    decInfo->size_image_data = 0;
    // calls decode size from lsb function
    if (decode_size_from_lsb(str, &decInfo->size_image_data) == e_success)
    {
        return e_success;
    }
    else
        return e_failure;
}

/* Decodes size from LSB bits of stego image
 * Input: Image data and destination variable pointer
 * Output: Decodes and stores 4 bytes from the 32 bytes image data to destination variable
 * Return: e_success or e_failure
 */
Status decode_size_from_lsb(char *str, uint *size)
{
    // 4 byte data decoded from 32 byte image data
    for (int i = 0; i < 32; i++)
    {
        *size <<= 1;
        *size |= (str[i] & 1);
    }
    return e_success;
}

/* Decode file extenstion from stego image
 * Input: decInfo and file extenstion size
 * Output: Decodes the file extenstion and stores in decInfo
 * Return: e_success or e_failure
 */
Status decode_file_extn(uint size, DecodeInfo *decInfo)
{
    // variable i declared
    uint i;
    // loop runs till size
    for (i = 0; i < size; i++)
    {
        // reads stego image data to decInfo->image_data
        fread(decInfo->image_data, sizeof(char), MAX_IMAGE_BUF_SIZE, decInfo->fptr_stego_image);
        if (decode_byte_from_lsb(decInfo->decoded_data, decInfo->image_data) == e_success)
        {
            // stores decoded data
            decInfo->extn_output_file[i] = decInfo->decoded_data[0];
        }
        else
            return e_failure;
    }
    decInfo->extn_output_file[i] = 0;
    return e_success;
}

/* Decode file size from stego image
 * Input: decInfo
 * Output: Decodes the file size and stores in decInfo
 * Return: e_success or e_failure
 */
Status decode_file_size(DecodeInfo *decInfo)
{
    char str[32];
    // reads data from stego image to str
    fread(str, sizeof(char), 32, decInfo->fptr_stego_image);
    decInfo->size_image_data = 0;
    // calls decode size from lsb
    if (decode_size_from_lsb(str, &decInfo->size_image_data) == e_success)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

/* Decode file data from stego image and write to output file
 * Input: decInfo
 * Output: Decodes the file data and write to output file
 * Return: e_success or e_failure
 */
Status decode_file_data(DecodeInfo *decInfo)
{
    // variable i declared
    uint i;
    // for loop runs till size of file
    for (i = 0; i < decInfo->size_image_data; i++)
    {
        // reads data from stego image to decInfo->image_data
        fread(decInfo->image_data, sizeof(char), MAX_IMAGE_BUF_SIZE, decInfo->fptr_stego_image);
        // calls decode byte from lsb function
        if (decode_byte_from_lsb(decInfo->decoded_data, decInfo->image_data) == e_success)
        {
            // writes decoded data to output file
            fwrite(decInfo->decoded_data, sizeof(char), 1, decInfo->fptr_output);
        }
        else
        {
            return e_failure;
        }
    }

    return e_success;
}

/* Do decoding function
 * Inputs: decInfo
 * Output: Calls each decoding functions one by one and checks if it is executed successfully
 * Return Value: e_success or e_failure
 */
Status do_decoding(DecodeInfo *decInfo)
{
    // Calls each decoding functions one by one and checks if it is executed successfully
    if (open_decode_files(decInfo) == e_success)
    {
        printf("INFO : Files are opened successfully\n");
    }
    else
    {
        printf("ERROR : Opening files failed\n");
        return e_failure;
    }

    if (decode_magic_string(decInfo) == e_success)
    {
        printf("INFO : Decoding Magic string successful\n");
    }
    else
    {
        printf("ERROR : Decoding Magic string failed\n");
        return e_failure;
    }
    if (decode_file_extn_size(decInfo) == e_success)
    {
        printf("INFO : Decoding file extn size successful\n");
    }
    else
    {
        printf("ERROR : Decoding file extn size failed\n");
        return e_failure;
    }
    if (decode_file_extn(decInfo->size_image_data, decInfo) == e_success)
    {
        printf("INFO : Decoding file extn successful\n");
        if (decInfo->output_fname == NULL)
        {
            // Creates default output file with decoded file extension as output file name is not mentioned by user
            char str[12] = "decoded";
            strcat(str, decInfo->extn_output_file);
            decInfo->output_fname = str;
            printf("INFO : Output file name not mentioned / unsupported. Creating %s as default with decoded file extension\n", decInfo->output_fname);
            // opening output file
            decInfo->fptr_output = fopen(decInfo->output_fname, "w");
            printf("INFO : Opened %s\n", decInfo->output_fname);
        }
    }
    else
    {
        printf("ERROR : Decoding file extn failed\n");
        return e_failure;
    }
    if (decode_file_size(decInfo) == e_success)
    {
        printf("INFO : Decoding file size successful\n");
    }
    else
    {
        printf("ERROR : Decoding file size failed\n");
        return e_failure;
    }
    if (decode_file_data(decInfo) == e_success)
    {
        printf("INFO : Decoding file data successful\n");
    }
    else
    {
        printf("ERROR: Decoding file data failed\n");
        return e_failure;
    }
    return e_success;
}