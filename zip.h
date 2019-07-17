// Header for zip files

#include <stdint.h>

// method tags for selection of search criteria inside search function
enum methodTags
{
	NOTHING 	= 0x0,
	ASCENDING	= 0x1,
	DESCENDING	= 0x2,
	EOCDRONLY	= 0x4
};

// location information for a zip file
struct zipDataLocations
{
	long endCentralDirectoryRecordLocation;
};

// structure containing an outline for a central directory file header
struct centralDirectoryFileHeaderData
{
	uint32_t signature;			// 0x50 0x4b 0x01 0x02
	uint16_t versionCreatedWith;		// version of utility that created the zip
	uint16_t versionNeededToExtract;	// version needed to extract
	uint16_t flags;			// general purpose bit flag
	uint16_t compressionMethod;		// compression method used
	uint16_t lastModTime;			// last modified time
	uint16_t lastModDate;			// last modified date
	uint32_t crc32;				// crc32
	uint32_t compressedSize;		// file compressed size
	uint32_t uncompressedSize;		// file uncompressed size
	uint16_t fileNameLength;		// file name length
	uint16_t extraFieldLength;		// extra field length
	uint16_t fileCommentLength;		// file comment length in bytes
	uint16_t diskNumberStart;		// disk number file starts on
	uint16_t internalFileAttributes;	// internal file internal file attributes
	uint32_t externalFileAttributes;	// external file attributes
	uint32_t offsetLocalFileHeader;		// relative offset of local file header
	char *fileName;				// file name
	uint8_t *extraField;			// extra field
	char *fileComment;			// file comment
	struct centralDirectoryFileHeaderData *next;	// pointer to next element of the CD
};

// structure containing the end of central directory record data
struct endCentralDirectoryRecordData
{
	uint32_t signature;			// The signature of the end of central directory record 0x50 0x4b 0x05 0x06
	uint16_t diskNumber;			// The number of this disk ( containing end of central directory record)
	uint16_t diskNumberCdStart;		// Number of the disk on which the central directory record starts
	uint16_t diskEntries;			// Number of central directory entries on this disk
	uint16_t totalEntries;			// Number of total entries in the central directory
	uint32_t centralDirectorySize;		// Size of central directory in bytes
	uint32_t offsetCdStart;			// Offset of the start of the central directory on the disk wihcih the CD starts
	uint16_t commentLength;			// The Length of the following comment field in bytes
	char *comment;				// Pointer to the comment
};

// structure for the data of a zip file
struct zipFileDataStructure
{
	char *fileName;
	struct zipDataLocations locations;
	struct endCentralDirectoryRecordData endCentralDirectoryRecord;
	struct centralDirectoryFileHeaderData *root;
};

// function prototypes
int findEndOfCentralDirectoryLocation(FILE *, struct zipDataLocations *);
void freeCentralDirectoryFileHeaderData(struct zipFileDataStructure *);
void getEndCentralDirectoryData(FILE *, struct zipFileDataStructure *);
uint32_t getCentralDirectoryData(FILE *, struct zipFileDataStructure *, uint32_t);
void sortCd(struct zipFileDataStructure *, uint8_t);
void printCd(struct zipFileDataStructure *);
void printEocdr(struct zipFileDataStructure *);
