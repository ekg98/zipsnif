// zipsnif:  Simple program to search inside a zip file and return information about the file

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "zip.h"

void getZipLocations(FILE *, struct zipDataLocations *);

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
		fprintf(stderr, "zipsnif: One file at a time please.\n");
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

	struct zipDataLocations locations;

	getZipLocations(zipName, &locations);

	printf("Central Directory File Header is %ld bytes from beginning of file %s.\n", locations.centralDirectoryFileHeader, argv[1]);

	fclose(zipName);
	return 0;
}

void getZipLocations(FILE *zipFile, struct zipDataLocations *zipFileLocations)
{
	int zipData;
	long byteLocation = 0;

	// locate central directory file header 0x50 0x4b 0x01 0x02
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
				// locate 0x01 after 0x4b
				byteLocation++;
				if((zipData = fgetc(zipFile)) == 0x01)
				{
					// locate 0x02 after 0x01
					byteLocation++;
					if((zipData = fgetc(zipFile)) == 0x02)
					{
						zipFileLocations->centralDirectoryFileHeader = tempLocation;
						break;
					}
				}
			}

		}

		byteLocation++;
	}

	return;
}
