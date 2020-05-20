#pragma once

//Serialize templatized implementations
namespace Serialize {

template<typename Type>
Archive& Archive::Serialize(Type& obj)
{
    switch(_mode)
    {
    case SaveArchive:   Save(obj);  break;
    case LoadArchive:   Load(obj);  break;
    default:            Error();    break;
    }
    return *this;
}

template<typename Type>
Archive& Archive::operator<<(Type& obj)
{
    SetSave();
    Save(obj);
    return *this;
}
template<typename Type>
Archive& Archive::operator>>(Type& obj)
{
    SetLoad();
    Load(obj);
    return *this;
}

template<typename Type> 
if_Serializable<Type, void> Archive::Save(Type& obj)
{
    if(IsError()) return;
    SaveType(&obj);
    obj.Serialize(*this);
}
template<typename Type>
if_Serializable<Type, void> Archive::Load(Type& obj)
{
    if(IsError()) return;
    const TypeInfo* pTypeInfo = LoadType();
    if(!pTypeInfo || (pTypeInfo != obj.GetTypeInfo()))
        return Error();
    obj.Serialize(*this);
}

template<typename Type>
if_Serializable<Type, void> Archive::Save(Type*& pObj)
{
    if(IsError()) return;
    ObjId& objId = _mapObjId[pObj];
    if(objId)
    {
        SaveDint(objId);
        return;
    }
    objId = _nextObjId++;
    SaveDint(objId);
    SaveType(pObj);
    pObj->Serialize(*this);
}
template<typename Type>
if_Serializable<Type, void> Archive::Load(Type*& pObj)
{
    if(IsError()) return;
    ObjId objId = LoadDint();
    Type*& pNew = (Type*&)_mapIdObj[objId];
    if(!pNew && (objId != ID_NULL))
    {
        const TypeInfo* pTypeInfo = LoadType();
        if(!pTypeInfo)
            return Error();
        HASH hash = Type::s_typeinfo.Hash();
        pNew = (Type*)pTypeInfo->Create();
        if(!pNew->IsOfType(hash))
        {
            delete pNew;    //pNew is not of type Type.
            return Error();
        }
        pNew->Serialize(*this);
    }
    if(pObj)
        delete pObj;
    pObj = pNew;
}

template<typename Type, size_t count>
if_Serializable<Type, void> Archive::Save(Type(&array)[count])      //array[] of serializable derived object
{
    if(IsError()) return;
    SaveDint(count);
    SaveType(array);
    for(auto& item : array)
        item.Serialize(*this);
}
template<typename Type, size_t count>
if_Serializable<Type, void> Archive::Load(Type(&array)[count])
{
    if(IsError()) return;
    uint32 arcCount = LoadDint();
    if(arcCount != count)
        return Error();
    const TypeInfo* pTypeInfo = LoadType();
    if(!pTypeInfo || (pTypeInfo != array->GetTypeInfo()))
        return Error();
    for(auto& item : array)
        item.Serialize(*this);
}

template<typename Type>
if_PlainOldData<Type, void> Archive::Save(Type& data)
{
    if(IsError()) return;
    save(&data, sizeof(data));
}
template<typename Type>
if_PlainOldData<Type, void> Archive::Load(Type& data)
{
    if(IsError()) return;
    load(&data, sizeof(data));
}

template<typename Type>
if_PlainOldData<Type, void> Archive::Save(Type*& pObj)
{
    if(IsError()) return;
    ObjId& objId = _mapObjId[pObj];
    if(objId)
    {
        SaveDint(objId);
        return;
    }
    objId = _nextObjId++;
    SaveDint(objId);
    Save(*pObj);
}
template<typename Type>
if_PlainOldData<Type, void> Archive::Load(Type*& pObj)
{
    if(IsError()) return;
    ObjId objId = LoadDint();
    Type*& pNew = (Type*&)_mapIdObj[objId];
    if(!pNew && (objId != ID_NULL))
    {
        pNew = new Type;
        Load(*pNew);
    }
    if(pObj != pNew)
    {
        if(pObj)
            delete pObj;
        pObj = pNew;
    }
}

template<typename Type, size_t count>
if_PlainOldData<Type, void> Archive::Save(Type(&array)[count])      //array[] of POD types (non-ints)
{
    if(IsError()) return;
    SaveDint(count);
    save(&array, sizeof(array));
}
template<typename Type, size_t count>
if_PlainOldData<Type, void> Archive::Load(Type(&array)[count])
{
    if(IsError()) return;
    uint32 arcCount = LoadDint();
    if(arcCount > count)
        return Error();
    load(&array, sizeof(array));
}

template<typename Type>
if_IntegralType<Type, void> Archive::Save(Type& data)
{
    if(IsError()) return;
    using unType = typename std::make_unsigned<Type>::type;
    unType un_data = *(unType*)&data;
    unType un_nbo = ByteOrder(un_data);
    save((void*)&un_nbo, sizeof(Type));
}
template<typename Type>
if_IntegralType<Type, void> Archive::Load(Type& data)
{
    if(IsError()) return;
    using unType = typename std::make_unsigned<Type>::type;
    unType un_data = {};
    load(&un_data, sizeof(Type));
    unType un_BO = ByteOrder(un_data);
    data = *(Type*)&un_BO;
}

template<typename Type, size_t count>
if_IntegralType<Type, void> Archive::Save(Type(&array)[count])      //array[] of ints (byte order)
{
    if(IsError()) return;
    SaveDint(count);
    for(auto & item : array)
        Save(item);
}
template<typename Type, size_t count>
if_IntegralType<Type, void> Archive::Load(Type(&array)[count])
{
    if(IsError()) return;
    uint32 arcCount = LoadDint();
    if(arcCount > count)
        return Error();
    for(auto & item : array)
        Load(item);
}

template<size_t count>
void Archive::Save(char(&sz)[count])
{
    if(IsError()) return;
    char* p2 = sz;
    uint32 len = 0;
    while(*p2++ && count > len++);
    if(count < len)
        return Error();
    SaveDint(len);
    if(len)
        save(sz, len);
}
template<size_t count>
void Archive::Load(char(&sz)[count])
{
    if(IsError()) return;
    uint32 size = LoadDint();
    if(count <= size)
        return Error();
    if(size)
        load(sz, size);
    sz[size] = '\0';
}

template<typename Type>
void Archive::Save(std::shared_ptr<Type>& ptr)
{
    if(IsError()) return;
    Type* pType = ptr.get();
    Save(pType);
}
template<typename Type>
void Archive::Load(std::shared_ptr<Type>& ptr)
{
    if(IsError()) return;
    Type* pType = nullptr;
    Load(pType);
    if(pType)
    {
        std::shared_ptr<Type>& type = (std::shared_ptr<Type>&)_mapObjShared[pType];
        if(!type)
            type = std::shared_ptr<Type>(pType);
        ptr = type;
    }
}

template<typename Type>
void Archive::Save(std::unique_ptr<Type>& ptr)
{
    if(IsError()) return;
    Type* type = ptr.get();
    Save(type);
}
template<typename Type>
void Archive::Load(std::unique_ptr<Type>& ptr)
{
    if(IsError()) return;
    Type* pType = nullptr;
    Load(pType);
    ptr = std::unique_ptr<Type>(pType);
}

template<typename Type>
void Archive::Save(std::vector<Type>& vector)
{
    if(IsError()) return;
    uint32 size = uint32(vector.size());
    SaveDint(size);
    for(Type& item : vector)
        Save(item);
}
template<typename Type>
void Archive::Load(std::vector<Type>& vector)
{
    if(IsError()) return;
    uint32 size = LoadDint();
    vector.clear();
    vector.reserve(size);
    for(uint32 i = 0; i < size; i++)
    {
        Type type = {};;
        Load(type);
        vector.push_back(type);
    }
}

template<typename Type, size_t count>
void Archive::Save(std::array<Type, count>& array)
{
    if(IsError()) return;
    SaveDint(count);
    for(Type& item : array)
        Save(item);
}
template<typename Type, size_t count>
void Archive::Load(std::array<Type, count>& array)
{
    if(IsError()) return;
    uint32 arcCount = LoadDint();
    if(arcCount != count)
        return Error();
    for(Type& item : array)
        Load(item);
}

template<typename Type>
void Archive::Save(std::list<Type>& list)
{
    if(IsError()) return;
    uint32 size = uint32(list.size());
    SaveDint(size);
    for(Type& item : list)
        Save(item);
}
template<typename Type>
void Archive::Load(std::list<Type>& list)
{
    if(IsError()) return;
    uint32 size = LoadDint();
    list.clear();
    for(uint32 i = 0; i < size; i++)
    {
        Type type = {};
        Load(type);
        list.push_back(type);
    }
}

template<typename Key, typename Value>
void Archive::Save(std::map<Key, Value>& map)
{
    if(IsError()) return;
    uint32 size = uint32(map.size());
    SaveDint(size);
    for(auto& pair : map)
    {
        Save(pair.first);
        Save(pair.second);
    }
}
template<typename Key, typename Value>
void Archive::Load(std::map<Key, Value>& map)
{
    if(IsError()) return;
    uint32 size = LoadDint();
    map.clear();
    for(uint32 i = 0; i < size; i++)
    {
        Key key = {};
        Value value = {};
        Load(key);
        Load(value);
        map[key] = value;
    }
}

#if __cplusplus < 201703L
#define constexpr
#endif

template<typename Type>
if_IntegralType<Type, Type> ByteOrder(Type data)
{
    static const uint32 i = 1;
    if((*(BYTE*)&i) == 1)
    {
        if constexpr(sizeof(data) == sizeof(uint16))
        {
            return Type(((uint16(data) << 8) & 0xff00U) |
                        ((uint16(data) >> 8) & 0x00ffU));
        }
        else if constexpr(sizeof(data) == sizeof(uint32))
        {
            return Type(((uint32(data) << 24) & 0xff000000U) |
                        ((uint32(data) <<  8) & 0x00ff0000U) |
                        ((uint32(data) >>  8) & 0x0000ff00U) |
                        ((uint32(data) >> 24) & 0x000000ffU));
        }
        else if constexpr(sizeof(data) == sizeof(uint64))
        {
            return Type(((uint64(data) << 56) & 0xff00000000000000ULL) |
                        ((uint64(data) << 40) & 0x00ff000000000000ULL) |
                        ((uint64(data) << 24) & 0x0000ff0000000000ULL) |
                        ((uint64(data) <<  8) & 0x000000ff00000000ULL) |
                        ((uint64(data) >>  8) & 0x00000000ff000000ULL) |
                        ((uint64(data) >> 24) & 0x0000000000ff0000ULL) |
                        ((uint64(data) >> 40) & 0x000000000000ff00ULL) |
                        ((uint64(data) >> 56) & 0x00000000000000ffULL));
        }
        return data;
    }
    return data;
}

#if __cplusplus < 201703L
#undef constexpr
#endif

}//namespace Serialize

