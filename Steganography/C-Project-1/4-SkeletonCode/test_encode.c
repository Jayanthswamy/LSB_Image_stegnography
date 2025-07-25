#include <stdio.h>
#include "encode.h"
#include "types.h"
#include<string.h>
#include"decode.h"

int main(int argc, char *argv[])
{
	EncodeInfo encInfo;
	DecodeInfo decInfo;
	
	//check_operation function
    int res = check_operation_type(argv);


	if(res == e_encode)
	{
		printf("Selected encoding\n");
		if(argc == 4 || argc == 5)
		{
			//read and validate function
			if(read_and_validate_encode_args(argv,&encInfo) == e_success)
			{
				printf("Read and validation is done\n");

				// do_encoding function
				if(do_encoding(&encInfo) == e_success)
				{
					printf("do_encoding is done\n");
				}
				else
				{
					printf("do_encoding is failed\n");
				}	
			}

			else
			{
				printf("Read and validation is Failed\n");
			}
		}
		else
		{
			printf("pass the correct arguments 5 or 4\n");
		}

	}
	
	else if(res == e_decode)
	{
		printf("Selected decoding\n");
		if(argc == 3 || argc == 4)
		{
			if(read_and_validate_decode_args(argv,&decInfo) == e_success)
			{
				printf("read_validate_decode_args is secessfull\n");
				if(do_decoding(&decInfo) == e_success)
				{
					printf("do_decoding is sucessfull\n");
				}
				else
				{
					printf("do_decoding is failed\n");
				}	
			}
			else
			{
				printf("read_validate_decode_args is failed\n");
			}
		}
		else
		{
			printf("Pass the correct argument\n");
		}

	}
	else 
	{
		printf("Error : For Encoding : ./a.out -e beautiful.bmp secret.txt\n");
		printf("Error : For decoding : ./a.out -d output.bmp\n");
	}

    return 0;
}


OperationType check_operation_type (char *argv[])
{
	if(argv[1] != NULL)
	{
		
		if(!strcmp (argv[1] ,"-e" ))
		{
			return e_encode;
		}
		else if(!strcmp (argv[1],"-d"))
		{
			return e_decode;
		}
		else
		{
			return e_unsupported;
		}
	}
	else
		return e_unsupported;
}
