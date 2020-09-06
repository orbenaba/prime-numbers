//Father process
#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#define FILEPATH "D:\\MutexParentProcess\\Debug\\mutexParentProcess.exe"
#define PROCESSES_AMOUNT 5

int main()
{
	int i;
	int size_prime_area = 80000;//72,000 = 1,000,000/ln(1,000,000) + 8000(spair)
	//size_prime_area * 4 cause each integer is 4 bytes + 4 bytes for the shared index
	HANDLE hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, (size_prime_area * 4), "prime_area");
	//Shared index to the processes
	HANDLE hIndex = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 4,"index");
	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Could not create mappinf area %d.\n"), GetLastError());
		return 1;
	}
	//Shared mutex to all the son - processes

	HANDLE mutex = CreateMutexA(NULL, FALSE, "mutex");
	//Create 5 processes
	STARTUPINFOA si;
	PROCESS_INFORMATION pi[PROCESSES_AMOUNT];
	HANDLE handles[PROCESSES_AMOUNT];
	LPCSTR pFile = FILEPATH;
	INT size = strlen(FILEPATH) + 16;
	//length is increased by 16 because:
	//2 - two spaces
	//13 - digits of the range
	//1 - null terminator
	PCHAR param = (PCHAR)malloc(size * sizeof(CHAR));
	for (i = 0; i < PROCESSES_AMOUNT; i++)
	{
		sprintf_s(param, size, "%s %d %d", FILEPATH, i * 200000, (i + 1) * 200000);
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi[i], sizeof(pi[i]));
		handles[i] = CreateProcessA(NULL, param, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi[i]);
	}
	//Ensuring that all the processes were done
	WaitForMultipleObjects(PROCESSES_AMOUNT, handles, TRUE, INFINITE);
	CloseHandle(mutex);
	free(param);
	//destrcut the handles
	for (i = 0; i < PROCESSES_AMOUNT; i++)
	{
		CloseHandle(handles[i]);
		CloseHandle(pi[i].hProcess);
		CloseHandle(pi[i].hThread);
	}
	CloseHandle(hMapFile);
	return 0;
}
