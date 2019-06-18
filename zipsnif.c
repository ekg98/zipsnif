// zipsnif:  Simple program to search inside a zip file and return information about the file

#include <stdio.h>
#include <stdlib.h>
#include "zip.h"

void findEndOfCentralDirectoryLocation(FILE *, struct zipDataLocations *);
void getEndCentralDirectoryData(FILE *, struct zipFileDataStructure *);

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

	findEndOfCentralDirectoryLocation(zipName, &zipNameStructure.locations);
	getEndCentralDirectoryData(zipName, &zipNameStructure);

	//printf("Central Directory File Header is %ld bytes from beginning of file %s.\n", zipNameStructure.locations.centralDirectoryFileHeader, argv[1]);
	printf("End of central directory record is %ld bytes from beginning of file %s.\n", zipNameStructure.locations.endCentralDirectoryRecordLocation, argv[1]);

	fclose(zipName);
	return 0;
}

void findEndOfCentralDirectoryLocation(FILE *zipFile, struct zipDataLocations *zipFileLocations)
{
	int zipData;
	long byteLocation = 0;

	rewind(zipFile);

	// locate the end of central directory record
	while((zipData = fgetc(zipFile)) != EOF)
	{
		// locate the 0x50
		if(zipData == 0x50)
		{
			// store the start of a 0x50 string in a temporary location
			long tempLocation;
			tempLocation = byteLocation;

			// locate 0x4b after 0x50
			byteLocation++;
			if((zipData = fgetc(zipFile)) == 0x4b)
			{
				// locate 0x05 after 0x4b
				byteLocation++;
				if((zipData = fgetc(zipFile)) == 0x05)
				{
					// locate 0x06 after 0x05
					byteLocation++;
					if((zipData = fgetc(zipFile)) == 0x06)
					{
						zipFileLocations->endCentralDirectoryRecordLocation = tempLocation;
						break;
					}
				}
			}

		}

		byteLocation++;
	}

	byteLocation = 0;
	rewind(zipFile);

	return;
}

// getEndCentralDirectoryData
void getEndCentralDirectoryData(FILE *zipFile, struct zipFileDataStructure *dataStructure)
{
	uint32_t *fourByteTemp = NULL;
	uint16_t *twoByteTemp = NULL;
	uint8_t *oneByteTemp = NULL;
	void *ptr = NULL;

	fseek(zipFile, dataStructure->locations.endCentralDirectoryRecordLocation, SEEK_SET);

	//fread(ptr, 4, 1, zipFile);

	/*printf("%#x %#x %#x %#x\n", fgetc(zipFile), fgetc(zipFile), fgetc(zipFile), fgetc(zipFile));*/
}
