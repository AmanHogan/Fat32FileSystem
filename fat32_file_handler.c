/*
	Aman Hogan-Bailey
	1001830469

*/

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>


// Initializing delimeters
// Max command line size
// Max number of arguments
#define MAX_NUM_ARGUMENTS 5
#define WHITESPACE " \t\n" 
#define MAX_COMMAND_SIZE 255 


// Initializing global starting address
int32_t starting_address;

// Directory Structure Defintion
struct __attribute__((__packed__)) DirectoryEntry
{
  char DIR_Name[11];
  uint8_t DIR_Attr;
  uint8_t unused[8];
  uint16_t DIR_FirstClusterHigh;
  uint8_t unused1[4];
  uint16_t DIR_FirstClusterLow;
  uint32_t DIR_FileSize;
};

// Initializing global BPB variables
int16_t BPB_BytesPerSec;
int8_t BPB_SecPerClus;
int16_t BPB_RsvdSecCnt;
int8_t BPB_NumFATs;
int32_t BPB_FATSz32;
char saved_filename[100];


// 'compare' Function compares filename with the Fat32 filename convention
// 'file_directory_exists' Function validates that File Directory Exists
// 'LBAToOffset' changes from a logical address to a physical address
int compare(char IMG_Name[], char input[]);
int file_directory_exists(struct DirectoryEntry dir[], char token[]);
int LBAToOffset(int sector);

int main()
{
	// Initializing variable that willl determine if a file is currently open or not
	int is_file_open = 0;
	FILE *fp;
	
	// A structure to hold all the attributes of the directory.
	struct DirectoryEntry dir[16];
	
	// Command Line String Argument
	char *cmd_str = (char *)malloc(MAX_COMMAND_SIZE);

	// While the program has not been exited...
	while (1)
	{
		// Print out the mfs prompt
		printf("mfs> ");

		// Read the command from the commandline and wait here until the user inputs something 
		while (!fgets(cmd_str, MAX_COMMAND_SIZE, stdin));

		// Parse input
		char *token[MAX_NUM_ARGUMENTS];

		// Pointer to point to the token
		// keep track of its original value so we can deallocate
		int token_count = 0;

		char *arg_ptr;
		char *working_str = strdup(cmd_str);
		char *working_root = working_str;

		// Tokenize the input stringswith whitespace used as the delimiter
		while(((arg_ptr=strsep(&working_str, WHITESPACE))!= NULL)&&(token_count < MAX_NUM_ARGUMENTS))
		{
			token[token_count] = strndup(arg_ptr, MAX_COMMAND_SIZE);
			if (strlen(token[token_count]) == 0)
			{
				token[token_count] = NULL;
			}
				
			token_count++;
		}

		// If nothing is entered, continue asking for input
		if (token[0] == NULL)
		{
			continue;
		}
		
		// If user enters 'Quit' or 'quit', program forces exit
		else if (strcmp("quit", token[0]) == 0 || strcmp("Quit", token[0]) == 0)
		{
			exit(0);
		}


		// If the user enters "open", open the file specifed
		else if (strcmp("open", token[0]) == 0)
		{
			// If nothing was specified after 'open', then show error
			if (token[1] == NULL)
			{
				printf("Error: File system image not foun\n");
				continue;
			}
			
			// Attempt to open the file
			else
			{
				
				fp = fopen(token[1], "r");
				// If file does not exist, show error
				if (fp == NULL)
				{
					printf("Error: File system image not found.\n");
					continue;
				}
				
				// If file is already open, show error
				else if (is_file_open == 1)
				{
					printf("Error: File system image already open.\n");
					continue;
				}
				
				// If no errors, read file
				else
				{
					// Reading File information and stroing them 
					fseek(fp, 11, SEEK_SET);
					fread(&BPB_BytesPerSec, 2, 1, fp);					
					fseek(fp, 13, SEEK_SET);
					fread(&BPB_SecPerClus, 1, 1, fp);					
					fseek(fp, 14, SEEK_SET);
					fread(&BPB_RsvdSecCnt, 1, 2, fp);
					fseek(fp, 16, SEEK_SET);
					fread(&BPB_NumFATs, 1, 1, fp);					
					fseek(fp, 36, SEEK_SET);
					fread(&BPB_FATSz32, 1, 4, fp);


					// Using formula to calculate the root address
					starting_address=(BPB_BytesPerSec*BPB_RsvdSecCnt)+(BPB_NumFATs*BPB_FATSz32*BPB_BytesPerSec);
					fseek(fp, starting_address, SEEK_SET);
					int i = 0;
					for (i = 0; i < 16; i++)
					{
						fread(&dir[i], sizeof(dir[i]), 1, fp);
					}

					// Set variable to 1, stating that a file is currently open
					is_file_open = 1;
				}
			}
		}
		

		// If the user entered 'close', clos the file that is currently open
		else if (strcmp("close", token[0]) == 0)
		{
			// If the file is open, close the file
			if (is_file_open == 1)
			{
				// Close the file, reset fp, and reset is_file_open
				fclose(fp);
				
				is_file_open = 0;
			}
			
			else
			{
				printf("Error: File system not open.\n");
			}
	
			continue;
		}


		// Notify useer that they can only use any other command if 
		// a file is curently open. 
		else if (((strcmp("undel", token[0])==0)||
					(strcmp("read", token[0])==0) ||
					(strcmp("ls", token[0])==0)|| 
					(strcmp("info", token[0])==0)||
					(strcmp("cd", token[0])==0) ||
					(strcmp("get", token[0])==0)|| 
					(strcmp("delete", token[0])==0) || 
					(strcmp("stat", token[0])==0)) && 
					(is_file_open==0))
		{
			printf("Error: File system image must be opened first.\n");
		}

		// If a command is entered, and the file is open, then proceed...
		else if (((strcmp("undel", token[0]) == 0) || 
					(strcmp("read", token[0]) == 0) ||
					(strcmp("ls", token[0]) == 0)|| 
					(strcmp("info", token[0]) == 0)|| 
					(strcmp("cd", token[0]) == 0) ||
					(strcmp("get", token[0]) == 0)|| 
					(strcmp("delete", token[0]) == 0) || 
					(strcmp("stat", token[0]) == 0)) && 
					(is_file_open==1))
		{
						// If user enters 'ls', print the files in the current directory
			if (strcmp("ls", token[0]) == 0)
			{
				int i = 0;
				// Traverse directory size
				for(i = 0; i < 16; i++)
				{
					char null_term_string[12];
					memset(&null_term_string, 0, 12);
					
					// If file is READ ONLY, ARCHIVE, a DIRECTORY, and is NOT a deleted file
					// print it to the console
					if ((dir[i].DIR_Attr == 0x01 || dir[i].DIR_Attr == 0x10 || dir[i].DIR_Attr == 0x20 ||
               dir[i].DIR_Attr == 0x30) && dir[i].DIR_Name[0] != (signed char)0xe5)
					{
						strncpy(null_term_string, dir[i].DIR_Name, 11);
						printf("%s\n", null_term_string);
						
					}
					continue;
				}
			}
			
			// If user entered 'info', print out the infromation about the file
			// in base 10 and in hexadecimal
			else if (strcmp("info", token[0])== 0)
			{
				printf("BPB_BytesPerSec : %10d\t", BPB_BytesPerSec);
				printf("%10x\n", BPB_BytesPerSec);
				printf("BPB_SecPerClus : %10d\t", BPB_SecPerClus);
				printf("%10x\n", BPB_SecPerClus);
				printf("BPB_RsvdSecCnt : %10d\t", BPB_RsvdSecCnt);
				printf("%10x\n", BPB_RsvdSecCnt);
				printf("BPB_NumFATs : %10d\t", BPB_NumFATs);
				printf("%10x\n", BPB_NumFATs);
				printf("BPB_FATSz32 : %10d\t", BPB_FATSz32);
				printf("%10x\n", BPB_FATSz32);
				printf("\n");
				continue;
			}




			// If user enters 'stat', print out the attributes
			else if (strcmp("stat", token[0]) == 0)
			{
				if(token[1] == NULL)
				{
					printf("Too few arguments for 'stat'.\n");
					continue;
				}
				// Initializing the index of the file in file directory
				int file_index = 0;
				
				// Find if the file directory exists, and if it does, return the index
				file_index = file_directory_exists(dir, token[1]);
				
				// If index is out of bounds, then file did not exist
				if (file_index == -1)
				{
					printf("Error: File not found.\n");
				}
				
				// If file exists in directory and has an index, print it out along
				// with its attributes
				else
				{
					printf("Attribute: %d\nSize: %d\nFirst Cluster Low: %d\n",dir[file_index].DIR_Attr,
					dir[file_index].DIR_FileSize, dir[file_index].DIR_FirstClusterLow );
				}
				continue;
			}
			
			// If the user enters 'read', the program will read the file given the specified position and specified bytes
			else if (strcmp("read", token[0]) == 0)
			{
				// If user enters too few argumetns for read, notify them
				if (token[1] == NULL || token[2] == NULL || token[3] == NULL)
				{
					printf("Error: too few arguments for 'read'\n");
				}
				
				// If user enter a valid command line, then read the specified file
				else
				{
					// Check to see that the file or directory in the current directory exists
					int index_counter= file_directory_exists(dir,token[1]);
					
					// If it does not exist, notify the user
					if(index_counter==-1)
					{
						printf("Error: File not found \n");
					}
					
					
					else
					{
						// Convert string input to number input
						// FInd how many bytes the user wants to read
						int total_bytes = atoi(token[3]);
						int position = atoi(token[2]);
						int cluster_size = dir[index_counter].DIR_FirstClusterLow;
						fseek(fp, position + LBAToOffset(cluster_size), SEEK_SET);
						char *temp_str = (char *)malloc(total_bytes);
						
						// Read the number of bytes specified by the user
						fread(temp_str,total_bytes,1,fp);
						printf("%s\n",temp_str);
						free(temp_str);
					}
				}
			}
			
			// If user enters 'cd', move the current directory to the specified directory
			else if (strcmp("cd", token[0]) == 0)
			{
				// Initialize variable that decides whether a directory was found or not
				int directory_was_found = 0;
				
				// If there are no arguments after 'cd', notify user
				if (token[1] == NULL)
				{
					printf("Error: Too few arguments for 'cd'\n");
				}
				
				// If the user entered '.' or '..' after 'cd', then take action:
				else
				{
					// Initialize file directory index
					int directory_index = 0;

					// If the user entered '.' or '..' after 'cd', change current directory
					if (!strcmp(token[1], "..") || !strcmp(token[1], "."))
					{
						// While the index is less than max file size, loop through file directory
						for(directory_index = 0; directory_index < 16; directory_index++)
						{	
							if (strstr(dir[directory_index].DIR_Name, token[1]) != NULL)
							{
								// If parent and root directory are the same ...
								if (dir[directory_index].DIR_FirstClusterLow == 0)
								{
									dir[directory_index].DIR_FirstClusterLow = 2;
								}
								
								int sub_directory_index = 0;
								fseek(fp, LBAToOffset(dir[directory_index].DIR_FirstClusterLow), SEEK_SET);
								for (sub_directory_index = 0; sub_directory_index < 16; sub_directory_index++)
								{
									fread(&dir[sub_directory_index], sizeof(dir[sub_directory_index]), 1, fp);
								}
								break;
							}
						}
					}
					
					// If neither '.' or '..', were entered, then use the token as the filename,
					// Check to see if file exists, and switch to that directory
					else
					{
						// Initialize filename
						char file_name[100];
							
						// Loop through file directory...
						for(directory_index = 0; directory_index < 16; directory_index++)
						{
							// Store token into filename
							strncpy(file_name, token[1], strlen(token[1]));
							
							// If file in file directory has DIECTORY ATTRIBUTE and has the same name as a directory, from
							// the compare function, then change directory							
							if (dir[directory_index].DIR_Attr != 0x20 && compare(dir[directory_index].DIR_Name, file_name))
							{
								// Change to the specified directory
								fseek(fp, LBAToOffset(dir[directory_index].DIR_FirstClusterLow), SEEK_SET);
								int i = 0;
								for (i = 0; i < 16; i++)
								{	
									fread(&dir[i], sizeof(dir[i]), 1, fp);
								}
								
								// A driectory was found
								directory_was_found = 1;
								break;
							}
						}
						
						// If no directory was found, notify the user
						if (directory_was_found == 0)
						{
							printf("Error: Directory not found.\n");
						}
					}
				}
       
				continue;
			}

			// If the user enters 'get', take the file from the fat32 and put it into the program's working directory
			else if (strcmp("get", token[0]) == 0)
			{
				// If nothing was entered after 'get', show that too few arguments were printed
				// Plz dont crash here....
				if (token[1] == NULL)
				{
					printf("Too few arguments for 'get'\n");
				}
				
				
				else
				{
					// Find if the file or directory in file directory exists
					int index_of_file = file_directory_exists(dir, token[1]);
					
					// If the entity does not exist, tell the user
					if (index_of_file == -1)
					{
						printf("Error: File not found.\n");
					}
					
					// If file or directory does exist, move that file to working directory
					else
					{
						FILE *file_handle;
						
						// Initializing the cluster and the size of file
						int dir_size = dir[index_of_file].DIR_FileSize;
						int cluster_size = dir[index_of_file].DIR_FirstClusterLow;
						
						// Open that file and read it, and write it to current directory
						file_handle = fopen(token[1], "w");
						fseek(fp, LBAToOffset(cluster_size), SEEK_SET);
						char *temp_ptr = (char *)malloc(dir_size);
						fread(temp_ptr, dir_size, 1, fp);
						fwrite(temp_ptr, dir_size, 1, file_handle);
						free(temp_ptr);
						fclose(file_handle);
					}
				}
			}
			
			// If user enters undelete, undelete the file that is specified
			else if(strcmp("undel", token[0])==0)
			{
				// user did not specefiy a name
				if(token[1] == NULL)
				{
					printf("Error: to few arguments for 'undel'.\n");
				}
				
				// Undelete file
				else
				{
					// Loop through file directory, if the file entered is the sa
					int pos= 0;
					for(pos= 0; pos< 16; pos++)
					{
						if (dir[pos].DIR_Name[0] == '?' )
						{
							strncpy(dir[pos].DIR_Name, saved_filename, strlen(saved_filename));
					      dir[pos].DIR_Attr = 0x1;
						}
					}

				}
				
			}

			// If the user enters 'delete', the item will be deleted
			else if(strcmp("delete", token[0])==0)
			{
				// If nothing is entered after deleter, notify user
				if(token[1] == NULL)
				{
					printf("Error: Too few arguments for 'delete'.\n");
				}
				
				
				int file_index = 0;
				// Get index of the file specified
				file_index = file_directory_exists(dir, token[1]);
				
				// If index is out of bounds, then file or directry did not exist
				if(file_index == -1)
				{
					printf("Directory or File did not exist.\n");
				}
				
				// Set file entry to '0xe5'
				else
				{
					strncpy(saved_filename, dir[file_index].DIR_Name, 11);
					dir[file_index].DIR_Name[0] = '?';
					dir[file_index].DIR_Attr = 0;
				}
			}
		}
	 
		// If an invalid command was entered, notify user
		else
		{
			printf("Error: Unknown command.\n");
			continue;
		}
		free(working_root);
	}
	
	return 0;
}

// Functtion uses LBAT formula
int LBAToOffset(int sector)
{
	return ((sector - 2) * BPB_BytesPerSec) + 
				(BPB_BytesPerSec * BPB_RsvdSecCnt) + 
				(BPB_NumFATs * BPB_FATSz32 * BPB_BytesPerSec);
}

// Function compares the directory name witht the parameter name
// Returns: '1' for matching, '0' for no match
// Parameters: name of directory and unformatted filename
int compare(char IMG_Name[], char input[])
{
	// Creating temporary array that will hold the input array
	char temp_array[12];
	strcpy(temp_array, input);
	
	// Initializing size of expaneded name
	char expanded_name[12];
	memset(expanded_name, ' ', 12);

	// Tokenizing, getting rid of extension
	char *token = strtok(temp_array, ".");
	strncpy(expanded_name, token, strlen(token));
	token = strtok(NULL, ".");

	if (token)
	{
		strncpy((char *)(expanded_name + 8), token, strlen(token));
	}
	
	// Null terminating expanded name
	expanded_name[11] = '\0';
	
	int i;
	// Upercasing the expanded name for convention
	for (i = 0; i < 11; i++)
	{
		expanded_name[i] = toupper(expanded_name[i]);
	}

	// If a match was found, return 1
	if (strncmp(expanded_name, IMG_Name, 11) == 0)
	{
		return 1;
	}

  // If no match was found, return 0
  return 0;
}


// Function validates that File Directory Exists
// Parameters: file diretory, unformatted filename
// Returns: position of file in DirectoryEntry dir
int file_directory_exists(struct DirectoryEntry dir[], char token[])
{
  // Loop through size of file, 
  // and if the file exists, return that position
  int position = 0;
  for(position = 0; position < 16; position++)
  {
		// If the File Entry attributes are valid, return the file entry position
		// Attributes checked: READ ONLY, DIRECTORY, ARCHIVE, VALID NAME 
		if ((dir[position].DIR_Name[0] != (signed char)0xe5) && 
			(compare(dir[position].DIR_Name, token)) && (dir[position].DIR_Name[0] == 0x2e|| 
			dir[position].DIR_Attr == 0x20 || dir[position].DIR_Attr == 0x10 || 
			dir[position].DIR_Attr == 0x01 ))
		{
			return position;
		}
  }
  
  // If file or directory is not found, return an 'out of bounds position'
  return -1;
}
