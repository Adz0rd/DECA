#include "DiskScanner.h"

IDiskScanner::~IDiskScanner() {}

DiskScanner::~DiskScanner()
{
	#ifdef DEBUG_MODE
	printf(">> Destructing DiskScanner object\n");
	#endif

	delete diskPath;
}

int DiskScanner::compareSig(unsigned char *sig1, unsigned char *sig2, int size)
{
	unsigned char sig1Val = 0;
	unsigned char sig2Val = 0;

	for (int i = 0; i < size; i++)
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


int DiskScanner::hexCheck(unsigned char *sig1, unsigned char *sig2, int sigSize)
{
	char *chSig1 = new char[sigSize + 1];
	char *chSig2 = new char[sigSize + 1];

	long sig1Val = 0;
	long sig2Val = 0;

	for (int i = 0; i < sigSize; i++)
	{
		chSig1[i] = sig1[i];
		chSig2[i] = sig2[i];
	}

	chSig1[sigSize] = '\0';
	chSig2[sigSize] = '\0';

	sig1Val = strtol(chSig1, NULL, 16);
	sig2Val = strtol(chSig2, NULL, 16);

	delete[] chSig1;
	delete[] chSig2;

	if (sig1Val > sig2Val)
		return 1;
	else if (sig1Val < sig2Val)
		return -1;
	else
		return 0;
}

int DiskScanner::binarySearch(unsigned char *arrayList, unsigned char *sig, int min, int max, int sigSize)
{
	// Not found.
	if (min > max)
		return -1;

	// Find the midpoint.
	int mid = min + ((max - min) / 2);

	// Get to the middle of the array list.
	unsigned char *arrayListPtr = arrayList;
	arrayListPtr += (mid*sigSize);

	// Check to see if the middle signature is greater/less than the key sig.
	int res = hexCheck(arrayListPtr, sig, sigSize);

	// Middle sig is greater.
	if (res == 1)
	{
		binarySearch(arrayList, sig, min, mid - 1, sigSize);
	}
	// Key sig is greater.
	else if (res == -1)
	{
		binarySearch(arrayList, sig, mid + 1, max, sigSize);
	}
	// Key has been found.
	else
		return 0;

	return -1;
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

void DiskScanner::addSignature(SIG_DATA *sigData)
{
	printf(">> Adding Signature: %u", sigData->sigID);
	SIG_DATA dataPoint;

	dataPoint.sigID = sigData->sigID;
	dataPoint.sigHeader = new unsigned char[maxSize];

	for (int i = 0; i < maxSize; i++)
		dataPoint.sigHeader[i] = sigData->sigHeader[i];

	printf("... Complete\n");
	this->sigDataList.push_back(dataPoint);
}

void DiskScanner::lockSignatureList()
{
	this->scanResult = new unsigned int[sigDataList.size()*2];

	int i = 0;
	for (int j = 0; j < sigDataList.size(); j++)
	{
		this->scanResult[i] = this->sigDataList[j].sigID;
		this->scanResult[i+1] = 0;
		i+=2;
	}

	this->numSigs = sigDataList.size();
}

unsigned int *DiskScanner::scanChunkDatabase()
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

	// Reserve space for header data.
	unsigned char *headerSet = new unsigned char[this->maxSize*this->chunkSize];
	unsigned char *headerSetPtr = headerSet;

	// Scan the chunks into memory; (i == sector number).
	for (unsigned int i = 0; i < this->chunkSize; i++)
	{
		// Read in the sector.
		if (FAILED(ReadFile(this->diskHandle, chunkData, this->sectorSize, &bytesRead, NULL)))
			return NULL;

		// Get a pointer to the start of the data.
		chunkPtr = chunkData;

		// Add the headers into a temporary buffer.
		memcpy(headerSetPtr, chunkPtr, uCharSize*this->maxSize);
		headerSetPtr += maxSize*uCharSize;
	}
	// TODO: CHECK TO SEE IF ALL VALUES ARE 0, IF SO RETURN.
	// Reset the chunk pointer to the start of the data.
	headerSetPtr = headerSet;

	for (unsigned int i = 0; i < this->numSigs; i++)
	{
		for (unsigned int j = 0; j < this->numSigs; j++)
		{
			#ifdef DEBUG_MODE
			printCompareSig(headerSetPtr, this->sigDataList[i].sigHeader, maxSize*uCharSize);
			#endif

			// Compare the current signature with the valid header.
			if (compareSig(headerSetPtr, this->sigDataList[i].sigHeader, maxSize*uCharSize) == 0)
			{
				#ifdef DEBUG_MODE
				printf("match\n");
				#endif

				// The signatures are equal, get the ID and incrememt the resultSet.
				this->scanResult[i * 2]++;
			}
			else
			{
				#ifdef DEBUG_MODE
				printf("no match\n");
				#endif
			}


			headerSetPtr += maxSize*uCharSize;
		}
	}

	// Free memory and return results.
	delete[] chunkData;
	delete[] headerSet;

	#ifdef DEBUG_MODE

	printf(">> Scan Results\n");
	int j = 0;
	for (int i = 0; i < this->sigDataList.size(); i++)
	{
		printf(">> Signature ID:%u\t Headers Found: %u\n", scanResult[j], scanResult[j+1]);
		j += 2;
	}

	printf(">> Scanning complete.\n");
	#endif

	return this->scanResult;
}

int DiskScanner::scanChunk(SIG_ARR *sigArray, Response *returnStruct)
{/*
	// Temporary pointer to the signature structure.
	SIG_ARR *sigPtr = sigArray;
	int uCharSize = sizeof(unsigned char);

	// Reserve space for chunk data.
	unsigned char *chunkData = new unsigned char[this->sectorSize*uCharSize];
	unsigned char *chunkPtr;
	DWORD bytesRead = 0;

	// Store the headers and footers.
	unsigned char *headerSet = new unsigned char[sigPtr->maxSignatureSize*this->chunkSize];
	unsigned char *footerSet = new unsigned char[sigPtr->maxSignatureSize*this->chunkSize];

	unsigned char *headerSetPtr = headerSet;
	unsigned char *footerSetPtr = footerSet;
	
	// Scan the chunks into memory; (i == sector number).
	for (unsigned int i = 0; i < this->chunkSize; i++)
	{
		// Read in the sector.
		if (FAILED(ReadFile(this->diskHandle, chunkData, this->sectorSize, &bytesRead, NULL)))
			return -1;

		// Get a pointer to the start of the data.
		chunkPtr = chunkData;

		// Add the headers into a temporary buffer.
		memcpy(headerSetPtr, chunkPtr, uCharSize*sigPtr->maxSignatureSize);
		headerSetPtr += sigPtr->maxSignatureSize*uCharSize;

		// Add the footers into a temporary buffer.
		chunkPtr += (this->sectorSize*uCharSize) - (sigPtr->maxSignatureSize*uCharSize);
		memcpy(footerSetPtr, chunkPtr, uCharSize*sigPtr->maxSignatureSize);
		footerSetPtr += sigPtr->maxSignatureSize*uCharSize;
	}
	
	// Reset the chunk pointer to the start of the data.
	chunkPtr = chunkData;

	// Create the response struct.
	Response *responsePtr = new Response;
	
	// Initialise the struct with signature IDs.
	for (unsigned int i = 0; i < sigPtr->numSigPairs; i++)
	{
		responsePtr->scanResultsArr[i] = new ScanResult;
		responsePtr->scanResultsArr[i]->sigID = sigPtr->sigArray[i]->sigID;
		responsePtr->scanResultsArr[i]->footerCount = 0;
		responsePtr->scanResultsArr[i]->headerCount = 0;
	}
	
	// Reset the scanning pointers.
	headerSetPtr = headerSet;
	footerSetPtr = footerSet;

	// Check each signature in the signature database with each signature found in the scanning.
	for (unsigned int i = 0; i < sigPtr->numSigPairs; i++)
	{
		for (unsigned int j = 0; j < this->chunkSize; j++)
		{
			// Check to see if the header is valid and compare.
			if (sigPtr->sigArray[i]->sigHeader != NULL)
			{
				#ifdef DEBUG_MODE
				printCompareSig(headerSetPtr, sigPtr->sigArray[i]->sigHeader, sigPtr->maxSignatureSize*uCharSize);
				#endif

				// Compare the current signature with the valid header.
				if (compareSig(headerSetPtr, sigPtr->sigArray[i]->sigHeader, sigPtr->maxSignatureSize*uCharSize) == 0)
				{
					#ifdef DEBUG_MODE
					printf("match\n");
					#endif

					// The signatures are equal, get the ID and incrememt the resultSet.
					unsigned int thisID = sigPtr->sigArray[i]->sigID;

					// Increment the header result set.
					responsePtr->scanResultsArr[i]->headerCount++;
				}
				#ifdef DEBUG_MODE
				else
					printf("no match\n");
				#endif
			}

			if (sigPtr->sigArray[i]->sigFooter != NULL)
			{
				#ifdef DEBUG_MODE
				printCompareSig(footerSetPtr, sigPtr->sigArray[i]->sigFooter, sigPtr->maxSignatureSize*uCharSize);
				#endif

				// Compare the current signature with the valid footer.
				if (compareSig(footerSetPtr, sigPtr->sigArray[i]->sigFooter, sigArray->maxSignatureSize*uCharSize) == 0)
				{
					// The signatures are equal, get the ID and incrememt the resultSet.
					unsigned int thisID = sigPtr->sigArray[i]->sigID;

					// Increment the footer result set.
					responsePtr->scanResultsArr[i]->footerCount++;
				}
				#ifdef DEBUG_MODE
				else
					printf("no match\n");
				#endif
			}

			// Increment the header and footer scanning sets.
			headerSetPtr += sigPtr->maxSignatureSize*uCharSize;
			footerSetPtr += sigPtr->maxSignatureSize*uCharSize;
		}
	}

	// Free memory and return results.
	delete[] chunkData;
	delete[] headerSet;
	delete[] footerSet;
	
	memcpy(returnStruct, responsePtr, sizeof(*responsePtr));
	*/
	return 0;
}

int DiskScanner::scanChunkBST(SIG_ARR *sigArray, Response *returnStruct)
{/*
	// Temporary pointer to the signature structure.
	SIG_ARR *sigPtr = sigArray;
	int uCharSize = sizeof(unsigned char);

	// Reserve space for chunk data.
	unsigned char *chunkData = new unsigned char[this->sectorSize*uCharSize];
	unsigned char *chunkPtr;
	DWORD bytesRead = 0;

	// Store the headers and footers.
	unsigned char *headerSet = new unsigned char[sigPtr->maxSignatureSize*this->chunkSize];
	unsigned char *footerSet = new unsigned char[sigPtr->maxSignatureSize*this->chunkSize];

	unsigned char *headerSetPtr = headerSet;
	unsigned char *footerSetPtr = footerSet;

	// Scan the chunks into memory; (i == sector number).
	for (unsigned int i = 0; i < this->chunkSize; i++)
	{
		// Read in the sector.
		if (FAILED(ReadFile(this->diskHandle, chunkData, this->sectorSize, &bytesRead, NULL)))
			return -1;

		// Get a pointer to the start of the data.
		chunkPtr = chunkData;

		// Add the headers into a temporary buffer.
		memcpy(headerSetPtr, chunkPtr, uCharSize*sigPtr->maxSignatureSize);
		headerSetPtr += sigPtr->maxSignatureSize*uCharSize;

		// Add the footers into a temporary buffer.
		chunkPtr += (this->sectorSize*uCharSize) - (sigPtr->maxSignatureSize*uCharSize);
		memcpy(footerSetPtr, chunkPtr, uCharSize*sigPtr->maxSignatureSize);
		footerSetPtr += sigPtr->maxSignatureSize*uCharSize;
	}

	// Reset the chunk pointer to the start of the data.
	chunkPtr = chunkData;

	// Create the response struct.
	Response *responsePtr = new Response;

	// Initialise the struct with signature IDs.
	for (unsigned int i = 0; i < sigPtr->numSigPairs; i++)
	{
		responsePtr->scanResultsArr[i] = new ScanResult;
		responsePtr->scanResultsArr[i]->sigID = sigPtr->sigArray[i]->sigID;
		responsePtr->scanResultsArr[i]->footerCount = 0;
		responsePtr->scanResultsArr[i]->headerCount = 0;
	}

	// Reset the scanning pointers.
	headerSetPtr = headerSet;
	footerSetPtr = footerSet;

	// Check each signature in the signature database with each signature found in the scanning.
	for (unsigned int i = 0; i < sigPtr->numSigPairs; i++)
	{
		if (binarySearch(chunkPtr, sigPtr->sigArray[i]->sigHeader, 0, this->chunkSize, sigPtr->maxSignatureSize) == 0)
			responsePtr->scanResultsArr[i]->headerCount++;
	}

	// Free memory and return results.
	delete[] chunkData;
	delete[] headerSet;
	delete[] footerSet;

	memcpy(returnStruct, responsePtr, sizeof(*responsePtr));
	*/
	return 0;
}

ScanResult *DiskScanner::scanChunkList(SIG_ARR *sigArray)
{/*
	// Temporary pointer to the signature structure.
	SIG_ARR *sigPtr = sigArray;

	unsigned int maxSize = sigPtr->maxSignatureSize;
	unsigned int numPairs = sigPtr->numSigPairs;

	return NULL;

	printf("Max Size: %u\n NumPairs: %u", maxSize, numPairs);

	if ((numPairs <= 0) || maxSize <= 0)
		return NULL;

	int uCharSize = sizeof(unsigned char);

	// Reserve space for chunk data.
	unsigned char *chunkData = new unsigned char[this->sectorSize*uCharSize];
	unsigned char *chunkPtr;
	DWORD bytesRead = 0;

	// Store the headers and footers.
	unsigned char *headerSet = new unsigned char[maxSize*this->chunkSize];
	unsigned char *footerSet = new unsigned char[sigPtr->maxSignatureSize*this->chunkSize];

	unsigned char *headerSetPtr = headerSet;
	unsigned char *footerSetPtr = footerSet;

	#ifdef DEBUG_MODE
	printf("Reading chunk...\n");
	#endif

	// Scan the chunks into memory; (i == sector number).
	for (unsigned int i = 0; i < this->chunkSize; i++)
	{
		// Read in the sector.
		if (FAILED(ReadFile(this->diskHandle, chunkData, this->sectorSize, &bytesRead, NULL)))
			return NULL;

		// Get a pointer to the start of the data.
		chunkPtr = chunkData;

		// Add the headers into a temporary buffer.
		memcpy(headerSetPtr, chunkPtr, uCharSize*maxSize);
		headerSetPtr += maxSize*uCharSize;

		// Add the footers into a temporary buffer.
		chunkPtr += (this->sectorSize*uCharSize) - (maxSize*uCharSize);
		memcpy(footerSetPtr, chunkPtr, uCharSize*maxSize);
		footerSetPtr += maxSize*uCharSize;
	}

	#ifdef DEBUG_MODE
	printf("Signatures read in from HD, building return list.\n");
	#endif

	// Reset the chunk pointer to the start of the data.
	chunkPtr = chunkData;

	// Create the response struct.
	Response *responsePtr = new Response;

	responsePtr->scanResults = new ScanResult;
	ScanResult *scanResultPtr = responsePtr->scanResults;
	SIG_DATA *sigDataPtr = this->sigDataList[0];

	scanResultPtr->sigID = sigDataPtr->sigID;
	scanResultPtr->headerCount = 0;

	// Initialise the struct with signature IDs.
	for (unsigned int i = 1; i < numPairs; i++)
	{
		scanResultPtr->next = new ScanResult;
		sigDataPtr = sigDataPtr->next;

		scanResultPtr->sigID = sigDataPtr->sigID;
		scanResultPtr->footerCount = 0;
		scanResultPtr->headerCount = 0;
	}

	// Reset the scanning pointers.
	headerSetPtr = headerSet;
	footerSetPtr = footerSet;

	sigDataPtr = sigPtr->sigArray;
	scanResultPtr = responsePtr->scanResults;

	#ifdef DEBUG_MODE
	printf("Preparing to search for signature matches...\n");
	#endif

	// Check each signature in the signature database with each signature found in the scanning.
	for (unsigned int i = 0; i < numPairs; i++)
	{
		for (unsigned int j = 0; j < this->chunkSize; j++)
		{
			// Check to see if the header is valid and compare.
			if (sigDataPtr->sigHeader != NULL)
			{
				#ifdef DEBUG_MODE
				printCompareSig(headerSetPtr, sigDataPtr->sigHeader, maxSize*uCharSize);
				#endif

				// Compare the current signature with the valid header.
				if (compareSig(headerSetPtr, sigDataPtr->sigHeader, maxSize*uCharSize) == 0)
				{
					#ifdef DEBUG_MODE
					printf("match\n");
					#endif

					// The signatures are equal, get the ID and incrememt the resultSet.
					unsigned int thisID = sigDataPtr->sigID;

					// Increment the header result set.
					scanResultPtr->headerCount++;
				}
				#ifdef DEBUG_MODE
				else
					printf("no match\n");
				#endif
			}

			// Increment the header and footer scanning sets.
			headerSetPtr += maxSize*uCharSize;
			footerSetPtr += maxSize*uCharSize;
		}

		//sigDataPtr = sigDataPtr->next;
		scanResultPtr = scanResultPtr->next;
		headerSetPtr = headerSet;
		footerSetPtr = footerSet;

	}

	// Free memory and return results.
	delete[] chunkData;
	delete[] headerSet;
	delete[] footerSet;

	//memcpy(returnStruct, responsePtr, sizeof(*responsePtr));

	#ifdef DEBUG_MODE
	printf("Chunk scan complete. Returning.\n");
	#endif
	
	return responsePtr->scanResults;

	*/
	return NULL;
}


void DiskScanner::readAttributes(SIG_ARR *sigArray)
{/*
	//printf("maxSignatureSize: %d\nnumSigPairs: %d\nsigID: %d\n", sigArray->maxSignatureSize, sigArray->numSigPairs, sigArray->sigArray[0]->sigHeader);

	printf("Received string 1:%c%c%c%c%c%c\n", sigArray->sigArray->sigHeader[0], 
		sigArray->sigArray->sigHeader[1], 
		sigArray->sigArray->sigHeader[2], 
		sigArray->sigArray->sigHeader[3],
		sigArray->sigArray->sigHeader[4],
		sigArray->sigArray->sigHeader[5]);

	printf("Received string 2:%c%c%c%c%c%c\n", sigArray->sigArray->sigHeader[0],
		sigArray->sigArray->sigHeader[1],
		sigArray->sigArray->sigHeader[2],
		sigArray->sigArray->sigHeader[3],
		sigArray->sigArray->sigHeader[4],
		sigArray->sigArray->sigHeader[5]);
	*/	
}

int DiskScanner::scanChunkTest(SIG_ARR *sigArray)
{/*
	// Set the sector size.
	DWORD junkData;
	DISK_GEOMETRY pdg;
	DeviceIoControl(this->diskHandle, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &pdg, sizeof(pdg), &junkData, NULL);
	this->sectorSize = pdg.BytesPerSector;
	int numMatches = 0;

	int signatureSize = 3;
	unsigned char *testSignature = new unsigned char[3];
	testSignature[0] = 0xFF;
	testSignature[1] = 0xD8;
	testSignature[2] = 0xFF;

	unsigned char *chunkData = new unsigned char[this->sectorSize];
	DWORD bytesRead;

	// Scan the chunks into memory; (i == sector number).
	for (unsigned int sectorNum = 0; sectorNum < this->chunkSize; sectorNum++)
	{
		// Read in a header.
		if (FAILED(ReadFile(this->diskHandle, chunkData, this->sectorSize, &bytesRead, NULL)))
			return -1;

		unsigned char *currentSig = new unsigned char[signatureSize*sizeof(unsigned char)];
		currentSig[0] = chunkData[0];
		currentSig[1] = chunkData[1];
		currentSig[2] = chunkData[2];

		#ifdef DEBUG_MODE
		printf("[Sector %d] Signature: %X%X%X | Header: %X%X%X\n", sectorNum, testSignature[0], testSignature[1], testSignature[2], chunkData[0], chunkData[1], chunkData[2]);
		#endif

		if (compareSig(currentSig, testSignature, signatureSize) == 0)
		{
			printf("... found match\n");
			numMatches++;
		}
		else
			printf("... no match\n");

		delete currentSig;
	}

	printf("Found %d matches\n.", numMatches);

	delete chunkData;
	delete testSignature; */
	return 0;
}