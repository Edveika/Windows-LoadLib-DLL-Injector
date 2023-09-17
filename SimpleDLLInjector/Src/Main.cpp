#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

DWORD GetProcId(const char* procName);

int main(int argc, const char* argv[])
{
	if (argc != 3)
	{
		std::cout << "USAGE: ./app_name path_to_dll process_name" << std::endl;
		return 1;
	}

	// procId
	DWORD procId = 0;

	// Wait for proc Id
	while (!procId)
		// Get process Id
		procId = GetProcId(argv[3]);

	// Call OpenProcess with read & write premissions
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, procId);

	// Check if hProcess is not NULL or invalid access
	if (hProc && hProc != INVALID_HANDLE_VALUE)
	{
		// Call VirtualAllocEx & Allocate memory in external process
		// It knows which process via hProc HANDLE
		// How much memory we need for our string path? MAX_PATH - longest length for a string that represents the path
		// MEM_COMMIT - real commited memory
		// MEM_RESERVE - reserved memory
		// PAGE_READWRITE because we need read & write premissions
		void* location = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		// Location cannot be 0
		if (location)
			WriteProcessMemory(hProc, location, argv[2], strlen(argv[2]) + 1, 0);

		// Create remote thread 
		HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, location, 0, 0);

		// If hThread is done calling
		if (hThread)
			// We are done, close the thread
			CloseHandle(hThread);
	}

	// If process is found
	if (hProc)
		// Close process handle
		CloseHandle(hProc);

	return 0;
}

DWORD GetProcId(const char* procName)
{
	DWORD procId = 0;
	// Create snapshot fo the processes
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	// Make sure the process is not NULL & not invalid handle
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		// procEntry receives each process entry from snapshot
		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(procEntry);

		// Get the first process
		if (Process32First(hSnap, &procEntry))
		{
			do
			{
				// Do a string compare
				// NOTE: _stricmp is string insensitive compare
				if (!_stricmp(procEntry.szExeFile, procName))
				{
					// Grab the process Id from procEntry
					procId = procEntry.th32ProcessID;
					// Break
					break;
				}
				// Loop through processes by calling Process32Next
				// &procEntry keeps receiving new proc entry
			} while (Process32Next(hSnap, &procEntry));
		}
	}
	// Close handle
	CloseHandle(hSnap);
	// Return the procId
	return procId;
}