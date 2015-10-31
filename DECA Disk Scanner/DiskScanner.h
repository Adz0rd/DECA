/*
DiskScanner.h
Written by Taylor.
*/

#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <vector>

#ifdef DECADISKSCANNER_EXPORTS
#define DISKSCANNER_API __declspec(dllexport)
#endif

//#define DEBUG_MODE 1

extern "C"
{
	// Signature data structure.
	struct SIG_DATA
	{
		unsigned int sigID;
		unsigned int sigLength;
		unsigned char *sigHeader;
	};
}

// DiskScanner interface class.
class IDiskScanner
{
public:
	// Initialisation methods.
	virtual ~IDiskScanner() = 0;
	virtual void buildScanner(unsigned int, unsigned int, char *, unsigned int, unsigned int) = 0;
	virtual int mountVolume() = 0;
	virtual int unmountVolume() = 0;

	// Signature loading methods.
	virtual void addSignature(unsigned int sigID, unsigned int sigLength, unsigned char *sigHeader) = 0;
	virtual void lockSignatureList() = 0;

	// Scanning methods.
	virtual unsigned int *scanChunk() = 0;
	virtual unsigned int *scanChunk_BST() = 0;
	virtual unsigned int *scanChunkBySector() = 0;
	virtual unsigned int *scanChunkBySector_BST() = 0;
	virtual int moveOffset(long offset) = 0;
};

class DiskScanner : public IDiskScanner
{
public:
	// Constructors/Destructors.
	~DiskScanner();

	// Build the scanner object.
	void buildScanner(unsigned int chunkSize, unsigned int sectorSize, char *diskPath, unsigned int startOffset, unsigned int maxSize);

	// Mount and unmount the volume.
	int mountVolume();
	int unmountVolume();

	// Signature initilisation methods.
	void addSignature(unsigned int sigID, unsigned int sigLength, unsigned char *sigHeader);	// Add a signature to the database.
	void lockSignatureList();																	// Lock signatures in for scanning.

	// Signature scanning methods.
	unsigned int *scanChunk();						// Scan a chunk one sector at a time and then compare all sectors/signatures.
	unsigned int *scanChunk_BST();					// Scan a chunk one sector at a time and then compare utilising a Binary Search Tree algorithm.
	unsigned int *scanChunkBySector();				// Scan a chunk and compare signatures sector by sector.
	unsigned int *scanChunkBySector_BST();			// Scan a chunk and compare signatures sector by sector utilising a Binary Search Tree algorithm.
	int moveOffset(long offset);					// Move the offset to a new location.

private:
	HANDLE diskHandle;						// Win32 handle to disk.
	unsigned int chunkSize;					// Number of sectors to scan per scanning method call.
	unsigned int sectorSize;				// Size of each sector.
	unsigned int startOffset;				// Starting scan location.
	char *diskPath;							// Path to the disk.
	std::vector<SIG_DATA> sigDataList;		// Image data list.
	unsigned int numSigs;					// Total number of signatures scanned.
	unsigned int maxSize;					// Maximum signature size.
	unsigned int *scanResult;				// Return array.

	// Methods for internal printing and comparison.
	int compareSig(unsigned char *sig1, unsigned char *sig2, unsigned int size);	// Signature comparison.
	void printCompareSig(unsigned char *sig1, unsigned char *sig2, int size);		// Signature comparison with output.
	int binarySearch(unsigned char *sig, int min, int max, int sigSize);			// Recursive binary seach.
	int hexCheck(unsigned char *sig1, unsigned char *sig2, int sigSize);			// Compare hex values.
};

// Exported C functions for DLL communication from client.
extern "C"
{
	// Get a new instance of a DiskScanner object.
	DISKSCANNER_API IDiskScanner *createScanner()
	{
		return new DiskScanner();
	}

	DISKSCANNER_API void buildScanner(IDiskScanner *diskScanner, unsigned int chunkSize, unsigned int sectorSize, char *diskPath, unsigned int startOffset, unsigned int maxSize)
	{
		return diskScanner->buildScanner(chunkSize, sectorSize, diskPath, startOffset, maxSize);
	}

	DISKSCANNER_API int mountVolume(IDiskScanner *diskScanner)
	{
		return diskScanner->mountVolume();
	}

	DISKSCANNER_API	int unmountVolume(IDiskScanner *diskScanner)
	{
		return diskScanner->unmountVolume();
	}

	// Scan the first/next available chunk.
	DISKSCANNER_API void addSignature(IDiskScanner *diskScanner, unsigned int sigID, unsigned int sigLength, unsigned char *sigHeader)
	{
		diskScanner->addSignature(sigID, sigLength, sigHeader);
	}


	DISKSCANNER_API void lockSignatureList(IDiskScanner *diskScanner)
	{
		diskScanner->lockSignatureList();
	}

	DISKSCANNER_API unsigned int *scanChunk(IDiskScanner *diskScanner)
	{
		return diskScanner->scanChunk();
	}

	DISKSCANNER_API unsigned int *scanChunk_BST(IDiskScanner *diskScanner)
	{
		return diskScanner->scanChunk_BST();
	}

	DISKSCANNER_API unsigned int *scanChunkBySector(IDiskScanner *diskScanner)
	{
		return diskScanner->scanChunkBySector();
	}

	DISKSCANNER_API unsigned int *scanChunkBySector_BST(IDiskScanner *diskScanner)
	{
		return diskScanner->scanChunkBySector_BST();
	}

	DISKSCANNER_API int moveOffset(IDiskScanner *diskScanner, long offset)
	{
		return diskScanner->moveOffset(offset);
	}
}