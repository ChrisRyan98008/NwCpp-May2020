#pragma once

#include <map>
#include <list>
#include <array>
#include <vector>
#include <memory>
#include <string>

#include "types.h"
#include "Serializable.h"
#include "DataSource.h"

namespace Serialize {

template<class Type> constexpr bool is_IntegralType = std::is_integral<Type>::value;
template<class Type> constexpr bool is_Serializable = std::is_base_of<SerializableBase, Type>::value;
template<class Type> constexpr bool is_PlainOldData = (std::is_pod<Type>::value && !std::is_integral<Type>::value);

template<class Type, class RetType = void> using if_IntegralType = std::enable_if_t<is_IntegralType<Type>, RetType>;
template<class Type, class RetType = void> using if_Serializable = std::enable_if_t<is_Serializable<Type>, RetType>;
template<class Type, class RetType = void> using if_PlainOldData = std::enable_if_t<is_PlainOldData<Type>, RetType>;

template<typename Type> if_IntegralType<Type, Type> ByteOrder(Type data);

class Archive
{
public:
    enum Mode { Unknown, SaveArchive, LoadArchive, };

    Archive(IDataSource& source, Mode mode= Unknown) : _source(source), _mode(mode) { Reset(); }
    virtual ~Archive() = default;

    template<typename Type>           Archive& operator<<(Type& obj);
    template<typename Type>           Archive& operator>>(Type& obj);
    template<typename Type>                 Archive& Serialize(Type& obj);

public:
    void Reset();
    bool CheckPoint();

    void SetSave()  { if(_mode != SaveArchive) { Reset(); _mode = SaveArchive; } }
    void SetLoad()  { if(_mode != LoadArchive) { Reset(); _mode = LoadArchive; } }
    bool IsSave()   { return _mode == SaveArchive; };
    bool IsLoad()   { return _mode == LoadArchive; };
    bool IsError()  { return _error > 0; }

protected:
    void Error() { _error++; }

    template<typename Type>                 if_Serializable<Type, void> Save(Type& obj);            //serializable derived object
    template<typename Type>                 if_Serializable<Type, void> Load(Type& obj);

    template<typename Type>                 if_Serializable<Type, void> Save(Type*& obj);           //pointer to a serializable derived object
    template<typename Type>                 if_Serializable<Type, void> Load(Type*& obj);

    template<typename Type, size_t count>   if_Serializable<Type, void> Save(Type(&array)[count]);  //array[] of serializable derived object
    template<typename Type, size_t count>   if_Serializable<Type, void> Load(Type(&array)[count]);

    template<typename Type>                 if_PlainOldData<Type, void> Save(Type& data);           //POD types/struct (non-ints)
    template<typename Type>                 if_PlainOldData<Type, void> Load(Type& data);

    template<typename Type>                 if_PlainOldData<Type, void> Save(Type*& data);          //pointer to POD types/struct (non-ints)
    template<typename Type>                 if_PlainOldData<Type, void> Load(Type*& data);          //can not re-instanciate a derived object that is not type Type

    template<typename Type, size_t count>   if_PlainOldData<Type, void> Save(Type(&array)[count]);  //array[] of POD types/struct (non-ints)
    template<typename Type, size_t count>   if_PlainOldData<Type, void> Load(Type(&array)[count]);

    template<typename Type>                 if_IntegralType<Type, void> Save(Type& data);           //ints (using ByteOrder)
    template<typename Type>                 if_IntegralType<Type, void> Load(Type& data);

    template<typename Type, size_t count>   if_IntegralType<Type, void> Save(Type(&array)[count]);  //array[] of arithmetic
    template<typename Type, size_t count>   if_IntegralType<Type, void> Load(Type(&array)[count]);

    template<typename Type>                 void    Save(std::shared_ptr<Type>& ptr);               //shared_ptr<>
    template<typename Type>                 void    Load(std::shared_ptr<Type>& ptr);

    template<typename Type>                 void    Save(std::unique_ptr<Type>& ptr);               //unique_ptr<>
    template<typename Type>                 void    Load(std::unique_ptr<Type>& ptr);

    template<typename Type>                 void    Save(std::vector<Type>& vector);                 //vector<>
    template<typename Type>                 void    Load(std::vector<Type>& vector);

    template<typename Type>                 void    Save(std::list<Type>& list);                    //lists<>
    template<typename Type>                 void    Load(std::list<Type>& list);

    template<typename Type, size_t count>   void    Save(std::array<Type, count>& array);           //array<>
    template<typename Type, size_t count>   void    Load(std::array<Type, count>& array);

    template<typename Key, typename Value>  void    Save(std::map<Key, Value>& map);                //maps<>
    template<typename Key, typename Value>  void    Load(std::map<Key, Value>& map);

    template<size_t count>                  void    Save(char(&sz)[count]);                         //char[] (sz)
    template<size_t count>                  void    Load(char(&sz)[count]);

    void                                            Save(std::string& str);                         //C++ string
    void                                            Load(std::string& str);

    void                                            Save(void* pVoid, uint32 size);                 //blob
    void                                            Load(void* pVoid, uint32 size);

protected:
    void                                            SaveType(SerializableBase* pObj);               //objId/hash
    const TypeInfo*                                 LoadType();

    void                                            SaveDint(uint32 dint);                          //dynamic sized INT, 7 bits at a time (8th bit==stop-bit)
    uint32                                          LoadDint();

    int32                                           save(void* pData, uint32 size);                 //data source interface
    int32                                           load(void* pData, uint32 size);

private:
    IDataSource&    _source;
    Mode            _mode   = Unknown;
    uint32          _error  = 0;

    enum { BIG_PRIME = 2038074743, };
    void Hash(BYTE* pData, uint32 size) { while(size--) { _hash = (_hash + *pData++) * 0x0101; _hash ^= (_hash >> 3); } }
    uint32          _hash   = BIG_PRIME;

private:
    using ObjId  = uint32;
    using TypeId = uint32;

    enum { ID_NULL  = 1, ID_START = 2, };
    TypeId  _nextTypeId = ID_START;
    ObjId   _nextObjId  = ID_START;

    std::map<const TypeInfo*, TypeId>   _mapTypeId;
    std::map<void*, ObjId>              _mapObjId;

    std::map<TypeId, const TypeInfo*>   _mapIdType;
    std::map<ObjId, void*>              _mapIdObj;

    using shared_base_ptr = std::shared_ptr<SerializableBase>;
    std::map<void*, shared_base_ptr>    _mapObjShared;
};

}//namespace Serialize

#include "Archive.hh"

