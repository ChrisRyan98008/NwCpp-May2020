
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

class Node2 : public Serializable<Node2, Node>
{
    using Base = Serializable;
public:
    using shared_ptr = std::shared_ptr<Node2>;
    using unique_ptr = std::unique_ptr<Node2>;

    Node2(int32 data = 0, shared_ptr pLeft = nullptr, shared_ptr pRight = nullptr)
        : Base(pLeft, pRight), _data(data)
    {}

    void Serialize(Archive& arc)
    {
        Base::Serialize(arc);
        arc.Serialize(_data);
    }

    template<typename ...Args> static auto make_shared(Args...args) { return std::make_shared<Node2>(args...); }
    template<typename ...Args> static auto make_unique(Args...args) { return std::make_unique<Node2>(args...); }
protected:
    int32 _data;

    virtual std::ostream& TextOut(std::ostream& os) const
    {
        Base::TextOut(os);
        return os << _data;
    }
};

Node2::shared_ptr GenerateNode2Tree()
{
    return  Node2::make_shared(1,
                Node2::make_shared(2,
                    Node2::make_shared(4,
                        Node2::make_shared(9),
                        nullptr),
                    Node2::make_shared(5,
                        Node2::make_shared(10,
                            nullptr,
                            Node2::make_shared(8)),
                        Node2::make_shared(11))),
                Node2::make_shared(3,
                    Node2::make_shared(6,
                        Node2::make_shared(12),
                        Node2::make_shared(13)),
                    Node2::make_shared(7,
                        Node2::make_shared(14),
                        Node2::make_shared(15))));
}

int main()
{
    Node2::shared_ptr p2Tree = GenerateNode2Tree();

    std::cout << "Node2 Tree:\n";
    std::cout << Util::DrawTree<decltype(p2Tree)>(p2Tree, true) << "\n";

    return 0;
}

