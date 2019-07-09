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
	// declaration for sortMethod: contains the sorting method i.e. ascending or decending order.  This is a bitfield.
	uint8_t sortMethod = 0;

	// execution of the program without arguments
	if(argc == 1)
	{
		printf("zipsnif: Utility to examine inside a zip file and return information about it.\n");
		printf("\tUseage: zipsnif <arguments> <file>\n");
		exit(0);
	}

	// check for arguments
	while(--argc > 0)
	{
		if(*(*(++argv)) == '-')
		{
			while((argChar = (*++(*argv))))
			{
				switch(argChar)
				{
					// help menu is selected
					case 'h':
					case 'H':
						printf("zipsnif: Utility to examine inside a zip file and return information about it.\n");
						printf("\tUseage: zipsnif <arguments> <file>\n");
						printf("\nArguments:\n");
						printf("\t-h\tThis help menu.\n");
						printf("\nSorting arguments: (default is ascending sort)\n");
						printf("\t-a\tAscending alphabetical order sorting of file contents.\n");
						printf("\t-d\tDescending alphabetical order sorting of file contents.\n");
						exit(0);
						break;

					// ascending sort method is used.  Mask out DESCENDING
					case 'a':
					case 'A':
						sortMethod &= DESCENDING;
						sortMethod |= ASCENDING;
						break;

					// decending sort method is used.  Mask out ASCENDING
					case 'd':
					case 'D':
						sortMethod &= ASCENDING;
						sortMethod |= DESCENDING;
						break;

					default:
						fprintf(stderr,"zipsnif: Unknown argument(s)\n");
						exit(1);
						break;
				}
			}
		}
	}
/*
	FILE *zipName;

	if((zipName = fopen(argv[1], "r")) == NULL)
	{
		fprintf(stderr, "zipsnif: Error opening file %s.\n", argv[1]);
		exit(1);
	}
	else if(ferror(zipName))
	{
		fprintf(stderr, "zipsnif: Error reading file %s\n.", argv[1]);
		exit(1);
	}

	struct zipFileDataStructure zipNameStructure;
	zipNameStructure.root = NULL;

	// get the end of cd offset and the data from it
	if(findEndOfCentralDirectoryLocation(zipName, &zipNameStructure.locations))
		;
	else
	{
		fprintf(stderr,"zipsnif: Error %s not a zip file.\n", argv[1]);
		exit(1);
	}

	getEndCentralDirectoryData(zipName, &zipNameStructure);

	uint32_t offset = zipNameStructure.endCentralDirectoryRecord.offsetCdStart;

	// obtain data for all the files in the archive cd
	for(uint16_t filesRemaining = zipNameStructure.endCentralDirectoryRecord.totalEntries; filesRemaining > 0; --filesRemaining)
	{
		offset = getCentralDirectoryData(zipName, &zipNameStructure, offset);
	}

	// sort the CD
	sortCd(&zipNameStructure, sortMethod);

	// print the CD
	printCd(&zipNameStructure);

	// free the central directory file headers
	freeCentralDirectoryFileHeaderData(&zipNameStructure);
	// close the file
	fclose(zipName);
*/
	return 0;
}
