#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zip.h"

// findEndOfCentralDirectoryLocation: finds the end of central directory offset
int findEndOfCentralDirectoryLocation(FILE *zipFile, struct zipDataLocations *zipFileLocations)
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
						rewind(zipFile);
						byteLocation = 0;
						return 1;
					}
				}
			}

		}

		byteLocation++;
	}

	byteLocation = 0;
	rewind(zipFile);

	return 0;
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

	// obtain the disk number where end of central directory starts
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->endCentralDirectoryRecord.diskNumber = twoByteTemp;

	// obtain the disk number where the central directory starts
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->endCentralDirectoryRecord.diskNumberCdStart = twoByteTemp;

	// obtain the number central directory disk entries
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->endCentralDirectoryRecord.diskEntries = twoByteTemp;

	// obtain the total amount of entries in the central directory
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->endCentralDirectoryRecord.totalEntries = twoByteTemp;

	// obtain the size of the central directory in bytes
	fread(&fourByteTemp, 4, 1, zipFile);
	dataStructure->endCentralDirectoryRecord.centralDirectorySize = fourByteTemp;

	// obtain the offset of the central directory
	fread(&fourByteTemp, 4, 1, zipFile);
	dataStructure->endCentralDirectoryRecord.offsetCdStart = fourByteTemp;

	// obtain the comment length in bytes
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->endCentralDirectoryRecord.commentLength = twoByteTemp;

	// archive comment (variable)
	dataStructure->endCentralDirectoryRecord.comment = (char *) malloc(dataStructure->endCentralDirectoryRecord.commentLength + 1);
	fread(dataStructure->endCentralDirectoryRecord.comment, 1, dataStructure->endCentralDirectoryRecord.commentLength, zipFile);
	*(((dataStructure->endCentralDirectoryRecord.comment) + (dataStructure->endCentralDirectoryRecord.commentLength)) + 1) = '\0';

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

	// version made by
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->root->versionCreatedWith = twoByteTemp;

	// version needed to extract
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->root->versionNeededToExtract = twoByteTemp;

	// flags
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->root->flags = twoByteTemp;

	// compression method
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->root->compressionMethod = twoByteTemp;

	// file modification time
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->root->lastModTime = twoByteTemp;

	// file modification date
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->root->lastModDate = twoByteTemp;

	// crc32
	fread(&fourByteTemp, 4, 1, zipFile);
	dataStructure->root->crc32 = fourByteTemp;

	// compressed size
	fread(&fourByteTemp, 4, 1, zipFile);
	dataStructure->root->compressedSize = fourByteTemp;

	// uncompressed size
	fread(&fourByteTemp, 4, 1, zipFile);
	dataStructure->root->uncompressedSize = fourByteTemp;

	// file name length
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->root->fileNameLength = twoByteTemp;

	// extra field length
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->root->extraFieldLength = twoByteTemp;

	// file comment length
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->root->fileCommentLength = twoByteTemp;

	// disk number on which this file exists
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->root->diskNumberStart = twoByteTemp;

	// Internal attributes
	fread(&twoByteTemp, 2, 1, zipFile);
	dataStructure->root->internalFileAttributes = twoByteTemp;

	// external attributes
	fread(&fourByteTemp, 4, 1, zipFile);
	dataStructure->root->externalFileAttributes = fourByteTemp;

	// offset of local header
	fread(&fourByteTemp, 4, 1, zipFile);
	dataStructure->root->offsetLocalFileHeader = fourByteTemp;

	// file name (variable)
	dataStructure->root->fileName = (char *) malloc(dataStructure->root->fileNameLength + 1);
	fread(dataStructure->root->fileName, 1, dataStructure->root->fileNameLength, zipFile);
	*(((dataStructure->root->fileName) + (dataStructure->root->fileNameLength)) + 1) = '\0';

	// extra field (variable)
	dataStructure->root->extraField = (uint8_t *) malloc(dataStructure->root->extraFieldLength);
	fread(dataStructure->root->extraField, 1, dataStructure->root->extraFieldLength, zipFile);
	/*for(int extraCounter = dataStructure->root->extraFieldLength; extraCounter > 0; extraCounter--)
	{
		if(extraCounter == dataStructure->root->extraFieldLength)
			printf("Extra field: ");
		printf("%x", *(dataStructure->root->extraField + extraCounter));
	}
	printf("\n");*/

	// file comment (variable)
	dataStructure->root->fileComment = (char *) malloc(dataStructure->root->fileCommentLength + 1);
	fread(dataStructure->root->fileComment, 1, dataStructure->root->fileCommentLength, zipFile);
	*(((dataStructure->root->fileComment) + (dataStructure->root->fileCommentLength)) + 1) = '\0';

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

// sortCd: Sort central directory structure
void sortCd(struct zipFileDataStructure *dataStructure, uint8_t method)
{
	struct centralDirectoryFileHeaderData *currentCd = NULL;
	struct centralDirectoryFileHeaderData *nextCd = NULL;
	struct centralDirectoryFileHeaderData *prevCd = NULL;
	struct centralDirectoryFileHeaderData *walkingCd = dataStructure->root;

	if(method & ASCENDING)
		;	// continue here

	switch(method)
	{
		case ASCENDING:
		{	// runs through the scructure totalEntries amount of times
			for(int numFiles = dataStructure->endCentralDirectoryRecord.totalEntries; numFiles > 0; --numFiles)
			{
				while(walkingCd->next != NULL)
				{
					// set the current Cd to the current one
					currentCd = walkingCd;

					// compare the names between the current Cd and the next Cd
					if(strcmp(walkingCd->fileName, walkingCd->next->fileName) < 0)
						;
					else
					{
						// entering sorting on first element of linked structure
						if(prevCd == NULL)
						{
							// temporarally record old root
							nextCd = dataStructure->root;

							// make old root->next as the new root
							dataStructure->root = dataStructure->root->next;

							// copy the new root->next into the old root->next
							nextCd->next = dataStructure->root->next;

							// make old root as new root->next
							dataStructure->root->next = nextCd;

							// change walkingCd to the new root location
							walkingCd = dataStructure->root;
						}
						// entering sorting on second element of linked structure
						else
						{
							// prevCd->next is the current root instead of dataStructure->root because of indirecton

							// temporarlly record old root
							nextCd = prevCd->next;

							// make old root->next as the new root
							prevCd->next = prevCd->next->next;

							// copy the new root->next into the old root->next
							nextCd->next = prevCd->next->next;

							// make the old root the new root->next
							prevCd->next->next = nextCd;

							// change walkingCd to the new root location`
							walkingCd = prevCd->next;


						}
					}

					// records the current walking cd in prev cd for next run
					prevCd = walkingCd;

					// walks the current Cd to the next Cd
					walkingCd = walkingCd->next;
				}

				// resets the variables for the next run of the loop`
				prevCd = NULL;
				currentCd = NULL;
				nextCd = NULL;
				walkingCd = dataStructure->root;
			}

			break;

		}

		case DESCENDING:
			break;

		default:
			fprintf(stderr, "zipsnif: Incorrect search method used.\n");
			exit(1);
			break;
	}
}

// printCd: Prints the files within the CD
void printCd(struct zipFileDataStructure *dataStructure)
{
	struct centralDirectoryFileHeaderData *walkingCd = dataStructure->root;

	while(walkingCd != NULL)
	{
		printf("%s\n", walkingCd->fileName);
		walkingCd = walkingCd->next;
	}
}