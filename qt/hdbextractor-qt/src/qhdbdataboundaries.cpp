#include "qhdbdataboundaries.h"
#include <float.h>
#include <limits.h>

QHdbDataBoundaries::QHdbDataBoundaries()
{
    clear();
}

void QHdbDataBoundaries::clear()
{
    minX = 0.0;
    maxX = 0.0;
    minY = 0.0;
    maxY = 0.0;

    minTimestamp = 0.0;
    maxTimestamp = 0.0;

    xBoundsSet = false;
    yBoundsSet = false;
    tsBoundsSet = false;
}

void  QHdbDataBoundaries::updateX(double m) {
    if(m < minX || !xBoundsSet)
        minX = m;
    if(m > maxX || !xBoundsSet)
        maxX = m;
    if(!xBoundsSet)
        xBoundsSet = true;
}

void QHdbDataBoundaries:: updateY(double y)
{
    if(y > maxY || !yBoundsSet)
        maxY = y;
    if(y < minY || !yBoundsSet)
        minY = y;
    if(!yBoundsSet)
        yBoundsSet = true;
}

void  QHdbDataBoundaries::updateTimestamp(double m)
{
    if(m > maxTimestamp || !tsBoundsSet)
        maxTimestamp = m;
    if( m < minTimestamp || !tsBoundsSet)
        minTimestamp = m;
    if(!tsBoundsSet)
        tsBoundsSet = true;
}

