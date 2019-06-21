// zipsnif:  Simple program to search inside a zip file and return information about the file

#include <stdio.h>
#include <stdlib.h>
#include "zip.h"

void findEndOfCentralDirectoryLocation(FILE *, struct zipDataLocations *);
void getEndCentralDirectoryData(FILE *, struct zipFileDataStructure *);
void getCentralDirectoryData(FILE *, struct zipFileDataStructure *, uint32_t);

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

	findEndOfCentralDirectoryLocation(zipName, &zipNameStructure.locations);
	getEndCentralDirectoryData(zipName, &zipNameStructure);
	getCentralDirectoryData(zipName, &zipNameStructure, zipNameStructure.endCentralDirectoryRecord.offsetCdStart);

	//printf("End of central directory record is %ld bytes from beginning of file %s.\n", zipNameStructure.locations.endCentralDirectoryRecordLocation, argv[1]);

	// free end of cd comment memory
	free(zipNameStructure.endCentralDirectoryRecord.comment);
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
	uint32_t fourByteTemp;
	uint16_t twoByteTemp;
	uint8_t oneByteTemp;
	char *comment = NULL;
	char *commentPtr = NULL;

	// set the file cursor location to the end of central directory record
	fseek(zipFile, dataStructure->locations.endCentralDirectoryRecordLocation, SEEK_SET);

	// obtain the end of central directory signature
	fread(&fourByteTemp, 4, 1, zipFile);
	dataStructure->endCentralDirectoryRecord.signature = fourByteTemp;
	printf("Signaute: %#x\n", dataStructure->endCentralDirectoryRecord.signature);

	// obtain the disk number where end of central directory starts
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->endCentralDirectoryRecord.diskNumber = twoByteTemp;
	printf("Disk number: %d\n", dataStructure->endCentralDirectoryRecord.diskNumber);

	// obtain the disk number where the central directory starts
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->endCentralDirectoryRecord.diskNumberCdStart = twoByteTemp;
	printf("Disk number where CD starts: %d\n", dataStructure->endCentralDirectoryRecord.diskNumberCdStart);

	// obtain the number central directory disk entries
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->endCentralDirectoryRecord.diskEntries = twoByteTemp;
	printf("Disk entries: %d\n", dataStructure->endCentralDirectoryRecord.diskEntries);

	// obtain the total amount of entries in the central directory
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->endCentralDirectoryRecord.totalEntries = twoByteTemp;
	printf("Total entries in CD: %d\n", dataStructure->endCentralDirectoryRecord.totalEntries);

	// obtain the size of the central directory in bytes
	fread(&fourByteTemp, 4, 1, zipFile);
	dataStructure->endCentralDirectoryRecord.centralDirectorySize = fourByteTemp;
	printf("Size of CD in bytes: %d\n", dataStructure->endCentralDirectoryRecord.centralDirectorySize);

	// obtain the offset of the central directory
	fread(&fourByteTemp, 4, 1, zipFile);
	dataStructure->endCentralDirectoryRecord.offsetCdStart = fourByteTemp;
	printf("CD offset: %#x\n", dataStructure->endCentralDirectoryRecord.offsetCdStart);

	// obtain the comment length in bytes
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->endCentralDirectoryRecord.commentLength = twoByteTemp;
	printf("Comment length: %d\n", dataStructure->endCentralDirectoryRecord.commentLength);

	// allocate memory for the comment
	comment = (char *) malloc(twoByteTemp + 1);
	commentPtr = comment;

	// obtain the comment and append end of string indicator
	fread(comment, 1, twoByteTemp, zipFile);
	commentPtr = commentPtr + (twoByteTemp + 1);
	*commentPtr = '\0';
	dataStructure->endCentralDirectoryRecord.comment = comment;
	printf("Comment: %s\n", comment);

	comment = NULL;
	commentPtr = NULL;

	return;
}

void getCentralDirectoryData(FILE *zipFile, struct zipFileDataStructure *dataStructure, uint32_t offset)
{
	struct centralDirectoryFileHeaderData *tempCd = NULL;

	// temporary variables
	uint8_t oneByteTemp;
	uint16_t twoByteTemp;
	uint32_t fourByteTemp;

	// allocate memory for the first central directory file header if there is none
	if(dataStructure->root == NULL)
	{
		dataStructure->root = (struct centralDirectoryFileHeaderData *) malloc(sizeof(struct centralDirectoryFileHeaderData));
		dataStructure->root->next = NULL;
	}
	else	// allocate for a new entry and push the old on down the line
	{
		tempCd = dataStructure->root;
		dataStructure->root = (struct centralDirectoryFileHeaderData *) malloc(sizeof(struct centralDirectoryFileHeaderData));
		dataStructure->root->next = tempCd;
		tempCd = NULL;
	}

	// set offset position to the start of the cd
	fseek(zipFile, offset, SEEK_SET);

	// signature of central directory file header
	fread(&fourByteTemp, 4, 1, zipFile);
	dataStructure->root->signature = fourByteTemp;
	printf("Signature: %#x\n", dataStructure->root->signature);

	// version made by
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->root->versionCreatedWith = twoByteTemp;
	printf("Version made by: %#x\n", dataStructure->root->versionCreatedWith);

	// version needed to extract
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->root->versionNeededToExtract = twoByteTemp;
	printf("Version needed to extract: %d\n", dataStructure->root->versionNeededToExtract);

	// flags
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->root->flags = twoByteTemp;
	printf("Flags: %#x\n", dataStructure->root->flags);

	// compression method
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->root->compressionMethod = twoByteTemp;
	printf("Compression method: %d\n", dataStructure->root->compressionMethod);

	// file modification time
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->root->lastModTime = twoByteTemp;
	printf("Modification time: %#x\n", dataStructure->root->lastModTime);

	// file modification date
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->root->lastModDate = twoByteTemp;
	printf("Modification date: %#x\n", dataStructure->root->lastModDate);

	// crc32
	fread(&fourByteTemp, 4, 1, zipFile);
	dataStructure->root->crc32 = fourByteTemp;
	printf("crc32: %#x\n", dataStructure->root->crc32);

	// compressed size
	fread(&fourByteTemp, 4, 1, zipFile);
	dataStructure->root->compressedSize = fourByteTemp;
	printf("Compressed size: %d\n", dataStructure->root->compressedSize);

	// uncompressed size
	fread(&fourByteTemp, 4, 1, zipFile);
	dataStructure->root->uncompressedSize = fourByteTemp;
	printf("Uncompressed size: %d\n", dataStructure->root->uncompressedSize);

	// file name length
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->root->fileNameLength = twoByteTemp;
	printf("File name length: %d\n", dataStructure->root->fileNameLength);

	// extra field length
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->root->extraFieldLength = twoByteTemp;
	printf("Extra field length: %d\n", dataStructure->root->extraFieldLength);

	// file comment length
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->root->fileCommentLength = twoByteTemp;
	printf("File comment length: %d\n", dataStructure->root->fileCommentLength);

	// Point to the root central directory so you can test it when freeing the structure
	tempCd = dataStructure->root;
	// free allocated central directories.  Temporary for testing.  Need seperate function for this.
	while(tempCd != NULL)
	{
		tempCd = dataStructure->root->next;
		free(dataStructure->root);
	}

	return;
}
