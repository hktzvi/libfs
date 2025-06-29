#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <functional>

#define SLEEP_MSEC 1000

// Utils

void LogSuccess(const std::wstring& functionName, const std::wstring& details) {
    std::wcout << L"[OK] " << functionName << L" " << details << std::endl;
}

void LogFailure(const std::wstring& functionName, const std::wstring& message) {
    std::wcerr << L"[FAIL] " << functionName << L": " << message << std::endl;
}

bool FileExistsAndSizeEquals(const std::wstring& filePath, DWORD expectedSize) {
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(filePath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE)
        return false;
    DWORD actualSize = findData.nFileSizeLow;
    FindClose(hFind);
    return !(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (actualSize == expectedSize);
}

void WriteDummyContent(const std::wstring& filePath) {
    HANDLE h = CreateFileW(filePath.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, 0, nullptr);
    if (h != INVALID_HANDLE_VALUE) {
        const wchar_t* data = L"data";
        DWORD written = 0;
        WriteFile(h, data, 4, &written, nullptr);
        CloseHandle(h);
    }
}

// Tests

void CreateFileWCreateAlways(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);

    std::wstring path = dir + L"\\CreateAlways.txt";
    WriteDummyContent(path);

    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"CreateFileW", L"CREATE_ALWAYS failed");
        return;
    }
    CloseHandle(h);

    if (FileExistsAndSizeEquals(path, 0))
        LogSuccess(L"CreateFileW", L"DesiredAccess=GENERIC_WRITE CreationDisposition=CREATE_ALWAYS Flags=FILE_ATTRIBUTE_NORMAL");
    else
        LogFailure(L"CreateFileW", L"Expected file to be truncated");
}

void CreateFileWCreateNew(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\CreateNew.txt";
    DeleteFileW(path.c_str());

    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"CreateFileW", L"CREATE_NEW failed unexpectedly");
        return;
    }
    CloseHandle(h);

    if (FileExistsAndSizeEquals(path.c_str(), 0))
        LogSuccess(L"CreateFileW", L"DesiredAccess=GENERIC_WRITE CreationDisposition=CREATE_NEW Flags=FILE_ATTRIBUTE_NORMAL");
    else
        LogFailure(L"CreateFileW", L"File was not created as expected");
}

void CreateFileWTruncateExisting(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\TruncateExisting.txt";
    WriteDummyContent(path);

    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"CreateFileW", L"TRUNCATE_EXISTING failed");
        return;
    }
    CloseHandle(h);

    if (FileExistsAndSizeEquals(path.c_str(), 0))
        LogSuccess(L"CreateFileW", L"DesiredAccess=GENERIC_WRITE CreationDisposition=TRUNCATE_EXISTING Flags=FILE_ATTRIBUTE_NORMAL");
    else
        LogFailure(L"CreateFileW", L"Expected file to be truncated but size was not zero");
}

void CreateFileWOpenExisting(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\OpenExisting.txt";
    WriteDummyContent(path);

    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"CreateFileW", L"OPEN_EXISTING failed");
        return;
    }
    CloseHandle(h);

    if (FileExistsAndSizeEquals(path.c_str(), 4))
        LogSuccess(L"CreateFileW", L"DesiredAccess=GENERIC_READ CreationDisposition=OPEN_EXISTING Flags=FILE_ATTRIBUTE_NORMAL");
    else
        LogFailure(L"CreateFileW", L"File should retain dummy content");
}

void CreateFileWOpenAlways(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\OpenAlways.txt";
    DeleteFileW(path.c_str());

    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"CreateFileW", L"OPEN_ALWAYS failed");
        return;
    }
    CloseHandle(h);

    if (FileExistsAndSizeEquals(path.c_str(), 0))
        LogSuccess(L"CreateFileW", L"DesiredAccess=GENERIC_WRITE CreationDisposition=OPEN_ALWAYS Flags=FILE_ATTRIBUTE_NORMAL");
    else
        LogFailure(L"CreateFileW", L"File should have been created");
}

void CreateFileWReadWriteAccess(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\ReadWriteAccess.txt";
    DeleteFileW(path.c_str());

    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"CreateFileW", L"ReadWrite access test failed to create file");
        return;
    }

    const char* text = "abcde";
    DWORD written = 0;
    BOOL writeOK = WriteFile(h, text, 5, &written, nullptr);
    CloseHandle(h);

    if (writeOK && FileExistsAndSizeEquals(path.c_str(), 5))
        LogSuccess(L"CreateFileW", L"DesiredAccess=GENERIC_READ|GENERIC_WRITE CreationDisposition=CREATE_ALWAYS Flags=FILE_ATTRIBUTE_NORMAL");
    else
        LogFailure(L"CreateFileW", L"File was not written correctly with read/write access");
}

void CreateSymbolicLinkWFileLink(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring target = dir + L"\\CreateSymbolicLinkWFileLink_target.txt";
    std::wstring link = dir + L"\\CreateSymbolicLinkWFileLink_symlink.txt";

    // Setup target file
    HANDLE h = CreateFileW(target.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"CreateSymbolicLinkW", L"Failed to create target file");
        return;
    }
    DWORD written;
    WriteFile(h, "hello", 5, &written, nullptr);
    CloseHandle(h);

    DeleteFileW(link.c_str());

    // Attempt to create symlink
    if (!CreateSymbolicLinkW(link.c_str(), target.c_str(), 0)) {
        DWORD err = GetLastError();
        LogFailure(L"CreateSymbolicLinkW", L"File link creation failed. Error code: " + std::to_wstring(err));
        return;
    }

    // Verify that link exists and is a reparse point
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(link.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE && (findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
        FindClose(hFind);
        LogSuccess(L"CreateSymbolicLinkW", L"File link created successfully");
    } else {
        LogFailure(L"CreateSymbolicLinkW", L"Symbolic link file not found or not a reparse point");
    }
}

void CreateSymbolicLinkWDirectoryLink(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring targetDir = dir + L"\\CreateSymbolicLinkWDirectoryLink_target";
    std::wstring linkDir = dir + L"\\CreateSymbolicLinkWDirectoryLink_symlink";

    // Setup target directory
    CreateDirectoryW(targetDir.c_str(), nullptr);
    RemoveDirectoryW(linkDir.c_str());

    // Create symlink with SYMBOLIC_LINK_FLAG_DIRECTORY
    if (!CreateSymbolicLinkW(linkDir.c_str(), targetDir.c_str(), SYMBOLIC_LINK_FLAG_DIRECTORY)) {
        DWORD err = GetLastError();
        LogFailure(L"CreateSymbolicLinkW", L"Directory link creation failed. Error code: " + std::to_wstring(err));
        return;
    }

    // Verify reparse point
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(linkDir.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE && (findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
        FindClose(hFind);
        LogSuccess(L"CreateSymbolicLinkW", L"Directory link created successfully");
    } else {
        LogFailure(L"CreateSymbolicLinkW", L"Directory link not found or not a reparse point");
    }
}

void CreateHardLinkWBasic(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring original = dir + L"\\CreateHardLinkWBasic_original.txt";
    std::wstring link = dir + L"\\CreateHardLinkWBasic_link.txt";

    DeleteFileW(original.c_str());
    DeleteFileW(link.c_str());

    // Setup: create original file
    HANDLE h = CreateFileW(original.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"CreateHardLinkW", L"Failed to create original file");
        return;
    }
    WriteFile(h, "hardlink", 8, nullptr, nullptr);
    CloseHandle(h);

    // Create hard link
    if (!CreateHardLinkW(link.c_str(), original.c_str(), nullptr)) {
        DWORD err = GetLastError();
        LogFailure(L"CreateHardLinkW", L"Failed to create hard link. Error: " + std::to_wstring(err));
        DeleteFileW(original.c_str());
        return;
    }

    // Compare file IDs and volume serial numbers
    BY_HANDLE_FILE_INFORMATION info1 = {}, info2 = {};
    HANDLE h1 = CreateFileW(original.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    HANDLE h2 = CreateFileW(link.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);

    if (h1 == INVALID_HANDLE_VALUE || h2 == INVALID_HANDLE_VALUE) {
        LogFailure(L"CreateHardLinkW", L"Failed to open one or both files for identity check");
    } else if (GetFileInformationByHandle(h1, &info1) && GetFileInformationByHandle(h2, &info2)) {
        if (info1.nFileIndexHigh == info2.nFileIndexHigh &&
            info1.nFileIndexLow == info2.nFileIndexLow &&
            info1.dwVolumeSerialNumber == info2.dwVolumeSerialNumber) {
            LogSuccess(L"CreateHardLinkW", L"Original and hard link point to the same physical file");
        } else {
            LogFailure(L"CreateHardLinkW", L"Original and hard link do not match by file ID");
        }
    }

    if (h1 != INVALID_HANDLE_VALUE) CloseHandle(h1);
    if (h2 != INVALID_HANDLE_VALUE) CloseHandle(h2);
    DeleteFileW(original.c_str());
    DeleteFileW(link.c_str());
}

void CreateHardLinkWTargetMissing(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring missing = dir + L"\\CreateHardLinkWTargetMissing_missing.txt";
    std::wstring link = dir + L"\\CreateHardLinkWTargetMissing_link.txt";

    DeleteFileW(missing.c_str());
    DeleteFileW(link.c_str());

    if (!CreateHardLinkW(link.c_str(), missing.c_str(), nullptr)) {
        DWORD err = GetLastError();
        if (err == ERROR_FILE_NOT_FOUND) {
            LogSuccess(L"CreateHardLinkW", L"Correctly failed to create link to nonexistent target");
        } else {
            LogFailure(L"CreateHardLinkW", L"Failed with unexpected error: " + std::to_wstring(err));
        }
    } else {
        LogFailure(L"CreateHardLinkW", L"Unexpectedly succeeded in linking to nonexistent file");
        DeleteFileW(link.c_str());
    }
}

void CreateHardLinkWAlreadyExists(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring original = dir + L"\\CreateHardLinkWAlreadyExists_original.txt";
    std::wstring link = dir + L"\\CreateHardLinkWAlreadyExists_existing.txt";

    DeleteFileW(original.c_str());
    DeleteFileW(link.c_str());

    HANDLE h1 = CreateFileW(original.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    HANDLE h2 = CreateFileW(link.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h1 != INVALID_HANDLE_VALUE) CloseHandle(h1);
    if (h2 != INVALID_HANDLE_VALUE) CloseHandle(h2);

    if (!CreateHardLinkW(link.c_str(), original.c_str(), nullptr)) {
        DWORD err = GetLastError();
        if (err == ERROR_ALREADY_EXISTS) {
            LogSuccess(L"CreateHardLinkW", L"Correctly failed to create hard link when link name already exists");
        } else {
            LogFailure(L"CreateHardLinkW", L"Unexpected error: " + std::to_wstring(err));
        }
    } else {
        LogFailure(L"CreateHardLinkW", L"Unexpectedly succeeded when link name already exists");
    }

    DeleteFileW(original.c_str());
    DeleteFileW(link.c_str());
}

void CreateHardLinkWModifyLinkReflectsInOriginal(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring original = dir + L"\\CreateHardLinkWModifyLinkReflectsInOriginal_original.txt";
    std::wstring link = dir + L"\\CreateHardLinkWModifyLinkReflectsInOriginal_link.txt";

    DeleteFileW(original.c_str());
    DeleteFileW(link.c_str());

    // Create original and write initial data
    HANDLE hOrig = CreateFileW(original.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hOrig == INVALID_HANDLE_VALUE) {
        LogFailure(L"CreateHardLinkW", L"Failed to create original file");
        return;
    }
    WriteFile(hOrig, "start", 5, nullptr, nullptr);
    CloseHandle(hOrig);

    if (!CreateHardLinkW(link.c_str(), original.c_str(), nullptr)) {
        LogFailure(L"CreateHardLinkW", L"Failed to create link");
        DeleteFileW(original.c_str());
        return;
    }

    // Modify via link
    HANDLE hLink = CreateFileW(link.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hLink == INVALID_HANDLE_VALUE) {
        LogFailure(L"CreateHardLinkW", L"Failed to open link for writing");
        DeleteFileW(original.c_str());
        DeleteFileW(link.c_str());
        return;
    }
    SetFilePointer(hLink, 0, nullptr, FILE_BEGIN);
    WriteFile(hLink, "sync", 4, nullptr, nullptr);
    CloseHandle(hLink);

    // Read via original
    char buffer[6] = {};
    HANDLE hRead = CreateFileW(original.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hRead != INVALID_HANDLE_VALUE) {
        ReadFile(hRead, buffer, 5, nullptr, nullptr);
        CloseHandle(hRead);
    }

    if (strncmp(buffer, "synct", 5) == 0 || strncmp(buffer, "sync", 4) == 0) {
        LogSuccess(L"CreateHardLinkW", L"Modification via link correctly reflected in original");
    } else {
        LogFailure(L"CreateHardLinkW", L"Content mismatch between linked files");
    }

    DeleteFileW(original.c_str());
    DeleteFileW(link.c_str());
}

void CreateDirectoryWBasic(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\CreateDirectoryWBasic";

    // Cleanup
    RemoveDirectoryW(path.c_str());

    // Create directory
    if (CreateDirectoryW(path.c_str(), nullptr)) {
        WIN32_FIND_DATAW findData;
        HANDLE hFind = FindFirstFileW(path.c_str(), &findData);
        if (hFind != INVALID_HANDLE_VALUE && (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            FindClose(hFind);
            LogSuccess(L"CreateDirectoryW", L"Successfully created basic directory");
        } else {
            LogFailure(L"CreateDirectoryW", L"Directory creation succeeded but path not found or not a directory");
        }
    } else {
        LogFailure(L"CreateDirectoryW", L"Failed to create basic directory");
    }
}

void CreateDirectoryWAlreadyExists(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\CreateDirectoryWAlreadyExists";

    // Ensure it exists
    CreateDirectoryW(path.c_str(), nullptr);

    if (!CreateDirectoryW(path.c_str(), nullptr)) {
        DWORD err = GetLastError();
        if (err == ERROR_ALREADY_EXISTS) {
            LogSuccess(L"CreateDirectoryW", L"Correctly failed to create already existing directory");
        } else {
            LogFailure(L"CreateDirectoryW", L"Failed for unexpected reason. Error: " + std::to_wstring(err));
        }
    } else {
        LogFailure(L"CreateDirectoryW", L"Unexpectedly succeeded in creating an existing directory");
    }
}

void CreateDirectoryWWithSubdirectories(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring root = dir + L"\\CreateDirectoryWWithSubdirectories";
    std::wstring sub1 = dir + L"\\CreateDirectoryWWithSubdirectories\\sub1";
    std::wstring sub2 = dir + L"\\CreateDirectoryWWithSubdirectories\\sub1\\sub2";

    RemoveDirectoryW(sub2.c_str());
    RemoveDirectoryW(sub1.c_str());
    RemoveDirectoryW(root.c_str());

    // Step-by-step manual creation
    if (CreateDirectoryW(root.c_str(), nullptr) &&
        CreateDirectoryW(sub1.c_str(), nullptr) &&
        CreateDirectoryW(sub2.c_str(), nullptr)) {
        WIN32_FIND_DATAW data;
        HANDLE hFind = FindFirstFileW(sub2.c_str(), &data);
        if (hFind != INVALID_HANDLE_VALUE && (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            FindClose(hFind);
            LogSuccess(L"CreateDirectoryW", L"Successfully created nested directory path step-by-step");
        } else {
            LogFailure(L"CreateDirectoryW", L"Final subdirectory not found after creation");
        }
    } else {
        LogFailure(L"CreateDirectoryW", L"Failed to create nested directories manually");
    }
}

void CreateDirectoryWInvalidPath(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring invalidPath = dir + L"?:\\invalid\\path";

    if (!CreateDirectoryW(invalidPath.c_str(), nullptr)) {
        DWORD err = GetLastError();
        LogSuccess(L"CreateDirectoryW", L"Correctly failed to create directory with invalid path. Error: " + std::to_wstring(err));
    } else {
        LogFailure(L"CreateDirectoryW", L"Unexpectedly succeeded in creating a directory with invalid path");
    }
}

void CreateDirectoryWRelativePath(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L".\\CreateDirectoryWRelativePath";

    RemoveDirectoryW(path.c_str());

    if (CreateDirectoryW(path.c_str(), nullptr)) {
        WIN32_FIND_DATAW findData;
        HANDLE hFind = FindFirstFileW(path.c_str(), &findData);
        if (hFind != INVALID_HANDLE_VALUE && (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            FindClose(hFind);
            LogSuccess(L"CreateDirectoryW", L"Successfully created directory using relative path");
        } else {
            LogFailure(L"CreateDirectoryW", L"Relative directory creation succeeded but path not found or not a directory");
        }
    } else {
        LogFailure(L"CreateDirectoryW", L"Failed to create directory using relative path");
    }
}

void CopyFileWBasicCopy(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring src = dir + L"\\CopyFileWBasicCopy_source.txt";
    std::wstring dst = dir + L"\\CopyFileWBasicCopy_dest.txt";

    // Setup
    HANDLE hSrc = CreateFileW(src.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hSrc == INVALID_HANDLE_VALUE) {
        LogFailure(L"CopyFileW", L"Setup failed to create source file");
        return;
    }
    const char* data = "source content";
    DWORD written;
    WriteFile(hSrc, data, (DWORD)strlen(data), &written, nullptr);
    CloseHandle(hSrc);

    DeleteFileW(dst.c_str());

    // Execute
    BOOL result = CopyFileW(src.c_str(), dst.c_str(), TRUE);
    if (!result) {
        LogFailure(L"CopyFileW", L"Basic copy failed");
        return;
    }

    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(dst.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        FindClose(hFind);
        LogSuccess(L"CopyFileW", L"Basic file copy succeeded with failIfExists=TRUE");
    } else {
        LogFailure(L"CopyFileW", L"Destination file not found after copy");
    }
}

void CopyFileWFailIfExists(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring src = dir + L"\\CopyFileWFailIfExists_source.txt";
    std::wstring dst = dir + L"\\CopyFileWFailIfExists_dest.txt";

    // Setup source file
    HANDLE hSrc = CreateFileW(src.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hSrc == INVALID_HANDLE_VALUE) {
        LogFailure(L"CopyFileW", L"Setup failed to create source file");
        return;
    }
    DWORD written;
    WriteFile(hSrc, "A", 1, &written, nullptr);
    CloseHandle(hSrc);

    // Setup destination file
    HANDLE hDst = CreateFileW(dst.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hDst != INVALID_HANDLE_VALUE) {
        WriteFile(hDst, "B", 1, &written, nullptr);
        CloseHandle(hDst);
    }

    // Execute
    BOOL result = CopyFileW(src.c_str(), dst.c_str(), TRUE);
    if (!result) {
        LogSuccess(L"CopyFileW", L"Correctly failed to overwrite existing file with failIfExists=TRUE");
    } else {
        LogFailure(L"CopyFileW", L"Unexpectedly overwrote destination file when failIfExists=TRUE");
    }
}

void CopyFileWOverwriteAllowed(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring src = dir + L"\\CopyFileWOverwriteAllowed_source.txt";
    std::wstring dst = dir + L"\\CopyFileWOverwriteAllowed_dest.txt";

    // Setup source file
    HANDLE hSrc = CreateFileW(src.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hSrc == INVALID_HANDLE_VALUE) {
        LogFailure(L"CopyFileW", L"Setup failed to create source file");
        return;
    }
    DWORD written;
    WriteFile(hSrc, "X", 1, &written, nullptr);
    CloseHandle(hSrc);

    // Setup destination file
    HANDLE hDst = CreateFileW(dst.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hDst != INVALID_HANDLE_VALUE) {
        WriteFile(hDst, "Y", 1, &written, nullptr);
        CloseHandle(hDst);
    }

    // Execute
    BOOL result = CopyFileW(src.c_str(), dst.c_str(), FALSE);
    if (!result) {
        LogFailure(L"CopyFileW", L"Overwrite failed when failIfExists=FALSE");
        return;
    }

    // Verify content was updated (length = 1)
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(dst.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE && findData.nFileSizeLow == 1) {
        FindClose(hFind);
        LogSuccess(L"CopyFileW", L"Successfully overwrote destination with failIfExists=FALSE");
    } else {
        LogFailure(L"CopyFileW", L"File exists but size did not match expected overwrite");
    }
}

void CopyFileWSourceMissing(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring src = dir + L"\\CopyFileWSourceMissing_missing_source.txt";
    std::wstring dst = dir + L"\\CopyFileWSourceMissing_output.txt";

    DeleteFileW(src.c_str());
    DeleteFileW(dst.c_str());

    BOOL result = CopyFileW(src.c_str(), dst.c_str(), TRUE);
    if (!result) {
        LogSuccess(L"CopyFileW", L"Correctly failed to copy from nonexistent source");
    } else {
        LogFailure(L"CopyFileW", L"Unexpectedly succeeded copying from missing source file");
    }
}

void CopyFileWSubdirToParent(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring base = dir + L"\\CopyFileWSubdirToParent";
    std::wstring subdir = base + L"\\sub";
    std::wstring src = subdir + L"\\file.txt";
    std::wstring dst = base + L"\\copied_from_sub.txt";

    // Setup: create subdirectory and source file
    CreateDirectoryW(base.c_str(), nullptr);
    CreateDirectoryW(subdir.c_str(), nullptr);

    HANDLE hSrc = CreateFileW(src.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hSrc == INVALID_HANDLE_VALUE) {
        LogFailure(L"CopyFileW", L"Setup failed to create source file in subdirectory");
        return;
    }
    DWORD written;
    WriteFile(hSrc, "sub->parent", 11, &written, nullptr);
    CloseHandle(hSrc);

    DeleteFileW(dst.c_str());

    // Perform copy
    BOOL result = CopyFileW(src.c_str(), dst.c_str(), TRUE);
    if (result && FileExistsAndSizeEquals(dst, 11)) {
        LogSuccess(L"CopyFileW", L"Copied file from subdirectory to parent directory");
    } else {
        LogFailure(L"CopyFileW", L"Failed to copy from subdirectory to parent");
    }
}

void CopyFileWParentToSubdir(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring base = dir + L"\\CopyFileWParentToSubdir";
    std::wstring subdir = base + L"\\child";
    std::wstring src = base + L"\\file.txt";
    std::wstring dst = subdir + L"\\copied_from_parent.txt";

    // Setup: create parent and subdirectory and source file
    CreateDirectoryW(base.c_str(), nullptr);
    CreateDirectoryW(subdir.c_str(), nullptr);

    HANDLE hSrc = CreateFileW(src.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hSrc == INVALID_HANDLE_VALUE) {
        LogFailure(L"CopyFileW", L"Setup failed to create source file in parent");
        return;
    }
    DWORD written;
    WriteFile(hSrc, "parent->sub", 10, &written, nullptr);
    CloseHandle(hSrc);

    DeleteFileW(dst.c_str());

    // Perform copy
    BOOL result = CopyFileW(src.c_str(), dst.c_str(), TRUE);
    if (result && FileExistsAndSizeEquals(dst, 10)) {
        LogSuccess(L"CopyFileW", L"Copied file from parent directory into subdirectory");
    } else {
        LogFailure(L"CopyFileW", L"Failed to copy from parent to subdirectory");
    }
}

void CopyFileWFromSymbolicLink(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring target = dir + L"\\CopyFileWFromSymbolicLink_target.txt";
    std::wstring symlink = dir + L"\\CopyFileWFromSymbolicLink_symlink.txt";
    std::wstring copyDest = dir + L"\\CopyFileWFromSymbolicLink_copy.txt";

    // Setup: Create actual target file
    HANDLE h = CreateFileW(target.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"CopyFileW", L"Failed to create symbolic link target");
        return;
    }
    const char* content = "target content";
    DWORD written;
    WriteFile(h, content, (DWORD)strlen(content), &written, nullptr);
    CloseHandle(h);

    // Create symlink pointing to the target
    DeleteFileW(symlink.c_str());
    if (!CreateSymbolicLinkW(symlink.c_str(), target.c_str(), 0)) {
        DWORD err = GetLastError();
        LogFailure(L"CreateSymbolicLinkW", L"Failed to create symlink to file. Error: " + std::to_wstring(err));
        return;
    }

    // Remove previous copy
    DeleteFileW(copyDest.c_str());

    // Perform copy using the symlink as source
    if (!CopyFileW(symlink.c_str(), copyDest.c_str(), TRUE)) {
        LogFailure(L"CopyFileW", L"Failed to copy using symbolic link as source");
        return;
    }

    // Validate copy exists and is not a reparse point (i.e. not a symlink)
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(copyDest.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        bool isNotReparse = !(findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT);
        FindClose(hFind);
        if (isNotReparse) {
            LogSuccess(L"CopyFileW", L"Copied contents of symbolic link (followed target, not the link)");
        } else {
            LogFailure(L"CopyFileW", L"Destination is unexpectedly a symbolic link");
        }
    } else {
        LogFailure(L"CopyFileW", L"Copy destination not found after symbolic link copy");
    }
}

void MoveFileWRenameSameDirectory(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring original = dir + L"\\MoveFileWRenameSameDirectory_original.txt";
    std::wstring renamed = dir + L"\\MoveFileWRenameSameDirectory_renamed.txt";

    // Setup: create the original file
    HANDLE h = CreateFileW(original.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"MoveFileW", L"Failed to create file for rename");
        return;
    }
    WriteFile(h, "abc", 3, nullptr, nullptr);
    CloseHandle(h);
    DeleteFileW(renamed.c_str());

    // Perform rename
    if (MoveFileW(original.c_str(), renamed.c_str())) {
        WIN32_FIND_DATAW findData;
        HANDLE hFind = FindFirstFileW(renamed.c_str(), &findData);
        if (hFind != INVALID_HANDLE_VALUE) {
            FindClose(hFind);
            LogSuccess(L"MoveFileW", L"Successfully renamed file in same directory");
        } else {
            LogFailure(L"MoveFileW", L"File rename succeeded but target not found");
        }
    } else {
        LogFailure(L"MoveFileW", L"Rename in same directory failed");
    }
}

void MoveFileWIntoSubdirectory(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring subdir = dir + L"\\MoveFileWIntoSubdirectory_dir";
    std::wstring original = dir + L"\\MoveFileWIntoSubdirectory.txt";
    std::wstring destination = subdir + L"\\moved.txt";

    CreateDirectoryW(subdir.c_str(), nullptr);

    // Setup file to move
    HANDLE h = CreateFileW(original.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"MoveFileW", L"Failed to create source file");
        return;
    }
    WriteFile(h, "test", 4, nullptr, nullptr);
    CloseHandle(h);
    DeleteFileW(destination.c_str());

    // Perform move
    if (MoveFileW(original.c_str(), destination.c_str())) {
        WIN32_FIND_DATAW findData;
        HANDLE hFind = FindFirstFileW(destination.c_str(), &findData);
        if (hFind != INVALID_HANDLE_VALUE) {
            FindClose(hFind);
            LogSuccess(L"MoveFileW", L"Successfully moved file into subdirectory");
        } else {
            LogFailure(L"MoveFileW", L"File move succeeded but destination not found");
        }
    } else {
        LogFailure(L"MoveFileW", L"Move into subdirectory failed");
    }
}

void MoveFileWDestinationExists(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring src = dir + L"\\MoveFileWDestinationExists_src.txt";
    std::wstring dst = dir + L"\\MoveFileWDestinationExists_dst.txt";

    // Create both source and destination
    HANDLE h1 = CreateFileW(src.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    HANDLE h2 = CreateFileW(dst.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h1 == INVALID_HANDLE_VALUE || h2 == INVALID_HANDLE_VALUE) {
        if (h1 != INVALID_HANDLE_VALUE) CloseHandle(h1);
        if (h2 != INVALID_HANDLE_VALUE) CloseHandle(h2);
        LogFailure(L"MoveFileW", L"Failed to set up source/destination files");
        return;
    }
    CloseHandle(h1);
    CloseHandle(h2);

    // Should fail due to destination already existing
    if (!MoveFileW(src.c_str(), dst.c_str())) {
        LogSuccess(L"MoveFileW", L"Correctly failed to move over existing destination");
    } else {
        LogFailure(L"MoveFileW", L"Unexpectedly overwrote destination file");
    }
}

void MoveFileWSymbolicLinkItself(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring target = dir + L"\\MoveFileWSymbolicLinkItself_target.txt";
    std::wstring link = dir + L"\\MoveFileWSymbolicLinkItself_symlink.txt";
    std::wstring moved = dir + L"\\MoveFileWSymbolicLinkItself_symlink_moved.txt";

    // Setup target file
    HANDLE h = CreateFileW(target.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"MoveFileW", L"Failed to create symlink target");
        return;
    }
    WriteFile(h, "real", 4, nullptr, nullptr);
    CloseHandle(h);

    // Create symlink
    DeleteFileW(link.c_str());
    DeleteFileW(moved.c_str());
    if (!CreateSymbolicLinkW(link.c_str(), target.c_str(), 0)) {
        LogFailure(L"CreateSymbolicLinkW", L"Could not create symbolic link");
        return;
    }

    // Move the symlink
    if (!MoveFileW(link.c_str(), moved.c_str())) {
        LogFailure(L"MoveFileW", L"Failed to move the symbolic link itself");
        return;
    }

    // Validate moved symlink
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(moved.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE && (findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
        FindClose(hFind);
        LogSuccess(L"MoveFileW", L"Successfully moved the symbolic link (not the target)");
    } else {
        LogFailure(L"MoveFileW", L"Expected reparse point at new symlink path");
    }
}

void MoveFileWFromSubdirectoryToParent(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring base = dir + L"\\MoveFileWFromSubdirectoryToParent";
    std::wstring subdir = base + L"\\sub";
    std::wstring src = subdir + L"\\file.txt";
    std::wstring dst = base + L"\\moved.txt";

    // Setup: ensure directories exist and create the file in subdir
    CreateDirectoryW(base.c_str(), nullptr);
    CreateDirectoryW(subdir.c_str(), nullptr);
    HANDLE h = CreateFileW(src.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"MoveFileW", L"Setup failed to create file in subdirectory");
        return;
    }
    WriteFile(h, "abc", 3, nullptr, nullptr);
    CloseHandle(h);
    DeleteFileW(dst.c_str());

    // Move file up one level
    if (MoveFileW(src.c_str(), dst.c_str())) {
        WIN32_FIND_DATAW findData;
        HANDLE hFind = FindFirstFileW(dst.c_str(), &findData);
        if (hFind != INVALID_HANDLE_VALUE) {
            FindClose(hFind);
            LogSuccess(L"MoveFileW", L"Successfully moved file from subdirectory to parent");
        } else {
            LogFailure(L"MoveFileW", L"Move succeeded but destination file not found");
        }
    } else {
        LogFailure(L"MoveFileW", L"Move from subdirectory to parent failed");
    }
}

void MoveFileWDirectoryMoveBasic(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring srcDir = dir + L"\\MoveFileWDirectoryMoveBasic_src";
    std::wstring dstDir = dir + L"\\MoveFileWDirectoryMoveBasic_dst";
    std::wstring nestedFile = srcDir + L"\\nested.txt";
    std::wstring expectedFile = dstDir + L"\\nested.txt";

    // Setup
    CreateDirectoryW(srcDir.c_str(), nullptr);
    HANDLE h = CreateFileW(nestedFile.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h != INVALID_HANDLE_VALUE) {
        WriteFile(h, "abc", 3, nullptr, nullptr);
        CloseHandle(h);
    }

    RemoveDirectoryW(dstDir.c_str());
    DeleteFileW(expectedFile.c_str());

    // Execute
    if (MoveFileW(srcDir.c_str(), dstDir.c_str())) {
        HANDLE hVerify = CreateFileW(expectedFile.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
        if (hVerify != INVALID_HANDLE_VALUE) {
            CloseHandle(hVerify);
            LogSuccess(L"MoveFileW", L"Successfully moved directory with nested file");
        } else {
            LogFailure(L"MoveFileW", L"Directory moved, but nested file missing");
        }
    } else {
        LogFailure(L"MoveFileW", L"Failed to move directory");
    }
}

void MoveFileWDirectoryRenameInPlace(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring original = dir + L"\\MoveFileWDirectoryRenameInPlace_original";
    std::wstring renamed = dir + L"\\MoveFileWDirectoryRenameInPlace_renamed";

    CreateDirectoryW(original.c_str(), nullptr);
    RemoveDirectoryW(renamed.c_str());

    if (MoveFileW(original.c_str(), renamed.c_str())) {
        WIN32_FIND_DATAW data;
        HANDLE h = FindFirstFileW(renamed.c_str(), &data);
        if (h != INVALID_HANDLE_VALUE) {
            FindClose(h);
            LogSuccess(L"MoveFileW", L"Successfully renamed directory in-place");
        } else {
            LogFailure(L"MoveFileW", L"Rename succeeded but target directory not found");
        }
    } else {
        LogFailure(L"MoveFileW", L"Directory rename failed");
    }
}

void MoveFileWDirectoryIntoSubdirectory(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring parent = dir + L"\\MoveFileWDirectoryIntoSubdirectory_parent";
    std::wstring child = parent + L"\\sub";

    CreateDirectoryW(parent.c_str(), nullptr);
    CreateDirectoryW(child.c_str(), nullptr);

    if (!MoveFileW(parent.c_str(), child.c_str())) {
        LogSuccess(L"MoveFileW", L"Correctly failed to move directory into its own subdirectory");
    } else {
        LogFailure(L"MoveFileW", L"Unexpectedly succeeded in moving directory into subdirectory");
        // Undo it just in case
        MoveFileW(child.c_str(), parent.c_str());
    }
}

void MoveFileWDirectoryOverwriteExisting(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring srcDir = dir + L"\\MoveFileWDirectoryOverwriteExisting_src";
    std::wstring dstDir = dir + L"\\MoveFileWDirectoryOverwriteExisting_dst";

    CreateDirectoryW(srcDir.c_str(), nullptr);
    CreateDirectoryW(dstDir.c_str(), nullptr);

    if (!MoveFileW(srcDir.c_str(), dstDir.c_str())) {
        LogSuccess(L"MoveFileW", L"Correctly failed to overwrite existing destination directory");
    } else {
        LogFailure(L"MoveFileW", L"Unexpectedly overwrote existing directory");
        MoveFileW(dstDir.c_str(), srcDir.c_str()); // Attempt recovery
    }
}

void DeleteFileWBasic(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\DeleteFileWBasic.txt";

    // Setup: create file
    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"DeleteFileW", L"Setup failed to create file");
        return;
    }
    CloseHandle(h);

    // Attempt delete
    if (DeleteFileW(path.c_str())) {
        WIN32_FIND_DATAW data;
        HANDLE hFind = FindFirstFileW(path.c_str(), &data);
        if (hFind == INVALID_HANDLE_VALUE) {
            LogSuccess(L"DeleteFileW", L"Successfully deleted existing file");
        } else {
            FindClose(hFind);
            LogFailure(L"DeleteFileW", L"DeleteFileW returned TRUE but file still exists");
        }
    } else {
        LogFailure(L"DeleteFileW", L"Failed to delete existing file");
    }
}

void DeleteFileWNotExists(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\DeleteFileWNotExists.txt";

    // Ensure file doesn't exist
    DeleteFileW(path.c_str());

    if (!DeleteFileW(path.c_str())) {
        DWORD err = GetLastError();
        if (err == ERROR_FILE_NOT_FOUND) {
            LogSuccess(L"DeleteFileW", L"Correctly failed to delete nonexistent file");
        } else {
            LogFailure(L"DeleteFileW", L"Failed with unexpected error code: " + std::to_wstring(err));
        }
    } else {
        LogFailure(L"DeleteFileW", L"Unexpectedly succeeded deleting nonexistent file");
    }
}

void DeleteFileWReadOnlyFile(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\DeleteFileWReadOnlyFile.txt";

    // Setup
    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_READONLY, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"DeleteFileW", L"Setup failed to create read-only file");
        return;
    }
    CloseHandle(h);

    // Attempt delete
    if (!DeleteFileW(path.c_str())) {
        DWORD err = GetLastError();
        if (err == ERROR_ACCESS_DENIED) {
            LogSuccess(L"DeleteFileW", L"Correctly failed to delete read-only file");
        } else {
            LogFailure(L"DeleteFileW", L"Failed with unexpected error code: " + std::to_wstring(err));
        }
    } else {
        LogFailure(L"DeleteFileW", L"Unexpectedly succeeded deleting read-only file");

        // Clean up
        SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_NORMAL);
        DeleteFileW(path.c_str());
    }
}

void DeleteFileWOnDirectory(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\DeleteFileWOnDirectory_testdir";

    // Setup
    CreateDirectoryW(path.c_str(), nullptr);

    if (!DeleteFileW(path.c_str())) {
        DWORD err = GetLastError();
        if (err == ERROR_ACCESS_DENIED || err == ERROR_FILE_NOT_FOUND) {
            LogSuccess(L"DeleteFileW", L"Correctly failed to delete a directory using DeleteFileW");
        } else {
            LogFailure(L"DeleteFileW", L"Failed on directory with unexpected error: " + std::to_wstring(err));
        }
    } else {
        LogFailure(L"DeleteFileW", L"Unexpectedly succeeded deleting a directory");
    }

    // Cleanup
    RemoveDirectoryW(path.c_str());
}

void DeleteFileWRelativePath(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L".\\DeleteFileWRelativePath.txt";

    // Setup
    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"DeleteFileW", L"Failed to create file using relative path");
        return;
    }
    CloseHandle(h);

    if (DeleteFileW(path.c_str())) {
        WIN32_FIND_DATAW data;
        HANDLE hFind = FindFirstFileW(path.c_str(), &data);
        if (hFind == INVALID_HANDLE_VALUE) {
            LogSuccess(L"DeleteFileW", L"Successfully deleted file via relative path");
        } else {
            FindClose(hFind);
            LogFailure(L"DeleteFileW", L"File deletion succeeded but file still exists");
        }
    } else {
        LogFailure(L"DeleteFileW", L"Failed to delete file via relative path");
    }
}

void DeleteFileWInSubdirectory(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring ndir = dir + L"\\DeleteFileWInSubdirectory_dir";
    std::wstring file = dir + L"\\DeleteFileWInSubdirectory_dir\\nested.txt";

    // Setup: create subdirectory and file
    CreateDirectoryW(ndir.c_str(), nullptr);
    HANDLE h = CreateFileW(file.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"DeleteFileW", L"Failed to create file in subdirectory");
        return;
    }
    WriteFile(h, "abc", 3, nullptr, nullptr);
    CloseHandle(h);

    // Attempt to delete
    if (DeleteFileW(file.c_str())) {
        HANDLE hFind = FindFirstFileW(file.c_str(), nullptr);
        if (hFind == INVALID_HANDLE_VALUE) {
            LogSuccess(L"DeleteFileW", L"Successfully deleted file located in subdirectory");
        } else {
            FindClose(hFind);
            LogFailure(L"DeleteFileW", L"DeleteFileW returned TRUE but file still exists");
        }
    } else {
        LogFailure(L"DeleteFileW", L"Failed to delete file in subdirectory");
    }

    // Cleanup
    RemoveDirectoryW(ndir.c_str());
}

void RemoveDirectoryWBasic(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\RemoveDirectoryWBasic";

    // Setup
    CreateDirectoryW(path.c_str(), nullptr);

    // Remove
    if (RemoveDirectoryW(path.c_str())) {
        WIN32_FIND_DATAW data;
        HANDLE hFind = FindFirstFileW(path.c_str(), &data);
        if (hFind == INVALID_HANDLE_VALUE) {
            LogSuccess(L"RemoveDirectoryW", L"Successfully removed empty directory");
        } else {
            FindClose(hFind);
            LogFailure(L"RemoveDirectoryW", L"RemoveDirectoryW returned TRUE but directory still exists");
        }
    } else {
        LogFailure(L"RemoveDirectoryW", L"Failed to remove empty directory");
    }
}

void RemoveDirectoryWNonEmpty(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\RemoveDirectoryWNonEmpty";
    std::wstring file = dir + L"\\RemoveDirectoryWNonEmpty\\file.txt";

    // Setup
    CreateDirectoryW(path.c_str(), nullptr);
    HANDLE h = CreateFileW(file.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h != INVALID_HANDLE_VALUE) CloseHandle(h);

    // Attempt
    if (!RemoveDirectoryW(path.c_str())) {
        DWORD err = GetLastError();
        if (err == ERROR_DIR_NOT_EMPTY) {
            LogSuccess(L"RemoveDirectoryW", L"Correctly failed to remove non-empty directory");
        } else {
            LogFailure(L"RemoveDirectoryW", L"Failed with unexpected error: " + std::to_wstring(err));
        }
    } else {
        LogFailure(L"RemoveDirectoryW", L"Unexpectedly succeeded removing non-empty directory");
    }

    // Cleanup
    DeleteFileW(file.c_str());
    RemoveDirectoryW(path.c_str());
}

void RemoveDirectoryWRelativePath(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L".\\RemoveDirectoryWRelativePath";

    // Setup
    CreateDirectoryW(path.c_str(), nullptr);

    // Remove
    if (RemoveDirectoryW(path.c_str())) {
        HANDLE hFind = FindFirstFileW(path.c_str(), nullptr);
        if (hFind == INVALID_HANDLE_VALUE) {
            LogSuccess(L"RemoveDirectoryW", L"Successfully removed directory via relative path");
        } else {
            FindClose(hFind);
            LogFailure(L"RemoveDirectoryW", L"Remove succeeded but directory still exists");
        }
    } else {
        LogFailure(L"RemoveDirectoryW", L"Failed to remove directory using relative path");
    }
}

void RemoveDirectoryWNotExist(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\RemoveDirectoryWNotExist";

    // Ensure clean state
    RemoveDirectoryW(path.c_str());

    if (!RemoveDirectoryW(path.c_str())) {
        DWORD err = GetLastError();
        if (err == ERROR_PATH_NOT_FOUND || err == ERROR_FILE_NOT_FOUND) {
            LogSuccess(L"RemoveDirectoryW", L"Correctly failed to remove nonexistent directory");
        } else {
            LogFailure(L"RemoveDirectoryW", L"Failed with unexpected error: " + std::to_wstring(err));
        }
    } else {
        LogFailure(L"RemoveDirectoryW", L"Unexpectedly succeeded removing nonexistent directory");
    }
}

void RemoveDirectoryWWithTrailingSlash(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring base = dir + L"\\RemoveDirectoryWWithTrailingSlash";
    wchar_t pathWithSlash[MAX_PATH];
    swprintf_s(pathWithSlash, L"%s\\", base.c_str());

    // Setup
    CreateDirectoryW(base.c_str(), nullptr);

    // Attempt removal
    if (RemoveDirectoryW(pathWithSlash)) {
        HANDLE hFind = FindFirstFileW(base.c_str(), nullptr);
        if (hFind == INVALID_HANDLE_VALUE) {
            LogSuccess(L"RemoveDirectoryW", L"Successfully removed directory with trailing slash");
        } else {
            FindClose(hFind);
            LogFailure(L"RemoveDirectoryW", L"Remove succeeded but directory still exists");
        }
    } else {
        LogFailure(L"RemoveDirectoryW", L"Failed to remove directory with trailing slash");
    }
}

void RemoveDirectoryWHasSubdirectory(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring parent = dir + L"\\RemoveDirectoryWHasSubdirectory";
    std::wstring child = dir + L"\\RemoveDirectoryWHasSubdirectory\\sub";

    // Setup
    CreateDirectoryW(parent.c_str(), nullptr);
    CreateDirectoryW(child.c_str(), nullptr);

    // Attempt to remove parent (should fail)
    if (!RemoveDirectoryW(parent.c_str())) {
        DWORD err = GetLastError();
        if (err == ERROR_DIR_NOT_EMPTY) {
            LogSuccess(L"RemoveDirectoryW", L"Correctly failed to remove directory containing subdirectory");
        } else {
            LogFailure(L"RemoveDirectoryW", L"Failed with unexpected error: " + std::to_wstring(err));
        }
    } else {
        LogFailure(L"RemoveDirectoryW", L"Unexpectedly succeeded removing directory that still has a subdirectory");
        // Cleanup if it succeeded (not expected)
        RemoveDirectoryW(child.c_str());
        RemoveDirectoryW(parent.c_str());
    }

    // Cleanup
    RemoveDirectoryW(child.c_str());
    RemoveDirectoryW(parent.c_str());
}

void RemoveDirectoryWSubdirectoryOfParent(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring parent = dir + L"\\RemoveDirectoryWSubdirectoryOfParent";
    std::wstring subdir = dir + L"\\RemoveDirectoryWSubdirectoryOfParent\\child";

    // Setup
    CreateDirectoryW(parent.c_str(), nullptr);
    CreateDirectoryW(subdir.c_str(), nullptr);

    // Attempt to remove child directory
    if (RemoveDirectoryW(subdir.c_str())) {
        HANDLE hFind = FindFirstFileW(subdir.c_str(), nullptr);
        if (hFind == INVALID_HANDLE_VALUE) {
            LogSuccess(L"RemoveDirectoryW", L"Successfully removed subdirectory of parent");
        } else {
            FindClose(hFind);
            LogFailure(L"RemoveDirectoryW", L"Subdirectory still exists after RemoveDirectoryW returned TRUE");
        }
    } else {
        LogFailure(L"RemoveDirectoryW", L"Failed to remove valid subdirectory");
    }

    // Cleanup parent
    RemoveDirectoryW(parent.c_str());
}

void SetFileAttributesWReadOnly(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\SetFileAttributesWReadOnly.txt";

    // Setup: create file and make it read-only
    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"SetFileAttributesW", L"Failed to create file for READONLY test");
        return;
    }
    CloseHandle(h);

    if (!SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_READONLY)) {
        LogFailure(L"SetFileAttributesW", L"Failed to set FILE_ATTRIBUTE_READONLY");
        DeleteFileW(path.c_str());
        return;
    }

    // Attempt to open file for writing — should fail
    HANDLE hWrite = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hWrite == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        if (err == ERROR_ACCESS_DENIED) {
            LogSuccess(L"SetFileAttributesW", L"READONLY file correctly blocked write access");
        } else {
            LogFailure(L"SetFileAttributesW", L"Unexpected error when opening read-only file: " + std::to_wstring(err));
        }
    } else {
        LogFailure(L"SetFileAttributesW", L"Was able to open read-only file for writing (unexpected)");
        CloseHandle(hWrite);
    }

    // Cleanup
    SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_NORMAL);
    DeleteFileW(path.c_str());
}

void SetFileAttributesWHidden(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\SetFileAttributesWHidden.txt";

    // Setup: create file
    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"SetFileAttributesW", L"Failed to create file for HIDDEN test");
        return;
    }
    CloseHandle(h);

    if (!SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_HIDDEN)) {
        LogFailure(L"SetFileAttributesW", L"Failed to set FILE_ATTRIBUTE_HIDDEN");
        DeleteFileW(path.c_str());
        return;
    }

    // Confirm file is directly findable by full name
    WIN32_FIND_DATAW data;
    HANDLE hFindDirect = FindFirstFileW(path.c_str(), &data);
    if (hFindDirect == INVALID_HANDLE_VALUE) {
        LogFailure(L"SetFileAttributesW", L"Could not find hidden file by full name");
    } else {
        FindClose(hFindDirect);

        // Confirm file is NOT found in wildcard search like "*"
        bool foundInWildcard = false;
        HANDLE hFindWildcard = FindFirstFileW(L"*", &data);
        while (hFindWildcard != INVALID_HANDLE_VALUE) {
            if (_wcsicmp(data.cFileName, path.c_str()) == 0) {
                foundInWildcard = true;
                break;
            }
            if (!FindNextFileW(hFindWildcard, &data))
                break;
        }
        if (hFindWildcard != INVALID_HANDLE_VALUE) FindClose(hFindWildcard);

        if (foundInWildcard) {
            LogFailure(L"SetFileAttributesW", L"Hidden file was unexpectedly found in wildcard search");
        } else {
            LogSuccess(L"SetFileAttributesW", L"Hidden file correctly omitted from wildcard enumeration");
        }
    }

    // Cleanup
    SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_NORMAL);
    DeleteFileW(path.c_str());
}

void SetFileAttributesWSystem(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\SetFileAttributesWSystem.txt";

    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"SetFileAttributesW", L"Failed to create file for SYSTEM test");
        return;
    }
    CloseHandle(h);

    if (SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_SYSTEM)) {
        DWORD attrs = GetFileAttributesW(path.c_str());
        if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_SYSTEM)) {
            LogSuccess(L"SetFileAttributesW", L"Successfully set FILE_ATTRIBUTE_SYSTEM");
        } else {
            LogFailure(L"SetFileAttributesW", L"SYSTEM attribute not confirmed after setting");
        }
    } else {
        LogFailure(L"SetFileAttributesW", L"Failed to set FILE_ATTRIBUTE_SYSTEM");
    }

    // Cleanup
    SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_NORMAL);
    DeleteFileW(path.c_str());
}

void SetFileAttributesWInvalidFile(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\SetFileAttributesWInvalidFile.txt";

    DeleteFileW(path.c_str()); // Ensure it does not exist

    if (!SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_READONLY)) {
        DWORD err = GetLastError();
        if (err == ERROR_FILE_NOT_FOUND) {
            LogSuccess(L"SetFileAttributesW", L"Correctly failed to set attributes on nonexistent file");
        } else {
            LogFailure(L"SetFileAttributesW", L"Failed with unexpected error: " + std::to_wstring(err));
        }
    } else {
        LogFailure(L"SetFileAttributesW", L"Unexpectedly succeeded on nonexistent file");
        SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_NORMAL);
        DeleteFileW(path.c_str());
    }
}

void SetFileAttributesWClearAttributes(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\SetFileAttributesWClearAttributes.txt";

    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"SetFileAttributesW", L"Failed to create file with preset attributes");
        return;
    }
    CloseHandle(h);

    SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY); // Explicitly apply attributes

    if (SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_NORMAL)) {
        DWORD attrs = GetFileAttributesW(path.c_str());
        if (attrs == FILE_ATTRIBUTE_NORMAL) {
            LogSuccess(L"SetFileAttributesW", L"Successfully cleared attributes to FILE_ATTRIBUTE_NORMAL");
        } else {
            LogFailure(L"SetFileAttributesW", L"Unexpected attributes after clearing: " + std::to_wstring(attrs));
        }
    } else {
        LogFailure(L"SetFileAttributesW", L"Failed to clear attributes to FILE_ATTRIBUTE_NORMAL");
    }

    DeleteFileW(path.c_str());
}

void SetEndOfFileTruncate(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\SetEndOfFileTruncate.txt";

    // Setup: create file with data
    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE | GENERIC_READ, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"SetEndOfFile", L"Failed to create file");
        return;
    }
    DWORD written;
    WriteFile(h, "1234567890", 10, &written, nullptr); // 10 bytes

    // Move pointer to position 5
    if (SetFilePointer(h, 5, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
        LogFailure(L"SetEndOfFile", L"Failed to move file pointer before truncation");
        CloseHandle(h);
        DeleteFileW(path.c_str());
        return;
    }

    if (SetEndOfFile(h)) {
        // Verify file size is now 5
        DWORD size = GetFileSize(h, nullptr);
        if (size == 5) {
            LogSuccess(L"SetEndOfFile", L"Successfully truncated file from 10 to 5 bytes");
        } else {
            LogFailure(L"SetEndOfFile", L"Expected size 5, got " + std::to_wstring(size));
        }
    } else {
        LogFailure(L"SetEndOfFile", L"Truncate call failed");
    }

    CloseHandle(h);
    DeleteFileW(path.c_str());
}

void SetEndOfFileExtend(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\SetEndOfFileExtend.txt";

    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE | GENERIC_READ, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"SetEndOfFile", L"Failed to create file");
        return;
    }

    DWORD written;
    WriteFile(h, "abc", 3, &written, nullptr); // Initial size = 3

    // Move pointer forward to offset 10
    if (SetFilePointer(h, 10, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
        LogFailure(L"SetEndOfFile", L"Failed to move file pointer for extension");
        CloseHandle(h);
        DeleteFileW(path.c_str());
        return;
    }

    if (SetEndOfFile(h)) {
        DWORD size = GetFileSize(h, nullptr);
        if (size == 10) {
            LogSuccess(L"SetEndOfFile", L"Successfully extended file from 3 to 10 bytes");
        } else {
            LogFailure(L"SetEndOfFile", L"Expected size 10, got " + std::to_wstring(size));
        }
    } else {
        LogFailure(L"SetEndOfFile", L"Extension call failed");
    }

    CloseHandle(h);
    DeleteFileW(path.c_str());
}

void SetEndOfFileAtExactSize(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\SetEndOfFileAtExactSize.txt";

    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE | GENERIC_READ, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"SetEndOfFile", L"Failed to create file");
        return;
    }

    DWORD written;
    WriteFile(h, "xyz", 3, &written, nullptr); // Size = 3

    // Move pointer to current end
    if (SetFilePointer(h, 3, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
        LogFailure(L"SetEndOfFile", L"Failed to set pointer to end");
        CloseHandle(h);
        DeleteFileW(path.c_str());
        return;
    }

    if (SetEndOfFile(h)) {
        DWORD size = GetFileSize(h, nullptr);
        if (size == 3) {
            LogSuccess(L"SetEndOfFile", L"SetEndOfFile had no effect when pointer is at EOF (as expected)");
        } else {
            LogFailure(L"SetEndOfFile", L"Unexpected file size after no-op SetEndOfFile: " + std::to_wstring(size));
        }
    } else {
        LogFailure(L"SetEndOfFile", L"Call failed when pointer was already at EOF");
    }

    CloseHandle(h);
    DeleteFileW(path.c_str());
}

void SetEndOfFileInvalidHandle(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    HANDLE hInvalid = (HANDLE)-1; // Known invalid handle

    if (!SetEndOfFile(hInvalid)) {
        DWORD err = GetLastError();
        if (err == ERROR_INVALID_HANDLE) {
            LogSuccess(L"SetEndOfFile", L"Correctly failed with ERROR_INVALID_HANDLE");
        } else {
            LogFailure(L"SetEndOfFile", L"Failed with unexpected error: " + std::to_wstring(err));
        }
    } else {
        LogFailure(L"SetEndOfFile", L"Unexpectedly succeeded with invalid handle");
    }
}

void SetEndOfFileOnReadOnlyHandle(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\SetEndOfFileOnReadOnlyHandle.txt";

    // Setup: create file
    HANDLE hWrite = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hWrite != INVALID_HANDLE_VALUE) CloseHandle(hWrite);

    HANDLE hReadOnly = CreateFileW(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hReadOnly == INVALID_HANDLE_VALUE) {
        LogFailure(L"SetEndOfFile", L"Failed to open file in read-only mode");
        DeleteFileW(path.c_str());
        return;
    }

    SetFilePointer(hReadOnly, 0, nullptr, FILE_BEGIN);

    if (!SetEndOfFile(hReadOnly)) {
        DWORD err = GetLastError();
        if (err == ERROR_ACCESS_DENIED) {
            LogSuccess(L"SetEndOfFile", L"Correctly failed to truncate with read-only handle");
        } else {
            LogFailure(L"SetEndOfFile", L"Failed with unexpected error: " + std::to_wstring(err));
        }
    } else {
        LogFailure(L"SetEndOfFile", L"Unexpectedly succeeded in truncating with read-only handle");
    }

    CloseHandle(hReadOnly);
    DeleteFileW(path.c_str());
}

void SetFileAttributesWReadonlyBlocksWrite(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\SetFileAttributesWReadonly.txt";
    DeleteFileW(path.c_str());

    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"SetFileAttributesW", L"Failed to create file");
        return;
    }
    CloseHandle(h);

    SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_READONLY);

    HANDLE hWrite = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hWrite == INVALID_HANDLE_VALUE && GetLastError() == ERROR_ACCESS_DENIED) {
        LogSuccess(L"SetFileAttributesW", L"Correctly blocked write to READONLY file");
    } else {
        LogFailure(L"SetFileAttributesW", L"Unexpectedly succeeded writing to READONLY file");
        if (hWrite != INVALID_HANDLE_VALUE) CloseHandle(hWrite);
    }

    SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_NORMAL);
    DeleteFileW(path.c_str());
}

void SetFileAttributesWHiddenHidesFromWildcard(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\SetFileAttributesWHidden.txt";
    DeleteFileW(path.c_str());

    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"SetFileAttributesW", L"Failed to create file");
        return;
    }
    CloseHandle(h);

    SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_HIDDEN);

    WIN32_FIND_DATAW data;
    HANDLE hFind = FindFirstFileW(L"*", &data);
    bool found = false;

    while (hFind != INVALID_HANDLE_VALUE) {
        if (_wcsicmp(data.cFileName, path.c_str()) == 0) {
            found = true;
            break;
        }
        if (!FindNextFileW(hFind, &data)) break;
    }
    if (hFind != INVALID_HANDLE_VALUE) FindClose(hFind);

    if (!found) {
        LogSuccess(L"SetFileAttributesW", L"HIDDEN file was correctly excluded from wildcard listing");
    } else {
        LogFailure(L"SetFileAttributesW", L"HIDDEN file was still visible in wildcard listing");
    }

    SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_NORMAL);
    DeleteFileW(path.c_str());
}

void SetFileAttributesWNormalClearsOtherFlags(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\SetFileAttributesWNormal.txt";
    DeleteFileW(path.c_str());

    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"SetFileAttributesW", L"Failed to create file");
        return;
    }
    CloseHandle(h);

    SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_NORMAL);

    DWORD attrs = GetFileAttributesW(path.c_str());
    if (attrs == FILE_ATTRIBUTE_NORMAL) {
        LogSuccess(L"SetFileAttributesW", L"Successfully cleared attributes to FILE_ATTRIBUTE_NORMAL");
    } else {
        LogFailure(L"SetFileAttributesW", L"Expected NORMAL, but found: " + std::to_wstring(attrs));
    }

    DeleteFileW(path.c_str());
}

void SetFileAttributesWOnInvalidPathFails(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\SetFileAttributesWInvalidPath.txt";
    DeleteFileW(path.c_str());  // Ensure file doesn't exist

    BOOL result = SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_READONLY);
    DWORD err = GetLastError();

    if (!result && err == ERROR_FILE_NOT_FOUND) {
        LogSuccess(L"SetFileAttributesW", L"Correctly failed on nonexistent file with ERROR_FILE_NOT_FOUND");
    } else {
        LogFailure(L"SetFileAttributesW", L"Unexpected result on invalid path. Error: " + std::to_wstring(err));
    }
}

void SetFileAttributesWDirectoryReadonly(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring ndir = dir + L"\\SetFileAttributesWDirectoryReadonly";
    std::wstring nestedFile = dir + L"\\SetFileAttributesWDirectoryReadonly\\file.txt";

    RemoveDirectoryW(ndir.c_str());
    CreateDirectoryW(ndir.c_str(), nullptr);
    SetFileAttributesW(ndir.c_str(), FILE_ATTRIBUTE_READONLY);

    HANDLE h = CreateFileW(nestedFile.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h != INVALID_HANDLE_VALUE) {
        LogSuccess(L"SetFileAttributesW", L"Was able to create file inside READONLY directory (expected)");
        CloseHandle(h);
        DeleteFileW(nestedFile.c_str());
    } else {
        LogFailure(L"SetFileAttributesW", L"Failed to create file inside READONLY directory");
    }

    SetFileAttributesW(ndir.c_str(), FILE_ATTRIBUTE_NORMAL);
    RemoveDirectoryW(ndir.c_str());
}

void SetFileAttributesWDirectoryHidden(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring ndir = dir + L"\\SetFileAttributesWDirectoryHidden";
    RemoveDirectoryW(ndir.c_str());
    CreateDirectoryW(ndir.c_str(), nullptr);

    SetFileAttributesW(ndir.c_str(), FILE_ATTRIBUTE_HIDDEN);

    WIN32_FIND_DATAW data;
    HANDLE hFind = FindFirstFileW(L"*", &data);
    bool found = false;

    while (hFind != INVALID_HANDLE_VALUE) {
        if (_wcsicmp(data.cFileName, dir.c_str()) == 0) {
            found = true;
            break;
        }
        if (!FindNextFileW(hFind, &data)) break;
    }
    if (hFind != INVALID_HANDLE_VALUE) FindClose(hFind);

    if (!found) {
        LogSuccess(L"SetFileAttributesW", L"HIDDEN directory correctly excluded from wildcard search");
    } else {
        LogFailure(L"SetFileAttributesW", L"HIDDEN directory unexpectedly appeared in wildcard listing");
    }

    SetFileAttributesW(ndir.c_str(), FILE_ATTRIBUTE_NORMAL);
    RemoveDirectoryW(ndir.c_str());
}

void SetFileAttributesWDirectorySystem(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring ndir = dir + L"\\SetFileAttributesWDirectorySystem";
    RemoveDirectoryW(ndir.c_str());
    CreateDirectoryW(ndir.c_str(), nullptr);

    SetFileAttributesW(ndir.c_str(), FILE_ATTRIBUTE_SYSTEM);

    DWORD attrs = GetFileAttributesW(ndir.c_str());
    if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_SYSTEM)) {
        LogSuccess(L"SetFileAttributesW", L"Successfully applied SYSTEM attribute to directory");
    } else {
        LogFailure(L"SetFileAttributesW", L"SYSTEM attribute not applied correctly");
    }

    SetFileAttributesW(ndir.c_str(), FILE_ATTRIBUTE_NORMAL);
    RemoveDirectoryW(ndir.c_str());
}

void SetFileAttributesWDirectoryNormalClearsOthers(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring ndir = dir + L"\\SetFileAttributesWDirectoryNormal";
    RemoveDirectoryW(ndir.c_str());
    CreateDirectoryW(ndir.c_str(), nullptr);

    // Apply multiple attributes
    SetFileAttributesW(ndir.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);

    // Clear with NORMAL
    SetFileAttributesW(ndir.c_str(), FILE_ATTRIBUTE_NORMAL);

    DWORD attrs = GetFileAttributesW(ndir.c_str());
    if (attrs == FILE_ATTRIBUTE_DIRECTORY || attrs == FILE_ATTRIBUTE_NORMAL) {
        LogSuccess(L"SetFileAttributesW", L"Successfully cleared attributes using FILE_ATTRIBUTE_NORMAL");
    } else {
        LogFailure(L"SetFileAttributesW", L"Unexpected attributes after clearing: " + std::to_wstring(attrs));
    }

    RemoveDirectoryW(ndir.c_str());
}

void SetFileAttributesWDirectoryInvalidPathFails(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring ndir = dir + L"\\SetFileAttributesWDirectoryInvalidPath";
    RemoveDirectoryW(ndir.c_str());  // Ensure it doesn't exist

    BOOL result = SetFileAttributesW(ndir.c_str(), FILE_ATTRIBUTE_HIDDEN);
    DWORD err = GetLastError();

    if (!result && err == ERROR_FILE_NOT_FOUND) {
        LogSuccess(L"SetFileAttributesW", L"Correctly failed to set attribute on nonexistent directory");
    } else {
        LogFailure(L"SetFileAttributesW", L"Unexpected result or error when setting attribute on missing directory");
    }
}

void SetFileInformationByHandleChangeTimes(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\SetFileInformationByHandleChangeTimes.txt";

    HANDLE h = CreateFileW(path.c_str(), FILE_WRITE_ATTRIBUTES | FILE_READ_ATTRIBUTES, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"SetFileInformationByHandle", L"Failed to create file for timestamp test");
        return;
    }

    // Step 1: Read original timestamps
    FILE_BASIC_INFO original = {};
    if (!GetFileInformationByHandleEx(h, FileBasicInfo, &original, sizeof(original))) {
        LogFailure(L"SetFileInformationByHandle", L"Failed to get original timestamps");
        CloseHandle(h);
        DeleteFileW(path.c_str());
        return;
    }

    // Step 2: Prepare a deliberately different time (1 hour later)
    ULONGLONG later = ((ULONGLONG)original.CreationTime.QuadPart) + (ULONGLONG)3600 * 10'000'000ULL; // 1 hour in 100ns units
    FILE_BASIC_INFO newTimes = original;
    newTimes.CreationTime.QuadPart = later;
    newTimes.LastWriteTime.QuadPart = later;

    // Step 3: Apply the change
    if (!SetFileInformationByHandle(h, FileBasicInfo, &newTimes, sizeof(newTimes))) {
        DWORD err = GetLastError();
        LogFailure(L"SetFileInformationByHandle", L"Failed to set new timestamps. Error: " + std::to_wstring(err));
        CloseHandle(h);
        DeleteFileW(path.c_str());
        return;
    }

    // Step 4: Read back and compare
    FILE_BASIC_INFO updated = {};
    if (!GetFileInformationByHandleEx(h, FileBasicInfo, &updated, sizeof(updated))) {
        LogFailure(L"SetFileInformationByHandle", L"Failed to read back updated timestamps");
        CloseHandle(h);
        DeleteFileW(path.c_str());
        return;
    }

    if (updated.CreationTime.QuadPart == later && updated.LastWriteTime.QuadPart == later) {
        LogSuccess(L"SetFileInformationByHandle", L"Successfully updated file times and confirmed change");
    } else {
        LogFailure(L"SetFileInformationByHandle", L"Timestamps did not update as expected");
    }

    CloseHandle(h);
    DeleteFileW(path.c_str());
}

void SetFileInformationByHandleRenameFile(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring original = dir + L"\\SetFileInformationByHandleRenameFile_original.txt";
    const wchar_t* renamed = L"SetFileInformationByHandleRenameFile_renamed.txt";

    // Setup
    DeleteFileW(L"SetFileInformationByHandleRenameFile_renamed.txt");
    HANDLE h = CreateFileW(original.c_str(), GENERIC_WRITE | DELETE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"SetFileInformationByHandle", L"Failed to create original file for rename");
        return;
    }

    // Setup rename info
    wchar_t buffer[sizeof(FILE_RENAME_INFO) + MAX_PATH * sizeof(wchar_t)] = {};
    auto* info = reinterpret_cast<FILE_RENAME_INFO*>(buffer);
    info->ReplaceIfExists = FALSE;
    info->RootDirectory = nullptr;
    info->FileNameLength = (DWORD)(wcslen(renamed) * sizeof(wchar_t));
    wcscpy_s(info->FileName, MAX_PATH, renamed);

    // Perform rename
    if (!SetFileInformationByHandle(h, FileRenameInfo, info, sizeof(FILE_RENAME_INFO) + info->FileNameLength)) {
        DWORD err = GetLastError();
        LogFailure(L"SetFileInformationByHandle", L"Rename failed. Error: " + std::to_wstring(err));
        CloseHandle(h);
        DeleteFileW(original.c_str());
        return;
    }

    CloseHandle(h);

    WIN32_FIND_DATAW data;
    HANDLE hFind = FindFirstFileW(renamed, &data);
    if (hFind != INVALID_HANDLE_VALUE) {
        FindClose(hFind);
        LogSuccess(L"SetFileInformationByHandle", L"Successfully renamed file using FILE_RENAME_INFO");
    } else {
        LogFailure(L"SetFileInformationByHandle", L"File rename succeeded but new file not found");
    }

    // Cleanup
    DeleteFileW(renamed);
}

void SetFileInformationByHandleRenameOverwrite(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring source = dir + L"\\SetFileInformationByHandleRenameOverwrite_src.txt";
    const wchar_t* dest = L"SetFileInformationByHandleRenameOverwrite_dst.txt";

    // Setup destination file to be overwritten
    HANDLE hDst = CreateFileW(dest, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hDst != INVALID_HANDLE_VALUE) {
        WriteFile(hDst, "existing", 8, nullptr, nullptr);
        CloseHandle(hDst);
    }

    // Create source file and open for rename
    HANDLE h = CreateFileW(source.c_str(), DELETE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"SetFileInformationByHandle", L"Failed to create source file for overwrite test");
        DeleteFileW(dest);
        return;
    }

    wchar_t buffer[sizeof(FILE_RENAME_INFO) + MAX_PATH * sizeof(wchar_t)] = {};
    auto* info = reinterpret_cast<FILE_RENAME_INFO*>(buffer);
    info->ReplaceIfExists = TRUE;
    info->RootDirectory = nullptr;
    info->FileNameLength = (DWORD)(wcslen(dest) * sizeof(wchar_t));
    wcscpy_s(info->FileName, MAX_PATH, dest);

    if (!SetFileInformationByHandle(h, FileRenameInfo, info, sizeof(FILE_RENAME_INFO) + info->FileNameLength)) {
        DWORD err = GetLastError();
        LogFailure(L"SetFileInformationByHandle", L"Rename with overwrite failed. Error: " + std::to_wstring(err));
        CloseHandle(h);
        DeleteFileW(source.c_str());
        DeleteFileW(dest);
        return;
    }

    CloseHandle(h);

    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(dest, &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        FindClose(hFind);
        LogSuccess(L"SetFileInformationByHandle", L"Successfully renamed file and replaced existing target");
    } else {
        LogFailure(L"SetFileInformationByHandle", L"Rename succeeded but target not found");
    }

    // Cleanup
    DeleteFileW(dest);
}

void SetFileInformationByHandleRenameWithRoot(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring baseDir = dir + L"\\SetFileInformationByHandleRenameWithRoot_dir";
    std::wstring fileName = dir + L"\\source.txt";
    const wchar_t* newName = L"renamed.txt";

    CreateDirectoryW(baseDir.c_str(), nullptr);

    // Open directory handle
    HANDLE hDir = CreateFileW(baseDir.c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                              FILE_FLAG_BACKUP_SEMANTICS, nullptr);
    if (hDir == INVALID_HANDLE_VALUE) {
        LogFailure(L"SetFileInformationByHandle", L"Failed to open root directory handle");
        return;
    }

    // Create file inside directory
    wchar_t fullPath[MAX_PATH];
    swprintf_s(fullPath, L"%s\\%s", baseDir.c_str(), fileName.c_str());
    HANDLE hFile = CreateFileW(fullPath, DELETE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        LogFailure(L"SetFileInformationByHandle", L"Failed to create file in root directory");
        CloseHandle(hDir);
        return;
    }

    wchar_t buffer[sizeof(FILE_RENAME_INFO) + MAX_PATH * sizeof(wchar_t)] = {};
    auto* info = reinterpret_cast<FILE_RENAME_INFO*>(buffer);
    info->ReplaceIfExists = FALSE;
    info->RootDirectory = hDir;
    info->FileNameLength = (DWORD)(wcslen(newName) * sizeof(wchar_t));
    wcscpy_s(info->FileName, MAX_PATH, newName);

    if (!SetFileInformationByHandle(hFile, FileRenameInfo, info, sizeof(FILE_RENAME_INFO) + info->FileNameLength)) {
        DWORD err = GetLastError();
        LogFailure(L"SetFileInformationByHandle", L"Rename using RootDirectory failed. Error: " + std::to_wstring(err));
        CloseHandle(hFile);
        CloseHandle(hDir);
        DeleteFileW(fullPath);
        return;
    }

    CloseHandle(hFile);
    CloseHandle(hDir);

    wchar_t renamedPath[MAX_PATH];
    swprintf_s(renamedPath, L"%s\\%ls", baseDir.c_str(), newName);
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(renamedPath, &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        FindClose(hFind);
        LogSuccess(L"SetFileInformationByHandle", L"Successfully renamed file using RootDirectory");
    } else {
        LogFailure(L"SetFileInformationByHandle", L"Rename succeeded but renamed file not found");
    }

    // Cleanup
    DeleteFileW(renamedPath);
    RemoveDirectoryW(baseDir.c_str());
}

void SetFileInformationByHandleDeleteFile(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\SetFileInformationByHandleDeleteFile.txt";

    // Setup
    HANDLE h = CreateFileW(path.c_str(), DELETE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"SetFileInformationByHandle", L"Failed to create file for disposition test");
        return;
    }

    FILE_DISPOSITION_INFO info = {};
    info.DeleteFile = TRUE;

    if (!SetFileInformationByHandle(h, FileDispositionInfo, &info, sizeof(info))) {
        DWORD err = GetLastError();
        LogFailure(L"SetFileInformationByHandle", L"Set disposition failed. Error: " + std::to_wstring(err));
        CloseHandle(h);
        DeleteFileW(path.c_str());
        return;
    }

    // Close handle — this is when deletion is committed
    CloseHandle(h);

    HANDLE hFind = FindFirstFileW(path.c_str(), nullptr);
    if (hFind == INVALID_HANDLE_VALUE) {
        LogSuccess(L"SetFileInformationByHandle", L"File successfully deleted using FILE_DISPOSITION_INFO");
    } else {
        FindClose(hFind);
        LogFailure(L"SetFileInformationByHandle", L"Disposition applied but file still exists");
        DeleteFileW(path.c_str());
    }
}

void SetFileInformationByHandleTruncateFile(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\SetFileInformationByHandleTruncateFile.txt";

    // Setup: create file and write content
    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE | GENERIC_READ, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"SetFileInformationByHandle", L"Failed to create file for truncate test");
        return;
    }

    DWORD written;
    WriteFile(h, "1234567890", 10, &written, nullptr); // 10 bytes

    FILE_END_OF_FILE_INFO info = {};
    info.EndOfFile.QuadPart = 0;

    if (!SetFileInformationByHandle(h, FileEndOfFileInfo, &info, sizeof(info))) {
        DWORD err = GetLastError();
        LogFailure(L"SetFileInformationByHandle", L"Failed to truncate. Error: " + std::to_wstring(err));
        CloseHandle(h);
        DeleteFileW(path.c_str());
        return;
    }

    DWORD size = GetFileSize(h, nullptr);
    if (size == 0) {
        LogSuccess(L"SetFileInformationByHandle", L"File successfully truncated to 0 bytes using FILE_END_OF_FILE_INFO");
    } else {
        LogFailure(L"SetFileInformationByHandle", L"Expected 0-byte file, got size " + std::to_wstring(size));
    }

    CloseHandle(h);
    DeleteFileW(path.c_str());
}

void SetFileInformationByHandleDeleteReadOnlyFile(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\SetFileInformationByHandleDeleteReadOnlyFile.txt";

    // Create file with READONLY attribute
    HANDLE h = CreateFileW(path.c_str(), DELETE | FILE_WRITE_ATTRIBUTES, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_READONLY, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"SetFileInformationByHandle", L"Failed to create read-only file");
        return;
    }

    // Clear READONLY attribute first
    if (!SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_NORMAL)) {
        LogFailure(L"SetFileInformationByHandle", L"Failed to clear READONLY attribute");
        CloseHandle(h);
        DeleteFileW(path.c_str());
        return;
    }

    FILE_DISPOSITION_INFO info = {};
    info.DeleteFile = TRUE;

    if (!SetFileInformationByHandle(h, FileDispositionInfo, &info, sizeof(info))) {
        DWORD err = GetLastError();
        LogFailure(L"SetFileInformationByHandle", L"Disposition on read-only file failed. Error: " + std::to_wstring(err));
        CloseHandle(h);
        DeleteFileW(path.c_str());
        return;
    }

    CloseHandle(h);

    HANDLE hFind = FindFirstFileW(path.c_str(), nullptr);
    if (hFind == INVALID_HANDLE_VALUE) {
        LogSuccess(L"SetFileInformationByHandle", L"Successfully deleted previously read-only file using FILE_DISPOSITION_INFO");
    } else {
        FindClose(hFind);
        LogFailure(L"SetFileInformationByHandle", L"File still exists after deletion attempt");
        DeleteFileW(path.c_str());
    }
}

void WriteFileBasic(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\WriteFileBasic.txt";
    DeleteFileW(path.c_str());

    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE | GENERIC_READ, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"WriteFile", L"Failed to create file");
        return;
    }

    const char* text = "hello world";
    DWORD written = 0;
    if (!WriteFile(h, text, (DWORD)strlen(text), &written, nullptr)) {
        LogFailure(L"WriteFile", L"WriteFile failed");
        CloseHandle(h);
        DeleteFileW(path.c_str());
        return;
    }

    // Read and verify content
    char buffer[32] = {};
    SetFilePointer(h, 0, nullptr, FILE_BEGIN);
    DWORD read = 0;
    ReadFile(h, buffer, written, &read, nullptr);
    CloseHandle(h);

    if (strncmp(buffer, text, written) == 0) {
        LogSuccess(L"WriteFile", L"Successfully wrote and verified file content");
    } else {
        LogFailure(L"WriteFile", L"Content mismatch in written file");
    }

    DeleteFileW(path.c_str());
}

void WriteFileAppendMode(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\WriteFileAppendMode.txt";
    DeleteFileW(path.c_str());

    // Create and write initial data
    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h != INVALID_HANDLE_VALUE) {
        WriteFile(h, "123", 3, nullptr, nullptr);
        CloseHandle(h);
    }

    // Open with FILE_APPEND_DATA
    h = CreateFileW(path.c_str(), FILE_APPEND_DATA | GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"WriteFile", L"Failed to open file for append");
        return;
    }

    WriteFile(h, "456", 3, nullptr, nullptr);
    CloseHandle(h);

    // Verify content
    char buffer[16] = {};
    HANDLE hVerify = CreateFileW(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    DWORD read = 0;
    ReadFile(hVerify, buffer, sizeof(buffer), &read, nullptr);
    CloseHandle(hVerify);

    if (strncmp(buffer, "123456", 6) == 0) {
        LogSuccess(L"WriteFile", L"Append operation succeeded and verified");
    } else {
        LogFailure(L"WriteFile", L"Append failed or content mismatch");
    }

    DeleteFileW(path.c_str());
}

void WriteFileWithOffset(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\WriteFileWithOffset.txt";
    DeleteFileW(path.c_str());

    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE | GENERIC_READ, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"WriteFile", L"Failed to create file");
        return;
    }

    WriteFile(h, "abcdef", 6, nullptr, nullptr);
    SetFilePointer(h, 3, nullptr, FILE_BEGIN); // Overwrite from position 3
    WriteFile(h, "XYZ", 3, nullptr, nullptr);

    // Read result
    char buffer[16] = {};
    SetFilePointer(h, 0, nullptr, FILE_BEGIN);
    ReadFile(h, buffer, 6, nullptr, nullptr);
    CloseHandle(h);

    if (strncmp(buffer, "abcXYZ", 6) == 0) {
        LogSuccess(L"WriteFile", L"Successfully overwrote part of file using offset");
    } else {
        LogFailure(L"WriteFile", L"Offset write failed or incorrect content");
    }

    DeleteFileW(path.c_str());
}

void WriteFileToReadOnlyFile(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\WriteFileToReadOnlyFile.txt";
    DeleteFileW(path.c_str());

    // Create file
    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_READONLY, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"WriteFile", L"Failed to create read-only file");
        return;
    }

    DWORD written;
    BOOL success = WriteFile(h, "fail", 4, &written, nullptr);
    DWORD err = GetLastError();
    CloseHandle(h);
    SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_NORMAL);
    DeleteFileW(path.c_str());

    if (!success && err == ERROR_ACCESS_DENIED) {
        LogSuccess(L"WriteFile", L"Correctly failed to write to read-only file");
    } else {
        LogFailure(L"WriteFile", L"Unexpectedly succeeded writing to read-only file");
    }
}

void WriteFileInvalidHandle(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    HANDLE h = (HANDLE)-1; // Invalid handle
    DWORD written;
    BOOL success = WriteFile(h, "fail", 4, &written, nullptr);
    DWORD err = GetLastError();

    if (!success && err == ERROR_INVALID_HANDLE) {
        LogSuccess(L"WriteFile", L"Correctly failed with invalid handle");
    } else {
        LogFailure(L"WriteFile", L"Unexpected result with invalid handle");
    }
}

void WriteFileBeyondEOF(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\SetFilePointerBeyondEOFThenWrite.txt";
    DeleteFileW(path.c_str());

    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE | GENERIC_READ, 0, nullptr,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"SetFilePointer", L"Failed to create file");
        return;
    }

    SetFilePointer(h, 1024, nullptr, FILE_BEGIN);  // Seek to offset 1024
    WriteFile(h, "X", 1, nullptr, nullptr);        // Write 1 byte

    DWORD size = GetFileSize(h, nullptr);
    if (size == 1025) {
        LogSuccess(L"SetFilePointer", L"Successfully extended file by seeking past EOF and writing");
    } else {
        LogFailure(L"SetFilePointer", L"Expected file size 1025, got " + std::to_wstring(size));
    }

    CloseHandle(h);
    DeleteFileW(path.c_str());
}

void LockFileBasicExclusive(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\LockFileBasicExclusive.txt";
    DeleteFileW(path.c_str());

    HANDLE h1 = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h1 == INVALID_HANDLE_VALUE) {
        LogFailure(L"LockFile", L"Failed to create file");
        return;
    }

    WriteFile(h1, "abcdefghij", 10, nullptr, nullptr);  // 10 bytes
    SetFilePointer(h1, 0, nullptr, FILE_BEGIN);

    // Lock first 5 bytes
    if (!LockFile(h1, 0, 0, 5, 0)) {
        LogFailure(L"LockFile", L"Failed to lock file region");
        CloseHandle(h1);
        DeleteFileW(path.c_str());
        return;
    }

    // Open second handle and attempt to write to the locked region
    HANDLE h2 = CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h2 == INVALID_HANDLE_VALUE) {
        LogFailure(L"LockFile", L"Failed to open second handle");
        UnlockFile(h1, 0, 0, 5, 0);
        CloseHandle(h1);
        DeleteFileW(path.c_str());
        return;
    }

    SetFilePointer(h2, 0, nullptr, FILE_BEGIN);
    DWORD written = 0;
    BOOL result = WriteFile(h2, "XXXXX", 5, &written, nullptr);
    DWORD err = GetLastError();

    if (!result && err == ERROR_LOCK_VIOLATION) {
        LogSuccess(L"LockFile", L"Second write to locked region correctly failed with ERROR_LOCK_VIOLATION");
    } else {
        LogFailure(L"LockFile", L"Unexpected result writing to locked region");
    }

    // Cleanup
    UnlockFile(h1, 0, 0, 5, 0);
    CloseHandle(h1);
    CloseHandle(h2);
    DeleteFileW(path.c_str());
}

void LockFileNonOverlappingSuccess(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\LockFileNonOverlappingSuccess.txt";
    DeleteFileW(path.c_str());

    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"LockFile", L"Failed to create file");
        return;
    }

    WriteFile(h, "abcdefghij", 10, nullptr, nullptr);  // 10 bytes

    // Lock bytes 0–4
    if (!LockFile(h, 0, 0, 5, 0)) {
        LogFailure(L"LockFile", L"Failed to lock region");
        CloseHandle(h);
        DeleteFileW(path.c_str());
        return;
    }

    // Try writing to byte 5–9
    SetFilePointer(h, 5, nullptr, FILE_BEGIN);
    DWORD written;
    BOOL result = WriteFile(h, "ZZZZZ", 5, &written, nullptr);

    if (result) {
        LogSuccess(L"LockFile", L"Successfully wrote to non-overlapping region while lock was held");
    } else {
        LogFailure(L"LockFile", L"Write to non-overlapping region failed");
    }

    UnlockFile(h, 0, 0, 5, 0);
    CloseHandle(h);
    DeleteFileW(path.c_str());
}

void LockFileUnlockRegion(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\LockFileUnlockRegion.txt";
    DeleteFileW(path.c_str());

    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"LockFile", L"Failed to create file");
        return;
    }

    WriteFile(h, "1234567890", 10, nullptr, nullptr);
    LockFile(h, 0, 0, 10, 0);
    UnlockFile(h, 0, 0, 10, 0);

    SetFilePointer(h, 0, nullptr, FILE_BEGIN);
    DWORD written;
    BOOL result = WriteFile(h, "XXXXXXXXXX", 10, &written, nullptr);

    if (result) {
        LogSuccess(L"LockFile", L"Successfully wrote to file after unlocking");
    } else {
        LogFailure(L"LockFile", L"Write failed after unlock");
    }

    CloseHandle(h);
    DeleteFileW(path.c_str());
}

void LockFileAlreadyLockedFails(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\LockFileAlreadyLockedFails.txt";
    DeleteFileW(path.c_str());

    HANDLE h1 = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h1 == INVALID_HANDLE_VALUE) {
        LogFailure(L"LockFile", L"Failed to create file");
        return;
    }

    HANDLE h2 = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    LockFile(h1, 0, 0, 5, 0);
    BOOL result = LockFile(h2, 0, 0, 5, 0);
    DWORD err = GetLastError();

    if (!result && err == ERROR_LOCK_VIOLATION) {
        LogSuccess(L"LockFile", L"Correctly failed to lock an already-locked region");
    } else {
        LogFailure(L"LockFile", L"Unexpected result when locking an already-locked region");
    }

    UnlockFile(h1, 0, 0, 5, 0);
    CloseHandle(h1);
    CloseHandle(h2);
    DeleteFileW(path.c_str());
}

void LockFileInvalidHandle(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    HANDLE h = (HANDLE)-1;
    BOOL result = LockFile(h, 0, 0, 5, 0);
    DWORD err = GetLastError();

    if (!result && err == ERROR_INVALID_HANDLE) {
        LogSuccess(L"LockFile", L"Correctly failed with ERROR_INVALID_HANDLE");
    } else {
        LogFailure(L"LockFile", L"Unexpected result using invalid handle");
    }
}

void UnlockFileBasic(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\UnlockFileBasic.txt";
    DeleteFileW(path.c_str());

    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                           nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"UnlockFile", L"Failed to create test file");
        return;
    }

    WriteFile(h, "abcdefghij", 10, nullptr, nullptr);
    LockFile(h, 0, 0, 10, 0);
    UnlockFile(h, 0, 0, 10, 0);

    DWORD written = 0;
    SetFilePointer(h, 0, nullptr, FILE_BEGIN);
    BOOL result = WriteFile(h, "XXXXXXXXXX", 10, &written, nullptr);

    if (result) {
        LogSuccess(L"UnlockFile", L"Successfully unlocked file and restored write access");
    } else {
        LogFailure(L"UnlockFile", L"Write failed after unlocking file");
    }

    CloseHandle(h);
    DeleteFileW(path.c_str());
}

void UnlockFileWithoutLockFails(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\UnlockFileWithoutLockFails.txt";
    DeleteFileW(path.c_str());

    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                           nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"UnlockFile", L"Failed to create test file");
        return;
    }

    BOOL result = UnlockFile(h, 0, 0, 5, 0);
    DWORD err = GetLastError();

    if (!result && err == ERROR_NOT_LOCKED) {
        LogSuccess(L"UnlockFile", L"Correctly failed to unlock an unlocked region");
    } else {
        LogFailure(L"UnlockFile", L"Unexpected result unlocking non-locked region");
    }

    CloseHandle(h);
    DeleteFileW(path.c_str());
}

void UnlockFileWrongRegionFails(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\UnlockFileWrongRegionFails.txt";
    DeleteFileW(path.c_str());

    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                           nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"UnlockFile", L"Failed to create test file");
        return;
    }

    LockFile(h, 0, 0, 10, 0); // Lock bytes 0–9

    // Try to unlock bytes 5–14 (mismatch)
    BOOL result = UnlockFile(h, 5, 0, 10, 0);
    DWORD err = GetLastError();

    if (!result && err == ERROR_NOT_LOCKED) {
        LogSuccess(L"UnlockFile", L"Correctly failed to unlock mismatched region");
    } else {
        LogFailure(L"UnlockFile", L"Unexpected result unlocking wrong region");
    }

    UnlockFile(h, 0, 0, 10, 0);
    CloseHandle(h);
    DeleteFileW(path.c_str());
}

void UnlockFileWithInvalidHandle(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    HANDLE h = (HANDLE)-1;
    BOOL result = UnlockFile(h, 0, 0, 5, 0);
    DWORD err = GetLastError();

    if (!result && err == ERROR_INVALID_HANDLE) {
        LogSuccess(L"UnlockFile", L"Correctly failed to unlock using invalid handle");
    } else {
        LogFailure(L"UnlockFile", L"Unexpected result with invalid handle");
    }
}

void UnlockFilePartialUnlockThenAccess(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\UnlockFilePartialUnlockThenAccess.txt";
    DeleteFileW(path.c_str());

    // Lock full 10 bytes
    HANDLE h1 = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                            nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    HANDLE h2 = CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                            nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (h1 == INVALID_HANDLE_VALUE || h2 == INVALID_HANDLE_VALUE) {
        LogFailure(L"UnlockFile", L"Failed to open file handles");
        if (h1 != INVALID_HANDLE_VALUE) CloseHandle(h1);
        if (h2 != INVALID_HANDLE_VALUE) CloseHandle(h2);
        DeleteFileW(path.c_str());
        return;
    }

    WriteFile(h1, "abcdefghij", 10, nullptr, nullptr);
    LockFile(h1, 0, 0, 10, 0); // lock 0–9
    UnlockFile(h1, 5, 0, 5, 0); // unlock 5–9

    // Try writing to unlocked portion (should succeed)
    SetFilePointer(h2, 5, nullptr, FILE_BEGIN);
    DWORD written = 0;
    BOOL writeResult = WriteFile(h2, "ZZZZZ", 5, &written, nullptr);

    if (writeResult) {
        LogSuccess(L"UnlockFile", L"Write to partially unlocked region succeeded");
    } else {
        LogFailure(L"UnlockFile", L"Write to unlocked region failed");
    }

    UnlockFile(h1, 0, 0, 5, 0);
    CloseHandle(h1);
    CloseHandle(h2);
    DeleteFileW(path.c_str());
}

void ReadFileBasic(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\ReadFileBasic.txt";
    DeleteFileW(path.c_str());

    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE | GENERIC_READ, 0, nullptr,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"ReadFile", L"Failed to create file");
        return;
    }

    WriteFile(h, "hello", 5, nullptr, nullptr);
    SetFilePointer(h, 0, nullptr, FILE_BEGIN);

    char buffer[6] = {};
    DWORD read = 0;
    if (ReadFile(h, buffer, 5, &read, nullptr) && read == 5 && strncmp(buffer, "hello", 5) == 0) {
        LogSuccess(L"ReadFile", L"Successfully read expected contents");
    } else {
        LogFailure(L"ReadFile", L"Unexpected contents or length from read");
    }

    CloseHandle(h);
    DeleteFileW(path.c_str());
}

void ReadFilePartial(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\ReadFilePartial.txt";
    DeleteFileW(path.c_str());

    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE | GENERIC_READ, 0, nullptr,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    WriteFile(h, "abcdef", 6, nullptr, nullptr);
    SetFilePointer(h, 0, nullptr, FILE_BEGIN);

    char buffer[4] = {};
    DWORD read = 0;
    if (ReadFile(h, buffer, 3, &read, nullptr) && read == 3 && strncmp(buffer, "abc", 3) == 0) {
        LogSuccess(L"ReadFile", L"Successfully read 3 of 6 bytes as expected");
    } else {
        LogFailure(L"ReadFile", L"Unexpected result reading part of file");
    }

    CloseHandle(h);
    DeleteFileW(path.c_str());
}

void ReadFileAtEOFReturnsZero(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\ReadFileAtEOFReturnsZero.txt";
    DeleteFileW(path.c_str());

    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE | GENERIC_READ, 0, nullptr,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    WriteFile(h, "x", 1, nullptr, nullptr);
    SetFilePointer(h, 1, nullptr, FILE_BEGIN); // move to EOF

    char ch = 0;
    DWORD read = 0;
    if (ReadFile(h, &ch, 1, &read, nullptr) && read == 0) {
        LogSuccess(L"ReadFile", L"Correctly returned 0 bytes at EOF");
    } else {
        LogFailure(L"ReadFile", L"Unexpected result at EOF: bytes read = " + std::to_wstring(read));
    }

    CloseHandle(h);
    DeleteFileW(path.c_str());
}

void ReadFileClosedHandleFails(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\ReadFileClosedHandleFails.txt";
    DeleteFileW(path.c_str());

    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE | GENERIC_READ, 0, nullptr,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    WriteFile(h, "fail", 4, nullptr, nullptr);
    CloseHandle(h);

    char buffer[4];
    DWORD read = 0;
    BOOL result = ReadFile(h, buffer, 4, &read, nullptr);
    DWORD err = GetLastError();

    if (!result && err == ERROR_INVALID_HANDLE) {
        LogSuccess(L"ReadFile", L"Correctly failed to read from closed handle");
    } else {
        LogFailure(L"ReadFile", L"Unexpectedly succeeded reading from closed handle");
    }

    DeleteFileW(path.c_str());
}

void ReadFileInvalidHandleFails(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    HANDLE h = (HANDLE)-1;
    char buffer[4];
    DWORD read = 0;

    BOOL result = ReadFile(h, buffer, 4, &read, nullptr);
    DWORD err = GetLastError();

    if (!result && err == ERROR_INVALID_HANDLE) {
        LogSuccess(L"ReadFile", L"Correctly failed with ERROR_INVALID_HANDLE");
    } else {
        LogFailure(L"ReadFile", L"Unexpected result using invalid handle");
    }
}

void ReadFileStart(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\SetFilePointerToStart.txt";
    DeleteFileW(path.c_str());

    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"SetFilePointer", L"Failed to create file");
        return;
    }

    WriteFile(h, "abc", 3, nullptr, nullptr);
    SetFilePointer(h, 0, nullptr, FILE_BEGIN);  // Move to start

    char ch = 0;
    DWORD read = 0;
    ReadFile(h, &ch, 1, &read, nullptr);

    if (read == 1 && ch == 'a') {
        LogSuccess(L"SetFilePointer", L"Successfully read first byte after moving to FILE_BEGIN");
    } else {
        LogFailure(L"SetFilePointer", L"Unexpected byte or failed read at FILE_BEGIN");
    }

    CloseHandle(h);
    DeleteFileW(path.c_str());
}

void ReadFileMiddle(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\SetFilePointerToMiddle.txt";
    DeleteFileW(path.c_str());

    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    WriteFile(h, "0123456789", 10, nullptr, nullptr);
    SetFilePointer(h, 5, nullptr, FILE_BEGIN);  // Move to byte index 5

    char ch = 0;
    DWORD read;
    ReadFile(h, &ch, 1, &read, nullptr);

    if (read == 1 && ch == '5') {
        LogSuccess(L"SetFilePointer", L"Successfully read middle byte after pointer moved");
    } else {
        LogFailure(L"SetFilePointer", L"Failed to read correct byte at middle");
    }

    CloseHandle(h);
    DeleteFileW(path.c_str());
}

void ReadFileEnd(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\SetFilePointerToEnd.txt";
    DeleteFileW(path.c_str());

    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    WriteFile(h, "abc", 3, nullptr, nullptr);
    SetFilePointer(h, 0, nullptr, FILE_END);  // Move to EOF

    char ch = 0;
    DWORD read;
    BOOL success = ReadFile(h, &ch, 1, &read, nullptr);

    if (success && read == 0) {
        LogSuccess(L"SetFilePointer", L"Correctly reached EOF and read 0 bytes");
    } else {
        LogFailure(L"SetFilePointer", L"Expected EOF, but read succeeded with data");
    }

    CloseHandle(h);
    DeleteFileW(path.c_str());
}

void FindFirstFileWExactMatch(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    const wchar_t* filename = L"FindFirstFileWExactMatch.txt";
    DeleteFileW(filename);

    HANDLE h = CreateFileW(filename, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h != INVALID_HANDLE_VALUE) CloseHandle(h);

    WIN32_FIND_DATAW data;
    HANDLE find = FindFirstFileW(filename, &data);

    if (find != INVALID_HANDLE_VALUE && wcscmp(data.cFileName, filename) == 0) {
        LogSuccess(L"FindFirstFileW", L"Successfully found file by exact name");
        FindClose(find);
    } else {
        LogFailure(L"FindFirstFileW", L"Failed to find file by exact name");
    }

    DeleteFileW(filename);
}

void FindFirstFileWWildcardMatch(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    const wchar_t* filename = L"FindFirstFileWWildcardMatch.log";
    DeleteFileW(filename);

    HANDLE h = CreateFileW(filename, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h != INVALID_HANDLE_VALUE) CloseHandle(h);

    WIN32_FIND_DATAW data;
    HANDLE find = FindFirstFileW(L"FindFirstFileWWildcardMatch.*", &data);
    bool found = false;

    while (find != INVALID_HANDLE_VALUE) {
        if (wcscmp(data.cFileName, filename) == 0) {
            found = true;
            break;
        }
        if (!FindNextFileW(find, &data)) break;
    }

    if (find != INVALID_HANDLE_VALUE) FindClose(find);

    if (found) {
        LogSuccess(L"FindFirstFileW", L"Wildcard pattern successfully matched file");
    } else {
        LogFailure(L"FindFirstFileW", L"Wildcard search did not find expected file");
    }

    DeleteFileW(filename);
}

void FindFirstFileWExtensionMatch(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    const wchar_t* filename = L"FindFirstFileWExtensionMatch.txt";
    DeleteFileW(filename);

    HANDLE h = CreateFileW(filename, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h != INVALID_HANDLE_VALUE) CloseHandle(h);

    WIN32_FIND_DATAW data;
    HANDLE find = FindFirstFileW(L"*.txt", &data);
    bool found = false;

    while (find != INVALID_HANDLE_VALUE) {
        if (wcscmp(data.cFileName, filename) == 0) {
            found = true;
            break;
        }
        if (!FindNextFileW(find, &data)) break;
    }

    if (find != INVALID_HANDLE_VALUE) FindClose(find);

    if (found) {
        LogSuccess(L"FindFirstFileW", L"Successfully matched file using extension pattern *.txt");
    } else {
        LogFailure(L"FindFirstFileW", L"Failed to find file using *.txt pattern");
    }

    DeleteFileW(filename);
}

void FindFirstFileWNonexistentFails(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    const wchar_t* pattern = L"FindFirstFileWDoesNotExist.*";

    WIN32_FIND_DATAW data;
    HANDLE find = FindFirstFileW(pattern, &data);
    DWORD err = GetLastError();

    if (find == INVALID_HANDLE_VALUE && err == ERROR_FILE_NOT_FOUND) {
        LogSuccess(L"FindFirstFileW", L"Correctly failed to find nonexistent file with ERROR_FILE_NOT_FOUND");
    } else {
        LogFailure(L"FindFirstFileW", L"Unexpected success or error when searching nonexistent file");
        if (find != INVALID_HANDLE_VALUE) FindClose(find);
    }
}

void FindFirstFileWHiddenFileVisibleByName(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    const wchar_t* filename = L"FindFirstFileWHiddenFile.txt";
    DeleteFileW(filename);

    HANDLE h = CreateFileW(filename, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, nullptr);
    if (h != INVALID_HANDLE_VALUE) CloseHandle(h);

    WIN32_FIND_DATAW data;
    HANDLE find = FindFirstFileW(filename, &data);

    if (find != INVALID_HANDLE_VALUE && (data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) {
        LogSuccess(L"FindFirstFileW", L"Successfully found hidden file by name");
        FindClose(find);
    } else {
        LogFailure(L"FindFirstFileW", L"Failed to find hidden file by name");
    }

    SetFileAttributesW(filename, FILE_ATTRIBUTE_NORMAL);
    DeleteFileW(filename);
}

void FindFirstFileWDirectoryExactMatch(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    const wchar_t* ndir = L"FindFirstFileWDirectoryExactMatch";
    RemoveDirectoryW(ndir);
    CreateDirectoryW(ndir, nullptr);

    WIN32_FIND_DATAW data;
    HANDLE find = FindFirstFileW(ndir, &data);

    if (find != INVALID_HANDLE_VALUE && (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        LogSuccess(L"FindFirstFileW", L"Successfully matched directory by exact name");
        FindClose(find);
    } else {
        LogFailure(L"FindFirstFileW", L"Failed to match directory by exact name");
    }

    RemoveDirectoryW(ndir);
}

void FindFirstFileWDirectoryWildcard(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    const wchar_t* ndir = L"FindFirstFileWDirectoryWildcard";
    RemoveDirectoryW(ndir);
    CreateDirectoryW(ndir, nullptr);

    WIN32_FIND_DATAW data;
    HANDLE find = FindFirstFileW(L"FindFirstFileWDirectory*", &data);
    bool found = false;

    while (find != INVALID_HANDLE_VALUE) {
        if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            wcscmp(data.cFileName, ndir) == 0) {
            found = true;
            break;
        }
        if (!FindNextFileW(find, &data)) break;
    }

    if (find != INVALID_HANDLE_VALUE) FindClose(find);

    if (found) {
        LogSuccess(L"FindFirstFileW", L"Wildcard search successfully matched directory");
    } else {
        LogFailure(L"FindFirstFileW", L"Wildcard search failed to find expected directory");
    }

    RemoveDirectoryW(ndir);
}

void FindFirstFileWDirectoryHiddenAttribute(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    const wchar_t* ndir = L"FindFirstFileWDirectoryHidden";
    RemoveDirectoryW(ndir);
    CreateDirectoryW(ndir, nullptr);
    SetFileAttributesW(ndir, FILE_ATTRIBUTE_HIDDEN);

    WIN32_FIND_DATAW data;
    HANDLE find = FindFirstFileW(ndir, &data);

    if (find != INVALID_HANDLE_VALUE && (data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) &&
        (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        LogSuccess(L"FindFirstFileW", L"Successfully found hidden directory and attribute is correct");
    } else {
        LogFailure(L"FindFirstFileW", L"Failed to find or detect hidden attribute on directory");
    }

    if (find != INVALID_HANDLE_VALUE) FindClose(find);

    SetFileAttributesW(ndir, FILE_ATTRIBUTE_NORMAL);
    RemoveDirectoryW(ndir);
}

void FindFirstFileWDirectoryEnumerateSubdirs(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    const wchar_t* parent = L"FindFirstFileWDirectoryEnumerateSubdirs";
    const wchar_t* subdir = L"FindFirstFileWDirectoryEnumerateSubdirs\\sub";

    RemoveDirectoryW(subdir);
    RemoveDirectoryW(parent);
    CreateDirectoryW(parent, nullptr);
    CreateDirectoryW(subdir, nullptr);

    wchar_t searchPattern[MAX_PATH];
    swprintf_s(searchPattern, L"%s\\*", parent);

    WIN32_FIND_DATAW data;
    HANDLE find = FindFirstFileW(searchPattern, &data);
    bool foundSubdir = false;

    while (find != INVALID_HANDLE_VALUE) {
        if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            wcscmp(data.cFileName, L"sub") == 0) {
            foundSubdir = true;
            break;
        }
        if (!FindNextFileW(find, &data)) break;
    }

    if (find != INVALID_HANDLE_VALUE) FindClose(find);

    if (foundSubdir) {
        LogSuccess(L"FindFirstFileW", L"Successfully enumerated subdirectory using wildcard pattern");
    } else {
        LogFailure(L"FindFirstFileW", L"Failed to enumerate expected subdirectory");
    }

    RemoveDirectoryW(subdir);
    RemoveDirectoryW(parent);
}

void FindFirstFileWDirectoryDotDot(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    const wchar_t* ndir = L"FindFirstFileWDirectoryDotDot";
    RemoveDirectoryW(ndir);
    CreateDirectoryW(ndir, nullptr);

    wchar_t pattern[MAX_PATH];
    swprintf_s(pattern, L"%s\\*", ndir);

    WIN32_FIND_DATAW data;
    HANDLE find = FindFirstFileW(pattern, &data);
    bool foundDot = false, foundDotDot = false;

    while (find != INVALID_HANDLE_VALUE) {
        if (wcscmp(data.cFileName, L".") == 0) foundDot = true;
        if (wcscmp(data.cFileName, L"..") == 0) foundDotDot = true;
        if (!FindNextFileW(find, &data)) break;
    }

    if (find != INVALID_HANDLE_VALUE) FindClose(find);

    if (foundDot && foundDotDot) {
        LogSuccess(L"FindFirstFileW", L"Successfully enumerated . and .. directory entries");
    } else {
        LogFailure(L"FindFirstFileW", L"Failed to find . and .. entries in directory");
    }

    RemoveDirectoryW(ndir);
}

void FindFirstFileWRecursiveFiles(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    const wchar_t* baseDir = L"FindFirstFileWRecursiveFiles";
    const wchar_t* subDir = L"FindFirstFileWRecursiveFiles\\nested";
    const wchar_t* files[] = {
        L"FindFirstFileWRecursiveFiles\\file1.txt",
        L"FindFirstFileWRecursiveFiles\\nested\\file2.txt"
    };

    // Setup: create directory and files
    RemoveDirectoryW(subDir);
    RemoveDirectoryW(baseDir);
    CreateDirectoryW(baseDir, nullptr);
    CreateDirectoryW(subDir, nullptr);

    for (int i = 0; i < 2; ++i) {
        HANDLE h = CreateFileW(files[i], GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h != INVALID_HANDLE_VALUE) CloseHandle(h);
    }

    // Recursive traversal to find all .txt files
    std::vector<std::wstring> found;
    std::function<void(const std::wstring&)> recurse = [&](const std::wstring& dir) {
        std::wstring search = dir + L"\\*";
        WIN32_FIND_DATAW data;
        HANDLE hFind = FindFirstFileW(search.c_str(), &data);

        if (hFind == INVALID_HANDLE_VALUE) return;

        do {
            if (wcscmp(data.cFileName, L".") == 0 || wcscmp(data.cFileName, L"..") == 0) continue;

            std::wstring fullPath = dir + L"\\" + data.cFileName;
            if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                recurse(fullPath); // Recurse into subdirectory
            } else if (wcsstr(data.cFileName, L".txt")) {
                found.push_back(fullPath);
            }
        } while (FindNextFileW(hFind, &data));
        FindClose(hFind);
    };

    recurse(baseDir);

    if (found.size() == 2) {
        LogSuccess(L"FindFirstFileW", L"Successfully found all expected files recursively");
    } else {
        LogFailure(L"FindFirstFileW", L"Expected 2 files, found " + std::to_wstring(found.size()));
    }

    // Cleanup
    for (const auto& f : files) DeleteFileW(f);
    RemoveDirectoryW(subDir);
    RemoveDirectoryW(baseDir);
}

void FindFirstFileWRecursiveDirectories(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    const wchar_t* baseDir = L"FindFirstFileWRecursiveDirectories";
    const wchar_t* subDirs[] = {
        L"FindFirstFileWRecursiveDirectories\\a",
        L"FindFirstFileWRecursiveDirectories\\a\\b",
        L"FindFirstFileWRecursiveDirectories\\a\\b\\c"
    };

    // Setup
    for (int i = 2; i >= 0; --i) {
        RemoveDirectoryW(subDirs[i]);
    }
    RemoveDirectoryW(baseDir);
    CreateDirectoryW(baseDir, nullptr);
    for (int i = 0; i < 3; ++i) {
        CreateDirectoryW(subDirs[i], nullptr);
    }

    // Recursive directory discovery
    std::vector<std::wstring> found;
    std::function<void(const std::wstring&)> recurse = [&](const std::wstring& dir) {
        std::wstring search = dir + L"\\*";
        WIN32_FIND_DATAW data;
        HANDLE hFind = FindFirstFileW(search.c_str(), &data);

        if (hFind == INVALID_HANDLE_VALUE) return;

        do {
            if (wcscmp(data.cFileName, L".") == 0 || wcscmp(data.cFileName, L"..") == 0) continue;

            if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                std::wstring sub = dir + L"\\" + data.cFileName;
                found.push_back(sub);
                recurse(sub);
            }
        } while (FindNextFileW(hFind, &data));
        FindClose(hFind);
    };

    recurse(baseDir);

    if (found.size() == 3) {
        LogSuccess(L"FindFirstFileW", L"Successfully found all nested directories recursively");
    } else {
        LogFailure(L"FindFirstFileW", L"Expected 3 directories, found " + std::to_wstring(found.size()));
    }

    // Cleanup (deepest to shallowest)
    for (int i = 2; i >= 0; --i) RemoveDirectoryW(subDirs[i]);
    RemoveDirectoryW(baseDir);
}

void GetCompressedFileSizeWCompressedFile(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\GetCompressedFileSizeWCompressed.txt";
    DeleteFileW(path.c_str());

    // Step 1: Create file and write compressible data
    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"GetCompressedFileSizeW", L"Failed to create file");
        return;
    }

    std::wstring content(1024 * 100, L'A'); // 100 KB of highly compressible data
    DWORD written;
    WriteFile(h, content.c_str(), (DWORD)(content.size() * sizeof(wchar_t)), &written, nullptr);
    CloseHandle(h);

    // Step 2: Open and apply NTFS compression
    h = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"GetCompressedFileSizeW", L"Failed to reopen file for compression");
        DeleteFileW(path.c_str());
        return;
    }

    USHORT compressionFormat = COMPRESSION_FORMAT_DEFAULT;
    DWORD bytesReturned = 0;
    BOOL compressed = DeviceIoControl(h, FSCTL_SET_COMPRESSION, &compressionFormat,
                                      sizeof(compressionFormat), nullptr, 0, &bytesReturned, nullptr);
    CloseHandle(h);

    if (!compressed) {
        LogFailure(L"GetCompressedFileSizeW", L"Compression not supported or failed");
        DeleteFileW(path.c_str());
        return;
    }

    // Step 3: Compare logical size vs compressed size
    h = CreateFileW(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"GetCompressedFileSizeW", L"Failed to reopen for size comparison");
        DeleteFileW(path.c_str());
        return;
    }

    DWORD highLogical = 0;
    DWORD logicalSize = GetFileSize(h, &highLogical);

    DWORD highCompressed = 0;
    DWORD compressedSize = GetCompressedFileSizeW(path.c_str(), &highCompressed);
    CloseHandle(h);

    ULONGLONG logical = ((ULONGLONG)highLogical << 32) | logicalSize;
    ULONGLONG compressedVal = ((ULONGLONG)highCompressed << 32) | compressedSize;

    if (compressedVal < logical) {
        LogSuccess(L"GetCompressedFileSizeW", L"Compressed size is smaller than logical size");
    } else {
        LogFailure(L"GetCompressedFileSizeW", L"Expected smaller compressed size, got equal or larger");
    }

    DeleteFileW(path.c_str());
}

void GetCompressedFileSizeWSparseFile(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\GetCompressedFileSizeWSparse.txt";
    DeleteFileW(path.c_str());

    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE | GENERIC_READ, 0, nullptr,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"GetCompressedFileSizeW", L"Failed to create file");
        return;
    }

    // Mark the file as sparse
    DWORD returned = 0;
    if (!DeviceIoControl(h, FSCTL_SET_SPARSE, nullptr, 0, nullptr, 0, &returned, nullptr)) {
        LogFailure(L"GetCompressedFileSizeW", L"Failed to set sparse flag");
        CloseHandle(h);
        DeleteFileW(path.c_str());
        return;
    }

    // Seek forward and write 1 byte to create sparse region
    LARGE_INTEGER li;
    li.QuadPart = 1024 * 1024; // Seek 1MB forward
    SetFilePointerEx(h, li, nullptr, FILE_BEGIN);

    DWORD written;
    WriteFile(h, "X", 1, &written, nullptr);
    CloseHandle(h);

    // Step 2: Compare compressed vs logical size
    h = CreateFileW(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"GetCompressedFileSizeW", L"Failed to reopen sparse file");
        DeleteFileW(path.c_str());
        return;
    }

    DWORD highLogical = 0;
    DWORD logicalSize = GetFileSize(h, &highLogical);

    DWORD highCompressed = 0;
    DWORD compressedSize = GetCompressedFileSizeW(path.c_str(), &highCompressed);
    CloseHandle(h);

    ULONGLONG logical = ((ULONGLONG)highLogical << 32) | logicalSize;
    ULONGLONG compressed = ((ULONGLONG)highCompressed << 32) | compressedSize;

    if (compressed < logical) {
        LogSuccess(L"GetCompressedFileSizeW", L"Sparse file uses less space on disk than logical size");
    } else {
        LogFailure(L"GetCompressedFileSizeW", L"Expected compressed < logical, got " +
                    std::to_wstring(compressed) + L" vs " + std::to_wstring(logical));
    }

    DeleteFileW(path.c_str());
}

void GetDiskFreeSpaceWBasic(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\GetDiskFreeSpaceABasic.tmp";
    DeleteFileW(path.c_str());

    DWORD spc = 0, bps = 0, freeBefore = 0, total = 0;
    BOOL ok = GetDiskFreeSpaceA("C:\\", &spc, &bps, &freeBefore, &total);

    if (!ok || spc == 0 || bps == 0) {
        LogFailure(L"GetDiskFreeSpaceA", L"Initial call failed or invalid values");
        return;
    }

    // Write a file large enough to span multiple clusters (e.g., 1MB)
    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"GetDiskFreeSpaceA", L"Failed to create test file");
        return;
    }

    std::vector<char> buffer(spc * bps * 10, 'X');  // 10 clusters
    DWORD written;
    WriteFile(h, buffer.data(), (DWORD)buffer.size(), &written, nullptr);
    CloseHandle(h);

    // Query again
    DWORD freeAfter = 0;
    ok = GetDiskFreeSpaceA("C:\\", &spc, &bps, &freeAfter, &total);

    if (!ok || freeAfter == 0) {
        LogFailure(L"GetDiskFreeSpaceA", L"Second query failed or invalid values");
        DeleteFileW(path.c_str());
        return;
    }

    // Compare before vs after
    if (freeAfter < freeBefore) {
        LogSuccess(L"GetDiskFreeSpaceA", L"Free clusters decreased after file write as expected");
    } else {
        LogFailure(L"GetDiskFreeSpaceA", L"Free space did not decrease after writing file");
    }

    DeleteFileW(path.c_str());
}

void GetDiskFreeSpaceWFileDeletionRestoresSpace(const std::wstring& dir) {
	Sleep(SLEEP_MSEC);
    std::wstring path = dir + L"\\GetDiskFreeSpaceAFileDeletion.tmp";
    DeleteFileW(path.c_str());

    DWORD spc = 0, bps = 0, freeBefore = 0, total = 0;
    BOOL ok = GetDiskFreeSpaceA("C:\\", &spc, &bps, &freeBefore, &total);

    if (!ok || spc == 0 || bps == 0) {
        LogFailure(L"GetDiskFreeSpaceA", L"Initial query failed or invalid values");
        return;
    }

    // Write a moderately large file
    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        LogFailure(L"GetDiskFreeSpaceA", L"Failed to create test file");
        return;
    }

    std::vector<char> buffer(spc * bps * 10, 'X');  // 10 clusters
    DWORD written;
    WriteFile(h, buffer.data(), (DWORD)buffer.size(), &written, nullptr);
    CloseHandle(h);

    // Query after file write
    DWORD freeAfterWrite = 0;
    ok = GetDiskFreeSpaceA("C:\\", &spc, &bps, &freeAfterWrite, &total);
    if (!ok || freeAfterWrite == 0) {
        LogFailure(L"GetDiskFreeSpaceA", L"Query after write failed");
        DeleteFileW(path.c_str());
        return;
    }

    // Delete the file
    DeleteFileW(path.c_str());

    // Query again after deletion
    DWORD freeAfterDelete = 0;
    ok = GetDiskFreeSpaceA("C:\\", &spc, &bps, &freeAfterDelete, &total);
    if (!ok || freeAfterDelete == 0) {
        LogFailure(L"GetDiskFreeSpaceA", L"Query after deletion failed");
        return;
    }

    // Compare
    if (freeAfterDelete > freeAfterWrite) {
        LogSuccess(L"GetDiskFreeSpaceA", L"Free clusters increased after deleting file");
    } else {
        LogFailure(L"GetDiskFreeSpaceA", L"Expected free space to increase after deletion, but it did not");
    }
}

int wmain(int argc, wchar_t* argv[]) {
    if (argc != 2) {
        std::wcerr << L"Usage: tester.exe <target_root_path>\n";
        return 1;
    }

    std::wstring dir = argv[1];
    CreateDirectoryW(dir.c_str(), nullptr); // Ensure test root exists

    // --- FILE CREATION ---
    CreateFileWCreateAlways(dir);
    CreateFileWCreateNew(dir);
    CreateFileWTruncateExisting(dir);
    CreateFileWOpenExisting(dir);
    CreateFileWOpenAlways(dir);
    CreateFileWReadWriteAccess(dir);

    // --- SYMBOLIC LINKS ---
    CreateSymbolicLinkWFileLink(dir);
    CreateSymbolicLinkWDirectoryLink(dir);

    // --- HARD LINKS ---
    CreateHardLinkWBasic(dir);
    CreateHardLinkWTargetMissing(dir);
    CreateHardLinkWAlreadyExists(dir);
    CreateHardLinkWModifyLinkReflectsInOriginal(dir);

    // --- DIRECTORY CREATION ---
    CreateDirectoryWBasic(dir);
    CreateDirectoryWAlreadyExists(dir);
    CreateDirectoryWWithSubdirectories(dir);
    CreateDirectoryWInvalidPath(dir);
    CreateDirectoryWRelativePath(dir);

    // --- FILE COPY ---
    CopyFileWBasicCopy(dir);
    CopyFileWFailIfExists(dir);
    CopyFileWOverwriteAllowed(dir);
    CopyFileWSourceMissing(dir);
    CopyFileWSubdirToParent(dir);
    CopyFileWParentToSubdir(dir);
    CopyFileWFromSymbolicLink(dir);

    // --- FILE MOVE ---
    MoveFileWRenameSameDirectory(dir);
    MoveFileWIntoSubdirectory(dir);
    MoveFileWDestinationExists(dir);
    MoveFileWSymbolicLinkItself(dir);
    MoveFileWFromSubdirectoryToParent(dir);
    MoveFileWDirectoryMoveBasic(dir);
    MoveFileWDirectoryRenameInPlace(dir);
    MoveFileWDirectoryIntoSubdirectory(dir);
    MoveFileWDirectoryOverwriteExisting(dir);

    // --- FILE DELETE ---
    DeleteFileWBasic(dir);
    DeleteFileWNotExists(dir);
    DeleteFileWReadOnlyFile(dir);
    DeleteFileWOnDirectory(dir);
    DeleteFileWRelativePath(dir);
    DeleteFileWInSubdirectory(dir);

    // --- DIRECTORY DELETE ---
    RemoveDirectoryWBasic(dir);
    RemoveDirectoryWNonEmpty(dir);
    RemoveDirectoryWRelativePath(dir);
    RemoveDirectoryWNotExist(dir);
    RemoveDirectoryWWithTrailingSlash(dir);
    RemoveDirectoryWHasSubdirectory(dir);
    RemoveDirectoryWSubdirectoryOfParent(dir);

    // --- FILE ATTRIBUTES ---
    SetFileAttributesWReadOnly(dir);
    SetFileAttributesWHidden(dir);
    SetFileAttributesWSystem(dir);
    SetFileAttributesWInvalidFile(dir);
    SetFileAttributesWClearAttributes(dir);
    SetFileAttributesWReadonlyBlocksWrite(dir);
    SetFileAttributesWHiddenHidesFromWildcard(dir);
    SetFileAttributesWNormalClearsOtherFlags(dir);
    SetFileAttributesWOnInvalidPathFails(dir);
    SetFileAttributesWDirectoryReadonly(dir);
    SetFileAttributesWDirectoryHidden(dir);
    SetFileAttributesWDirectorySystem(dir);
    SetFileAttributesWDirectoryNormalClearsOthers(dir);
    SetFileAttributesWDirectoryInvalidPathFails(dir);

    // --- SET END OF FILE ---
    SetEndOfFileTruncate(dir);
    SetEndOfFileExtend(dir);
    SetEndOfFileAtExactSize(dir);
    SetEndOfFileInvalidHandle(dir);
    SetEndOfFileOnReadOnlyHandle(dir);

    // --- FILE INFORMATION ---
    SetFileInformationByHandleDeleteFile(dir);
    SetFileInformationByHandleTruncateFile(dir);
    SetFileInformationByHandleDeleteReadOnlyFile(dir);
    SetFileInformationByHandleRenameFile(dir);
    SetFileInformationByHandleRenameOverwrite(dir);
    SetFileInformationByHandleRenameWithRoot(dir);
    SetFileInformationByHandleChangeTimes(dir);

    // --- GET COMPRESSED FILE SIZE ---
    GetCompressedFileSizeWCompressedFile(dir);
    GetCompressedFileSizeWSparseFile(dir);

    // --- DISK FREE SPACE ---
    GetDiskFreeSpaceWBasic(dir);
    GetDiskFreeSpaceWFileDeletionRestoresSpace(dir);

    // --- WRITE FILE ---
    WriteFileBasic(dir);
    WriteFileWithOffset(dir);
    WriteFileBeyondEOF(dir);
    WriteFileAppendMode(dir);
    WriteFileToReadOnlyFile(dir);
    WriteFileInvalidHandle(dir);

    // --- READ FILE ---
    ReadFileBasic(dir);
    ReadFilePartial(dir);
    ReadFileAtEOFReturnsZero(dir);
    ReadFileClosedHandleFails(dir);
    ReadFileInvalidHandleFails(dir);
    ReadFileStart(dir);
    ReadFileMiddle(dir);
    ReadFileEnd(dir);

    // --- LOCK FILE ---
    LockFileBasicExclusive(dir);
    LockFileInvalidHandle(dir);
    LockFileNonOverlappingSuccess(dir);
    LockFileAlreadyLockedFails(dir);
    LockFileUnlockRegion(dir);

    // --- UNLOCK FILE ---
    UnlockFileBasic(dir);
    UnlockFileWithoutLockFails(dir);
    UnlockFileWrongRegionFails(dir);
    UnlockFileWithInvalidHandle(dir);
    UnlockFilePartialUnlockThenAccess(dir);

    return 0;
}
