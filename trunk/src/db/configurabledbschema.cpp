#include "configurabledbschema.h"
#include "dbschemaprivate.h"

#include <stdio.h>

/** \brief An abstract implementation of the DbSchema interface.
 * The class offers an implementation of the DbSchema interface adding the
 * management of an object called HdbXSettings, whose aim is storing
 * useful configuration parameters to be passed to the hdb extractor.
 *
 * Subclass ConfigurableDbSchema to implement DbSchema and provide a
 * HdbXSettings for your database schema related methods.
 *
 * @see MySqlHdbppSchema
 * @see MySqlHdbSchema
 */
ConfigurableDbSchema::ConfigurableDbSchema()
{
    d_ptr = new DbSchemaPrivate();
    d_ptr->hdbxSettings = NULL;
}

/** \brief Personalize queries.
 *
 * @param HdbXSettings a HdbXSettings object.
 *
 */
void ConfigurableDbSchema::setHdbXSettings(HdbXSettings *HdbXSettings)
{
    d_ptr->hdbxSettings = HdbXSettings;
}

