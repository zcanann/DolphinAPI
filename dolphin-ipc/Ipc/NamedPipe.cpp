#include "NamedPipe.h"

#include <codecvt>
#include <iostream>
#include <locale>
#include "process.h"
#include "windows.h"
#include "tchar.h"

#pragma optimize("", off)

NamedPipe::NamedPipe(std::string& sName) : m_sPipeName(sName)
{
    if (m_sPipeName.empty())
    {
        std::cout << "Error: Invalid pipe name" << std::endl;
        return;
    }

    std::wstring pipeName = L"\\\\.\\pipe\\" + std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(m_sPipeName);

    m_hPipe = ::CreateNamedPipe(
        pipeName.c_str(),                // pipe name 
        PIPE_ACCESS_DUPLEX,       // read/write access 
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,  // message-type pipe/message-read mode/blocking mode
        PIPE_UNLIMITED_INSTANCES, // max. instances  
        1024,              // output buffer size 
        1024,              // input buffer size 
        NMPWAIT_USE_DEFAULT_WAIT, // client time-out 
        NULL);                    // default security attribute 

    if (INVALID_HANDLE_VALUE == m_hPipe)
    {
        std::cout << "Error: Could not create named pipe" << std::endl;
    }
}

NamedPipe::~NamedPipe(void)
{
}

void NamedPipe::send(std::string& sData)
{
   // memset(&m_buffer[0], 0, BUFFER_SIZE);
    // memcpy(&m_buffer[0], sData.c_str(), __min(BUFFER_SIZE, sData.size()));
}

void NamedPipe::recv(std::string& sData)
{
    sData.clear(); // Clear old data, if any
    // sData.append(m_buffer);
}

void NamedPipe::Close()
{
    ::CloseHandle(m_hPipe);
    m_hPipe = NULL;
}

bool NamedPipe::Read()
{
    return false;
    DWORD drBytes = 0;
    BOOL bFinishedRead = FALSE;
    int read = 0;

    do
    {
        bFinishedRead = ::ReadFile( 
            m_hPipe,
            &m_buffer[read],
            m_buffer.size(),
            &drBytes,
            NULL);

        if(!bFinishedRead && ERROR_MORE_DATA != GetLastError())
        {
            bFinishedRead = FALSE;
            break;
        }

        read += drBytes;

    } while (!bFinishedRead);

    if(FALSE == bFinishedRead || 0 == drBytes)
    {
        std::cout << "ReadFile failed" << std::endl;
        return false;
    }

    return true;
}

bool NamedPipe::Write()
{
    return false;
    DWORD dwBytes;
    BOOL bResult = ::WriteFile(
        m_hPipe,
        m_buffer.data(),
        ::strlen(m_buffer.data())*sizeof(wchar_t) + 1,
        &dwBytes,
        NULL);

    if(FALSE == bResult || strlen(m_buffer.data())*sizeof(wchar_t) + 1 != dwBytes)
    {
        //SetEventData("WriteFile failed");
        return false;
    }

    return true;
}

#pragma optimize("", on)