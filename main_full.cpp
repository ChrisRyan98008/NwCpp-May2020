
#include <iostream>
#include <thread>

#include "util.h"
#include "Serialize.h"

using namespace Serialize;

class Node4 : public Serializable<Node4>
{
    using Base = Serializable;
public:
    using shared_ptr = std::shared_ptr<Node4>;

    Node4(std::string str = "") : _name(str) {}
    void Serialize(Archive& arc)
    {
        Base::Serialize(arc);
        arc.Serialize(_name);
        std::cout << (arc.IsSave() ? "<" : ">") << _name;
        arc.Serialize(_vector);
    }

    template<typename ...Rest>
    void Connect(shared_ptr first, Rest...rest)
    {
        Connect(first);
        Connect(rest...);
    }
    void Connect(shared_ptr first)
    {
        _vector.push_back(first);
    }

    template<typename ...Args> static auto make_shared(Args...args) { return std::make_shared<Node4>(args...); }
protected:
    std::string             _name;
    std::vector<shared_ptr> _vector;
};

Node4::shared_ptr GenerateNode4Data()
{
    Node4::shared_ptr a = Node4::make_shared("a");
    Node4::shared_ptr b = Node4::make_shared("b");
    Node4::shared_ptr c = Node4::make_shared("c");
    Node4::shared_ptr d = Node4::make_shared("d");
    Node4::shared_ptr e = Node4::make_shared("e");

    a->Connect(b, c, d, e);
    b->Connect(a, c, d, e);
    c->Connect(a, b, d, e);
    d->Connect(a, b, c, e);
    e->Connect(a, b, c, d);

    return a;
}

void Save(IDataSource& sink)
{
    Archive arc(sink);

    Node4::shared_ptr pOut = GenerateNode4Data();
    std::cout << "\nstart saving data\n";
    arc << pOut;
    std::cout << "\ndone saving data\n";
}

void Load(IDataSource& source)
{
    Archive arc(source);

    Node4::shared_ptr pIn;
    std::cout << "\nstart loading data\n";
    arc >> pIn;
    std::cout << "\ndone loading data\n";
}

void Server()
{
    SocketSource server;
    Save(server);
}

void Client()
{
    SocketSource client("localhost");
    Load(client);
}

int main()
{
    {
        FileSource file("test.arc", FileSource::Save);
        Save(file);
    }

    {
        FileSource file("test.arc", FileSource::Load);
        Load(file);
    }

    {
        std::cout << "Client/Server: Start\n";
        std::thread server(Server);
        std::thread client(Client);
        server.join();
        client.join();
        std::cout << "Client/Server: Done\n\n";
    }

    return 0;
}

