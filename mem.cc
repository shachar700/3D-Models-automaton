#include <stdio.h> // fprintf
#include <stdint.h> // uintptr_t
#include <windows.h> // everything else
#include <psapi.h> // Module processing. Has to be after windows.h
#include <tlhelp32.h> // getting the PID
// gcc mem.cc -o mem -lpsapi

// https://github.com/erayarslan/WriteProcessMemory-Example
// http://stackoverflow.com/q/32798185
// http://stackoverflow.com/q/36018838
int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: mem <trans|rot> [x y z]\n");
    exit(EXIT_FAILURE);
  }
  DWORD pid;
  HMODULE module_list[1024];
  DWORD num_results;
  char name[64];
  HMODULE base_addr;
  PROCESSENTRY32 entry;
  entry.dwSize = sizeof(entry);
  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

  while (Process32Next(snapshot, &entry)) {
    if (strcmp(entry.szExeFile, "hlmv.exe") == 0) {
      pid = entry.th32ProcessID;
      break;
    }
  }

  HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
  if (!handle) {
    fprintf(stderr, "Couldn't get a handle on hlmv.\n");
    exit(EXIT_FAILURE);
  }
  EnumProcessModules(handle, module_list, sizeof(module_list) / sizeof(HMODULE), &num_results);
  for (int i = 0; i < num_results / sizeof(HMODULE); i++) {
    GetModuleBaseName(handle, module_list[i], name, sizeof(name));
    if (strcmp(name, "hlmv.exe") == 0) {
      base_addr = module_list[i];
      break; // All that setup just to get the base address and handle
    }
  }
  
  LPVOID offset;
  if (strcmp(argv[1], "rot") == 0) {
    // Rotation offset: 0x237FD8 = 2326488
    offset = (LPVOID)((uintptr_t)base_addr + 2326488);
  } else if (strcmp(argv[1], "trans") == 0) {
    // Translation offset: 0x237FE4 = 2326500
    offset = (LPVOID)((uintptr_t)base_addr + 2326500);
  } else {
    fprintf(stderr, "Unknown type parameter: %s", argv[1]);
    exit(EXIT_FAILURE);
  }
  
  if (argc == 2) {
    float values[3];
    if (!ReadProcessMemory(handle, offset, &values, sizeof(values), NULL)) {
      // For non-magic numbers, see http://stackoverflow.com/q/1387064
      wchar_t message[256];
      FormatMessageW(4096, NULL, GetLastError(), 1024, message, 256, NULL);
      fprintf(stderr, "Error: %s\n", message);
      exit(EXIT_FAILURE);
    } else {
      fprintf(stdout, "Current values of %s: %f %f %f\n", argv[1], values[0], values[1], values[2]);
    }
  } else {
    float values[3];
    values[0] = atof(argv[2]);
    values[1] = atof(argv[3]);
    values[2] = atof(argv[4]);
    if (!WriteProcessMemory(handle, offset, &values, sizeof(values), NULL)) {
      // For non-magic numbers, see http://stackoverflow.com/q/1387064
      wchar_t message[256];
      FormatMessageW(4096, NULL, GetLastError(), 1024, message, 256, NULL);
      fprintf(stderr, "Error: %s\n", message);
      exit(EXIT_FAILURE);
    }
  }
  CloseHandle(handle);
}