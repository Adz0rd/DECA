/*
DiskScanner.h
Written by Taylor.

TODO:
- unsigned char instead of char?
- validation and error testing.
- each signature read in from disk is maxSignatureSize, this should be variable.
- Refactor/optimise scanning function.
- Read footer scanning.
*/

#include <windows.h>
#include <winioctl.h>
#include <stdio.h>

#ifdef DECADISKSCANNER_EXPORTS
#define DISKSCANNER_API __declspec(dllexport)
/* #else 
#define DISKSCANNER_API __declspec(dllimport) */
#endif

#define DEBUG_MODE 1

extern "C"
{/*
	struct SIG_DATA
	{
		unsigned int sigID;
		unsigned char *sigHeader;
		unsigned char *sigFooter;
	};

	// Structure for storing signature information.
	struct SIG_ARR
	{
	unsigned int maxSignatureSize;
	unsigned int numSigPairs;

	SIG_DATA *sigArray[1];
	};
	*/

	struct SIG_DATA
	{
		unsigned int sigID;
		unsigned char *sigHeader;
		unsigned char *sigFooter;
		SIG_DATA *next;
	};

	// Structure for storing signature information.
	struct SIG_ARR
	{
		unsigned int maxSignatureSize;
		unsigned int numSigPairs;

		SIG_DATA *sigArray;
	};


	struct ScanResult
	{
		unsigned int sigID;
		unsigned int headerCount;
		unsigned int footerCount;
	};

	struct Response
	{
		ScanResult *scanResultsArr[1];
	};

	/* LINKED LIST TESTING.
	struct SIG_DATA_LIST
	{
		unsigned int sigID;
		unsigned char *sigHeader;
		unsigned char *sigFooter;
		SIG_DATA_LIST *next;
	};

	// Structure for storing signature information.
	struct SIG_ARR_LIST
	{
		unsigned int maxSignatureSize;
		unsigned int numSigPairs;

		SIG_DATA_LIST *sigArray;
	};
	*/
}

// DiskScanner interface class.
class IDiskScanner
{
public:
	virtual ~IDiskScanner() = 0;
	virtual void buildScanner(unsigned int, unsigned int, char *, unsigned int) = 0;
	virtual int mountVolume() = 0;
	virtual int unmountVolume() = 0;
	virtual int scanChunk(SIG_ARR*, Response*) = 0;
	virtual int scanChunkBST(SIG_ARR*, Response*) = 0;
	virtual void readAttributes(SIG_ARR*) = 0;
	virtual int scanChunkTest(SIG_ARR*) = 0;
};

class DiskScanner : public IDiskScanner
{
public:
	~DiskScanner();

	// Build the scanner object.
	void buildScanner(unsigned int chunkSize, unsigned int sectorSize, char *diskPath, unsigned int startOffset);

	// Mount and unmount the volume.
	int mountVolume();
	int unmountVolume();

	// Scan the first/next available chunk.
	int scanChunk(SIG_ARR *sigArray, Response *returnStruct);
	int scanChunkBST(SIG_ARR *sigArray, Response *returnStruct);
	int scanChunkTest(SIG_ARR *sigArray);

	// Testing functions.
	void readAttributes(SIG_ARR* sigArray);

private:
	HANDLE diskHandle;
	unsigned int chunkSize;
	unsigned int sectorSize;
	unsigned int startOffset;
	char *diskPath;

	// Methods for internal printing and comparison.
	int compareSig(unsigned char *sig1, unsigned char *sig2, int size);
	void printCompareSig(unsigned char *sig1, unsigned char *sig2, int size);
	int binarySearch(unsigned char *arrayList, unsigned char *sig, int min, int max, int sigSize);
	int hexCheck(unsigned char *sig1, unsigned char *sig2, int sigSize);
};

// Exported C functions for DLL communication from client.
extern "C"
{
	// Get a new instance of a DiskScanner object.
	DISKSCANNER_API IDiskScanner *createScanner()
	{
		return new DiskScanner();
	}

	// Dispose the scanner (Must be called once client has finished using the object).
	DISKSCANNER_API void disposeScanner(IDiskScanner *diskScanner)
	{
		delete diskScanner;

	}

	DISKSCANNER_API void buildScanner(IDiskScanner *diskScanner, unsigned int chunkSize, unsigned int sectorSize, char *diskPath, unsigned int startOffset)
	{
		return diskScanner->buildScanner(chunkSize, sectorSize, diskPath, startOffset);
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
	DISKSCANNER_API int scanChunk(IDiskScanner *diskScanner, SIG_ARR *sigArray, Response *returnStruct)
	{
		return diskScanner->scanChunk(sigArray, returnStruct);
	}

	DISKSCANNER_API int scanChunkBST(IDiskScanner *diskScanner, SIG_ARR *sigArray, Response *returnStruct)
	{
		return diskScanner->scanChunk(sigArray, returnStruct);
	}

	DISKSCANNER_API void readSigData(IDiskScanner *diskScanner, SIG_ARR *sigArray)
	{
		diskScanner->readAttributes(sigArray);
	}

	DISKSCANNER_API int scanChunkTest(IDiskScanner *diskScanner, SIG_ARR *sigArray)
	{
		return diskScanner->scanChunkTest(sigArray);
	}
}