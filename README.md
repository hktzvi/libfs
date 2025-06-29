# Filesystem Library

This library contains variations of common operations using the Windows File API and ensures each outcome is expected. A simple program with main function is included for development.

[![Build](https://github.com/hktzvi/libfs/actions/workflows/main-push.yml/badge.svg)](https://github.com/hktzvi/libfs/actions/workflows/main-push.yml)

## Compile Example

```bash
g++ main.cpp -o testrunner.exe -municode
```

### Compilers Tested

- C:\Qt6.6.2\Tools\mingw1120_64\bin\g++.exe
- C:\TDM-GCC-64\bin\g++.exe

## Example Usage

```bash
.\testrunner.exe "Z:\Reese\win32"
```

## Example Output

```text
[OK] CreateFileW DesiredAccess=GENERIC_WRITE CreationDisposition=CREATE_ALWAYS Flags=FILE_ATTRIBUTE_NORMAL    
[OK] CreateFileW DesiredAccess=GENERIC_WRITE CreationDisposition=CREATE_NEW Flags=FILE_ATTRIBUTE_NORMAL       
[OK] CreateFileW DesiredAccess=GENERIC_WRITE CreationDisposition=TRUNCATE_EXISTING Flags=FILE_ATTRIBUTE_NORMAL
[OK] CreateFileW DesiredAccess=GENERIC_READ CreationDisposition=OPEN_EXISTING Flags=FILE_ATTRIBUTE_NORMAL     
[OK] CreateFileW DesiredAccess=GENERIC_WRITE CreationDisposition=OPEN_ALWAYS Flags=FILE_ATTRIBUTE_NORMAL      
[OK] CreateFileW DesiredAccess=GENERIC_READ|GENERIC_WRITE CreationDisposition=CREATE_ALWAYS Flags=FILE_ATTRIBUTE_NORMAL
[OK] CreateSymbolicLinkW File link created successfully
[OK] CreateSymbolicLinkW Directory link created successfully
[OK] CreateHardLinkW Original and hard link point to the same physical file
[OK] CreateHardLinkW Correctly failed to create link to nonexistent target
[OK] CreateHardLinkW Correctly failed to create hard link when link name already exists
[OK] CreateHardLinkW Modification via link correctly reflected in original
[OK] CreateDirectoryW Successfully created basic directory
[OK] CreateDirectoryW Correctly failed to create already existing directory
[OK] CreateDirectoryW Successfully created nested directory path step-by-step
[OK] CreateDirectoryW Correctly failed to create directory with invalid path. Error: 123
[OK] CreateDirectoryW Successfully created directory using relative path
[OK] CopyFileW Basic file copy succeeded with failIfExists=TRUE
[OK] CopyFileW Correctly failed to overwrite existing file with failIfExists=TRUE
[OK] CopyFileW Successfully overwrote destination with failIfExists=FALSE        
[OK] CopyFileW Correctly failed to copy from nonexistent source
[OK] CopyFileW Copied file from subdirectory to parent directory
[OK] CopyFileW Copied file from parent directory into subdirectory
[OK] CopyFileW Copied contents of symbolic link (followed target, not the link)
...
