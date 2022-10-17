#include "NamedPipe.h"

#include <codecvt>
#include <iostream>
#include <future>
#include <locale>
#include "process.h"
#include "windows.h"
#include "tchar.h"

#pragma optimize("", off)

NamedPipe::NamedPipe(std::string& sName, bool isOwner) : m_sPipeName(sName), m_isOwner(isOwner)
{
    if (m_sPipeName.empty())
    {
        std::cout << "Error: Invalid pipe name" << std::endl;
        return;
    }

    std::wstring pipeName = L"\\\\.\\pipe\\" + std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(m_sPipeName);

    if (m_isOwner)
    {
        m_hPipe = ::CreateNamedPipe(
            pipeName.c_str(),
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT,
            PIPE_UNLIMITED_INSTANCES,
            DWORD(m_buffer.size()),
            DWORD(m_buffer.size()),
            NMPWAIT_WAIT_FOREVER,
            NULL);

        ::ConnectNamedPipe(m_hPipe, NULL);
    }
    else
    {
        m_hPipe = ::CreateFile(
            pipeName.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);
    }

    if (m_hPipe == INVALID_HANDLE_VALUE)
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
    std::fill(m_buffer.begin(), m_buffer.end(), 0);
    memcpy(&m_buffer[0], sData.c_str(), __min(m_buffer.size(), sData.size()));

    DWORD bytesWritten;
    BOOL bResult = ::WriteFile(
        m_hPipe,
        sData.data(),
        DWORD(sData.size()),
        &bytesWritten,
        NULL);

    if (bResult == FALSE || DWORD(sData.size()) != bytesWritten)
    {
        std::cout << "WriteFile failed: " << GetLastError() << std::endl;
        return false;
    }

    return true;
}

bool NamedPipe::recv(std::string& sData)
{
    sData.clear();

    DWORD bytesRead = 0;
    BOOL bFinishedRead = FALSE;
    int read = 0;

    do
    {
        bool peak = ::PeekNamedPipe(
            m_hPipe,
            &m_buffer[read],
            (DWORD)m_buffer.size(),
            &bytesRead,
            0,
            0);

        if (!peak || bytesRead == 0)
        {
            return false;
        }

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

    if (bFinishedRead == FALSE || 0 == bytesRead)
    {
        std::cout << "ReadFile failed" << GetLastError() << std::endl;
        return false;
    }
    
    // sData.append(m_buffer.data());
    sData.resize(bytesRead);
    memcpy((void*)sData.data(), &m_buffer, bytesRead);

    return true;
}

void NamedPipe::close()
{
    ::CloseHandle(m_hPipe);
    m_hPipe = NULL;
}


#pragma optimize("", on)