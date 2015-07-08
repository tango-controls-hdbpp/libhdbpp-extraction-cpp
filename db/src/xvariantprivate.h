#ifndef XVARIANTPRIVATE_H
#define XVARIANTPRIVATE_H

#include <xvariant.h>

class XVariantPrivate
{
public:
    XVariantPrivate();

    XVariant::DataFormat mFormat;
    XVariant::DataType mType;
    XVariant::Writable mWritable;

    size_t mSize;

    unsigned int *mNullIndexes;
    unsigned int mNullValuesCount;
    unsigned int *mNullWIndexes;
    unsigned int mNullWValuesCount;

    char mTimestamp[TIMESTAMPLEN];

    char* mError;

    char mSource[SRCLEN];

    bool mIsValid;

    bool mIsNull;

    bool mIsWNull;

    void * val;

    void * w_val;

    short mQuality;

};

#endif // XVARIANTPRIVATE_H
