#ifndef QHDBDATABOUNDARIES_H
#define QHDBDATABOUNDARIES_H

class QHdbDataBoundaries
{
public:
    QHdbDataBoundaries();

    void clear();

    double minTimestamp, maxTimestamp;
    double minX, maxX, minY, maxY;

    void updateX(double m);

    void updateY(double y);

    void updateTimestamp(double m);

    bool xBoundsSet, yBoundsSet, tsBoundsSet;
};

#endif // QHDBDATABOUNDARIES_H
