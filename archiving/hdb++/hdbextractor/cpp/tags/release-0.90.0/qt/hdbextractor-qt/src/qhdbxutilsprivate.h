#ifndef QHDBXUTILSPRIVATE_H
#define QHDBXUTILSPRIVATE_H

#define ERRMSGLEN 256

class QHdbXUtilsPrivate
{
public:
    QHdbXUtilsPrivate();

    int nullDataCount;

    char errorMessage[ERRMSGLEN];
};

#endif // QHDBUTILSPRIVATE_H
