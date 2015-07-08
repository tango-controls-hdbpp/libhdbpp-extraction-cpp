#include "xvariant.h"
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <time.h>
#include <math.h> /* round */
#include <string.h> /* strerror */
#include "../hdbxmacros.h"
#include "datetimeutils.h"
#include "xvariantprivate.h"
#include "xvariantdatainfo.h"
#include "../test/xvariantstatstest.h"

void XVariant::cleanup()
{
    if(d->mSize > 0 && d->dataInfo->type == String && d->val != NULL)
    {
        char **ssi = (char **) d->val;
        for(size_t i = 0; i < d->mSize; i++)
        {
            char *si = (char *) ssi[i];
            delete si;
        }
    }
    delete_rdata();


    if(d->mSize > 0 && d->dataInfo->type == String && d->w_val != NULL)
    {
        char **ssi = (char **) d->val;
        for(size_t i = 0; i < d->mSize; i++)
        {
            char *si = (char *) ssi[i];
            delete si;
        }
    }
    delete_wdata();

    if(d->mError)
        delete d->mError;

    delete d;
    d = NULL;
}

XVariant::~XVariant()
{
    //printf("~XVariant destructor: calling cleanup %p\n", this);
    cleanup();
}

/** \brief The XVariant object is the Hdbextractor data container.
 *
 * This class allows storing every kind of data pertaining to an historical database.
 * It can contain, in turn, scalar values of different types (boolean, double, integer, string...)
 * and the bi dimensional counterpart, named Vector.
 *
 * You normally do not need to know how XVariant is created. From the user point of view, XVariant is
 * the data container from which you want to extract data.
 *
 * You usually will retrieve a list of XVariant objects. Each one of them represents the data contained
 * in a row of the historical database. XVariant has a timestamp marking the date and time data was saved,
 * a bunch of fields aimed at defining the type of data stored (Scalar or Vector, Double or Int or String,
 * read only or read write) and the size of the data stored. In the case of Vector, the data size indicates the
 * number of elements of the array. In the case of Scalar format, the size will always be 1.
 *
 * A validity flag is also used to indicate that the data memorized by XVariant has been correctly detected
 * when XVariant was constructed from the data stored into the database.
 *
 * A null flag indicates whether the value stored into the database is a NULL value or not.
 * NULL values are legitimate values and  they are normally stored by the archiver when a read error
 * occurs. In principle, if the attribute is read/write, both read and write values are NULL or not NULL,
 * due to an unsuccessful or successful read. The isNull and isWNull methods indicate this condition, for
 * the read and write data independently.
 *
 * \note
 * For a read only source, isWNull always returns true.
 *
 * \note
 * A NULL value into the database does not imply isValid returns false.
 * isValid returns false if either a parse error occurred or the data type, format or writable are invalid.
 *
 * @param source domain/family/member/attribute_name
 * @param timestam a timestamp in the format "2014-07-10 10:00:00"
 * @param strdataR a string representing the data read from the database
 * @param strdataW a string representing the data read from the database
 *        (write value of a read write quantity)
 * @param df the data format
 * @param dt the data type
 *
 * \note The writable property is assumed to be read only
 *
 * @see getFormat
 * @see getType
 * @see isValid
 * @see getWritable
 * @see toDouble
 * @see toDoubleVector
 */
XVariant::XVariant(const char * source, const char *timestamp, const char *strdata, DataFormat df, DataType dt, Writable wri)
{
    d = new XVariantPrivate(); /* allocates XVariantDataInfo */
    init_common(source, timestamp, df, dt);
    init_data();
    d->dataInfo->writable = wri;
    parse(strdata); /* at the end, after setting up other fields */
}

/** \brief The constructor for read write data.
 *
 * @param source domain/family/member/attribute_name
 * @param timestam a timestamp in the format "2014-07-10 10:00:00"
 * @param strdataR a string representing the data read from the database
 * @param strdataW a string representing the data read from the database
 *        (write value of a read write quantity)
 * @param df the data format
 * @param dt the data type
 *
 * \note The writable property is assumed to be read only
 *
 */
XVariant::XVariant(const char *source, const char *timestamp, const char *strdataR, const char *strdataW, DataFormat df, DataType dt)
{
    d = new XVariantPrivate(); /* allocates XVariantDataInfo */
    init_common(source, timestamp, df, dt);
    init_data();
    d->dataInfo->writable = RW;
    parse(strdataR, strdataW); /* at the end, after setting up other fields */
}

XVariant::XVariant(const char * source,
                   const char *timestamp,
                   const size_t size, DataFormat df,
                   DataType dt, Writable wri)
{
    d = new XVariantPrivate(); /* allocates XVariantDataInfo */
    d->dataInfo->writable = wri;
    init_common(source, timestamp, df, dt);
    init_data(size);
}

/** \brief Creates an empty (invalid) XVariant
 *
 * The XVariant created is invalid.
 */
XVariant::XVariant()
{
    d = new XVariantPrivate();  /* allocates XVariantDataInfo */
    init_data();
    d->dataInfo->format = FormatInvalid;
    d->dataInfo->writable = WritableInvalid;
    d->dataInfo->type = TypeInvalid;
}

/** \brief copy constructor
 *
 * Create a new variant initialized from the values of the other parameter
 *
 * @param other the XVariant to be cloned.
 *
 */
XVariant::XVariant(const XVariant &other)
{
    build_from(other);
}

XVariant & XVariant::operator=(const XVariant& other)
{
    cleanup();
    build_from(other);
    return *this;
}
void XVariant::build_from(const XVariant& other)
{
    d = new XVariantPrivate();
    init_data(); /* NULLify all pointers to data */

    d->dataInfo->writable = other.getWritable();
    d->dataInfo->format  = other.getFormat();
    d->dataInfo->type = other.getType();
    d->mSize = other.getSize();
    d->mIsValid = other.isValid();
    d->mIsNull = other.isNull();
    d->mIsWNull = other.isWNull();
    d->mQuality = other.getQuality();

    //    printf("\e[0;36mXVariant %p copy from %p this->d: %p: format %d wri %d size %ld\e[0m \n", this, &other, d,
    //           d->dataInfo->dataFormat, d->dataInfo->writable, d->mSize);

    if(d->dataInfo->writable == WritableInvalid || d->dataInfo->type == TypeInvalid ||
            d->dataInfo->format == FormatInvalid)
    {
        printf("\e[1;31mXVariant %p d %p:  FORMAT OR WRI OR TYPE INVALID in other %p\e[0m\n",
               this, d, &other);
        return;
    }
    d->dataInfo->setSource(other.getSource());
    // strncpy(d->mSource, other.getSource(), SRCLEN);
    strncpy(d->mTimestamp, other.getTimestamp(), TIMESTAMPLEN);
    mMakeError(other.getError());

    if(d->dataInfo->writable != WO)
    {
        if(d->dataInfo->type == XVariant::Double)
        {
            double *vd = new double[d->mSize];
            for(size_t i = 0; i < d->mSize; i++)
                vd[i] =  other.toDoubleP()[i];
            d->val = vd;
        }
        else if(d->dataInfo->type == XVariant::Int)
        {
            long int *vi =  new long int[d->mSize];

            for(size_t i = 0; i < d->mSize; i++)
                vi[i] = other.toLongIntP()[i];
            d->val = vi;
        }
        else if(d->dataInfo->type == XVariant::UInt)
        {
            unsigned long int *vi =  new unsigned long int[d->mSize];

            for(size_t i = 0; i < d->mSize; i++)
                vi[i] = other.toULongIntP()[i];
            d->val = vi;
        }
        else if(d->dataInfo->type == XVariant::Boolean)
        {
            bool *vb = new bool[d->mSize];
            for(size_t i = 0; i < d->mSize; i++)
                vb[i] = other.toBoolP()[i];
            d->val = vb;
        }
    }

    /* write part */
    if((d->dataInfo->writable == XVariant::RW || d->dataInfo->writable == XVariant::WO) && !d->mIsWNull)
    {
        if(d->dataInfo->type == XVariant::Double)
        {
            double *vd = new double[d->mSize];
            for(size_t i = 0; i < d->mSize; i++)
                vd[i] =  other.toDoubleP(false)[i];
            d->w_val = vd;
        }
        else if(d->dataInfo->type == XVariant::Int)
        {
            long int *vi =  new long int[d->mSize];
            for(size_t i = 0; i < d->mSize; i++)
                vi[i] = other.toLongIntP(false)[i];
            d->w_val = vi;
        }
        else if(d->dataInfo->type == XVariant::UInt)
        {
            unsigned long int *vi =  new unsigned long int[d->mSize];
            for(size_t i = 0; i < d->mSize; i++)
                vi[i] = other.toULongIntP(false)[i];
            d->w_val = vi;
        }
        else if(d->dataInfo->type == XVariant::Boolean)
        {
            bool *vb = new bool[d->mSize];
            for(size_t i = 0; i < d->mSize; i++)
                vb[i] = other.toBoolP(false)[i];
            d->w_val = vb;
        }
    }
}

/** \brief Returns the source name (tango full attribute name)
 */
const char* XVariant::getSource() const
{
    return d->dataInfo->source;
}

/** \brief Query the format of the data stored in the XVariant
 *
 * @return the DataFormat (XVariant::Vector, XVariant::Scalar, XVariant::Matrix)
 */
XVariant::DataFormat XVariant::getFormat() const
{
    return d->dataInfo->format;
}

/** \brief Returns the DataType stored by XVariant
 *
 */
XVariant::DataType XVariant::getType() const
{
    return d->dataInfo->type;
}

/** \brief Returns the Writable property, which tells if the attribute is read only, read write,
 *         write only and so on.
 *
 * @see XVariant::Writable
 */
XVariant::Writable XVariant::getWritable() const
{
    return d->dataInfo->writable;
}

/** \brief Returns whether the data stored by XVariant is valid
 *
 * @return true the data contained by XVariant is valid
 * @return false the data contained by XVariant is not valid (see getError)
 *
 * @see getError
 * @see isNull
 *
 * \note
 * isValid returns true if data is NULL. In other words, NULL values in the database
 * are deemed valid.
 *
 */
bool XVariant::isValid() const
{
    return d->mIsValid;
}

/** \brief Returns whether the read part of the data stored by XVariant is NULL or not
 *
 * @return true the data contained by XVariant is NULL
 * @return false the data contained by XVariant is not NULL (see isValid)
 *
 * \note
 * If isNull is true, then isValid will be true also, because a NULL XVariant represents
 * a NULL value stored into the database, which is perfectly legit.
 *
 * \note
 * In principle, read or write data parts can be NULL independently of each other,
 * even if it is unlikely in the context of the historical database archiver,
 * where NULL values in data indicate a read error. So in everyday's life both read and write data
 * are NULL or not NULL.
 *
 * @see getError
 * @see isValid
 * @see isWNull
 * @see setQuality
 *
 */
bool XVariant::isNull() const
{
    return d->mIsNull;
}

/** \brief Returns whether the write part of the data stored by XVariant is NULL or not
 *
 * @return true the write part of the data contained by XVariant is NULL
 * @return false the write part of the data contained by XVariant is not NULL (see isValid)
 *
 * \note
 * If isNull is true, then isValid will be true also, because a NULL XVariant represents
 * a NULL value stored into the database, which is perfectly legit.
 *
 * \note
 * In principle, read or write data parts can be NULL independently of each other,
 * even if it is unlikely in the context of the historical database archiver,
 * where NULL values in data indicate a read error. So in everyday's life both read and write data
 * are NULL or not NULL.
 *
 * @see getError
 * @see isValid
 * @see isNull
 * @see setQuality
 */
bool XVariant::isWNull() const
{
    return d->mIsWNull;
}

/** \brief Returns the description of the error reported by the last operation.
 *
 * \note All errors occurred before the last operation are discarded.
 */
const char *XVariant::getError() const
{
    return d->mError;
}

/** \brief Returns the size of the data stored by the XVariant
 *
 * @return the size of the data stored by the XVariant. This method is useful to
 *         know the size of a vector of data, in case XVariant encloses spectrum
 *         Tango attributes.
 */
size_t XVariant::getSize() const
{
    return d->mSize;
}

unsigned int XVariant::getNullValuesCount() const
{
    return d->mNullValuesCount;
}

unsigned int *XVariant::getNullValueIndexes() const
{
    return d->mNullIndexes;
}

unsigned int XVariant::getNullWValuesCount() const
{
    return d->mNullWValuesCount;
}

unsigned int *XVariant::getNullWValueIndexes() const
{
    return d->mNullWIndexes;
}

/** \brief Adds an element to a Vector at a certain position. The element to be added is
 *         passed as a string and is converted to the proper data type stored by the XVariant.
 *
 * @param readval the read value to be inserted at index position
 * @param index the index where readval has to be inserted. It must be less than the available
 *        space allocated for the XVariant. This method does not extend the XVariant size.
 *
 * If readval is null for the given index, then the stored value at position index depends on the
 * data type:
 * \list
 * Double: a NaN is stored;
 * Int: LONG_MAX is stored;
 * UInt: ULONG_MAX is stored;
 * Bool: false is stored.
 * String: an empty string is stored.
 *
 * Double type only supports NaN. You can look for indexes storing a NULL value with getNullValueIndexes.
 * @see getNullValuesCount
 * @see getNullValueIndexes
 *
 */
void XVariant::add(const char* readval, size_t index)
{
    char *endptr;
    d->mIsValid = true;
    errno = 0; /* To distinguish success/failure after call */
    if(readval != NULL && (d->dataInfo->writable == XVariant::RO) && index < d->mSize)
        d->mIsNull = false;
    if(readval != NULL && (d->dataInfo->writable == XVariant::WO) && index < d->mSize)
        d->mIsWNull = false;

    if(readval == NULL && index < d->mSize)
    {
        d->mNullIndexes = (unsigned int * ) realloc(d->mNullIndexes, sizeof(unsigned int) * d->mNullValuesCount + 1);
        d->mNullIndexes[d->mNullValuesCount] = index;
        d->mNullValuesCount++;
    }

    if((d->dataInfo->writable == XVariant::RO || d->dataInfo->writable == XVariant::WO) && index < d->mSize)
    {
        if(d->dataInfo->type == Double)
        {
            double val;
            if(readval == NULL)
                val = nan("Nan");
            else
                val = strtod(readval, &endptr);

            if(errno != 0 && val == 0)
                mMakeError(errno);
            else
            {
                double *dval;
                if(d->dataInfo->writable == XVariant::RO )
                    dval = (double *) d->val;
                else
                    dval = (double *) d->w_val;
                dval[index] = val;
            }
        }
        else if(d->dataInfo->type == Int)
        {
            long int val;
            if(readval == NULL)
                val = LONG_MAX;
            else
                val = strtol(readval, &endptr, 10);
            if(errno != 0 && val == 0)
                mMakeError(errno);
            else
            {
                long int *lval;
                if(d->dataInfo->writable == XVariant::RO )
                    lval = (long int *) d->val;
                else
                    lval = (long int *) d->w_val;
                lval[index] = val;
            }
        }
        else if(d->dataInfo->type == UInt)
        {
            long unsigned int val;
            if(readval == NULL)
                val = ULONG_MAX;
            else
                val = strtoul(readval, &endptr, 10);

            if(errno != 0 && val == 0)
                mMakeError(errno);
            else
            {
                long unsigned  int *lval;
                if(d->dataInfo->writable == XVariant::RO )
                    lval = (long unsigned  int *) d->val;
                else
                    lval = (long unsigned  int *) d->w_val;
                lval[index] = val;
            }
        }
        else if(d->dataInfo->type == Boolean)
        {
            bool booval;
            if(readval == NULL)
                booval = false;
            else
                booval = (strcasecmp(readval, "true") == 0 || strtol(readval, &endptr, 10) != 0);

            if(errno != 0)
                mMakeError(errno);
            else
            {
                bool *bval;
                if(d->dataInfo->writable == XVariant::RO )
                    bval = (bool *) d->val;
                else
                    bval = (bool *) d->w_val;
                bval[index] = booval;
            }
        }
    }
    if(errno != 0)
        d->mIsValid = false;
}

/** \brief Adds a read write value pair to a Vector at a certain position. The elements to be added are
 *         passed as a string and they are converted to the proper data type stored by the XVariant.
 *
 * @param readval the read value to be inserted at index position
 * @param writeval the write value to be inserted at index position
 * @param index the index where readval has to be inserted. It must be less than the available
 *        space allocated for the XVariant. This method does not extend the XVariant size.
 *
 * If either readval or write val is null for the given index, then the stored value at position index depends on the
 * data type:
 * \list
 * Double: a NaN is stored;
 * Int: LONG_MAX is stored;
 * UInt: ULONG_MAX is stored;
 * Bool: false is stored.
 * String: an empty string is stored.
 *
 * Double type only supports NaN. You can look for indexes storing a NULL value with getNullValueIndexes.
 * @see getNullValuesCount
 * @see getNullValueIndexes
 *
 */
void XVariant::add(const char* readval, const char* writeval, size_t index)
{
    d->mIsValid = true;
    errno = 0;

    if(readval != NULL && index < d->mSize)
        d->mIsNull = false;
    if(writeval != NULL && index < d->mSize)
        d->mIsWNull = false;

    if(readval == NULL && index < d->mSize)
    {
        d->mNullIndexes = (unsigned int * ) realloc(d->mNullIndexes, sizeof(unsigned int) * d->mNullValuesCount + 1);
        d->mNullIndexes[d->mNullValuesCount] = index;
        d->mNullValuesCount++;
    }

    if(writeval == NULL && index < d->mSize)
    {
        d->mNullWIndexes = (unsigned int * ) realloc(d->mNullWIndexes, sizeof(unsigned int) * d->mNullWValuesCount + 1);
        d->mNullWIndexes[d->mNullWValuesCount] = index;
        d->mNullWValuesCount++;
    }

    if((d->dataInfo->writable == XVariant::RW ) && index < d->mSize)
    {
        char *endptr = NULL;

        printf("RW: index %d readval %s write val %s\n", index, readval, writeval);
        if(d->dataInfo->type == Double)
        {
            long double val;
            if(readval == NULL)
                val = nan("NaN");
            else
            {
                val = strtold(readval, &endptr);
                printf("\e[1;33mstrtozabbulang %s: -> got %Le\e[0m\n", readval, val);
            }

            double wval;
            if(writeval == NULL)
                wval = nan("NaN");
            else
                wval = strtold(writeval, &endptr);
            if(errno != 0 && (val == 0 || wval == 0) )
            {
                mMakeError(errno);
            }
            else
            {
                double *dval = (double *) d->val;
                dval[index] = val;
                dval = (double *) d->w_val;
                dval[index] = wval;
                printf("added %Le - %Le as r and w\n", val, wval);
            }
        }
        else if(d->dataInfo->type == Int)
        {
            long int ival;
            if(readval == NULL)
                ival = LONG_MAX;
            else
                ival = strtol(readval, &endptr, 10);

            long int wival;
            if(writeval == NULL)
                wival = LONG_MAX;
            else
                wival = strtol(writeval, &endptr, 10);
            if(errno != 0 &&  (ival == 0 || wival == 0) )
                mMakeError(errno);
            else
            {
                long int *pival = (long int *) d->val;
                pival[index] = ival;
                pival = (long int *) d->w_val;
                pival[index] = wival;
            }
        }
        else if(d->dataInfo->type == UInt)
        {
            long unsigned int uival;
            long unsigned int wuival;

            if(readval == NULL)
                uival = ULONG_MAX;
            else
                uival = strtoul(readval, &endptr, 10);

            if(writeval == NULL)
                wuival = ULONG_MAX;
            else
                wuival = strtoul(writeval, &endptr, 10);

            if(errno != 0 &&  (uival == 0 || wuival == 0) )
                mMakeError(errno);
            else
            {
                long unsigned int *puival = (long unsigned int *) d->val;
                puival[index] = uival;
                puival = (long unsigned int *) d->w_val;
                puival[index] = wuival;
            }
        }
        else if(d->dataInfo->type == Boolean)
        {
            bool booval;
            bool wbooval;

            if(readval == NULL)
                booval = false;
            else
                booval = (strcasecmp(readval, "true") == 0 || strtol(readval, &endptr, 10) != 0);

            if(writeval == NULL)
                wbooval = false;
            else
                wbooval = (strcasecmp(writeval, "true") == 0 || strtol(writeval, &endptr, 10) != 0);

            if(errno != 0)
                mMakeError(errno);
            else
            {
                bool *bval = (bool *) d->val;
                bval[index] = booval;
                bval = (bool *) d->w_val;
                bval[index] = wbooval;
            }
        }
    }
    if(errno != 0)
        d->mIsValid = false;
}

/** \brief This method parses a string in order to extract the data and convert it to the
 *         correct internal representation. This is <em>used internally</em>.
 *
 * @param s The string representation of the data, as fetched from the database.
 */
void XVariant::parse(const char *s)
{
    errno = 0;
    d->mIsValid = true;
    d->mIsWNull = true;
    d->mIsNull = (s == NULL);


    //   QHdbextractorThread("PARSING %s\n", s);
    if(d->dataInfo->format == Scalar && d->dataInfo->writable == RO && !d->mIsNull)
    {
        if(d->dataInfo->type == Double)
        {
            double *v = new double[1];
            *v  = strtod(s, NULL);
            d->val = v;
            d->mSize = 1;
        }
        else if(d->dataInfo->type == Int)
        {
            int * i = new int[1];
            *i = strtol(s, NULL, 10);
            d->val = i;
            d->mSize = 1;
        }
        else if(d->dataInfo->type == UInt)
        {
            unsigned int *ui = new unsigned int[1];
            *ui = strtoul(s, NULL, 10);
            d->val = ui;
            d->mSize = 1;
        }
        else if(d->dataInfo->type == Boolean)
        {
            bool *b = new bool[1];
            *b = (strcasecmp(s, "true") == 0 || strtol(s, NULL, 10) != 0);
            d->val = b;
            d->mSize = 1;
        }
        else if(d->dataInfo->type == String)
        {
            char *c = new char[strlen(s) + 1];
            strncpy(c, s, strlen(s) + 1);
            d->val = c;
            d->mSize = 1;
        }
        else
            d->mIsValid = false;
    }
    else if(d->dataInfo->format == Vector && d->dataInfo->writable == RO && !d->mIsNull)
    {
        size_t i = 0;
        d->mSize = 0;
        char *saveptr;
        char *copy = new char[strlen(s) + 1];
        char *val = NULL;
        const char *delim = ", ";
        /* make a copy of s cuz strtok_r wants char * not const char *.
         * It will be deleted at the end
         */
        strncpy(copy, s, strlen(s) + 1);
        /* count the number of separators in the data */
        val = strtok_r(copy, delim, &saveptr);
        while(val != NULL)
        {
            d->mSize++;
            val = strtok_r(NULL, delim, &saveptr);
        }
        strncpy(copy, s, strlen(s) + 1);

        if(d->dataInfo->type == Double)
        {
            double *d_array = new double[d->mSize];
            val = strtok_r(copy, delim, &saveptr);
            /* split result returned by hdb (comma separated doubles) */
            while(val != NULL && errno == 0 && i < d->mSize)
            {
                *(d_array + i) = strtod(val, NULL);
                i++;
                val = strtok_r(NULL, delim, &saveptr);
            }
            d->val = d_array;
        }
        else if(d->dataInfo->type == Int)
        {
            long int *li_array = new long int[d->mSize];
            val = strtok_r(copy, delim, &saveptr);
            /* split result returned by hdb (comma separated doubles) */
            while(val != NULL && errno == 0 && i < d->mSize)
            {
                *(li_array + i) = strtol(val, NULL, 10);
                i++;
                val = strtok_r(NULL, delim, &saveptr);
            }
            d->val = li_array;
        }
        else if(d->dataInfo->type == UInt)
        {
            unsigned long int *uli_array = new unsigned long int[d->mSize];
            val = strtok_r(copy, delim, &saveptr);
            /* split result returned by hdb (comma separated doubles) */
            while(val != NULL && errno == 0 && i < d->mSize)
            {
                *(uli_array + i) = strtoul(val, NULL, 10);
                i++;
                val = strtok_r(NULL, delim, &saveptr);
            }
            d->val = uli_array;
        }
        else if(d->dataInfo->type == Boolean)
        {
            bool *b_array = new bool[d->mSize];
            val = strtok_r(copy, delim, &saveptr);
            /* split result returned by hdb (comma separated doubles) */
            while(val != NULL && errno == 0 && i < d->mSize)
            {
                *(b_array + i) = (strcasecmp(s, "true") == 0 || strtol(s, NULL, 10) != 0);
                i++;
                val = strtok_r(NULL, delim, &saveptr);
            }
            d->val = b_array;
        }
        else if(d->dataInfo->type == String)
        {

        }

        /* delete the copy of the string */
        delete copy;
    }

    /* Check for errors */
    if (errno != 0 &&  s != NULL)
    {
        perr("XVariant.parse: error converting \"%s\" -> \"%s\": \"%s\"", getSource(), s, strerror(errno));
        mMakeError(errno);
        d->mIsValid = false;
    }

    if(!d->mIsValid &&  s != NULL)
        perr("XVariant.parse(s): \"%s\": format %d writable %d type %d not supported",
             getSource(), d->dataInfo->format, d->dataInfo->writable, d->dataInfo->type);
}

void XVariant::parse(const char *sr, const char *sw)
{
    errno = 0;
    d->mIsValid = true;
    d->mIsNull = (sr == NULL);
    d->mIsWNull = (sw == NULL);

    //   QHdbextractorThread("PARSING %s\n", s);
    if(d->dataInfo->format == Scalar && d->dataInfo->writable == RW && (!d->mIsNull || !d->mIsWNull))
    {
        if(d->dataInfo->type == Double)
        {
            if(!d->mIsNull)
            {
                double *v = new double[1];
                *v  = strtod(sr, NULL);
                d->val = v;
            }
            if(!d->mIsWNull)
            {
                double *wv = new double[1];
                *wv  = strtod(sw, NULL);
                d->w_val = wv;
            }
            d->mSize = 1;
        }
        else if(d->dataInfo->type == Int)
        {
            if(!d->mIsNull)
            {
                long int *i = new long int[1];
                *i = strtol(sr, NULL, 10);
                d->val = i;
            }
            if(!d->mIsWNull)
            {
                long int *wi = new long int[1];
                *wi = strtol(sw, NULL, 10);
                d->w_val = wi;
            }
            d->mSize = 1;
        }
        else if(d->dataInfo->type == UInt)
        {
            if(!d->mIsNull)
            {
                unsigned long int *ui = new unsigned long int[1];
                *ui = strtoul(sr, NULL, 10);
                d->val = ui;
            }
            if(!d->mIsWNull)
            {
                unsigned long int *wui = new unsigned long int[1];
                *wui = strtoul(sw, NULL, 10);
                d->w_val = wui;
            }
            d->mSize = 1;
        }
        else if(d->dataInfo->type == Boolean)
        {
            if(!d->mIsNull)
            {
                bool *b = new bool[1];
                *b = (strcasecmp(sr, "true") == 0 || strtol(sr, NULL, 10) != 0);
                d->val = b;
            }
            if(!d->mIsWNull)
            {
                bool *wb = new bool[1];
                *wb = (strcasecmp(sw, "true") == 0 || strtol(sw, NULL, 10) != 0);
                d->w_val = wb;
            }
            d->mSize = 1;
        }
        else if(d->dataInfo->type == String)
        {
            if(!d->mIsNull)
            {

            }
            if(!d->mIsWNull)
            {

            }
            d->mSize = 1;
        }
        else
            d->mIsValid = false;
    }
    else if(d->dataInfo->format == Vector && d->dataInfo->writable == RW && (!d->mIsNull || !d->mIsWNull) )
    {
        size_t i = 0;
        d->mSize = 0;

        char *copy = NULL;
        const char *delim = ", ";
        char *saveptr;
        char *val = NULL;

        /* 1. read part */
        if(!d->mIsNull)
        {
            /* make a copy of s cuz strtok_r wants char * not const char *.
         * It will be deleted at the end
         */
            copy = new char[strlen(sr) + 1];
            strncpy(copy, sr, strlen(sr) + 1);
            /* count the number of separators in the data */
            val = strtok_r(copy, delim, &saveptr);
            while(val != NULL)
            {
                d->mSize++;
                val = strtok_r(NULL, delim, &saveptr);
            }
            strncpy(copy, sr, strlen(sr) + 1);

            if(d->dataInfo->type == Double)
            {
                double *d_array = new double[d->mSize];
                val = strtok_r(copy, delim, &saveptr);
                /* split result returned by hdb (comma separated doubles) */
                while(val != NULL && errno == 0 && i < d->mSize)
                {
                    *(d_array + i) = strtod(val, NULL);
                    i++;
                    val = strtok_r(NULL, delim, &saveptr);
                }
                d->val = d_array;
            }
            else if(d->dataInfo->type == Int)
            {
                long int *li_array = new long int[d->mSize];
                val = strtok_r(copy, delim, &saveptr);
                /* split result returned by hdb (comma separated doubles) */
                while(val != NULL && errno == 0 && i < d->mSize)
                {
                    *(li_array + i) = strtol(val, NULL, 10);
                    i++;
                    val = strtok_r(NULL, delim, &saveptr);
                }
                d->val = li_array;
            }
            else if(d->dataInfo->type == UInt)
            {
                unsigned long int *uli_array = new unsigned long int[d->mSize];
                val = strtok_r(copy, delim, &saveptr);
                /* split result returned by hdb (comma separated doubles) */
                while(val != NULL && errno == 0 && i < d->mSize)
                {
                    *(uli_array + i) = strtoul(val, NULL, 10);
                    i++;
                    val = strtok_r(NULL, delim, &saveptr);
                }
                d->val = uli_array;
            }
            else if(d->dataInfo->type == Boolean)
            {
                bool *b_array = new bool[d->mSize];
                val = strtok_r(copy, delim, &saveptr);
                /* split result returned by hdb (comma separated doubles) */
                while(val != NULL && errno == 0 && i < d->mSize)
                {
                    *(b_array + i) = (strcasecmp(sr, "true") == 0 || strtol(sr, NULL, 10) != 0);
                    i++;
                    val = strtok_r(NULL, delim, &saveptr);
                }
                d->val = b_array;
            }
            else if(d->dataInfo->type == String)
            {

            }

            /* delete the copy of the string */
            delete copy;

        } /* if(!d->mIsNull) */

        /* =========================================================================
         * 2. write part
         * =========================================================================
         */
        if(!d->mIsWNull)
        {
            copy = new char[strlen(sw) + 1];
            size_t wri_size = 0;

            /* make a copy of s cuz strtok_r wants char * not const char *.
             * It will be deleted at the end
             */
            strncpy(copy, sw, strlen(sw) + 1);
            /* count the number of separators in the data */
            val = strtok_r(copy, delim, &saveptr);
            while(val != NULL)
            {
                wri_size++;
                val = strtok_r(NULL, delim, &saveptr);
            }
            strncpy(copy, sw, strlen(sw) + 1);
            if(wri_size == d->mSize)
            {
                if(d->dataInfo->type == Double)
                {
                    double *d_array = new double[wri_size];
                    val = strtok_r(copy, delim, &saveptr);
                    /* split result returned by hdb (comma separated doubles) */
                    while(val != NULL && errno == 0 && i < d->mSize)
                    {
                        *(d_array + i) = strtod(val, NULL);
                        i++;
                        val = strtok_r(NULL, delim, &saveptr);
                    }
                    d->w_val = d_array;
                }
                else if(d->dataInfo->type == Int)
                {
                    long int *li_array = new long int[d->mSize];
                    val = strtok_r(copy, delim, &saveptr);
                    /* split result returned by hdb (comma separated doubles) */
                    while(val != NULL && errno == 0 && i < d->mSize)
                    {
                        *(li_array + i) = strtol(val, NULL, 10);
                        i++;
                        val = strtok_r(NULL, delim, &saveptr);
                    }
                    d->w_val = li_array;
                }
                else if(d->dataInfo->type == UInt)
                {
                    unsigned long int *uli_array = new unsigned long int[d->mSize];
                    val = strtok_r(copy, delim, &saveptr);
                    /* split result returned by hdb (comma separated doubles) */
                    while(val != NULL && errno == 0 && i < d->mSize)
                    {
                        *(uli_array + i) = strtoul(val, NULL, 10);
                        i++;
                        val = strtok_r(NULL, delim, &saveptr);
                    }
                    d->w_val = uli_array;
                }
                else if(d->dataInfo->type == Boolean)
                {
                    bool *b_array = new bool[d->mSize];
                    val = strtok_r(copy, delim, &saveptr);
                    /* split result returned by hdb (comma separated doubles) */
                    while(val != NULL && errno == 0 && i < d->mSize)
                    {
                        *(b_array + i) = (strcasecmp(sw, "true") == 0 || strtol(sw, NULL, 10) != 0);
                        i++;
                        val = strtok_r(NULL, delim, &saveptr);
                    }
                    d->w_val = b_array;
                }
                else if(d->dataInfo->type == String)
                {

                }
            } /* if(wri_size == d->mSize) */
            else
            {
                perr("XVariant.parse: error converting \"%s\":\n read and write sizes are different!", getSource());
                d->mIsValid = false;
            }

            /* delete the copy of the string */
            delete copy;

        } /* !d->mIsWNull */

    } /* else if(d->dataInfo->dataFormat == Vector && d->dataInfo->writable == RW && (!d->mIsNull || !d->mIsWNull) ) */

    /* Check for string to number conversion errors */
    if (errno != 0)
    {
        perr("XVariant.parse: error converting \"%s\":\n    READ: \"%s\";\n    WRITE: %s: \"%s\"",
             getSource(), sr, sw, strerror(errno));
        mMakeError(errno);
        d->mIsValid = false;
    }

    if(!d->mIsValid)
        perr("XVariant.parse(s): \"%s\": format %d writable %d type %d not supported or r/w size mismatch",
             getSource(), d->dataInfo->format, d->dataInfo->writable, d->dataInfo->type);
}

void XVariant::mMakeError(int errnum)
{
    mMakeError(strerror(errnum));
}

void XVariant::mMakeError(const char *msg)
{
    if(d->mError)
        delete d->mError;
    if(!msg)
        d->mError = NULL;
    else
    {
        d->mError = new char[strlen(msg) + 1];
        strncpy(d->mError, msg, strlen(msg) + 1);
    }
}

void XVariant::init_common(const char *source, const char *timestamp, DataFormat df, DataType dt)
{
    d->mIsValid = false;
    d->mSize = 0;
    strncpy(d->mTimestamp, timestamp, TIMESTAMPLEN);
    d->dataInfo->set(source, df, dt); /* writable will be set to invalid */
}
void XVariant::init_data(size_t size)
{
    d->w_val = NULL;
    d->val = NULL;
    d->mSize = size;
    d->mError = NULL;

    d->mIsValid = false;
    d->mIsNull = true;
    d->mIsWNull = true;

    XVariant::Writable wri = d->dataInfo->writable;

    if(d->dataInfo->type == Double)
    {
        if(wri == RW ||wri == RO)
            d->val = (double *) new double[size];
        if(wri == RW || wri == WO)
            d->w_val = (double *) new double[size];
    }
    else if(d->dataInfo->type == Int)
    {
        if(wri == RW ||wri == RO)
            d->val = (long int *) new long int[size];
        if(wri == RW || wri == WO)
            d->w_val = (long int *) new long int[size];
    }
    else if(d->dataInfo->type == UInt)
    {
        if(wri == RW ||wri == RO)
            d->val = (long unsigned int *) new long unsigned int[size];
        if(wri == RW || wri == WO)
            d->w_val = (long unsigned int *) new long unsigned int[size];
    }
    else if(d->dataInfo->type == Boolean)
    {
        if(wri == RW ||wri == RO)
            d->val = (bool *) new bool[size];
        if(wri == RW || wri == WO)
            d->w_val = (bool *) new bool[size];
    }
    else if(d->dataInfo->type == String)
    {
        if(wri == RW ||wri == RO)
            d->val = (char *) new char[size];
        if(wri == RW || wri == WO)
            d->w_val = (char *) new char[size];
    }
}

void XVariant::init_data()
{
    d->val = NULL;
    d->w_val = NULL;
    d->mError = NULL;
}

void XVariant::delete_rdata()
{
    if(d->val != NULL)
    {
        if(d->dataInfo->type == Double)
            delete (double *) d->val;
        else if(d->dataInfo->type == Int)
            delete (int *) d->val;
        else if(d->dataInfo->type == Boolean)
            delete (bool *) d->val;
        else if(d->dataInfo->type == String)
            delete (char *) d->val;
        //        printf("delete_rdata: XVariant %p deleted d %p d->val %p type %d\n", this, d, d->val, d->dataInfo->dataType);
        d->val = NULL;
    }
}

void XVariant::delete_wdata()
{
    if(d->w_val != NULL)
    {
        if(d->dataInfo->type == Double)
            delete (double *) d->w_val;
        else if(d->dataInfo->type == Int)
            delete (int *) d->w_val;
        else if(d->dataInfo->type == Boolean)
            delete (bool *) d->w_val;
        else if(d->dataInfo->type == String)
            delete (char *) d->w_val;
        d->w_val = NULL;
    }
}

/** \brief Convenience function that sets the quality factor of the XVariant taking a char
 *  pointer as argument. Can be used to set the quality factor directly from the database
 *  result, as char pointer.
 *
 * @param quality the quality factor, expressed as a string. A NULL value does not change
 * the quality factor previously memorized. A string that does not contain digits does not
 * change the quality factor.
 *
 * @return  a reference to this XVariant
 *
 * @see getError
 * @see setError
 *
 * \note Being the hdbextractor++ library not dependent on the Tango library, here is the
 * Tango AttrQuality enumeration mapping extracted by the include file idl/tango.h:
 * enum AttrQuality { ATTR_VALID, ATTR_INVALID, ATTR_ALARM, ATTR_CHANGING, ATTR_WARNING,  __max_AttrQuality=0xffffffff }
 *
 * \note <strong>Calling setQuality with value 1 (ATTR_INVALID) will set the flag isValid to false</strong>.
 *
 */
void XVariant::setQuality(const char *quality)
{
    if(quality)
    {
        char *endptr;
        long val = strtol(quality, &endptr, 10);
        if(endptr == quality)
            perr("XVariant.setQuality: no digits were found in \"%s\"", quality);
        else
            d->mQuality = (short) val;
    }
}

/** \brief Sets the error message associated to this XVariant
 *
 * @param error the error message. Can be NULL.
 *
 * @return a reference to this XVariant
 *
 * @see getError
 * @see setQuality
 * @see getQuality
 */
void XVariant::setError(const char *error)
{
    mMakeError(error);
}

/** \brief This method allows changing the timestamp of the XVariant
 *
 * @param ts the new timestamp.
 *
 * @return A reference to the same object with the timestamp changed.
 *
 * @see getTimestamp
 * @see getTime_tTimestamp
 * @see getTimevalTimestamp
 */
void XVariant::setTimestamp(const char* ts)
{
    strncpy(d->mTimestamp, ts, TIMESTAMPLEN);
}

/** \brief Changes the XVariant timestamp according to the time expressed in a double format,
 *  seconds.microseconds.
 *
 * @param tsmicro the new timestamp as a double value, where the integer part represents seconds
 * and the decimal one microseconds.
 *
 * @return A reference to the same object with the timestamp changed.
 *
 */
void XVariant::setTimestamp(double tsmicro)
{
    struct timeval tv;
    tv.tv_sec = (time_t) tsmicro;
    tv.tv_usec = (tsmicro - tv.tv_sec) * 1e6;
    setTimestamp(&tv);
}

/** \brief Changes the XVariant timestamp according to the timeval passed as argument.
 *
 * @param tv the new timestamp as a struct timeval, passed in as a pointer to a user
 * allocated structure.
 *
 * @return A reference to the same object with the timestamp changed.
 *
 */
void XVariant::setTimestamp(const struct timeval* tv)
{
    DateTimeUtils().toString(tv, d->mTimestamp, TIMESTAMPLEN);
}

/** \brief Returns the timestamp associated to the data stored by XVariant, in the form
 *         of a string.
 *
 * @return A string representation of the timestamp associated to the data.
 *
 * @see setTimestamp
 */
const char *XVariant::getTimestamp() const
{
    return  d->mTimestamp;
}

/** \brief Returns the timestamp associated to the data stored by XVariant, in the form
 *         of a time_t data type
 *
 * @return A time_t value containing the date/time associated to the data.
 */
time_t XVariant::getTime_tTimestamp() const
{
    return DateTimeUtils().toTime_t(d->mTimestamp);
}

/** \brief Returns the timestamp associated to the data stored by XVariant, in the form
 *         of a struct timeval data type
 *
 * @return A struct timeval value containing the date/time associated to the data: seconds and microsecs.
 */
struct timeval XVariant::getTimevalTimestamp() const
{
    return DateTimeUtils().toTimeval(d->mTimestamp);
}

/**
 * @brief Returns the quality factor as a short integer.
 *
 * @return the quality factor of the value stored by this XVariant. The meaning of the value
 * is directly mapped from the Tango AttrQuality enumeration.
 *
 * \note Being the hdbextractor++ library not dependent on the Tango library, here is the
 * Tango AttrQuality enumeration mapping extracted by the include file idl/tango.h:
 * enum AttrQuality { ATTR_VALID, ATTR_INVALID, ATTR_ALARM, ATTR_CHANGING, ATTR_WARNING,  __max_AttrQuality=0xffffffff }
 *
 */
short int XVariant::getQuality() const
{
    return d->mQuality;
}

/** \brief The conversion method that tries to convert the stored data into a vector of double
 *
 * @return a std vector of double representing the data saved into XVariant
 * @param read true (the default if not specified) returns the <em>read</em> value saved into the database.
 * @param read false returns the <em>write</em> value saved into the database.
 *
 */
std::vector<double> XVariant::toDoubleVector(bool read) const
{
    double *d_val;
    if(read)
        d_val = (double *) d->val;
    else
        d_val = (double *) d->w_val;

    std::vector<double> dvalues(d_val, d_val + d->mSize);
    return dvalues;
}

/** \brief The conversion method that tries to convert the stored data into a vector of
 *         unsigned long integers
 *
 * @return a std vector of int representing the data saved into XVariant
 * @param read true (the default if not specified) returns the <em>read</em> value saved into the database.
 * @param read false returns the <em>write</em> value saved into the database.
 *
 * \note unsigned shorts and unsigned ints are mapped to unsigned long.
 *
 */
std::vector<unsigned long int> XVariant::toULongIntVector(bool read) const
{
    unsigned long int *i_val;
    if(read)
        i_val = (unsigned long int *) d->val;
    else
        i_val = (unsigned long int *) d->w_val;

    std::vector<unsigned long int> ivalues(i_val, i_val + d->mSize);
    return ivalues;
}

/** \brief The conversion method that tries to convert the stored data into a vector of
 *         unsigned long integers
 *
 * @return a std vector of int representing the data saved into XVariant
 * @param read true (the default if not specified) returns the <em>read</em> value saved into the database.
 * @param read false returns the <em>write</em> value saved into the database.
 *
 * \note unsigned shorts and unsigned ints are mapped to unsigned long.
 *
 */
std::vector<long int> XVariant::toLongIntVector(bool read) const
{
    long int *i_val;
    if(read)
        i_val = (long int *) d->val;
    else
        i_val = (long int *) d->w_val;

    std::vector<long int> ivalues(i_val, i_val + d->mSize);
    return ivalues;
}

/** \brief The conversion method that tries to convert the stored data into a vector of booleans
 *
 * @return a std vector of bool representing the data saved into XVariant
 * @param read true (the default if not specified) returns the <em>read</em> value saved into the database.
 * @param read false returns the <em>write</em> value saved into the database.
 *
 */
std::vector<bool> XVariant::toBoolVector(bool read) const
{
    bool *b_val;
    if(read)
        b_val = (bool *) d->val;
    else
        b_val = (bool *) d->w_val;

    std::vector<bool> bvalues(b_val, b_val + d->mSize);
    return bvalues;
}

/** \brief The conversion method that tries to convert the stored data into a double scalar
 *
 * @return a double representing the data saved into XVariant
 * @param read true (the default if not specified) returns the <em>read</em> value saved into the database.
 * @param read false returns the <em>write</em> value saved into the database.
 *
 * \note If the data cannot be converted to a double scalar value, then isValid will return false.
 * On the other hand, no error message is set by this method.
 *
 *
 */
double XVariant::toDouble(bool read, bool *ok) const
{
    double v = nan("NaN");
    if(read && d->dataInfo->type == Double && d->dataInfo->format == Scalar && (d->dataInfo->writable == RO || d->dataInfo->writable == RW) && d->val != NULL)
        v = *((double *)d->val);
    else if(!read && d->dataInfo->type == Double && d->dataInfo->format == Scalar && (d->dataInfo->writable == RW || d->dataInfo->writable == WO) && d->w_val != NULL)
        v = *((double *)d->w_val);
    if(ok)
        *ok = d->mIsValid && (d->val != NULL || d->w_val != NULL);
    return v;
}

/** \brief The conversion method that tries to convert the stored data into a long scalar integer
 *
 * @return an int representing the data saved into XVariant
 * @param read true (the default if not specified) returns the <em>read</em> value saved into the database.
 * @param read false returns the <em>write</em> value saved into the database.
 *
 * \note If the data cannot be converted to a integer scalar value, then isValid will return false.
 * On the other hand, no error message is set by this method.
 *
 */
long int XVariant::toLongInt(bool read, bool *ok) const
{
    long int i = LONG_MIN;
    if(read && d->dataInfo->type == Int && d->dataInfo->format == Scalar && (d->dataInfo->writable == RO || d->dataInfo->writable == RW) && d->val != NULL)
        i = *((long int *)d->val);
    else if(read && d->dataInfo->type == Int && d->dataInfo->format == Scalar && (d->dataInfo->writable == RO || d->dataInfo->writable == RW) && d->w_val != NULL)
        i = *((long int *)d->w_val);
    if(ok)
        *ok = d->mIsValid && (d->val != NULL || d->w_val != NULL);
    return i;
}

/** \brief The conversion method that tries to convert the stored data into a long unsigned scalar integer
 *
 * @return an int representing the data saved into XVariant
 * @param read true (the default if not specified) returns the <em>read</em> value saved into the database.
 * @param read false returns the <em>write</em> value saved into the database.
 *
 * \note If the data cannot be converted to a integer scalar value, then isValid will return false.
 * On the other hand, no error message is set by this method.
 *
 */
unsigned long int XVariant::toULongInt(bool read, bool *ok) const
{
    unsigned long int i = -1UL;
    if(read && d->dataInfo->type == UInt && d->dataInfo->format == Scalar && (d->dataInfo->writable == RO || d->dataInfo->writable == RW) && d->val != NULL)
        i = *((unsigned long int *)d->val);
    else if(read && d->dataInfo->type == Int && d->dataInfo->format == Scalar && (d->dataInfo->writable == RO || d->dataInfo->writable == RW) && d->w_val != NULL)
        i = *((unsigned long int *)d->w_val);
    if(ok)
        *ok = d->mIsValid && (d->val != NULL || d->w_val != NULL);
    return i;
}


/** \brief The conversion method that tries to convert the stored data into a scalar boolean
 *
 * @return a bool representing the data saved into XVariant
 * @param read true (the default if not specified) returns the <em>read</em> value saved into the database.
 * @param read false returns the <em>write</em> value saved into the database.
 *
 * \note If the data cannot be converted to a boolean scalar value, then isValid will return false.
 * On the other hand, no error message is set by this method.
 *
 */
bool XVariant::toBool(bool read, bool *ok) const
{
    bool b = false;
    if(read && d->dataInfo->type == Boolean && d->dataInfo->format == Scalar && (d->dataInfo->writable == RO || d->dataInfo->writable == RW) && d->val != NULL)
        b = *((bool *)d->val);
    else if(!read && d->dataInfo->type == Boolean && d->dataInfo->format == Scalar && (d->dataInfo->writable == RO || d->dataInfo->writable == RW) && d->w_val != NULL)
        b = *((bool *)d->w_val);
    if(ok)
        *ok = d->mIsValid && (d->val != NULL || d->w_val != NULL);
    return b;
}

/** \brief The conversion method that tries to convert the stored data into a scalar string
 *
 * @return a string representing the data saved into XVariant
 * @param read true (the default if not specified) returns the <em>read</em> value saved into the database.
 * @param read false returns the <em>write</em> value saved into the database.
 *
 * \note If the data cannot be converted to a boolean scalar value, then isValid will return false.
 * On the other hand, no error message is set by this method.
 *
 */
std::string XVariant::toString(bool read, bool *ok) const
{
    std::string ret;
    if(read)
    {

    }
    if(ok)
        *ok = false;
    return ret;
}

/** \brief The conversion method that tries to convert the stored data into a vector of strings
 *
 * @return a std::vector of string representing the data saved into XVariant
 * @param read true (the default if not specified) returns the <em>read</em> value saved into the database.
 * @param read false returns the <em>write</em> value saved into the database.
 *
 * \note If the data cannot be converted to a vector of strings, then isValid will return false.
 * On the other hand, no error message is set by this method.
 *
 */
std::vector<std::string> XVariant::toStringVector() const
{
    std::vector<std::string> ret;

    return ret;
}

/** \brief Returns a pointer to a double addressing the start of data.
 *
 * Used with getSize allows to get the stored data in a "C" style.
 *
 * @see getSize
 * @see toLongIntP
 *
 * @param read true (default) get the pointer to the read data
 * @param read false get the pointer to the write data
 *
 * \note Check the return value of this method: if null, no data is currently
 *       stored or you are trying to extract a type of data different from the
 *       one memorized in XVariant.
 */
double *XVariant::toDoubleP(bool read) const
{
    if(read)
        return (double *) d->val ;
    return (double *) d->w_val;
}


/** \brief Returns a pointer to a long unsigned int addressing the start of data.
 *
 * Used with getSize allows to get the stored data in a "C" style.
 *
 * @see getSize
 * @see toDoubleP
 * @see toULongIntP
 *
 * @param read true (default) get the pointer to the read data
 * @param read false get the pointer to the write data
 *
 * \note Check the return value of this method: if null, no data is currently
 *       stored or you are trying to extract a type of data different from the
 *       one memorized in XVariant.
 *
 * \note shorts and ints are mapped to longs.
 */
unsigned long int *XVariant::toULongIntP(bool read) const
{
    if(read)
        return (unsigned long int *) d->val ;
    return (unsigned long int *) d->w_val;
}


/** \brief Returns a pointer to an int addressing the start of data.
 *
 * Used with getSize allows to get the stored data in a "C" style.
 *
 * @see getSize
 * @see toDoubleP
 *
 * @param read true (default) get the pointer to the read data
 * @param read false get the pointer to the write data
 *
 * \note Check the return value of this method: if null, no data is currently
 *       stored or you are trying to extract a type of data different from the
 *       one memorized in XVariant.
 *
 * \note shorts and ints are mapped to longs.
 */
long int *XVariant::toLongIntP(bool read) const
{
    if(read)
        return (long int *) d->val ;
    return (long int *) d->w_val;
}

/** \brief Returns a pointer to a boolean addressing the start of data.
 *
 * Used with getSize allows to get the stored data in a "C" style.
 *
 * @see getSize
 * @see toLongIntP for notes
 * @see toDoubleP for notes
 *
 */
bool *XVariant::toBoolP(bool read) const
{
    if(read)
        return (bool *) d->val ;
    return (bool *) d->w_val;
}

/** \brief Returns a char pointer addressing the start of data.
 *
 * Used with getSize allows to get the stored data in a "C" style.
 *
 * @see getSize
 * @see toLongIntP for notes
 * @see toDoubleP for notes
 *
 */
char **XVariant::toCharP(bool read) const
{
    if(read)
        return (char **) d->val ;
    return (char **) d->w_val;
}

/** \brief Tries to convert the value stored into a string.
 *
 * This method converts the data stored into an XVariant into a string.
 * For instance, if the XVariant stores a scalar Double data type which value is 0.12
 * then a string containing "0.12" is returned. The "%f" conversion specifier
 * is used to convert Double data type.
 * If the XVariant stores a Vector Double data type with values [10.1, 12.6, 9.1, -5.4]
 * then the string returned will be "10.1,12.6,9.1,-5.4". The "%f" conversion specifier is
 * used to convert doubles.
 * The "%ld" specifier is used to convert signed integers.
 * The "%lud" specifier is used to convert unsigned integers.
 *
 * @param read if true, convert to string the read value
 * @param read false, convert to string the set point (write) value.
 * @param ok if not null, store in ok the result of the conversion.
 *
 * @return a std::string representation of the stored values, as described above.
 *
 * \par Note
 * If the read is true and the read value is null or a conversion error occurs,
 * an empty string is returned. The same goes for the write value.
 */
std::string XVariant::convertToString(bool read, bool *ok)
{
    std::string ret;
    char err[256];
    char tmp[128] = "";
    if(ok != NULL)
        *ok = true;

    size_t siz = this->getSize();
    for(size_t i = 0 ; i < siz; i++)
    {
        strcpy(tmp, "");
        if(d->dataInfo->type == Double)
        {
            if(read && (d->dataInfo->writable == XVariant::RW || d->dataInfo->writable == XVariant::RO) && d->val != NULL)
                snprintf(tmp, 128, "%f", ((double *) d->val)[i]);
            else if(!read && (d->dataInfo->writable == XVariant::RW || d->dataInfo->writable == XVariant::WO) && d->w_val != NULL)
                snprintf(tmp, 128, "%f", ((double *) d->w_val)[i]);
        }
        else if(d->dataInfo->type == Int)
        {
            if(read && (d->dataInfo->writable == XVariant::RW || d->dataInfo->writable == XVariant::RO) && d->val != NULL)
                snprintf(tmp, 128, "%ld", ((long int *) d->val)[i]);
            else if(!read && (d->dataInfo->writable == XVariant::RW || d->dataInfo->writable == XVariant::WO) && d->w_val != NULL)
                snprintf(tmp, 128, "%ld", ((long int *) d->w_val)[i]);
        }
        else if(d->dataInfo->type == UInt)
        {
            if(read && (d->dataInfo->writable == XVariant::RW || d->dataInfo->writable == XVariant::RO) && d->val != NULL)
                snprintf(tmp, 128, "%lud",  ((unsigned long int *) d->val)[i]);
            else if(!read && (d->dataInfo->writable == XVariant::RW || d->dataInfo->writable == XVariant::WO) && d->w_val != NULL)
                snprintf(tmp, 128, "%lud", ((unsigned long int *) d->w_val)[i]);
        }
        else if(d->dataInfo->type == Boolean)
        {
            bool b = false;

            if(read && (d->dataInfo->writable == XVariant::RW || d->dataInfo->writable == XVariant::RO) && d->val != NULL)
                b = ((bool *) d->val)[i];
            else if(!read && (d->dataInfo->writable == XVariant::RW || d->dataInfo->writable == XVariant::WO) && d->w_val != NULL)
                b = ((bool *) d->w_val)[i];
            if(b)
                snprintf(tmp, 128, "true");
            else
                snprintf(tmp, 128, "true");
        }
        else if(d->dataInfo->type == String)
        {
            if(read && (d->dataInfo->writable == XVariant::RW || d->dataInfo->writable == XVariant::RO) && d->val != NULL)
                ret += std::string(((char **) d->val)[i]);
            else if(!read && (d->dataInfo->writable == XVariant::RW || d->dataInfo->writable == XVariant::WO) && d->w_val != NULL)
                ret += std::string(((char **) d->w_val)[i]);
        }
        else if(ok != NULL)
        {
            *ok = false;
            snprintf(err, 256, "XVariant.convertToString type %d unsupported (%s)", d->dataInfo->type, getSource());
            mMakeError(err);
        }

        ret = std::string(tmp);
        if(i < siz - 1 && ret.length() > 0)
            ret += std::string(",");
    }

    return ret;
}
