#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    int i = 0;

    // Count the number of arguments
    while (argv[i] != NULL) 
    {
        i++;
    }

    // Check for valid argument count
    if (i < 4) 
    {
        printf("Invalid number of arguments\nTry passing 4 arguments\n");
        return e_failure;
    }

    // Validate BMP file extension
    if (strstr(argv[2], ".bmp") == NULL) 
    {
        printf("Invalid file\nPlease provide the file with extension .bmp\n");
        return e_failure;
    }

    // Set output filename
    if (i >= 4) 
    {
        strncpy(decInfo->output_fname, argv[3], sizeof(decInfo->output_fname) - 1);
    } 
    else 
    {
        strncpy(decInfo->output_fname, "output", sizeof(decInfo->output_fname) - 1);
    }
    decInfo->output_fname[sizeof(decInfo->output_fname) - 1] = '\0'; // Ensure null termination

    // Safely copy input BMP filename
    strncpy(decInfo->stego_image_fname, argv[2], sizeof(decInfo->stego_image_fname) - 1);
    decInfo->stego_image_fname[sizeof(decInfo->stego_image_fname) - 1] = '\0';

    printf("Successfully validated\n");

    return e_success;
}


// Function to open the stego image file in binary mode
Status opende_files(DecodeInfo *decInfo)
{
    // Open the BMP file in binary mode
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    
    // Error handling for file opening
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);
        return e_failure;
    }

    return e_success;
}

// Function to decode data from stego image
Status do_decoding(DecodeInfo *decInfo)
{
    // Open the stego image file
    if (opende_files(decInfo) == e_failure)
    {
        printf("ERROR: Unable to open stego image file.\n");
        return e_failure;
    }

    // Skip the BMP header
    if (skip_bmp_header(decInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR: Failed to skip BMP header.\n");
        return e_failure;
    }

    // Decode magic string length
    if (decode_magic_stringlen(decInfo) == e_failure)
    {
        printf("ERROR: Failed to decode magic string length.\n");
        return e_failure;
    }

    // Decode magic string
    if (decode_magic_string(decInfo->magic_string, decInfo) == e_failure)
    {
        printf("ERROR: Failed to decode magic string.\n");
        return e_failure;
    }

    // Validate the magic string (ask user for input)
    char str[100];
    printf("Enter the magic string: ");
    scanf("%[^\n]", str);
    
    if (strcmp(str, decInfo->magic_string) != 0)
    {
        printf("ERROR: Invalid magic string entered.\n");
        return e_failure;
    }

    // Decode secret file extension length
    if (decode_secret_ext_len(decInfo) == e_failure)
    {
        printf("ERROR: Failed to decode secret file extension length.\n");
        return e_failure;
    }

    // Decode the secret file extension
    char file_extn[100];
    if (decode_secret_file_extn(file_extn, decInfo) == e_failure)
    {
        printf("ERROR: Failed to decode secret file extension.\n");
        return e_failure;
    }

    // Append the decoded extension to output filename
    strcat(decInfo->output_fname, file_extn);

    // Open the output file for writing
    decInfo->fptr_output_file = fopen(decInfo->output_fname, "w");
    if (decInfo->fptr_output_file == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->output_fname);
        return e_failure;
    }

    // Decode the size of the secret file
    if (decode_secret_file_size(decInfo) == e_failure)
    {
        printf("ERROR: Failed to decode secret file size.\n");
        return e_failure;
    }

    // Decode the secret file data
    if (decode_secret_file_data(decInfo) == e_failure)
    {
        printf("ERROR: Failed to decode secret file data.\n");
        return e_failure;
    }

    // Close all opened files
    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_output_file);

    return e_success;
}

// Function to skip the BMP header (first 54 bytes)
Status skip_bmp_header(FILE *fptr_stego_image)
{
    // Skip the BMP header by seeking to byte 54
    fseek(fptr_stego_image, 54, SEEK_SET);
    return e_success;
}

// Function to decode the length of the magic string
// Function to decode the length of the magic string
Status decode_magic_stringlen(DecodeInfo *decInfo)
{
    char secret_key_len[32];

    // Read 32 bits for the magic string length from the stego image
    fread(secret_key_len, 32, 1, decInfo->fptr_stego_image);

    // Decode the 32-bit integer for the magic string length
    decode_byte_to_int(&decInfo->magic_string_len, secret_key_len);

    return e_success;
}

// Function to decode a 32-bit integer from LSBs
Status decode_byte_to_int(int *data, char *image_arr)
{
    *data = 0;

    // Loop through 32 bits and accumulate the value
    for (int i = 0; i < 32; i++)
    {
        // Extract the LSB from each byte and shift it into the correct position
        *data = (*data << 1) | (image_arr[i] & 1);
    }

    return e_success;
}

// Function to decode the magic string
Status decode_magic_string(char *secret_key, DecodeInfo *decInfo)
{
    int length = decInfo->magic_string_len;
    char array[8];  // Buffer for 8 bits
    char ch;
    int i;

    // Loop through the length of the magic string
    for (i = 0; i < length; i++)
    {
        // Read 8 bits from the stego image
        fread(array, 8, 1, decInfo->fptr_stego_image);

        // Decode the 8 bits to a character
        decode_byte_to_lsb(&ch, array);

        // Store the decoded character in the secret key
        secret_key[i] = ch;
    }

    // Null-terminate the magic string
    secret_key[i] = '\0';

    return e_success;
}

// Function to decode a byte (8 bits) from LSBs
Status decode_byte_to_lsb(char *data, char *image_arr)
{
    *data = 0;

    // Loop through 8 bits and accumulate the value
    for (int i = 0; i < 8; i++)
    {
        // Extract the LSB from each byte and shift it into the correct position
        *data = (*data << 1) | (image_arr[i] & 1);
    }

    return e_success;
}

// Function to decode the length of the secret file extension
Status decode_secret_ext_len(DecodeInfo *decInfo)
{
    char file_ext_len[32];

    // Read the length of the file extension (32 bits)
    fread(file_ext_len, 32, 1, decInfo->fptr_stego_image);
    
    // Decode the 32-bit integer for extension length
    decode_byte_to_int(&decInfo->file_exten_size, file_ext_len);
    
    return e_success;
}

// Function to decode the secret file extension
Status decode_secret_file_extn(char *file_extn, DecodeInfo *decInfo)
{
    int length = decInfo->file_exten_size;  // Size is already in bytes
    char buffer[8];
    char ch;
    int i;

    // Read and decode the extension
    for (i = 0; i < length; i++)
    {
        fread(buffer, 8, 1, decInfo->fptr_stego_image);
        decode_byte_to_lsb(&ch, buffer);
        file_extn[i] = ch;
    }

    file_extn[i] = '\0';  // Null-terminate the extension
    return e_success;
}

// Function to decode the size of the secret file
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char file_data_len[32];

    // Read the file size (32 bits)
    fread(file_data_len, 32, 1, decInfo->fptr_stego_image);
    
    // Decode the file size
    decode_byte_to_int(&decInfo->sec_file_size, file_data_len);
    
    return e_success;
}

// Function to decode the secret file data
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    int file_size = decInfo->sec_file_size;
    char buffer[8];
    char ch;
    
    // Decode each byte of the secret file data
    for (int i = 0; i < file_size; i++)
    {
        fread(buffer, 8, 1, decInfo->fptr_stego_image);
        decode_byte_to_lsb(&ch, buffer);
        fputc(ch, decInfo->fptr_output_file);
    }
    
    return e_success;
}
