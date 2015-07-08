#include "configurabledbschema.h"
#include "dbschemaprivate.h"

#include <stdio.h>

/** \brief An abstract implementation of the DbSchema interface.
 * The class offers an implementation of the DbSchema interface adding the
 * management of an object called QueryConfiguration, whose aim is storing
 * useful configuration parameters to be passed to the hdb extractor.
 *
 * Subclass ConfigurableDbSchema to implement DbSchema and provide a
 * QueryConfiguration for your database schema related methods.
 *
 * @see MySqlHdbppSchema
 * @see MySqlHdbSchema
 */
ConfigurableDbSchema::ConfigurableDbSchema()
{
    d_ptr = new DbSchemaPrivate();
    d_ptr->queryConfiguration = NULL;
}

/** \brief Personalize queries.
 *
 * @param queryConfiguration a QueryConfiguration object.
 *
 */
void ConfigurableDbSchema::setQueryConfiguration(QueryConfiguration *queryConfiguration)
{
    printf("\e[1;35msetting query conf %p\e[0m\n", queryConfiguration);
    d_ptr->queryConfiguration = queryConfiguration;
}

