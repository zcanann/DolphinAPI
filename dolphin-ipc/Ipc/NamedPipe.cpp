#include "NamedPipe.h"

#include <codecvt>
#include <iostream>
#include <locale>
#include "process.h"
#include "windows.h"
#include "tchar.h"

#pragma optimize("", off)

NamedPipe::NamedPipe(std::string& sName, bool isServer) : m_sPipeName(sName), m_isServer(isServer)
{
    if (m_sPipeName.empty())
    {
        std::cout << "Error: Invalid pipe name" << std::endl;
        return;
    }

    std::wstring pipeName = L"\\\\.\\pipe\\" + std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(m_sPipeName);

    if (m_isServer)
    {
        m_hPipe = ::CreateNamedPipe(
            pipeName.c_str(),
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            m_buffer.size(),
            m_buffer.size(),
            NMPWAIT_USE_DEFAULT_WAIT,
            NULL);

        ConnectNamedPipe(m_hPipe, NULL);
    }
    else
    {
        m_hPipe = ::CreateFile(
            pipeName.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
    }

    if (INVALID_HANDLE_VALUE == m_hPipe)
    {
        std::cout << "Error: Could not create named pipe" << std::endl;
    }
}

NamedPipe::~NamedPipe()
{
    close();
}

bool NamedPipe::send(std::string& sData)
{
    // std::fill(m_buffer.begin(), m_buffer.end(), 0);
    // memcpy(&m_buffer[0], sData.c_str(), __min(m_buffer.size(), sData.size()));

    DWORD bytesWritten;
    BOOL bResult = ::WriteFile(
        m_hPipe,
        sData.data(),
        DWORD(sData.size()),
        &bytesWritten,
        NULL);

    if (FALSE == bResult || DWORD(sData.size()) != bytesWritten)
    {
        std::cout << "WriteFile failed" << std::endl;
        return false;
    }

    return true;
}

bool NamedPipe::recv(std::string& sData)
{
    sData.clear();
    sData.append(m_buffer.data());

    DWORD bytesRead = 0;
    BOOL bFinishedRead = FALSE;
    int read = 0;

    do
    {
        bFinishedRead = ::ReadFile(
            m_hPipe,
            &m_buffer[read],
            (DWORD)m_buffer.size(),
            &bytesRead,
            NULL);

        if (!bFinishedRead && ERROR_MORE_DATA != GetLastError())
        {
            bFinishedRead = FALSE;
            break;
        }

        read += bytesRead;

    } while (!bFinishedRead);

    if (FALSE == bFinishedRead || 0 == bytesRead)
    {
        std::cout << "ReadFile failed" << std::endl;
        return false;
    }

    return true;
}

void NamedPipe::close()
{
    ::CloseHandle(m_hPipe);
    m_hPipe = NULL;
}


#pragma optimize("", on)