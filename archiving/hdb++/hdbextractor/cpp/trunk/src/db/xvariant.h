#ifndef XVARIANT_H
#define XVARIANT_H

#include <sys/types.h>
#include <vector>
#include <string>

#define TIMESTAMPLEN    32
#define SRCLEN          256

class XVariantPrivate;

/** \brief Historical database data container.
 *
 * XVariant is aimed at storing data fetched from an historical database.
 * It can contain several kinds of data.
 * The creation and filling of XVariant are carried out by the Hdbextractor
 * library. The end user is normally involved in data extraction for statistics,
 * display or whatever.
 */
class XVariant
{
public:

    /**
     * @brief The DataFormat enum lists the different data formats supported
     *        by XVariant
     *
     * Scalar: single value
     * Vector: an array (Tango names it spectrum) of data
     * Matrix: a 2 dimensional matrix of data (what Tango calls Image).
     *
     */
    enum DataFormat { FormatInvalid = -1, Scalar, Vector, Matrix };

    /**
     * @brief The DataType enum lists the different data types that can be memorized
     *        into XVariant
     */
    enum DataType { TypeInvalid = -1, Int, UInt, Double, Boolean, String };

    /**
     * @brief The Writable enum lists the read/write properties of stored data.
     *
     * This enum maps the Tango different writable properties: read only, read write,
     * write only, read with write.
     *
     */
    enum Writable { WritableInvalid = -1, RO, WO, RWW, RW };

    XVariant(const char * source, const char *timestamp,
             const char* strdataR, const char *strdataW,
             DataFormat df, DataType dt);

    XVariant(const char * source, const char *timestamp,
             const char* strdataR, DataFormat df,
             DataType dt, Writable wri);

    XVariant(const char * source, const char *timestamp,
             const size_t size, DataFormat df,
             DataType dt, Writable wri);

    XVariant(const XVariant &other);

    XVariant();

    XVariant & operator=(const XVariant& other);

    virtual ~XVariant();

    const char *getSource() const;

    DataFormat getFormat() const;

    DataType getType() const;

    Writable getWritable() const;

    size_t getSize() const;

    short int getQuality() const;

    std::vector<double> toDoubleVector(bool read = true) const;

    std::vector<long int> toLongIntVector(bool read = true) const;

    std::vector<unsigned long int> toULongIntVector(bool read = true) const;

    std::vector<bool> toBoolVector(bool read = true) const;

    double toDouble(bool read = true, bool *ok = NULL) const;

    bool isValid() const;

    bool isNull() const;

    bool isWNull() const;

    unsigned long int toULongInt(bool read = true, bool *ok = NULL) const;

    long int toLongInt(bool read = true, bool *ok = NULL) const;

    bool toBool(bool read = true, bool *ok = NULL) const;

    const char *getTimestamp() const;

    const char *getError() const;

    void setTimestamp(double tsmicro);

    void setTimestamp(const char* ts);

    void setTimestamp(const struct timeval* tv);

    void setQuality(const char *quality);

    void setError(const char *error);

    time_t getTime_tTimestamp() const;

    struct timeval getTimevalTimestamp() const;

    void add(const char* readval, size_t index);

    void add(const char* readval, const char* writeval, size_t index);

    std::string toString(bool read = true, bool *ok = NULL) const;

    std::vector<std::string> toStringVector() const;

    double *toDoubleP(bool read = true) const;

    long int *toLongIntP(bool read = true) const;

    unsigned long int *toULongIntP(bool read = true) const;

    bool *toBoolP(bool read = true) const;

    char **toCharP(bool read = true) const;

    std::string convertToString(bool read = true, bool *ok = NULL);

    unsigned int getNullValuesCount() const;

    unsigned int *getNullValueIndexes() const;

    unsigned int getNullWValuesCount() const;

    unsigned int *getNullWValueIndexes() const;

private:

    void build_from(const XVariant& other);

    void cleanup();

    void parse(const char *s);

    void parse(const char *sr, const char *sw);

    void init_data();

    void init_data(size_t size);

    void init_common(const char *source, const char *timestamp, DataFormat df, DataType dt);

    void delete_rdata();

    void delete_wdata();

    void mMakeError(int errnum);

    void mMakeError(const char *msg);

private:

    XVariantPrivate *d;
};

#endif // XVARIANT_H
