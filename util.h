#pragma once

#include <memory>
#include <iostream>

std::ostream& operator << (std::ostream& os, const std::string& str)
{
    return os << str.c_str();
}

#include <random>

namespace Util
{

class Rand
{
    std::random_device _rand;
public:
    int get(int max = 99, int min = 0)
    {
        return _rand() % (max + 1 - min) + min;
    }
};


template<typename Type>
class DrawTree
{
public:
    DrawTree(const Type root, bool bNulls = false) :_root(root), _bNulls(bNulls) {}
    friend std::ostream& operator << (std::ostream& os, const DrawTree& tree)
    {
        return tree.Draw(os, tree._root, tree._bNulls);
    }

protected:
    const Type _root;
    bool _bNulls;

    template<typename Type2>
    std::ostream& Draw(std::ostream& os, Type2 node, bool bNulls, std::string indent = std::string()) const
    {
        os << *node << "\n";
        if(node->_pLeft)
        {
            os << indent << (node->_pRight || bNulls ? "+-->" : "\\-->");
            Draw(os, node->_pLeft, bNulls, indent + (node->_pRight || bNulls ? "|   " : "    "));
            if(!node->_pRight && bNulls)
                os << indent << "\\--x\n";
        }
        if(node->_pRight)
        {
            if(!node->_pLeft && bNulls)
                os << indent << "+--x\n";
            os << indent << "\\-->";
            Draw(os, node->_pRight, bNulls, indent + "    ");
        }
        return os;
    };
};

}
