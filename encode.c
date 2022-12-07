#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Read and validate encode arguments
 * Input: Command line arguments and encInfo
 * Output: File names are stored in encInfo
 * Return: e_success or e_failure
 */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // Checks if 2nd argument passed is a .bmp file
    if (strstr(argv[2], ".") != NULL && (strcmp(strstr(argv[2], "."), ".bmp") == 0))
    {
        // stores it in encInfo
        encInfo->src_image_fname = argv[2];
    }
    else
    {
        printf("INFO : Please mention image file correctly Eg:beautiful.bmp\n");
        // returns failure
        return e_failure;
    }
    // Checks if 3rd argument passed is a .txt , .c or .sh file
    if (strstr(argv[3], ".") != NULL && ((strcmp(strstr(argv[3], "."), ".txt") == 0 || strcmp(strstr(argv[3], "."), ".c") == 0 || strcmp(strstr(argv[3], "."), ".sh") == 0)))
    {

        // stores it in encInfo
        encInfo->secret_fname = argv[3];
    }
    else
    {
        printf("INFO : Please mention secret file correctly Eg:secret.txt\n");
        // returns failure
        return e_failure;
    }
    // checks if output filename with .bmp extension is provided or not
    if (argv[4] != NULL && strstr(argv[4], ".") != NULL && ((strcmp(strstr(argv[4], "."), ".bmp") == 0)))
    {
        // stores it in encInfo
        encInfo->stego_image_fname = argv[4];
    }
    else
    {
        // stores default name in  encInfo
        printf("INFO : Output filename not mentioned / mentioned without .bmp extension. Creating stego.bmp as default\n");
        encInfo->stego_image_fname = "stego.bmp";
    }
    return e_success;
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Open Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen ");
        fprintf(stderr, "ERROR : Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Open Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen ");
        fprintf(stderr, "ERROR : Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    // Open Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen ");
        fprintf(stderr, "ERROR : Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

/*
 * Check capacity of src image
 * Inputs: encInfo
 * Output: Checks if capacity of source image is greater than data to be encoded
 * Return Value: e_success or e_failure
 */
Status check_capacity(EncodeInfo *encInfo)
{
    // Gets image and secret file sizes
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    strcpy(encInfo->extn_secret_file, strstr(encInfo->secret_fname, "."));
    // Checks if capacity of source image is greater than data to be encoded
    if (encInfo->image_capacity > (strlen(MAGIC_STRING) * 8 + 32 + strlen(encInfo->extn_secret_file) * 8 + 32 + (encInfo->size_secret_file * 8)))
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);

    // Return image capacity
    return width * height * 3;
}

/* Get file size
 * Input: File pointer
 * Output: File size
 * Return: File size in bytes is returned
 */
uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);
    return ftell(fptr);
}

/* Copy bmp header of src image to stego image
 * Inputs: Source and Stego image file pointers
 * Output: Bmp header of src image is copied to  stego image
 * Return Value: e_success or e_failure
 */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    // string with 54 bytes
    char str[54];
    // rewinds src image pointer to start
    rewind(fptr_src_image);
    // copies bmp header to str and writes to stego image
    fread(str, sizeof(char), 54, fptr_src_image);
    fwrite(str, sizeof(char), 54, fptr_dest_image);
    return e_success;
}

/* Encode magic string to stego image
 * Inputs: Magic string and encInfo
 * Output: Magic string is encoded to stego image
 * Return Value: e_success or e_failure
 */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    // calls encode data to image function
    if (encode_data_to_image(magic_string, strlen(magic_string), encInfo->fptr_src_image, encInfo->fptr_stego_image, encInfo) == e_success)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

/* Encode function, which does the real encoding
 * Inputs: Data to encode,encInfo, source and stego image file pointers
 * Output: Data is encoded to stego image
 * Return Value: e_success or e_failure
 */
Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image, EncodeInfo *encInfo)
{
    // for loop that runs till the passed size
    for (int i = 0; i < size; i++)
    {
        // read 8 byte image data from src image
        fread(encInfo->image_data, sizeof(char), 8, fptr_src_image);
        // passes it to encode_byte_to_lsb and checks if it returns e_success
        if (encode_byte_to_lsb(data[i], encInfo->image_data) == e_success)
        {
            // writes to stego image
            fwrite(encInfo->image_data, sizeof(char), 8, fptr_stego_image);
        }
        // if encode_byte_to_lsb doesnt return e_success
        else
            // returns e_failure
            return e_failure;
    }
    return e_success;
}

/* Encode a byte into LSB of image data array
 * Inputs: 1 byte data to encode and 8 byte image data
 * Output: Data is encoded to image buffer
 * Return Value: e_success or e_failure
 */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    // mask variable declared and initialised
    uint mask = 1 << 7;
    // 1 byte data is encoded to LSB of 8 byte image data
    for (int i = 0; i < 8; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data & mask) >> (7 - i));
        mask >>= 1;
    }
    return e_success;
}

/* Encode secret file extension size to stego image
 * Inputs: Size to encode, src and stego image file pointers
 * Output: Data is encoded to stego image
 * Return Value: e_success or e_failure
 */
Status encode_secret_file_extn_size(int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char str[32];
    fread(str, sizeof(char), 32, fptr_src_image);
    // passes str and size to encode_size_to_lsb and checks if it returns e_success
    if (encode_size_to_lsb(str, size) == e_success)
    {
        // writes to stego image
        fwrite(str, sizeof(char), 32, fptr_stego_image);
        return e_success;
    }
    else
        return e_failure;
}

/* Encode size into LSB of image data array
 * Inputs: 4 byte data to encode and 32 byte image data
 * Output: Data is encoded to image buffer
 * Return Value: e_success or e_failure
 */
Status encode_size_to_lsb(char *buffer, int size)
{
    // mask variable declared and initialised
    uint mask = 1 << 31;
    // 4 byte data encoded to 32 byte image data
    for (int i = 0; i < 32; i++)
    {
        buffer[i] = (buffer[i] & 0xFE) | ((size & mask) >> (31 - i));
        mask >>= 1;
    }
    return e_success;
}

/* Encode secret file extension to stego image
 * Inputs: File extension and encInfo
 * Output: Data is encoded to stego image
 * Return Value: e_success or e_failure
 */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    // calls encode data to image function
    if (encode_data_to_image(file_extn, strlen(file_extn), encInfo->fptr_src_image, encInfo->fptr_stego_image, encInfo) == e_success)
        return e_success;
    else
        return e_failure;
}

/* Encode secret file size to stego image
 * Inputs: File size to encode and encInfo
 * Output: Data is encoded to stego image
 * Return Value: e_success or e_failure
 */
Status encode_secret_file_size(int file_size, EncodeInfo *encInfo)
{
    char str[32];
    fread(str, sizeof(char), 32, encInfo->fptr_src_image);
    // passes str and file_size to encode_size_to_lsb
    if (encode_size_to_lsb(str, file_size) == e_success)
    {
        // writes to stego image
        fwrite(str, sizeof(char), 32, encInfo->fptr_stego_image);
        return e_success;
    }
    else
        return e_failure;
}

/* Encode secret file data to stego image
 * Inputs: encInfo
 * Output: Data is encoded to stego image
 * Return Value: e_success or e_failure
 */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    // a buffer is created with size equal to secret file size
    char secret_buff[encInfo->size_secret_file];
    // sets secret file ptr back to the start
    fseek(encInfo->fptr_secret, 0, SEEK_SET);
    // reads data to buffer from secret file
    fread(secret_buff, sizeof(char), encInfo->size_secret_file, encInfo->fptr_secret);
    // calls encode data to image function
    if (encode_data_to_image(secret_buff, encInfo->size_secret_file, encInfo->fptr_src_image, encInfo->fptr_stego_image, encInfo) == e_success)
        return e_success;
    else
        return e_failure;
}

/* Copy remaining image data to output image 
 * Inputs: Src and stego image file pointers
 * Output: Remaining image data is copied to stego image
 * Return Value: e_success or e_failure
 */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    // Remaining image data is copied to stego image
    char ch;
    while (fread(&ch, sizeof(char), 1, fptr_src) > 0)
    {
        fwrite(&ch, sizeof(char), 1, fptr_dest);
    }
    return e_success;
}

/* Do encoding function
 * Inputs: encInfo
 * Output: Calls each encoding functions one by one and checks if it executed successfully
 * Return Value: e_success or e_failure
 */
Status do_encoding(EncodeInfo *encInfo)
{
    // Calls each encoding functions one by one and checks if it executed successfully else returns error
    if (open_files(encInfo) == e_success)
    {
        printf("INFO : Files are opened successfully\n");
    }
    else
    {
        printf("ERROR : Opening files failed\n");
        return e_failure;
    }
    if (check_capacity(encInfo) == e_success)
    {
        printf("INFO : Check capacity function successfully done\n");
    }
    else
    {
        printf("ERROR : Check capacity failed\n");
        return e_failure;
    }
    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
    {
        printf("INFO : Copying image header successful\n");
    }
    else
    {
        printf("ERROR : Copying image header failed\n");
        return e_failure;
    }
    if (encode_magic_string(MAGIC_STRING, encInfo) == e_success)
    {
        printf("INFO : Encoding Magic string done\n");
    }
    else
    {
        printf("ERROR : Encoding Magic string failed\n");
        return e_failure;
    }
    if (encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
    {
        printf("INFO : Encoding secret file extn size is success\n");
    }
    else
    {
        printf("ERROR : Encoding secret file extn size is failed\n\n");
        return e_failure;
    }
    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
    {
        printf("INFO : Encoding secret file extn is success\n");
    }
    else
    {
        printf("ERROR : Encoding secret file extn is failed\n");
    }
    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
    {
        printf("INFO : Encoding secret file size is success\n");
    }
    else
    {
        printf("ERROR : Encoding secret file size is failed\n");
    }
    if (encode_secret_file_data(encInfo) == e_success)
    {
        printf("INFO : Encoding secret file data is success\n");
    }
    else
    {
        printf("ERROR : Encoding secret file data is failed\n");
    }
    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
    {
        printf("INFO : Copying remaining image data is success\n");
    }
    else
    {
        printf("ERROR : Copying remaining image data is failed\n");
    }
    return e_success;
}
