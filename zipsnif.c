// zipsnif:  Simple program to search inside a zip file and return information about the file

// TODO: This has no error handling
// TODO: Handle multiple disks properly
// TODO: proper garbage handling

#include <stdio.h>
#include <stdlib.h>
#include "zip.h"

// funciton prototypes
void findEndOfCentralDirectoryLocation(FILE *, struct zipDataLocations *);
void getEndCentralDirectoryData(FILE *, struct zipFileDataStructure *);
uint32_t getCentralDirectoryData(FILE *, struct zipFileDataStructure *, uint32_t);
void freeCentralDirectoryFileHeaderData(struct zipFileDataStructure *);

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
	findEndOfCentralDirectoryLocation(zipName, &zipNameStructure.locations);
	getEndCentralDirectoryData(zipName, &zipNameStructure);

	uint32_t offset = zipNameStructure.endCentralDirectoryRecord.offsetCdStart;

	// obtain data for all the files in the archive cd
	for(uint16_t filesRemaining = zipNameStructure.endCentralDirectoryRecord.totalEntries; filesRemaining > 0; --filesRemaining)
	{
		offset = getCentralDirectoryData(zipName, &zipNameStructure, offset);
		printf("\n");
	}

	// free the central directory file headers
	freeCentralDirectoryFileHeaderData(&zipNameStructure);
	// close the file
	fclose(zipName);
	return 0;
}

// findEndOfCentralDirectoryLocation: finds the end of central directory offset
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

// getEndCentralDirectoryData:  obtain the end central directory data from a archive
void getEndCentralDirectoryData(FILE *zipFile, struct zipFileDataStructure *dataStructure)
{
	uint32_t fourByteTemp;
	uint16_t twoByteTemp;
	uint8_t oneByteTemp;

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

	// archive comment (variable)
	dataStructure->endCentralDirectoryRecord.comment = (char *) malloc(dataStructure->endCentralDirectoryRecord.commentLength + 1);
	fread(dataStructure->endCentralDirectoryRecord.comment, 1, dataStructure->endCentralDirectoryRecord.commentLength, zipFile);
	*(((dataStructure->endCentralDirectoryRecord.comment) + (dataStructure->endCentralDirectoryRecord.commentLength)) + 1) = '\0';
	printf("Comment: %s\n", dataStructure->endCentralDirectoryRecord.comment);

	// temporary garbage handling for comment
	free(dataStructure->endCentralDirectoryRecord.comment);

	return;
}

// getCentralDirectoryData: grabs the central directory data at offset
uint32_t getCentralDirectoryData(FILE *zipFile, struct zipFileDataStructure *dataStructure, uint32_t offset)
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

	// disk number on which this file exists
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->root->diskNumberStart = twoByteTemp;
	printf("Disk: %d\n", dataStructure->root->diskNumberStart);

	// Internal attributes
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->root->internalFileAttributes = twoByteTemp;
	printf("Internal attributes: %#x\n", dataStructure->root->internalFileAttributes);

	// external attributes
	fread(&fourByteTemp, 4, 1, zipFile);
	dataStructure->root->externalFileAttributes = fourByteTemp;
	printf("External attributes: %#x\n", dataStructure->root->externalFileAttributes);

	// offset of local header
	fread(&fourByteTemp, 4, 1, zipFile);
	dataStructure->root->offsetLocalFileHeader = fourByteTemp;
	printf("Local header offset: %#x\n", dataStructure->root->offsetLocalFileHeader);

	// file name (variable)
	dataStructure->root->fileName = (char *) malloc(dataStructure->root->fileNameLength + 1);
	fread(dataStructure->root->fileName, 1, dataStructure->root->fileNameLength, zipFile);
	*(((dataStructure->root->fileName) + (dataStructure->root->fileNameLength)) + 1) = '\0';
	printf("File name: %s\n", dataStructure->root->fileName);

	// extra field (variable)
	dataStructure->root->extraField = (uint8_t *) malloc(dataStructure->root->extraFieldLength);
	fread(dataStructure->root->extraField, 1, dataStructure->root->extraFieldLength, zipFile);
	for(int extraCounter = dataStructure->root->extraFieldLength; extraCounter > 0; extraCounter--)
	{
		if(extraCounter == dataStructure->root->extraFieldLength)
			printf("Extra field: ");
		printf("%x", *(dataStructure->root->extraField + extraCounter));
	}
	printf("\n");

	// file comment (variable)
	dataStructure->root->fileComment = (char *) malloc(dataStructure->root->fileCommentLength + 1);
	fread(dataStructure->root->fileComment, 1, dataStructure->root->fileCommentLength, zipFile);
	*(((dataStructure->root->fileComment) + (dataStructure->root->fileCommentLength)) + 1) = '\0';
	printf("File comment: %s\n", dataStructure->root->fileComment);


	// returns next offset
	uint32_t nextOffset = offset + 46 + dataStructure->root->fileNameLength + dataStructure->root->extraFieldLength + dataStructure->root->fileCommentLength;
	uint32_t tempSignature;

	// read the next signature and set the file cursor back to the beginning of the signature`
	fread(&tempSignature, 4, 1, zipFile);
	fseek(zipFile, nextOffset, SEEK_SET);

	// returns the next offset if it is a valid cd entry otherwise return 0
	return (tempSignature == 0x02014b50) ? nextOffset : 0;
}

// freeCentralDirectoryFileHeaderData: frees the malloc allocated data
void freeCentralDirectoryFileHeaderData(struct zipFileDataStructure *dataStructure)
{
	struct centralDirectoryFileHeaderData *tempCd = NULL;
	// free allocated central directories.  Temporary for testing.  Need seperate function for this.
	while(dataStructure->root != NULL)
	{
		tempCd = dataStructure->root->next;
		free(dataStructure->root->fileName);
		free(dataStructure->root->extraField);
		free(dataStructure->root->fileComment);
		free(dataStructure->root);
		dataStructure->root = tempCd;

	}

}
