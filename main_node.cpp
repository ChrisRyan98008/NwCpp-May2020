
#include <iostream>

#include "util.h"
#include "Serialize.h"

using namespace Serialize;

class Node : public Serializable<Node>
{
    using Base = Serializable;

public:
    using shared_ptr = std::shared_ptr<Node>;

    Node(int32 data = 0, shared_ptr pLeft = nullptr, shared_ptr pRight = nullptr)
        : _pLeft(pLeft), _pRight(pRight), _data(data)
    {}
        
    void Serialize(Archive& arc)
    {
        Base::Serialize(arc);
        arc.Serialize(_data);
        arc.Serialize(_pLeft);
        arc.Serialize(_pRight);
    }

    template<typename ...Args> static auto make_shared(Args...args) { return std::make_shared<Node>(args...); }
protected:
    int32           _data;
    shared_ptr      _pLeft;
    shared_ptr      _pRight;

    template<class Type> friend class Util::DrawTree;

    virtual std::ostream& TextOut(std::ostream& os) const { return os; }
    friend std::ostream& operator<<(std::ostream& os, const Node& node) { return node.TextOut(os); }
};

Node::shared_ptr GenerateNodeTree()
{
    return  Node::make_shared(1,
                Node::make_shared(2,
                    Node::make_shared(4,
                        Node::make_shared(9),
                        nullptr),
                    Node::make_shared(5,
                        Node::make_shared(10,
                            nullptr,
                            Node::make_shared(8)),
                        Node::make_shared(11))),
                Node::make_shared(3,
                    Node::make_shared(6,
                        Node::make_shared(12),
                        Node::make_shared(13)),
                    Node::make_shared(7,
                        Node::make_shared(14),
                        Node::make_shared(15))));
}

int main()
{
    {
        std::shared_ptr<Node> pOut = GenerateNodeTree();

        std::cout << "Tree out:\n";
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

        std::cout << "Tree In:\n";
        std::cout << Util::DrawTree<decltype(pIn)>(pIn, true) << "\n";
    }

    return 0;
}


