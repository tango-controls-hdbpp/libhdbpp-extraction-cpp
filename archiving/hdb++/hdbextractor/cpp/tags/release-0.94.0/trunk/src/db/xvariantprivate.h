#ifndef XVARIANTPRIVATE_H
#define XVARIANTPRIVATE_H

#include "xvariant.h"
#include "../sharedpointer.h"
#include "xvariantdatainfo.h"

class XVariantPrivate
{
public:
    XVariantPrivate();

    /* shared data info. Contains information which is common to
     * several XVariant data: source name, data type, format, writable.
     * This allows to save memory.
     *
     *
     * NOTE: this is allocated in XVariantPrivate constructor.
     */
    SharedPointer<XVariantDataInfo> dataInfo;

    size_t mSize;

    unsigned int *mNullIndexes;
    unsigned int mNullValuesCount;
    unsigned int *mNullWIndexes;
    unsigned int mNullWValuesCount;

    char mTimestamp[TIMESTAMPLEN];

    char* mError;

    bool mIsValid;

    bool mIsNull;

    bool mIsWNull;

    void * val;

    void * w_val;

    short mQuality;

};

#endif // XVARIANTPRIVATE_H
