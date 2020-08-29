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
	fread(dataStructure->endCentralDirectoryRecord.comment, sizeof(char), dataStructure->endCentralDirectoryRecord.commentLength, zipFile);
	*(((dataStructure->endCentralDirectoryRecord.comment) + (dataStructure->endCentralDirectoryRecord.commentLength)))  = '\0';

}

// getCentralDirectoryData: grabs the central directory data at offset
uint32_t getCentralDirectoryData(FILE *zipFile, struct zipFileDataStructure *dataStructure, uint32_t offset, struct offsetInfo *offsetInfo)
{
	//printf("offset = %#x\n", offset);

	struct centralDirectoryFileHeaderData *tempCd = NULL;

	// temporary variables
	uint8_t oneByteTemp;
	uint16_t twoByteTemp;
	uint32_t fourByteTemp;

	// allocate memory for the first central directory file header if there is none
	if(dataStructure->root == NULL)
	{
		dataStructure->root = (struct centralDirectoryFileHeaderData *) malloc(sizeof(struct centralDirectoryFileHeaderData));

		if(dataStructure->root == NULL)
		{
			fprintf(stderr, "Malloc error allocating new file structure.\n");
			exit(1);
		}

		dataStructure->root->next = NULL;
		dataStructure->root->fileName = NULL;
		dataStructure->root->extraField = NULL;
		dataStructure->root->fileComment = NULL;
	}
	else	// allocate for a new entry and push the old on down the line
	{
		tempCd = dataStructure->root;
		dataStructure->root = (struct centralDirectoryFileHeaderData *) malloc(sizeof(struct centralDirectoryFileHeaderData));

		if(dataStructure->root == NULL)
		{
			fprintf(stderr, "Malloc error allocating new file structure.\n");
			exit(1);
		}

		dataStructure->root->next = tempCd;
		dataStructure->root->fileName = NULL;
		dataStructure->root->extraField = NULL;
		dataStructure->root->fileComment = NULL;
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

	// fails here.
	/*if(offset == 0x8e940d5)
	{
		printf("got this far\n");
		printf("fileNameLength == %d\n", dataStructure->root->fileNameLength);
	}*/
	// file name (variable)

	// malloc fails here if set to 1 for null terminator on large zips.  For some reason levi thinks malloc is allocating a strange amount of memory possibly not leaving room for mallocs internals.
	dataStructure->root->fileName = (char *) malloc(dataStructure->root->fileNameLength + 1);
	if(dataStructure->root->fileName == NULL)
	{
			fprintf(stderr, "Malloc error allocating file name.\n");
			exit(1);
	}

	fread(dataStructure->root->fileName, 1, dataStructure->root->fileNameLength, zipFile);
	*(((dataStructure->root->fileName) + (dataStructure->root->fileNameLength))) = '\0';

	// extra field (variable)
	dataStructure->root->extraField = (uint8_t *) malloc(dataStructure->root->extraFieldLength);
	if(dataStructure->root->extraField == NULL)
	{
			fprintf(stderr, "Malloc error allocating extra field.\n");
			exit(1);
	}
	fread(dataStructure->root->extraField, 1, dataStructure->root->extraFieldLength, zipFile);

	// file comment (variable)
	dataStructure->root->fileComment = (char *) malloc(dataStructure->root->fileCommentLength + 1);
	if(dataStructure->root->fileComment == NULL)
	{
			fprintf(stderr, "Malloc error allocating file comment.\n");
			exit(1);
	}
	fread(dataStructure->root->fileComment, 1, dataStructure->root->fileCommentLength, zipFile);
	*(((dataStructure->root->fileComment) + (dataStructure->root->fileCommentLength))) = '\0';

	// returns next offset only on assumption.
	uint32_t nextOffset = offset + 46 + dataStructure->root->fileNameLength + dataStructure->root->extraFieldLength + dataStructure->root->fileCommentLength;
	uint32_t tempSignature;

	// read the next signature and set the file cursor back to the beginning of the signature`
	fread(&tempSignature, 4, 1, zipFile);
	fseek(zipFile, nextOffset, SEEK_SET);

	// this switch checks to see the next signature and returns it
	switch(tempSignature)
	{
		// end of central directory record
		case ZIPEOCDR:
			offsetInfo->status = ZIPEOCDR;
			return ZIPEOCDR;
			break;

		// central directory file header
		case ZIPCDFH:
			offsetInfo->status = ZIPCDFH;
			return nextOffset;
			break;

		// no known signature found
		default:
			offsetInfo->status = NOSIG;
			return NOSIG;
			break;
	}
}

// freeCFileHeaderData: frees the malloc allocated data
void freeFileHeaderData(struct zipFileDataStructure *dataStructure)
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

	// checks to see if ascending or descending bit is enabled.  When in doubt revert to ascending sort order.
	if((method & ASCENDING) == ASCENDING)
		method = ASCENDING;
	else if((method & DESCENDING) == DESCENDING)
		method = DESCENDING;
	else
		method = ASCENDING;

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
void printCdShort(struct zipFileDataStructure *dataStructure)
{
	struct centralDirectoryFileHeaderData *walkingCd = dataStructure->root;

	while(walkingCd != NULL)
	{
		printf("%s\n", walkingCd->fileName);
		walkingCd = walkingCd->next;
	}
}

// printEocdr: Prints the end of central directory record data
void printEocdr(struct zipFileDataStructure *dataStructure)
{
	printf("\nEnd of central directory record data for %s\n", dataStructure->fileName);
	printf("\n\tSignature: %#x\n", dataStructure->endCentralDirectoryRecord.signature);
	printf("\tOffset: %#lx\n",  dataStructure->locations.endCentralDirectoryRecordLocation);
	printf("\tDisk with EOCDR: %u\n", dataStructure->endCentralDirectoryRecord.diskNumber);

	printf("\n\tCD Starting disk: %u\n", dataStructure->endCentralDirectoryRecord.diskNumberCdStart);
	printf("\tCD Offset: %#x\n", dataStructure->endCentralDirectoryRecord.offsetCdStart);
	printf("\tCD entries on current disk: %u\n", dataStructure->endCentralDirectoryRecord.diskEntries);
	printf("\tTotal CD entries: %u\n", dataStructure->endCentralDirectoryRecord.totalEntries);
	printf("\tCD Size (bytes): %u\n", dataStructure->endCentralDirectoryRecord.centralDirectorySize);

	printf("\n\tComment Length: %u\n", dataStructure->endCentralDirectoryRecord.commentLength);
	printf("\tComment: %s\n", dataStructure->endCentralDirectoryRecord.comment);
	printf("\n");
}

// sigCheck:  Checks the next 4 byte sequence in a file starting at the input offset for a signature
enum signatureTags sigCheck(FILE *zipFile, enum signatureTags tag)
{
	uint32_t intToCheck;

	fread(&intToCheck, 4, 1, zipFile);
	fseek(zipFile, -4, SEEK_CUR);

	switch(intToCheck)
	{
		case ZIPEOCDR:
			return ZIPEOCDR;
			break;
		case ZIP64EOCDR:
			return ZIP64EOCDR;
			break;
		case ZIPCDFH:
			return ZIPCDFH;
			break;
		default:
			return NOSIG;
			break;
	}
}
