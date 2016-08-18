#ifndef MYSQLHDBPPSCHEMA_H
#define MYSQLHDBPPSCHEMA_H

#include <configurabledbschema.h>
#include <configurabledbschemahelper.h>
#include <xvariantlist.h>
#include <resultlistenerinterface.h>
#include <vector>
#include <string>

class DbSchemaPrivate;

class MySqlHdbppSchema : public ConfigurableDbSchema
{
public:
    MySqlHdbppSchema(ResultListener *resultListenerI);

    virtual ~MySqlHdbppSchema();

    virtual int fetchInThePast(const char *source,
                                const char *start_date,
                                const char *table_name,
                                const int att_id,
                                XVariant::DataType dataType,
                                XVariant::DataFormat format,
                                XVariant::Writable writable,
                                Connection *connection,
                                double *ptime_elapsed,
                                ConfigurableDbSchemaHelper::FillFromThePastMode mode);

    virtual bool getData(const char *source,
                                    const char *start_date,
                                    const char *stop_date,
                                    Connection *connection,
                                    int notifyEveryPercent);

    virtual bool getData(const std::vector<std::string> sources,
                                 const char *start_date,
                                 const char *stop_date,
                                 Connection *connection,
                                 int notifyEveryPercent);

    virtual bool getData(const char *source,
                                    const TimeInterval *time_interval,
                                    Connection *connection,
                                    int notifyEveryPercent);

    virtual bool getData(const std::vector<std::string> sources,
                                    const TimeInterval *time_interval,
                                    Connection *connection,
                                    int notifyEveryPercent);

    virtual bool getData(const char *source,
                  const char *start_date,
                  const char *stop_date,
                  Connection *connection,
                  int notifyEveryPercent,
                  int sourceIndex,
                  int totalSources, double *elapsed);

    virtual bool getSourcesList(Connection *connection, std::list<std::string>& result) const;

    virtual bool findSource(Connection *connection, const char *substring, std::list<std::string> &result) const;

    virtual bool findErrors(const char *source, const TimeInterval *time_interval,
                            Connection *connection) const;

    virtual int get(std::vector<XVariant>& variantlist);

    virtual const char *getError() const;

    virtual bool hasError() const;

    virtual bool isCancelled() const;

    virtual void cancel();

    void resetCancelledFlag() const;
private:

    bool mGetSourceProperties(const char* source,
                              Connection *connection,
                              XVariant::DataType *type,
                              XVariant::DataFormat *format,
                              XVariant::Writable *writable,
                              char* data_type,
                              int *id) const;

};

#endif // MYSQLHDBPPSCHEMA_H
