#ifndef ROW_H
#define ROW_H

/** \brief Defines an interface for a row of the database. <em>Used internally</em>.
 *
 */
class Row
{
public:
    Row();

    virtual ~Row();

    virtual char *getField(int) const = 0;

    virtual int getFieldCount() const = 0;

    virtual bool isClosed() const = 0;

    virtual void close() = 0;

};

#endif // ROW_H
