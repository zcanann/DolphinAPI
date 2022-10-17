#pragma once

#include "windows.h"
#include <string>
#include <vector>

class NamedPipe
{
public:
    NamedPipe(std::string& sName, bool isOwner);
    virtual ~NamedPipe(void);

    bool send(std::string& sData);
    bool recv(std::string& sData);

private:
    void close();

    const std::string m_sPipeName;
    HANDLE m_hPipe = nullptr;
    bool m_isOwner = false;
    bool m_hasConnected = false;
    std::vector<char> m_buffer = std::vector<char>(65536);
};
