#pragma once

#include <map>
#include <memory>

#include "types.h"

namespace Serialize {

class SerializableBase;
class TypeInfo
{
    static auto& Map() { static std::map<HASH, TypeInfo*> map; return map; }

    using PFNCreate = SerializableBase * (*)();
public:
    TypeInfo(const size_t hash, PFNCreate pfnCreate)
        : _hash(HASH(hash)), _pfnCreate(pfnCreate) { Map()[_hash] = this; }
    SerializableBase*   Create() const  { return _pfnCreate(); }
    const HASH          Hash() const    { return _hash; };
    static TypeInfo*    Find(HASH hash) { return Map()[hash]; }

private:
    HASH        _hash;
    PFNCreate   _pfnCreate;
};

class Archive;
class SerializableBase
{
    friend class Archive;
protected:
    virtual ~SerializableBase() {};
    virtual void            Serialize(Archive& arc)   {}
    virtual const TypeInfo* GetTypeInfo() const       { return nullptr; }
    virtual bool            IsOfType(HASH hash) const { return false; }
public:
    using shared_ptr = std::shared_ptr<SerializableBase>;
};

template<class Type, class Base = SerializableBase>
class Serializable : public Base
{
    friend class Archive;
    static const TypeInfo           s_typeinfo;
    static SerializableBase*        Create()            { return new Type; }
    virtual const TypeInfo*         GetTypeInfo() const { return &s_typeinfo; }
protected:
    virtual bool IsOfType(HASH hash) const
    {
        if(s_typeinfo.Hash() != hash)
            return Base::IsOfType(hash);
        return true;
    }
protected:
    template<typename... Types>     Serializable(Types&& ...args) : Base(args ...) {}
};
template<class Type, class Base>
const TypeInfo Serializable<Type, Base>::s_typeinfo(typeid(Type).hash_code(), Create);

} //namespace Serialize
