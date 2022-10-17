#pragma once

#include "windows.h"
#include <string>
#include <vector>

class NamedPipe
{
public:
    NamedPipe(std::string& sName, bool isServer);
    virtual ~NamedPipe(void);

    bool send(std::string& sData);
    bool recv(std::string& sData);

private:
    void close();

    const std::string m_sPipeName;
    HANDLE m_hPipe = nullptr;
    bool m_isServer = false;
    std::vector<char> m_buffer = std::vector<char>(65535);
};
