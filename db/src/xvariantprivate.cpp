#include "xvariantprivate.h"

XVariantPrivate::XVariantPrivate()
{
    mError = NULL;
    val = NULL;
    w_val = NULL;

    mQuality = 0;

    mNullIndexes = NULL;
    mNullValuesCount = 0;
    mNullWIndexes = NULL;
    mNullWValuesCount = 0;

    mFormat = XVariant::FormatInvalid;
    mType = XVariant::TypeInvalid;
    mIsValid = false;
    mIsNull = true;
    mSize = 0;
}
