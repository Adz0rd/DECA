/*
DiskScanner.h
Written by Taylor.
*/

#include <windows.h>
#include <winioctl.h>
#include <stdio.h>

#ifdef DECADISKSCANNER_EXPORTS
#define DISKSCANNER_API __declspec(dllexport)
#endif

#define DEBUG_MODE 1

extern "C"
{
	struct SIG_DATA
	{
		unsigned int sigID;
		unsigned char *sigHeader;
	};
	/*
	// Structure for storing signature information.
	struct SIG_ARR
	{
	unsigned int maxSignatureSize;
	unsigned int numSigPairs;

	SIG_DATA *sigArray[1];
	};
	

	struct SIG_DATA
	{
		unsigned int sigID;
		unsigned char *sigHeader;
		unsigned char *sigFooter;
		SIG_DATA *next;
	};
	*/
	// Structure for storing signature information.
	struct SIG_ARR
	{
		unsigned int maxSignatureSize;
		unsigned int numSigPairs;

		SIG_DATA *sigArray;
	};

	/*
	struct ScanResult
	{
		unsigned int sigID;
		unsigned int headerCount;
		unsigned int footerCount;
		ScanResult *next;
	};
	*/

	struct ScanResult
	{
		unsigned int sigID;
		unsigned int headerCount;
	};

	struct Response
	{
		ScanResult *scanResults;
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
	// Initialisation methods.
	virtual ~IDiskScanner() = 0;
	virtual void buildScanner(unsigned int, unsigned int, char *, unsigned int, unsigned int) = 0;
	virtual int mountVolume() = 0;
	virtual int unmountVolume() = 0;

	virtual void addSignature(SIG_DATA *sigData) = 0;
	virtual void lockSignatureList() = 0;

	// Scanning methods.
	virtual int scanChunk(SIG_ARR*, Response*) = 0;
	virtual unsigned int *scanChunkDatabase() = 0;
	virtual int scanChunkBST(SIG_ARR*, Response*) = 0;
	virtual ScanResult *scanChunkList(SIG_ARR*) = 0;

	// Testing methods.
	virtual void readAttributes(SIG_ARR*) = 0;
	virtual int scanChunkTest(SIG_ARR*) = 0;
};

class DiskScanner : public IDiskScanner
{
public:
	~DiskScanner();

	// Build the scanner object.
	void buildScanner(unsigned int chunkSize, unsigned int sectorSize, char *diskPath, unsigned int startOffset, unsigned int maxSize);

	// Mount and unmount the volume.
	int mountVolume();
	int unmountVolume();

	void addSignature(SIG_DATA *sigData);
	void lockSignatureList();

	// Scan the first/next available chunk.
	int scanChunk(SIG_ARR *sigArray, Response *returnStruct);
	unsigned int *scanChunkDatabase();
	int scanChunkBST(SIG_ARR *sigArray, Response *returnStruct);
	ScanResult *scanChunkList(SIG_ARR *sigArray);
	int scanChunkTest(SIG_ARR *sigArray);

	// Testing functions.
	void readAttributes(SIG_ARR* sigArray);

private:
	HANDLE diskHandle;
	unsigned int chunkSize;
	unsigned int sectorSize;
	unsigned int startOffset;
	char *diskPath;
	SIG_ARR *sigArray;
	SIG_DATA *sigDataList[1];
	unsigned int numSigs;
	unsigned int maxSize;
	unsigned int *scanResult;

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
	DISKSCANNER_API int scanChunk(IDiskScanner *diskScanner, SIG_ARR *sigArray, Response *returnStruct)
	{
		return diskScanner->scanChunk(sigArray, returnStruct);
	}

	DISKSCANNER_API int scanChunkBST(IDiskScanner *diskScanner, SIG_ARR *sigArray, Response *returnStruct)
	{
		return diskScanner->scanChunk(sigArray, returnStruct);
	}

	DISKSCANNER_API ScanResult *scanChunkList(IDiskScanner *diskScanner, SIG_ARR *sigArray)
	{
		return diskScanner->scanChunkList(sigArray);
	}

	DISKSCANNER_API void readSigData(IDiskScanner *diskScanner, SIG_ARR *sigArray)
	{
		diskScanner->readAttributes(sigArray);
	}

	DISKSCANNER_API int scanChunkTest(IDiskScanner *diskScanner, SIG_ARR *sigArray)
	{
		return diskScanner->scanChunkTest(sigArray);
	}

	DISKSCANNER_API void addSignature(IDiskScanner *diskScanner, SIG_DATA *sigData)
	{
		diskScanner->addSignature(sigData);
	}

	DISKSCANNER_API void lockSignatureList(IDiskScanner *diskScanner)
	{
		diskScanner->lockSignatureList();
	}

	DISKSCANNER_API unsigned int *scanChunkDatabase(IDiskScanner *diskScanner)
	{
		return diskScanner->scanChunkDatabase();
	}
}