// Header for zip files

#include <stdint.h>

// location information for a zip file
struct zipDataLocations
{
	long centralDirectoryFileHeader;
	long endCentralDirectoryRecordLocation;
};

// structure containing the end of central directory record data
struct endCentralDirectoryRecordData
{
	uint32_t signature;
	uint16_t diskNumber;
	uint16_t diskNumberWithCd;
	uint16_t diskEntries;
	uint16_t totalEntries;
	uint32_t centralDirectorySize;
	uint32_t offsetCdStart;
	uint16_t commentLength;
	char *comment;
};

// structure for the data of a zip file
struct zipFileDataStructure
{
	struct zipDataLocations locations;
	struct endCentralDirectoryRecordData endCentralDirectoryRecord;
};
