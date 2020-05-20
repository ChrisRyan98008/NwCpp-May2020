
#include <iostream>

#include "util.h"
#include "Serialize.h"

using namespace Serialize;

class Node : public Serializable<Node>
{
    using Base = Serializable;

public:
    using shared_ptr = std::shared_ptr<Node>;

    Node(shared_ptr pLeft = nullptr, shared_ptr pRight = nullptr)
        : _pLeft(pLeft), _pRight(pRight)
    {}

    void Serialize(Archive& arc)
    {
        Base::Serialize(arc);
        arc.Serialize(_pLeft);
        arc.Serialize(_pRight);
    }

    template<typename ...Args> static auto make_shared(Args...args) { return std::make_shared<Node>(args...); }
protected:
    shared_ptr      _pLeft;
    shared_ptr      _pRight;

    template<class Type> friend class Util::DrawTree;

    virtual std::ostream& TextOut(std::ostream& os) const { return os; }
    friend std::ostream& operator<<(std::ostream& os, const Node& node) { return node.TextOut(os); }
};

class AllTypes : public Serializable<AllTypes, Node>
{
    using Base = Serializable;

    struct PodStruct { int32 a; char b; };
    class SerClass : public Serializable<SerClass>
    {
        std::string _str;
    public:
        using shared_ptr = std::shared_ptr<SerClass>;
        using unique_ptr = std::unique_ptr<SerClass>;

        SerClass(std::string str = "str") : _str(str) {}
        void Serialize(Archive& arc) { arc.Serialize(_str); }

        template<typename ...Args> static auto make_shared(Args...args) { return std::make_shared<SerClass>(args...); }
        template<typename ...Args> static auto make_unique(Args...args) { return std::make_unique<SerClass>(args...); }
    };

public:
    AllTypes(char cData = {}, int32 iData = {}, shared_ptr pLeft = {}, shared_ptr pRight = {})
        : Base(pLeft, pRight), _cData(cData), _iData(iData)
    {
        _pSerClass = new SerClass("raw");
        _pUniqueSerClass = SerClass::make_unique("unique");
        _pSharedSerClass = SerClass::make_shared("shared");

        _pPodStruct = new PodStruct;
        _pUniquePodStruct = std::make_unique<PodStruct>();
        _pSharedPodStruct = std::make_shared<PodStruct>();

        Util::Rand rand;
        for(int i = rand.get(25, 15); i != 0; i--)
        {
            switch(rand.get(4))  //randomly populate collections
            {
            case 0: _stdArrayOfInts[rand.get(4)] = rand.get(9); break;
            case 1: _stdMapIntToInt[rand.get(9)] = rand.get(9); break;
            case 2: _stdListOfInts.push_back(rand.get(9));    break;
            case 3: _stdVectorOfInts.push_back(rand.get(9));    break;
            }
        }
    }

    ~AllTypes()
    {
        delete _pSerClass;
        delete _pPodStruct;
    }

    void Serialize(Archive& arc)
    {
        Base::Serialize(arc);

        if(arc.IsSave())
        {
            arc << _cData          << _iData           << _double          << _aChars     << _aInts            << _aDoubles;
            arc << _serClass       << _pSerClass       << _pSerClass_NULL  << _aSerClass  << _pUniqueSerClass  << _pSharedSerClass;
            arc << _podStruct      << _pPodStruct      << _pPodStruct_NULL << _aPodStruct << _pUniquePodStruct << _pSharedPodStruct;
            arc << _stdArrayOfInts << _stdVectorOfInts << _stdListOfInts   << _stdMapIntToInt;
        }
        else
        {
            arc >> _cData          >> _iData           >> _double          >> _aChars     >> _aInts            >> _aDoubles;
            arc >> _serClass       >> _pSerClass       >> _pSerClass_NULL  >> _aSerClass  >> _pUniqueSerClass  >> _pSharedSerClass;
            arc >> _podStruct      >> _pPodStruct      >> _pPodStruct_NULL >> _aPodStruct >> _pUniquePodStruct >> _pSharedPodStruct;
            arc >> _stdArrayOfInts >> _stdVectorOfInts >> _stdListOfInts   >> _stdMapIntToInt;
        }
    }

    template<typename ...Args> static auto make_shared(Args...args) { return std::make_shared<AllTypes>(args...); }
    using shared_ptr = std::shared_ptr<AllTypes>;
protected:
    virtual std::ostream& TextOut(std::ostream& os) const
    {
        Base::TextOut(os);
        os << _cData << "," << _iData;
        os << " A:(" << _stdArrayOfInts.size()  << "){";     for(auto& item : _stdArrayOfInts)  os << item << ",";                                     os << "},";
        os << " V:(" << _stdVectorOfInts.size() << "){";     for(auto& item : _stdVectorOfInts) os << item << ",";                                     os << "},";
        os << " L:(" << _stdListOfInts.size()   << "){";     for(auto& item : _stdListOfInts)   os << item << "->";                                    os << "},";
        os << " M:(" << _stdMapIntToInt.size()  << "){";     for(auto& pair : _stdMapIntToInt)  os << "{" << pair.first << "," << pair.second << "},"; os << "}";
        return os;
    }

    char                        _cData = 'a';
    int32                       _iData = 2;
    double                      _double = 2.2;
    char                        _aChars[20] = "qwertyuiop";
    int32                       _aInts[10] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    double                      _aDoubles[2] = {3.3,4.4};

    SerClass                    _serClass = {"member"};
    SerClass*                   _pSerClass = nullptr;
    SerClass*                   _pSerClass_NULL = nullptr;
    SerClass                    _aSerClass[3];
    SerClass::unique_ptr        _pUniqueSerClass;
    SerClass::shared_ptr        _pSharedSerClass;

    PodStruct                   _podStruct = {};
    PodStruct*                  _pPodStruct = nullptr;
    PodStruct*                  _pPodStruct_NULL = nullptr;
    PodStruct                   _aPodStruct[4] = {};
    std::unique_ptr<PodStruct>  _pUniquePodStruct;
    std::shared_ptr<PodStruct>  _pSharedPodStruct;

    std::array<int32, 5>        _stdArrayOfInts = {};
    std::vector<int32>          _stdVectorOfInts;
    std::list<int32>            _stdListOfInts;
    std::map<int32, int32>      _stdMapIntToInt;
};

AllTypes::shared_ptr GenerateAllTypesTree()
{
    return  AllTypes::make_shared('q', 60,
                AllTypes::make_shared('w', 55,
                    AllTypes::make_shared('e', 50),
                    AllTypes::make_shared('r', 45,
                        nullptr,
                        AllTypes::make_shared('t', 40))),
                AllTypes::make_shared('u', 35,
                    AllTypes::make_shared('i', 30,
                        AllTypes::make_shared('o', 25),
                        nullptr),
                    AllTypes::make_shared('p', 20)));
}

int main()
{
    {
        std::shared_ptr<Node> pOut = GenerateAllTypesTree();

        std::cout << "AllTypes Tree out:\n";
        std::cout << Util::DrawTree<decltype(pOut)>(pOut, true) << "\n";

        FileSource file("test.arc", FileSource::Save);
        Archive arc(file);
        arc << pOut;
    }

    {
        std::shared_ptr<Node> pIn;

        FileSource file("test.arc", FileSource::Load);
        Archive arc(file);
        arc >> pIn;

        std::cout << "AllTypes Tree In:\n";
        std::cout << Util::DrawTree<decltype(pIn)>(pIn, true) << "\n";
    }

    return 0;
}

