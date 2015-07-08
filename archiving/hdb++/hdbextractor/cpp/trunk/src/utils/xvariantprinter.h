#ifndef XVARIANTPRINTER_H
#define XVARIANTPRINTER_H

#include <vector>
#include <xvariant.h>

class XVariantPrinter
{
public:
    XVariantPrinter();

    void print(const XVariant& xv, int maxVectorElements = -1);

    void printValueList(const std::vector<XVariant > &valuelist, int maxVectorElements = -1);
};

#endif // XVARIANTPRINTER_H
