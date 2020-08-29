// zipsnif:  Simple program to search inside a zip file and return information about the file

// TODO: This has no error handling
// TODO: Handle multiple disks properly
// TODO: proper garbage handling

#include <stdio.h>
#include <stdlib.h>
#include "zip.h"

int main(int argc, char *argv[])
{
	int argChar;
	int argCounter = 1;
	// declaration for sortMethod: contains the sorting method i.e. ascending or decending order.  This is a bitfield.
	uint8_t sortMethod = 0;

	// execution of the program without arguments
	if(argc == 1)
	{
		printf("zipsnif: Utility to examine inside zip file(s) and return information about it.\n");
		printf("\tUseage: zipsnif <arguments> <file(s)>\n");
		printf("\tzipsnif -h for help.\n");
		exit(0);
	}

	// check for arguments
	while(argCounter < argc)
	{
		// checks each argument for a leading -
		if(*((argv[argCounter])) == '-')
		{
			// walks through the argument
			while((argChar = (*++(argv[argCounter]))))
			{
				switch(argChar)
				{
					// help menu is selected
					case 'h':
					case 'H':
						printf("zipsnif: Utility to examine inside zip file(s) and return information about it.\n");
						printf("\tUseage: zipsnif <arguments> <file(s)>\n");
						printf("\tWithout arguments generates a simple file listing.\n");
						printf("\nArguments:\n");
						printf("\t-h\tThis help menu.\n");
						printf("\t-e\tPrint only end of central directory record data.\n");
						printf("\nSorting arguments: (default is ascending sort)\n");
						printf("\t-a\tAscending alphabetical order sorting of file contents.\n");
						printf("\t-d\tDescending alphabetical order sorting of file contents.\n");
						exit(0);
						break;

					// ascending sort method is used.  Mask out DESCENDING
					case 'a':
					case 'A':
						sortMethod &= ~(DESCENDING);
						sortMethod |= ASCENDING;
						break;

					// decending sort method is used.  Mask out ASCENDING
					case 'd':
					case 'D':
						sortMethod &= ~(ASCENDING);
						sortMethod |= DESCENDING;
						break;

					// enable only displaying end of central directory record data
					case 'e':
					case 'E':
						sortMethod |= EOCDRONLY;
						break;

					default:
						fprintf(stderr,"zipsnif: Unknown argument(s)\n");
						exit(1);
						break;
				}
			}
		}
		else
		{
			FILE *zipName;
			struct zipFileDataStructure zipNameStructure;
			zipNameStructure.root = NULL;
			zipNameStructure.fileName = NULL;
			zipNameStructure.endCentralDirectoryRecord.comment = NULL;

			// open files and check for errors in opening
			if((zipName = fopen(argv[argCounter], "rb")) == NULL)
			{
				fprintf(stderr, "zipsnif: Error opening file %s.\n", argv[argCounter]);
				exit(1);
			}

			if(ferror(zipName))
			{
				fprintf(stderr, "zipsnif: Error reading file %s\n.", argv[argCounter]);
				exit(1);
			}

			zipNameStructure.fileName = argv[argCounter];

			// get the end of cd offset and the data from it
			if(findEndOfCentralDirectoryLocation(zipName, &zipNameStructure.locations))
				;
			else
			{
				fprintf(stderr,"zipsnif: Error %s not a zip file.\n", argv[1]);
				exit(1);
			}

			// get the end of central directory record data
			getEndCentralDirectoryData(zipName, &zipNameStructure);

			// evaluate to see if the user requested only the end of central directory record data printed
			if((sortMethod & EOCDRONLY) == EOCDRONLY)
			{
				// print the end of central directory record data
				printEocdr(&zipNameStructure);
			}
			else
			{
				// Set offset CD start and seek file to that location
				struct offsetInfo workingOffset;
				workingOffset.offset = zipNameStructure.endCentralDirectoryRecord.offsetCdStart;
				fseek(zipName, workingOffset.offset, SEEK_SET);

				// obtain data from the file for the CDFH
				for(uint16_t filesRemaining = zipNameStructure.endCentralDirectoryRecord.totalEntries; filesRemaining > 0; --filesRemaining)
				{
					//printf("check = %#x ", sigCheck(zipName, ZIPCDFH));
					if(sigCheck(zipName, ZIPCDFH) == ZIPCDFH)
						workingOffset.offset = getCentralDirectoryData(zipName, &zipNameStructure, workingOffset.offset, &workingOffset);
					else
						workingOffset.status = NOSIG;

					switch(workingOffset.status)
					{
						case ZIPCDFH:
							break;
						case ZIPEOCDR:
							printEocdr(&zipNameStructure);
							break;

						case NOSIG:
							fprintf(stderr, "zipsnif: no readable signature found.\n");
							exit(1);
							break;
						default:
							fprintf(stderr, "Like Ralph said, I'm in danger.\t%#x\n", workingOffset.offset);
							exit(1);
							break;
					}


				}

				// sort the CD
				sortCd(&zipNameStructure, sortMethod);

				// print the CD
				printCdShort(&zipNameStructure);

				// free the file headers
				freeFileHeaderData(&zipNameStructure);
			}

			// close the file
			fclose(zipName);
		}

		argCounter++;
	}

	return 0;
}
