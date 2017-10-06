//
// Copyright 2017 OSR Open Systems Resources, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from this
//    software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE
// 
#include <stdio.h>
#include <windows.h>
#include <winternl.h>


FILE_NOTIFY_INFORMATION GlobalChangeBuf[1024];

VOID
CALLBACK
FileIOCompletionRoutine(
    DWORD dwErrorCode,
    DWORD dwNumberOfBytesTransfered,
    LPOVERLAPPED lpOverlapped
    );

PCHAR 
ActionToString(
    DWORD Action
    );


//
// Some definitions missing in user mode but that exist in NTIFS.H
//
#define FILE_ACTION_ADDED_STREAM            0x00000006
#define FILE_ACTION_REMOVED_STREAM          0x00000007
#define FILE_ACTION_MODIFIED_STREAM         0x00000008
#define FILE_ACTION_REMOVED_BY_DELETE       0x00000009
#define FILE_ACTION_ID_NOT_TUNNELLED        0x0000000A
#define FILE_ACTION_TUNNELLED_ID_COLLISION  0x0000000B

#define FILE_NOTIFY_VALID_MASK          0x00000fff


void __cdecl wmain(int argc, wchar_t **argv)
{

    DWORD      bytesRet;
    HANDLE     dummyEvent;
    HANDLE     dirHandle;
    OVERLAPPED overlapped = {0};

    if (argc != 2) {
        printf("dirchange.exe <path>\n");
        return;
    }

    overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    printf("CreateFile for directory directory change notification %ls\n", 
           argv[1]);

    dirHandle = CreateFile(argv[1],
                           FILE_LIST_DIRECTORY,
                           FILE_SHARE_READ       |
                                FILE_SHARE_WRITE |
                                FILE_SHARE_DELETE,
                           0,
                           OPEN_EXISTING,
                           FILE_FLAG_OVERLAPPED  |
                                FILE_FLAG_BACKUP_SEMANTICS,
                           0);

    //
    // Make sure our CreateFile succeeded...
    //
    if (dirHandle == INVALID_HANDLE_VALUE) {

        printf("CreateFile failed with error 0x%x\n", GetLastError());

        return;
    }

    printf("Modifications to %ls will be shown as they happen\n", argv[1]);

    dummyEvent = CreateEvent(NULL, FALSE, 0, NULL);

    while (TRUE) {

        if (ReadDirectoryChangesW(dirHandle,
                                  &GlobalChangeBuf,
                                  sizeof(GlobalChangeBuf),
                                  TRUE,
                                  FILE_NOTIFY_VALID_MASK,
                                  &bytesRet,
                                  &overlapped,
                                  FileIOCompletionRoutine) == 0) {

            printf("ReadDirectoryChangesW failed with error 0x%x\n", 
                   GetLastError());

            return;
        }

        //
        // FileIOCompletionRoutine gets called if there's a change and we're
        // waiting alertable. Go to sleep and we'll wake up after a change
        // is delivered (at which point we'll go back up and pend another 
        // change notification)
        //
        WaitForSingleObjectEx(dummyEvent, INFINITE, TRUE);
    }

    return;
}

VOID
CALLBACK
FileIOCompletionRoutine(
    DWORD dwErrorCode,
    DWORD dwNumberOfBytesTransfered,
    LPOVERLAPPED lpOverlapped)
{
    PFILE_NOTIFY_INFORMATION changeInfo = &GlobalChangeBuf[0];
    UNICODE_STRING           us;

    if (!dwNumberOfBytesTransfered) {
        return;
    }

    while (TRUE) {

        printf("NewEntry:\n");
        printf("\tAction: %d (%s)\n", changeInfo->Action,
            ActionToString(changeInfo->Action));

        us.Buffer        = changeInfo->FileName;
        us.Length        = (USHORT)changeInfo->FileNameLength;
        us.MaximumLength = us.Length;
        printf("\tName: %wZ\n", &us);


        if (changeInfo->NextEntryOffset > 0) {
            changeInfo = (PFILE_NOTIFY_INFORMATION)
                            ((ULONG_PTR)changeInfo + 
                                            changeInfo->NextEntryOffset);
        } else {
            break;
        }

    }

    return;
}



PCHAR 
ActionToString(
    DWORD Action) 
{

    switch (Action) {
        case FILE_ACTION_ADDED:
            return "FILE_ACTION_ADDED";
        case FILE_ACTION_REMOVED:
            return "FILE_ACTION_REMOVED";
        case FILE_ACTION_MODIFIED:
            return "FILE_ACTION_MODIFIED";
        case FILE_ACTION_RENAMED_OLD_NAME:
            return "FILE_ACTION_RENAMED_OLD_NAME";
        case FILE_ACTION_RENAMED_NEW_NAME:
            return "FILE_ACTION_RENAMED_NEW_NAME";
        case FILE_ACTION_ADDED_STREAM:
            return "FILE_ACTION_ADDED_STREAM";
        case FILE_ACTION_REMOVED_STREAM:
            return "FILE_ACTION_REMOVED_STREAM";
        case FILE_ACTION_MODIFIED_STREAM:
            return "FILE_ACTION_MODIFIED_STREAM";
        case FILE_ACTION_REMOVED_BY_DELETE:
            return "FILE_ACTION_REMOVED_BY_DELETE";
        case FILE_ACTION_ID_NOT_TUNNELLED:
            return "FILE_ACTION_ID_NOT_TUNNELLED";
        case FILE_ACTION_TUNNELLED_ID_COLLISION:
            return "FILE_ACTION_TUNNELLED_ID_COLLISION";
        default:
            return "Unknown";
    }
}


