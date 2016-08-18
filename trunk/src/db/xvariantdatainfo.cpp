#include "xvariantdatainfo.h"
#include <string.h>

XVariantDataInfo::~XVariantDataInfo()
{
    if(source)
        delete source;
}

XVariantDataInfo::XVariantDataInfo(const XVariantDataInfo &other)
{
    int srclen = strlen(other.source);
    source = new char[srclen + 1];
    strncpy(source, other.source, srclen + 1);
    format = other.format;
    type = other.type;
    writable = other.writable;
}

XVariantDataInfo &XVariantDataInfo::operator=(const XVariantDataInfo &other)
{
    if(source)
        delete source;
    int srclen = strlen(other.source);
    source = new char[srclen + 1];
    strncpy(source, other.source, srclen + 1);
    this->format = other.format;
    this->type = other.type;
    this->writable = other.writable;
    return *this;
}

XVariantDataInfo::XVariantDataInfo(const char *src,
                                   XVariant::DataFormat df,
                                   XVariant::DataType dt,
                                   XVariant::Writable w)
{
    /* optimize memory, allocating a string with the required length */
    int srclen = strlen(src);
    source = new char[srclen + 1];
    strncpy(source, src, srclen + 1);
    format = df;
    type = dt;
    writable = w;
}

XVariantDataInfo::XVariantDataInfo()
{
    source = NULL;
    format = XVariant::FormatInvalid;
    type = XVariant::TypeInvalid;
    writable = XVariant::WritableInvalid;
}

void XVariantDataInfo::setSource(const char *src)
{
    if(source)
        delete source;
    int srclen = strlen(src);
    source = new char[srclen + 1];
    strncpy(source, src, srclen + 1);
}

void XVariantDataInfo::set(const char *src,
                           XVariant::DataFormat df,
                           XVariant::DataType dt,
                           XVariant::Writable w)
{
    setSource(src);
    format = df;
    type = dt;
    writable = w;
}

void XVariantDataInfo::set(const char *src, XVariant::DataFormat df, XVariant::DataType dt)
{
    setSource(src);
    format = df;
    type = dt;
}
