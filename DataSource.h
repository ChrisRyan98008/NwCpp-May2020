#pragma once

#include "types.h"
#include <vector>
#include <string>

#include <fstream>
#ifdef _MSC_VER
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <netdb.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>

using SOCKET        = int;
using ADDRINFO      = addrinfo;
using SOCKADDR      = sockaddr;
using SOCKADDR_IN   = sockaddr_in;

constexpr auto SOCKET_ERROR = -1;
#define closesocket(x) close(x)
#endif

namespace Serialize {

class IDataSource
{
public:
    virtual ~IDataSource() = default;
    virtual int32 save(void* pData, uint32 size) = 0;
    virtual int32 load(void* pData, uint32 size) = 0;
};

class FileSource : public IDataSource
{
    using Mode = std::ios_base::openmode;
    std::fstream _file;

    virtual int32 save(void* pData, uint32 size)  { _file.write((char*)pData, size); return size; };
    virtual int32 load(void* pData, uint32 size)  { _file.read( (char*)pData, size); return size; };

public:
    static const Mode Load = std::ios_base::binary | std::ios_base::in;
    static const Mode Save = std::ios_base::binary | std::ios_base::out | std::ios_base::trunc;

    FileSource(const char* pFilename, const Mode mode = Save) : _file(pFilename, mode) {}
};

class SocketSource : public IDataSource
{
    SOCKET _sock;

    virtual int32 save(void* pData, uint32 size)  { return (int32)::send(_sock, (char*)pData, size, 0); };
    virtual int32 load(void* pData, uint32 size)  { return (int32)::recv(_sock, (char*)pData, size, 0); };

    void Init()
    {
#ifdef _MSC_VER
        static WSADATA _wsaData = { 0 };
        static int wsa = WSAStartup(MAKEWORD(1, 0), &_wsaData);
#endif
    }

public:
    SocketSource(const char* pName, const short port = 27015) : _sock(0)        //client
    {
        Init();
        ADDRINFO* pAddr = nullptr;
        ::getaddrinfo(pName, std::to_string(port).c_str(), nullptr, &pAddr);
        for(ADDRINFO *ptr = pAddr; ptr; ptr = ptr->ai_next)
        {
            _sock = ::socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
            if(::connect(_sock, ptr->ai_addr, (int)ptr->ai_addrlen) != SOCKET_ERROR)
                break;

            ::closesocket(_sock);
        }
        ::freeaddrinfo(pAddr);
    }
    SocketSource(short port = 27015) : _sock(0)     //server
    {
        Init();
        int opt = 1;
        SOCKADDR_IN sa = {AF_INET, htons(port), {INADDR_ANY},};
        SOCKET sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
        ::bind(sock, (SOCKADDR *)&sa, sizeof(sa));
        ::listen(sock, SOMAXCONN);
        _sock = ::accept(sock, nullptr, nullptr);
        ::closesocket(sock);
    }
    ~SocketSource()
    {
        if(_sock)
            closesocket(_sock);
    }
};

class FilterSource : public IDataSource
{
    IDataSource& _source;
    BYTE         _mask = 0;

    virtual int32 save(void* pVoid, uint32 size)
    {
        std::vector<BYTE> data(size);
        Mask(data.data(), (BYTE*)pVoid, size);
        return _source.save(data.data(), size);
    }
    virtual int32 load(void* pData, uint32 size)
    {
        int32 ret = _source.load(pData, size);
        Mask((BYTE*)pData, (BYTE*)pData, size);
        return ret;
    }

    void Mask(BYTE* pDest, BYTE* pSrc, uint32 size)
    {
        while(size--)
            *pDest++ = *pSrc++ ^ _mask;
    }

public:
    FilterSource(IDataSource& source, BYTE mask = 0) : _source(source), _mask(mask) {}
    void SetMask(BYTE mask) { _mask = mask; }
};

class MemorySource : public IDataSource
{
    using Data = std::vector<BYTE>;
    Data    _blob;
    uint32  _offset = 0;

    virtual int32 save(void* pData, uint32 size)
    {
        _blob.insert(_blob.end(), ((BYTE*)pData), ((BYTE*)pData) + size);
        _offset += size;
        return size;
    };
    virtual int32 load(void* pData, uint32 size)
    {
        if(_blob.size() < _offset + size) return -1;
        std::memcpy(pData, _blob.data() + _offset, size);
        _offset += size;
        return size;
    };

public:
    MemorySource(const size_t size=0) { if(size) _blob.reserve(size); }
    MemorySource(const Data& blob) { SetData(blob); }

    void Reset() { _blob.clear(); _offset = 0; }

    Data GetData() const
    {
        Data blob;
        blob.assign(_blob.begin(), _blob.end());
        return blob;
    }
    void SetData(const Data& blob)
    {
        _blob.assign(blob.begin(), blob.end());
        _offset = 0;
    }
};

}//namespace Serialize

