#ifndef XVARIANTSTATSTEST_H
#define XVARIANTSTATSTEST_H


class XVariantStatsTest
{
public:

    static XVariantStatsTest *instance();

    void addVariant();

    void removeVariant();

    int getCount() const;

private:

    XVariantStatsTest();

    static XVariantStatsTest *_instance;

    int mCount;
};

#endif // XVARIANTSTATSTEST_H
