#include<stdio.h>
#include "encode.h"
#include <string.h>

#include "types.h"


/* Function Definitions */

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
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
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
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
	perror("fopen");
	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
	perror("fopen");
	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
	perror("fopen");
	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // Validate the source image filename
    if (strstr(argv[2], ".bmp") == NULL)
    {
	printf("argv[2] does not have .bmp extension\n");
	return e_failure;
    }
    encInfo->src_image_fname = argv[2];

    // Validate the secret filename
    if (argv[3] == NULL || strstr(argv[3], ".") == NULL)
    {
	printf("argv[3] does not have a valid file name\n");
	return e_failure;
    }
    encInfo->secret_fname = argv[3];

    // Validate the stego image filename
    if (argv[4] != NULL)
    {
	if (strstr(argv[4], ".bmp") == NULL)
	{
	    printf("argv[4] is not a .bmp file\n");
	    return e_failure;
	}
	encInfo->stego_image_fname = argv[4];
    }
    else
    {
	encInfo->stego_image_fname = "output.bmp";
    }

    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    printf("--------> Encoding process started <-------\n");
    // Open files for encoding
    if (open_files(encInfo) != e_success) 
    {
	printf("Failed to open files\n");
	return e_failure;
    }
    printf("Files opened successfully\n");



    // Read the magic string from the user
    printf("ENTER THE MAGIC STRING: ");
    scanf("%s", encInfo->magic_string);

    // Check capacity
    if (check_capacity(encInfo) != e_success) 
    {
	printf("Insufficient capacity\n");
	return e_failure;
    }
    printf("Capacity check successful\n");



    // Copy BMP header
    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success) 
    {
	printf("Failed to copy BMP header\n");
	return e_failure;
    }
    printf("BMP header copied successfully\n");

    // Encode the magic string length
    if (encode_secret_file_size(encInfo->magic_string_len, encInfo) != e_success) 
    {
	printf("Failed to encode magic string length\n");
	return e_failure;
    }
    printf("Magic string length encoded successfully\n");

    // Encode the magic string
    if (encode_magic_string(encInfo->magic_string, encInfo) != e_success) 
    {
	printf("Failed to encode magic string\n");
	return e_failure;
    }
    printf("Magic string encoded successfully\n");

    // Encode secret file extension size
    if (encode_secret_file_size(encInfo->secret_extn_size, encInfo) != e_success)
    {
	printf("Failed to encode secret file extension size\n");
	return e_failure;
    }
    printf("Secret file extension size encoded successfully\n");

    // Encode secret file extension
    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) != e_success) 
    {
	printf("Failed to encode secret file extension\n");
	return e_failure;
    }
    printf("Secret file extension encoded successfully\n");

    // Encode secret file data size
    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) != e_success) 
    {
	printf("Failed to encode secret file data size\n");
	return e_failure;
    }
    printf("Secret file data size encoded successfully\n");

    // Encode secret file data
    if (encode_secret_file_data(encInfo) != e_success) 
    {
	printf("Failed to encode secret file data\n");
	return e_failure;
    }
    printf("Secret file data encoded successfully\n");

    // Copy remaining image data
    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success) 
    {
	printf("Failed to copy remaining image data\n");
	return e_failure;
    }
    printf("Remaining image data copied successfully\n");

    return e_success;
}
Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->magic_string_len = strlen(encInfo->magic_string);
    
    // Get secret file extension and its size
    char *secret_ext = strstr(encInfo->secret_fname, ".");
    if (secret_ext != NULL)
    {
        strcpy(encInfo->extn_secret_file, secret_ext);
        encInfo->secret_extn_size = strlen(secret_ext);
    }
    else
    {
        printf("Error: Secret file does not have an extension.\n");
        return e_failure;
    }

    // Find secret file size
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    // Check capacity
    if (encInfo->image_capacity > 54 + 32 + (encInfo->magic_string_len * 8) + 
	    32 + (encInfo->secret_extn_size * 8) + 32 + (encInfo->size_secret_file * 8))
    {
        return e_success;
    }
    return e_failure;
}


uint get_file_size(FILE *fptr)
{
    fseek(fptr,0,SEEK_END);
    return ftell(fptr);
}




Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char bmp_header[54];

    fseek(fptr_src_image, 0, SEEK_SET);
    fread(bmp_header, 1,54, fptr_src_image);
    fwrite(bmp_header, 1,54, fptr_dest_image);

    if(54 == get_file_size(fptr_dest_image))
    {

	return e_success;
    }
    else
    {
	return e_failure;
    }
}





Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{


    if(encode_data_to_image(encInfo->magic_string,strlen(magic_string),encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_success)
    {
	printf("encode_data_to_image is sucessfull\n");
	return e_success;
    }
    else
    {
	printf("encode_data_to_image is failed\n");
	return e_failure;
    }
}




Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{

    char image_buffer[8];

    for(int i=0; i<size; i++)
    {
	fread(image_buffer,1,8,fptr_src_image);
	encode_byte_to_lsb(data[i],image_buffer);
	fwrite(image_buffer,1,8,fptr_stego_image);
    }
    return e_success;

}

/* Encode a byte into LSB of image data array */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    int index=0;
    for(int i=7; i>=0 ;i--)
    {
	//udating the bit in lsb,editing in image buffer and storeing in image buffer
	image_buffer[index] = (image_buffer[index] & ~1) | ((data & (1<<i)) >> i);//modify
	index++;
    }
    return e_success;

}

Status encode_secret_file_size(int file_size, EncodeInfo *encInfo)
{

    char ext_buffer[32];
    int index=0;

    fread(ext_buffer,1,32,encInfo->fptr_src_image);

    //integer to lsb
    for(int i=31; i>=0 ; i--)
    {
	ext_buffer[index] = (ext_buffer[index] & ~1) | ((file_size & (1<<i)) >> i);//modify
	index++;
    }
    fwrite(ext_buffer,1,32,encInfo->fptr_stego_image);
    return e_success;

}


Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{

    if(encode_data_to_image(encInfo->extn_secret_file,encInfo->secret_extn_size,encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_success)
    {
	return e_success;
    }
    else
    {
	return e_failure;
    }
}


Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char data_buff[encInfo->size_secret_file];
    fseek(encInfo->fptr_secret, 0, SEEK_SET); // new


    fread(data_buff,1,encInfo->size_secret_file,encInfo->fptr_secret);

    if(encode_data_to_image(data_buff,encInfo->size_secret_file,encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_success)
    {
	return e_success;
    }
    else
    {
	return e_failure;
    }
}


Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    int output_file_size = get_file_size(fptr_dest);
    int src_file_size = get_file_size(fptr_src);
    int size = src_file_size-output_file_size;

    fseek(fptr_src,output_file_size,SEEK_SET);

    char rem_buff[size];
    fread(rem_buff,1,size,fptr_src);
    fwrite(rem_buff,1,size,fptr_dest);

    return e_success;
}
