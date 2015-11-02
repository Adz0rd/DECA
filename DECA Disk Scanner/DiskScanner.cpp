/*
DiskScanner.cpp
Written by Taylor.
*/

#include "DiskScanner.h"

IDiskScanner::~IDiskScanner() {}

DiskScanner::~DiskScanner()
{
	#ifdef DEBUG_MODE
	printf(">> Destructing DiskScanner object\n");
	#endif

	delete diskPath;
}

inline int DiskScanner::compareSig(unsigned char *sig1, unsigned char *sig2, unsigned int size)
{
	unsigned int sig1Val = 0;
	unsigned int sig2Val = 0;
	
	for (unsigned int i = 0; i < size; i++)
	{
		sig1Val = sig1[i];
		sig2Val = sig2[i];

		if (sig1Val != sig2Val)
			return -1;
	}

	return 0;
}

void DiskScanner::printCompareSig(unsigned char *sig1, unsigned char *sig2, int size)
{
	printf(">> s1=");
	for (int i = 0; i < size; i++)
		printf("%X", sig1[i]);
	printf(", s2=");
	for (int i = 0; i < size; i++)
		printf("%X", sig2[i]);
	printf("...");
}


inline int DiskScanner::hexCheck(unsigned char *sig1, unsigned char *sig2, int sigSize)
{
	// Compare hex strings.
	for (int i = 0; i < sigSize; i++)
	{
		unsigned int chSig1 = sig1[i];
		unsigned int chSig2 = sig2[i];

		if (chSig1 > chSig2) {
			return 1;
		}

		else if (chSig1 < chSig2) {
			return -1;
		}
	}

	return 0;
}

int DiskScanner::binarySearch(unsigned char *sig, int min, int max, int sigSize)
{
	// Not found.
	if (min > max)
		return -1;

	// Find the midpoint.
	int mid = min + ((max - min) / 2);

	// Get to the middle of the array list.
	std::vector<SIG_DATA>::iterator sigDataIterator = this->sigDataList.begin();
	sigDataIterator += mid;

	// Check to see if the middle signature is greater/less than the key sig.
	int res = hexCheck(sigDataIterator._Ptr->sigHeader, sig, sigDataIterator._Ptr->sigLength);

	// Middle sig is greater.
	if (res == 1)
	{
		binarySearch(sig, min, mid - 1, sigSize);
	}
	// Key sig is greater.
	else if (res == -1)
	{
		binarySearch(sig, mid + 1, max, sigSize);
	}
	// Key has been found.
	else
	{
		MessageBox(NULL, "A key has been found", "Notice", NULL);
		// Check to see if a larger key can be found.
		int result = 0;
		do 
		{
			if(mid < this->sigDataList.size()-1)
			{
				sigDataIterator++;
	
				result = hexCheck(sigDataIterator._Ptr->sigHeader, sig, sigDataIterator._Ptr->sigLength);
				if (result == 0)
					mid++;
			}
			else
				result = -1;

		} while (result == 0);

		// Return the signature which best fits.
		this->scanResult[mid]++;
		return 0;
	}

	return -1;
}

bool DiskScanner::isSigInRange(unsigned char *sig)
{
	if ((hexCheck(sig, this->sigDataList[0].sigHeader, this->sigDataList[0].sigLength) == 1) && ((hexCheck(sig, this->sigDataList[numSigs - 1].sigHeader, this->sigDataList[numSigs - 1].sigLength)) == -1))
		return true;

	return false;
}

void DiskScanner::buildScanner(unsigned int chunkSize, unsigned int sectorSize, char *diskPath, unsigned int startOffset, unsigned int maxSize)
{
	// Build the scanner object.
	this->chunkSize = chunkSize;
	this->sectorSize = sectorSize;
	this->startOffset = startOffset;
	this->diskPath = new char[strlen(diskPath)];
	strcpy_s(this->diskPath, strlen(diskPath) + 1, diskPath);

	this->diskHandle = INVALID_HANDLE_VALUE;

	#ifdef DEBUG_MODE
	printf(">> Building scanner with parameters...\n chunkSize: %u\nsectorSize: %u\nstartOffset: %u\ndiskPath: %s\n",
		this->chunkSize, this->sectorSize, this->startOffset, this->diskPath);
	#endif

	this->numSigs = 0;
	this->maxSize = maxSize;
}

int DiskScanner::mountVolume()
{
	#ifdef DEBUG_MODE
	printf(">> Mounting volume\n");
	printf(">> Chunk size: %u\nSector Size: %u\nOffset: %u\nVolume Path: %s\n", this->chunkSize, this->sectorSize, this->startOffset, this->diskPath);
	#endif

	// Create a handle to the hard disk.
	this->diskHandle = CreateFile((LPCSTR)this->diskPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	// Check to see if the handle is valid.
	if (this->diskHandle == INVALID_HANDLE_VALUE)
	{
		// Get the last system error message and output it to the console.
		#ifdef DEBUG_MODE
		DWORD errorCode = GetLastError();
		LPVOID errorBuffer;

		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode,
			LANG_NEUTRAL, (LPTSTR)&errorBuffer, 0, NULL);

		printf(">> Mount volume failed with message: [%u] %s\n", errorCode, errorBuffer);
		#endif

		return -1;
	}

	#ifdef DEBUG_MODE
	printf(">> Mount succeeded! Moving to start offset\n");
	#endif

	// Skip to the disk offset.
	SetFilePointer(this->diskHandle, this->startOffset, NULL, FILE_CURRENT);

	#ifdef DEBUG_MODE
	printf(">> Disk ready for scanning.\n");
	#endif

	return 0;
}

int DiskScanner::unmountVolume()
{
	#ifdef DEBUG_MODE
	printf(">> Preparing to unmount volume.\n");
	#endif
	
	// Check to see if the handle is open, then close it.
	if (this->diskHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(this->diskHandle);
		this->diskHandle = INVALID_HANDLE_VALUE;

		#ifdef DEBUG_MODE
		printf(">> Successfully unmounted volume. Handle to device has been invalidated.\n");
		#endif

		return 0;
	}

	#ifdef DEBUG_MODE
	printf(">> Unable to unmount volume.\n");
	#endif
	
	return -1;
}

void DiskScanner::addSignature(unsigned int sigID, unsigned int sigLength, unsigned char *sigHeader)
{
	#ifdef DEBUG_MODE
	printf(">> Adding Signature: %u", sigID);
	#endif

	// Add a new signature from the sigData to the vector database.
	SIG_DATA dataPoint;
	dataPoint.sigID = sigID;
	dataPoint.sigHeader = new unsigned char[sigLength];
	dataPoint.sigLength = sigLength;

	// Save the signature data.
	for (unsigned int i = 0; i < sigLength; i++)
		dataPoint.sigHeader[i] = sigHeader[i];

	#ifdef DEBUG_MODE
	printf("... Complete\n");
	#endif
	this->sigDataList.push_back(dataPoint);
}

void DiskScanner::lockSignatureList()
{
	// Create an array to hold the header count results.
	this->scanResult = new unsigned int[sigDataList.size()];

	// Set each header to 0 (Array index is the signature ID).
	for (int i = 0; i < sigDataList.size(); i++)
		this->scanResult[i] = 0;

	// Save the size for faster reading.
	this->numSigs = sigDataList.size();
}

unsigned int *DiskScanner::scanChunk()
{
	if (this->numSigs < 1)
		return NULL;

	#ifdef DEBUG_MODE
	printf(">> Beginning Scan:\n");
	#endif

	int uCharSize = sizeof(unsigned char);

	// Reserve space for chunk data.
	unsigned char *chunkData = new unsigned char[this->sectorSize*uCharSize*this->chunkSize];
	unsigned char *chunkPtr;
	DWORD bytesRead = 0;

	// Reserve space for header data.
	unsigned char *headerSet = new unsigned char[this->maxSize*this->chunkSize];
	unsigned char *headerSetPtr = headerSet;

	unsigned int numDiskSigs = 0;

	// Read in the sector.
	if (FAILED(ReadFile(this->diskHandle, chunkData, this->sectorSize*this->chunkSize, &bytesRead, NULL)))
		return NULL;

	// Get a pointer to the start of the data.
	chunkPtr = chunkData;
	headerSetPtr = headerSet;

	for (unsigned int i = 0; i < this->chunkSize; i++)
	{
		memcpy(headerSetPtr, chunkPtr, uCharSize*this->maxSize);
		headerSetPtr += maxSize*uCharSize;
		chunkPtr += this->sectorSize;
	}

	// TODO: CHECK TO SEE IF ALL VALUES ARE 0, IF SO RETURN.
	// Reset the chunk pointer to the start of the data.
	headerSetPtr = headerSet;

	for (unsigned int i = 0; i < this->chunkSize; i++)
	{
		// Used to store the best fit signature for each header.
		SIG_DATA *tempSig = NULL;

		if(isSigInRange(headerSetPtr))
		{
			for (unsigned int j = 0; j < this->numSigs; j++)
			{
				#ifdef DEBUG_MODE
				printCompareSig(headerSetPtr, this->sigDataList[i].sigHeader, this->sigDataList[i].sigLength*uCharSize);
				#endif

				// Compare the current signature with the valid header.
				if (compareSig(headerSetPtr, this->sigDataList[j].sigHeader, this->sigDataList[j].sigLength*uCharSize) == 0)
				{
					#ifdef DEBUG_MODE
					printf("match\n");
					#endif

					// The signatures are equal, get the ID and incrememt the resultSet.
					if (tempSig == NULL)
					{
						// This is the first signature that matches the read in header.
						this->scanResult[j]++;
						tempSig = &this->sigDataList[j];
					}
					else
					{
						// There are multiple signatures that match the header. Compare the temporary signature with the new header; discard the temporary signature if the new header length is greater.
						if (tempSig->sigLength < this->sigDataList[j].sigLength)
						{
							this->scanResult[tempSig->sigID]--;
							this->scanResult[j]++;
							tempSig = &this->sigDataList[j];
						}
					}
				}
				else
				{
					#ifdef DEBUG_MODE
					printf("no match\n");
					#endif
				}
			}
		}

		headerSetPtr += maxSize*uCharSize;
	}

	// Free memory and return results.
	delete[] chunkData;
	delete[] headerSet;

	#ifdef DEBUG_MODE

	printf(">> Scan Results\n");
	for (int i = 0; i < this->sigDataList.size(); i++)
	{
		printf(">> Signature ID:%u\t Headers Found: %u\n", i, scanResult[i]);
	}

	printf(">> Scanning complete.\n");
	#endif

	return this->scanResult;
}

unsigned int *DiskScanner::scanChunk_BST()
{
	if (this->numSigs < 1)
		return NULL;

	#ifdef DEBUG_MODE
	printf(">> Beginning Scan:\n");
	#endif

	int uCharSize = sizeof(unsigned char);

	// Reserve space for chunk data.
	unsigned char *chunkData = new unsigned char[this->sectorSize*uCharSize*this->chunkSize];
	unsigned char *chunkPtr;
	DWORD bytesRead = 0;

	// Reserve space for header data.
	unsigned char *headerSet = new unsigned char[this->maxSize*this->chunkSize];
	unsigned char *headerSetPtr = headerSet;

	unsigned int numDiskSigs = 0;

	// Read in the sector.
	if (FAILED(ReadFile(this->diskHandle, chunkData, this->sectorSize*this->chunkSize, &bytesRead, NULL)))
		return NULL;

	// Get a pointer to the start of the data.
	chunkPtr = chunkData;
	headerSetPtr = headerSet;

	for (unsigned int i = 0; i < this->chunkSize; i++)
	{
		memcpy(headerSetPtr, chunkPtr, uCharSize*this->maxSize);
		headerSetPtr += maxSize*uCharSize;
		chunkPtr += this->sectorSize;
	}

	// TODO: CHECK TO SEE IF ALL VALUES ARE 0, IF SO RETURN.
	// Reset the chunk pointer to the start of the data.
	headerSetPtr = headerSet;
	
	// Check each signature in the signature database with each signature found in the scanning.
	for (unsigned int i = 0; i < this->chunkSize; i++)
	{
		if(isSigInRange(headerSetPtr))
		{
			#ifdef DEBUG_MODE
			printf(">> Finding signature: ");
			for (unsigned int i = 0; i < this->maxSize; i++)
				printf("%X", headerSetPtr[i]);
			#endif
		
			// Recursively binary search through the header database vector for the current chunkPtr.
			if (binarySearch(headerSetPtr, 0, this->numSigs, this->maxSize) == 0)
			{
				#ifdef DEBUG_MODE
				printf(" ...match\n");
				#endif
			}
			else
			{
				#ifdef DEBUG_MODE
				printf(" ...no match\n");
				#endif
			}
		}

		headerSetPtr += maxSize*uCharSize;
	}

	// Free memory and return results.
	delete[] chunkData;
	delete[] headerSet;
	
	#ifdef DEBUG_MODE
	
	printf(">> Scan Results\n");
	for (int i = 0; i < this->sigDataList.size(); i++)
	{
		printf(">> Signature ID:%u\t Headers Found: %u\n", i, scanResult[i]);
	}
	
	printf(">> Scanning complete.\n");
	#endif

	return this->scanResult;
}

unsigned int *DiskScanner::scanChunkBySector()
{
	if (this->numSigs < 1)
		return NULL;

	#ifdef DEBUG_MODE
	printf(">> Beginning Scan:\n");
	#endif

	int uCharSize = sizeof(unsigned char);

	// Reserve space for chunk data.
	unsigned char *chunkData = new unsigned char[this->sectorSize*uCharSize];
	unsigned char *chunkPtr;
	DWORD bytesRead = 0;

	unsigned int numDiskSigs = 0;

	// Scan the chunks into memory; (i == sector number).
	for (unsigned int i = 0; i < this->chunkSize; i++)
	{
		// Read in the sector.
		if (FAILED(ReadFile(this->diskHandle, chunkData, this->sectorSize, &bytesRead, NULL)))
			return NULL;

		// Get a pointer to the start of the data.
		chunkPtr = chunkData;

		SIG_DATA *tempSig = NULL;
		for (unsigned int j = 0; j < this->numSigs; j++)
		{
			#ifdef DEBUG_MODE
			printCompareSig(chunkPtr, this->sigDataList[j].sigHeader, this->sigDataList[j].sigLength*uCharSize);
			#endif

			// Compare the current signature with the valid header.
			if (compareSig(chunkPtr, this->sigDataList[j].sigHeader, this->sigDataList[j].sigLength*uCharSize) == 0)
			{
				#ifdef DEBUG_MODE
				printf("match\n");
				#endif

				// The signatures are equal, get the ID and incrememt the resultSet.
				if (tempSig == NULL)
				{
					// This is the first signature that matches the read in header.
					this->scanResult[j]++;
					tempSig = &this->sigDataList[j];
				}
				else
				{
					// There are multiple signatures that match the header. Compare the temporary signature with the new header; discard the temporary signature if the new header length is greater.
					if (tempSig->sigLength < this->sigDataList[j].sigLength)
					{
						this->scanResult[tempSig->sigID]--;
						this->scanResult[j]++;
						tempSig = &this->sigDataList[j];
					}
				}
			}
			else
			{
				#ifdef DEBUG_MODE
				printf("no match\n");
				#endif
			}
		}
	}

	// Free memory and return results.
	delete[] chunkData;

	#ifdef DEBUG_MODE

	printf(">> Scan Results\n");
	for (int i = 0; i < this->sigDataList.size(); i++)
	{
		printf(">> Signature ID:%u\t Headers Found: %u\n", i, scanResult[i]);
	}

	printf(">> Scanning complete.\n");
	#endif

	return this->scanResult;
}

unsigned int *DiskScanner::scanChunkBySector_BST()
{
	if (this->numSigs < 1)
		return NULL;

	#ifdef DEBUG_MODE
	printf(">> Beginning Scan:\n");
	#endif

	int uCharSize = sizeof(unsigned char);

	// Reserve space for chunk data.
	unsigned char *chunkData = new unsigned char[this->sectorSize*uCharSize];
	unsigned char *chunkPtr;
	DWORD bytesRead = 0;

	unsigned int numDiskSigs = 0;

	// Scan the chunks into memory; (i == sector number).
	for (unsigned int i = 0; i < this->chunkSize; i++)
	{
		// Read in the sector.
		if (FAILED(ReadFile(this->diskHandle, chunkData, this->sectorSize, &bytesRead, NULL)))
			return NULL;

		// Get a pointer to the start of the data.
		chunkPtr = chunkData;

		#ifdef DEBUG_MODE
		printf(">> Finding signature: ");
		for (unsigned int i = 0; i < this->maxSize; i++)
			printf("%X", chunkPtr[i]);
		#endif

		if (isSigInRange(chunkPtr))
		{
			// Recursively binary search through the header database vector for the current chunkPtr.
			if (binarySearch(chunkPtr, 0, this->numSigs, this->maxSize) == 0)
			{
				#ifdef DEBUG_MODE
				printf("... match\n");
				#endif
			}
			else
			{
				#ifdef DEBUG_MODE
				printf("... no match\n");
				#endif
			}
		}
	}

	// Free memory and return results.
	delete[] chunkData;

	#ifdef DEBUG_MODE

	printf(">> Scan Results\n");
	for (int i = 0; i < this->sigDataList.size(); i++)
	{
		printf(">> Signature ID:%u\t Headers Found: %u\n", i, scanResult[i]);
	}

	printf(">> Scanning complete.\n");
	#endif

	return this->scanResult;
}

int DiskScanner::moveOffset(long offset)
{
	// Skip to a new disk offset.
	if(FAILED(SetFilePointer(this->diskHandle, offset, NULL, FILE_CURRENT)))
		return -1;

	return 0;
}
