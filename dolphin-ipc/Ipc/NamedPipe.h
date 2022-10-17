#pragma once

#include "windows.h"
#include <string>
#include <vector>

class NamedPipe
{
public:
    NamedPipe(std::string& sName);
    virtual ~NamedPipe(void);

    void send(std::string& sData);
    void recv(std::string& sData);

private:
    void Close();
    bool Read();
    bool Write();

    const std::string m_sPipeName;
    HANDLE m_hPipe = nullptr;
    std::vector<char> m_buffer = std::vector<char>(65535);
};
