#ifndef XVARIANTLIST_H
#define XVARIANTLIST_H

#include <stdio.h>
#include <vector>
#include <xvariant.h>

/** \brief XVariantList represents a list of XVariant objects
 *
 */
class XVariantList
{
public:
    XVariantList();

    XVariantList(XVariant *xv);

    virtual ~XVariantList();

    XVariant *get(int i) const;

    void add(XVariant *xv);

    bool isEmpty() const;

    size_t size() const;

private:
    std::vector<XVariant *> _data;

};

#endif // XVARIANTLIST_H
