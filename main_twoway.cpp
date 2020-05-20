
#include <thread>
#include <iostream>

#include "util.h"
#include "Serialize.h"

using namespace Serialize;

class Node3 : public Serializable<Node3>
{
    using Base = Serializable;
public:
    using shared_ptr = std::shared_ptr<Node3>;

    Node3(std::string name = "", int value = 0) : _value(value), _name(name) {}

    void Insert(shared_ptr pNew)
    {
        if(*pNew < *this)
        {
            if(_pLeft) _pLeft->Insert(pNew);
            else        _pLeft = pNew;
        }
        else
        {
            if(_pRight) _pRight->Insert(pNew);
            else         _pRight = pNew;
        }
    }

    void Serialize(Archive& arc)
    {
        Base::Serialize(arc);
        arc.Serialize(_name);
        arc.Serialize(_value);
        arc.Serialize(_pLeft);
        arc.Serialize(_pRight);
    }

    template<class Type> friend class Util::DrawTree;
    bool operator<(const Node3& rhs) { return _value < rhs._value; }
    template<typename ...Args> static auto make_shared(Args...args) { return std::make_shared<Node3>(args...); }
protected:
    friend std::ostream& operator<<(std::ostream& os, const Node3& node) { return node.TextOut(os); }
    virtual std::ostream& TextOut(std::ostream& os) const
    {
        return os << _name << ":" << _value;
    }

    int32       _value;
    std::string _name;

    shared_ptr  _pLeft;
    shared_ptr  _pRight;
};

void Server()
{
    Util::Rand rand;
    std::cout << "Two Way Server: starting\n";
    SocketSource server;
    Archive arc(server);

    Node3::shared_ptr pTree = Node3::make_shared("Root", 5);
    int count = 5;
    while(count--)
    {
        std::cout << "<<S";
        arc << pTree;
        pTree = nullptr;

        std::cout << ">>S";
        arc >> pTree;
        pTree->Insert(Node3::make_shared("Server", rand.get(10)));
    }
    std::cout << "\nServer:\n" << Util::DrawTree<decltype(pTree)>(pTree, true) << "\n";
    std::cout << "Two Way Server: exiting\n";
}

void Client()
{
    std::cout << "Two Way Client: starting\n";
    SocketSource client("localhost");
    Archive arc(client);
    Util::Rand rand;
    int count = 5;
    while(count--)
    {
        Node3::shared_ptr pTree;
        std::cout << ">>C";
        arc >> pTree;

        pTree->Insert(Node3::make_shared("Client", rand.get(10)));

        std::cout << "<<C";
        arc << pTree;
    }
    std::cout << "\nTwo Way Client: exiting\n";
}


int main()
{
    std::cout << "Client/Server Synchronous Reversible-Two Way Archive: Start\n";

    std::thread server(Server);
    std::thread client(Client);
    server.join();
    client.join();

    std::cout << "Client/Server Synchronous Reversible-Two Way Archive: Done\n\n";
}


