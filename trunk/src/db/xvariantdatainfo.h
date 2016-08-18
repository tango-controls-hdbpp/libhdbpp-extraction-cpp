#ifndef XVARIANTDATAINFO_H
#define XVARIANTDATAINFO_H

#include <xvariant.h>

class XVariantDataInfo
{
public:
    XVariantDataInfo(const char *src,
                     XVariant::DataFormat df = XVariant::Scalar,
                     XVariant::DataType dt = XVariant::Double,
                     XVariant::Writable w = XVariant::RO);

    XVariantDataInfo();

    virtual ~XVariantDataInfo();

    XVariantDataInfo(const XVariantDataInfo &other);

    XVariantDataInfo & operator=(const XVariantDataInfo& other);

    void setSource(const char *src);

    void set(const char* src, XVariant::DataFormat df, XVariant::DataType dt, XVariant::Writable w);

    void set(const char* src, XVariant::DataFormat df, XVariant::DataType dt);

    XVariant::DataFormat format;
    XVariant::DataType type;
    XVariant::Writable writable;
    char * source;
};

#endif // XVARIANTDATAINFO_H
