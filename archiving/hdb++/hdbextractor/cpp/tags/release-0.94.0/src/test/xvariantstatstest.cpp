#include "xvariantstatstest.h"
#include <stdio.h>

XVariantStatsTest* XVariantStatsTest::_instance = NULL;

XVariantStatsTest *XVariantStatsTest::instance()
{
    if(!_instance)
        _instance = new XVariantStatsTest();
    return _instance;
}

void XVariantStatsTest::addVariant()
{
    mCount++;
    printf("\e[0;32m+ variant: count %d\e[0m\n", mCount);
}

void XVariantStatsTest::removeVariant()
{
    mCount--;
    printf("\e[0;35m- variant: count %d\e[0m\n", mCount);
}

int XVariantStatsTest::getCount() const
{
    return mCount;
}

XVariantStatsTest::XVariantStatsTest()
{
    mCount = 0;
}

