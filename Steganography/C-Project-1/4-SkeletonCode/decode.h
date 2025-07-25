#ifndef DECODE_H
#define DECODE_H

#include "types.h"
#include <stdio.h>
#include <string.h>

/* 
 * Structure to store information required for
 * decoding the secret file from the stego image.
 * Info about output and intermediate data is
 * also stored.
 */
typedef struct _DecodeInfo
{
    char stego_image_fname[100];    // Name of the stego image
    FILE *fptr_stego_image;         // File pointer to the stego image
    int magic_string_len;           // Length of the magic string
    int file_exten_size;            // Size of the file extension
    int data;                       // Data placeholder
    char magic_string[100];         // Magic string
    int sec_file_size;              // Size of the secret file

    /* Output file */
    char output_fname[100];         // Name of the output file
    FILE *fptr_output_file;         // File pointer to the output file

} DecodeInfo;

/* OperationType - might be an enum for different operation types */
OperationType check_operation_type(char *argv[]);

/* Read and validate the decoding arguments from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Get file pointers for input (stego image) and output files */
Status opende_files(DecodeInfo *decInfo);

/* Skip BMP image header */
Status skip_bmp_header(FILE *fptr_stego_image);

/* Decode the length of the magic string */
Status decode_magic_stringlen(DecodeInfo *decInfo);

/* Decode the magic string */
Status decode_magic_string(char *magic_string, DecodeInfo *decInfo);

/* Decode secret file extension length */
Status decode_secret_ext_len(DecodeInfo *decInfo);

/* Decode secret file extension */
Status decode_secret_file_extn(char *file_extn, DecodeInfo *decInfo);

/* Decode the secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Decode the secret file data */
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Decode a byte from the least significant bits of the image buffer */
Status decode_byte_to_lsb(char *data, char *image_buffer);

/* Decode an integer from the least significant bits of the image buffer */
Status decode_byte_to_int(int *data, char *image_buffer);

#endif // DECODE_H
