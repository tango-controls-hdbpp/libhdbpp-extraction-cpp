#include "xvariantlist.h"
#include "hdbxmacros.h"

XVariantList::XVariantList()
{

}

XVariantList::XVariantList(XVariant *xv)
{
    _data.push_back(xv);
}

XVariantList::~XVariantList()
{
    pinfo("~XVariantList size %d", (int) _data.size());
    for(size_t i = 0; i < _data.size(); i++)
    {
        if(_data[i])
            delete _data[i];
    }
    _data.clear();
}

void XVariantList::add(XVariant *xv)
{
    _data.push_back(xv);
}

bool XVariantList::isEmpty() const
{
    return _data.size() == 0;
}

size_t XVariantList::size() const
{
    return _data.size();
}

XVariant *XVariantList::get(int i) const
{
    if(i < (int) _data.size())
        return _data.at(i);
    return NULL;
}

