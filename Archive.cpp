
#include "Archive.h"

using namespace Serialize;

//Archive non-templatized implementations
void Archive::Reset()
{
    _mapTypeId.clear();
    _mapObjShared.clear();
    _mapIdType.clear();
    _mapIdObj.clear();
    _mapObjId.clear();
    _mapIdObj[ID_NULL]  = nullptr;
    _mapObjId[nullptr]  = ID_NULL;
    _nextObjId          = ID_START;
    _nextTypeId         = ID_START;
    _hash               = BIG_PRIME;
    _error              = 0;
}

bool Archive::CheckPoint()
{
    if(IsError()) return false;
    switch(_mode)
    {
    case SaveArchive:
        Save(_hash);
        return true;
    case LoadArchive:
    {
        uint32 hash = _hash;
        uint32 fileHash = {};
        Load(fileHash);
        if(fileHash == hash)
            return true;
    }
    default:
        break;
    }
    Error();
    return false;
}

void Archive::Save(std::string& str)
{
    if(IsError()) return;
    uint32 size = uint32(str.length());
    SaveDint(size);
    if(size && (save((BYTE*)str.data(), size) != int32(size)))
        return Error();
}
void Archive::Load(std::string& str)
{
    if(IsError()) return;
    uint32 size = LoadDint();
    str.clear();
    if(size)
    {
        std::string str2(size + 1, '\0');
        if(load((BYTE*)str2.data(), size) != int32(size))
            return Error();
        str.swap(str2);
    }
}

void Archive::Save(void* pVoid, uint32 size)
{
    if(IsError()) return;
    SaveDint(size);
    if(size && (save(pVoid, size) != int32(size)))
        return Error();
}
void Archive::Load(void* pVoid, uint32 size)
{
    if(IsError()) return;
    uint32 arcSize = LoadDint();
    if(arcSize > size)
        return Error();
    if(arcSize && (load(pVoid, arcSize) != int32(arcSize)))
        return Error();
}

void Archive::SaveType(SerializableBase* pObj)
{
    const TypeInfo* pTypeInfo = pObj->GetTypeInfo();
    TypeId& typeId = _mapTypeId[pTypeInfo];
    if(typeId)
        SaveDint(typeId);
    else
    {
        typeId = _nextTypeId++;
        SaveDint(typeId);
        HASH hash = pTypeInfo->Hash();
        Save(hash);
    }
}
const TypeInfo* Archive::LoadType()
{
    TypeId typeId = LoadDint();
    const TypeInfo*& pTypeInfo = _mapIdType[typeId];
    if(!pTypeInfo)
    {
        HASH hash = 0;
        Load(hash);
        pTypeInfo = TypeInfo::Find(hash);
    }
    return pTypeInfo;
}

void Archive::SaveDint(uint32 dint)
{
    do
    {
        uint8 u8 = uint8(dint & 0x7f);
        dint >>= 7;
        if(!dint) u8 |= 0x80;
        save(&u8, sizeof(u8));
    } while(dint);
}
uint32 Archive::LoadDint()
{
    uint32  dint = 0;
    uint32  shift = 0;
    uint8   u8 = 0;
    do
    {
        load(&u8, sizeof(u8));
        dint |= (uint32(u8 & 0x7f) << shift);
        shift += 7;
    } while(!(u8 & 0x80));
    return dint;
}

int32 Archive::save(void* pData, uint32 size)
{
    Hash((BYTE*)pData, size);
    return _source.save(pData, size);
}
int32 Archive::load(void* pData, uint32 size)
{
    int32 ret = _source.load(pData, size);
    Hash((BYTE*)pData, size);
    return ret;
}

