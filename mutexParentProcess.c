//Son processes
#include <windows.h>
#include <stdio.h>
void findPrimes(int low, int high);

int main(int argc, char**argv)
{
	if (argc != 3)
	{
		perror("Usage [%s] = 3\n",argv[0]);
		exit(-1);
	}
	int lowBound = atoi(argv[1]);
	int highBound = atoi(argv[2]);
	findPrimes(lowBound, highBound);
	return 0;	
}


void findPrimes(int low, int high)
{
	HANDLE mutex = CreateMutexA(NULL, FALSE, "mutex");
	HANDLE hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, "prime_area");
	HANDLE hIndex = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, "index");
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	int mem_buf_size = si.dwAllocationGranularity;
	int file_location = 0;
	int i, j;
	int flag, countPrimes = 0;
	LPSTR pBuf = (LPSTR)MapViewOfFile(hMapFile, FILE_ALL_ACCESS, 0, file_location, mem_buf_size);
	PINT arr = (PINT)pBuf;
	PINT index = (PINT)MapViewOfFile(hIndex, FILE_ALL_ACCESS, 0, 0, 4);
	printf("in %d level1\n", getpid());
	low = low % 2 ? low:low +1;
	for (i = low; i < high; i+=2)
	{
		flag = 0;
		for (j = 2; j * j <= i; j++)
			if (i % j == 0)
			{
				flag = 1;
				break;
			}
		//Find a prime = write it to the shared memory
		if (!flag)
		{
			printf("meanning to insert new prime %d\n", getpid());
			//Using the same mutex that the father gave to its sons
			DWORD waitResult = WaitForSingleObject(mutex, INFINITE);
			if (waitResult != WAIT_OBJECT_0)
				return -1;
			//arr[0] is the index of the size
			printf("sizeof(arr) = %d\n", sizeof(arr));
			arr[(*index)%mem_buf_size] = i;
			(*index)++;
			countPrimes++;
			printf("index = %d\n", *index);
			//Check if we need to pass over the next block
			if (countPrimes * (*index) % mem_buf_size)
			{
				printf("Switch between two map memories %d\n", getpid());
				UnmapViewOfFile(hMapFile);
				pBuf = (LPSTR)MapViewOfFile(hMapFile, FILE_ALL_ACCESS, 0, (*index) / mem_buf_size,mem_buf_size);
				arr = (PINT)pBuf;
			}
			ReleaseMutex(mutex);
		}
	}
	CloseHandle(hMapFile);
	CloseHandle(mutex);
	CloseHandle(hIndex);
	printf("\nTotal prime in range [%d, %d] is %d\n", low, high, countPrimes);
}