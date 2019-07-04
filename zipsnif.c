// zipsnif:  Simple program to search inside a zip file and return information about the file

// TODO: This has no error handling
// TODO: Handle multiple disks properly
// TODO: proper garbage handling

#include <stdio.h>
#include <stdlib.h>
#include "zip.h"

// funciton prototypes

int main(int argc, char *argv[])
{
	if(argc == 1)
	{
		printf("zipsnif: Utility to examine inside a zip and return information about it.\n");
		printf("\tUseage: zipsnif <file>\n");
		exit(0);
	}
	else if(argc > 2)
	{
		fprintf(stderr, "zipsnif: Too many arguments.\n");
		fprintf(stderr, "\tUseage: zipsnif <file>\n");
		exit(1);
	}

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
	sortCd(&zipNameStructure, 1);

	// print the CD
	printCd(&zipNameStructure, 1);

	// free the central directory file headers
	freeCentralDirectoryFileHeaderData(&zipNameStructure);
	// close the file
	fclose(zipName);
	return 0;
}
