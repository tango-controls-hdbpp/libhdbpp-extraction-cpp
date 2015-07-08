#ifndef HDBSCHEMA_H
#define HDBSCHEMA_H

#include <configurabledbschema.h>
#include <configurabledbschemahelper.h>
#include <xvariantlist.h>
#include <resultlistenerinterface.h>
#include <vector>
#include <string>

class DbSchemaPrivate;
class XVariantList;
class Connection;

/** \brief An implementation of the DbSchema interface specific to MySql. <em>Used internally</em>.
 *
 */
class MySqlHdbSchema : public ConfigurableDbSchema
{
public:
    MySqlHdbSchema(ResultListener *resultListenerI);

    virtual ~MySqlHdbSchema();

    virtual bool getData(const char *source,
                                    const char *start_date,
                                    const char *stop_date,
                                    Connection *connection,
                                    int notifyEveryNumRows);

    virtual bool getData(const std::vector<std::string> sources,
                                 const char *start_date,
                                 const char *stop_date,
                                 Connection *connection,
                                 int notifyEveryNumRows);

    virtual bool getData(const char *source,
                                    const TimeInterval *time_interval,
                                    Connection *connection,
                                    int notifyEveryRows);

    virtual bool getData(const std::vector<std::string> sources,
                                    const TimeInterval *time_interval,
                                    Connection *connection,
                                    int notifyEveryRows);


    virtual bool getSourcesList(Connection *connection, std::list<std::string>& result) const;

    virtual bool findSource(Connection *connection, const char *substring, std::list<std::string>& result) const;

    virtual bool findErrors(const char *source, const TimeInterval *time_interval,
                            Connection *connection) const;

    virtual int get(std::vector<XVariant>& variantlist);

    virtual const char *getError() const;

    virtual bool hasError() const;

    virtual bool fetchInThePast(const char *source,
                                          const char *start_date, const char *table_name,
                                          const int att_id,
                                          XVariant::DataType dataType,
                                          XVariant::DataFormat format,
                                          XVariant::Writable writable,
                                          Connection *connection,
                                          double *time_elapsed,
                                          ConfigurableDbSchemaHelper::FillFromThePastMode mode);

};

#endif // HDBSCHEMA_H
