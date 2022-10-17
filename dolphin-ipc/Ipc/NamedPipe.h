#pragma once

#include "windows.h"
#include <string>

class NamedPipe
{
public:
    NamedPipe(std::string& sName);
    virtual ~NamedPipe(void);

    void send(std::string& sData);
    void recv(std::string& sData);

private:
    void Init();

    void WaitForClient();
    void Close();
    bool Read();
    bool Write();

    const std::string m_sPipeName;
    HANDLE m_hPipe;
    char* m_buffer = nullptr;
};
