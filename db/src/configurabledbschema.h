#ifndef CONFIGURABLEDBSCHEMA_H
#define CONFIGURABLEDBSCHEMA_H

#include <dbschema.h>

class DbSchemaPrivate;

class ConfigurableDbSchema : public DbSchema
{
public:
    ConfigurableDbSchema();

    virtual void setQueryConfiguration(QueryConfiguration *queryConfiguration);


protected:
    DbSchemaPrivate *d_ptr;
};

#endif // CONFIGURABLEDBSCHEMA_H
