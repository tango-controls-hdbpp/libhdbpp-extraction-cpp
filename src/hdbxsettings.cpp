#include "hdbxsettings.h"
#include "configurationparser.h"
#include "hdbxmacros.h"
#include <stdlib.h>
#include <strings.h> /* strcasecmp */
#include <sstream>

/** \brief This class can be used to configure the behavior of the Hdbextractor queries
 *   and set options affecting the way you want data to be fetched.
 *
 * The configuration can be loaded from a file with the method loadFromFile.
 * Options can be set up or added with the add method.
 *
 * \par The configuration file.
 * The configuration file must be made up of lines of the kind:
 *
  \code
  # Configuration file. First option: a string:
  option = option1
  #
  optionNumeric=10.2  # This is a number. Comments can start on a configuration line
  # comments start with #

  # the line above is empty
  # The option below has the key, equals symbol and value separated by a tab character
  # This means that white spaces and tabs are ignored.
  #
  optionWithTab    =   aTab

  \endcode
 *
 * The format of the configuration file is the same used by the ConfigurationParser class.
 * Thus, it's possible to share the settings of an application in a single file.
 *
 * @see ConfigurationParser
 *
 * The QueryConfiguration stores key/value settings in the form of strings.
 * Numeric values can be retrieved in their expected format with the methods
 * getBool, getInt, getDouble.
 * The get method returns a string.
 *
 */
HdbXSettings::HdbXSettings()
{

}

HdbXSettings::~HdbXSettings()
{
    printf("\e[1;31mdeleting hdbxsettings START\e[0m\n");
    mMap.clear();
    printf("\e[1;31mdeleting hdbxsettings END\e[0m\n");
}

/** \brief Load the configuration from a text file.
 *
 * @param filename The full path to the configuration file.
 *
 * Uses ConfigurationParser to load or add entries to the QueryConfiguration.
 *
 * \note The current items, if any, are not removed from the configuration.
 *
 * @see add
 */
void HdbXSettings::loadFromFile(const char *filename)
{
    ConfigurationParser configParser;
    configParser.read(filename, mMap);
}

/** \brief Add a property or set its value.
 *
 * This method adds a property with name key if it does not exist or updates
 * its value if already present
 *
 * @param key the property name
 * @param value the property value
 *
 * \par Note The value is passed as a const char. The get, getInt, getDouble...
 *  methods will then convert the values into the desired type, if possible.
 */
void HdbXSettings::set(const char* key, const char *value)
{
    mMap[std::string(key)] = std::string(value);
}

void HdbXSettings::set(const char *key, const double d)
{
    std::ostringstream os;
    try
    {
        os << d;
        mMap[std::string(key)] = os.str();
    }
    catch(std::ios_base::failure)
    {

    }
}

void HdbXSettings::set(const char *key, const int i)
{
    std::ostringstream os;
    try
    {
        os << i;
        mMap[std::string(key)] = os.str();
    }
    catch(std::ios_base::failure)
    {

    }
}

void HdbXSettings::set(const char *key, bool b)
{
    if(b)
        mMap[std::string(key)] = std::string("true");
    else
        mMap[std::string(key)] = std::string("false");
}

/** \brief Returns true if the QueryConfiguration contains a value associated to the key,
 *         false otherwise.
 *
 * @param key the key whose value you're looking for.
 *
 * @return true QueryConfiguration contains a value associated to the key.
 * @return false QueryConfiguration does not contain the key.
 *
 * @see add
 * @see loadFromFile
 */
bool HdbXSettings::hasKey(const char *key) const
{
    return mMap.count(std::string(key)) > 0;
}

/** \brief Get the value associated to the key, if present, and transform it in an integer.
 *
 * @param key the key whose value is needed as an integer.
 * @param ok a pointer to a bool that, if not NULL, will contain the result of the
 *        conversion. If the string associated to the key cannot be converted to an
 *        integer, ok will be FALSE and the return value is 0.
 *
 * @return a long integer.
 *
 * \note strtol is internally used.
 *
 * \note The returned value is 0 also in the case that key is not found. So remember to
 *       check with hasKey before.
 *
 */
long int HdbXSettings::getInt(const char *key, bool *ok) const
{
    long int ret = 0;
    std::string val = mMap.at(key);
    char *endptr;
    if(val.size() > 0)
    {
        const char *c = val.c_str();
        ret = strtol(c, &endptr, 10);
        if(ok != NULL && ret == 0 && endptr == c) /* failed */
        {
            perr("QueryConfiguration::getInt: error converting \"%s\" to int", c);
            *ok = false;
        }
        else if(ok != NULL)
            *ok = true;
    }
    return ret;
}

/** \brief Returns the value associated to the key, as string.
 *
 * @see getInt
 * @see getDouble
 * @see getBool
 */
std::string HdbXSettings::get(const char *key) const
{
    return mMap.at(key);
}

/** \brief Returns the value associated to the key as a boolean.
 *
 * If key is contained in the configuration, its value is evaluated as a boolean.
 * The "true" string (case insensitive) and atoi(value) != 0 map to true.
 * Any other string, or key not found is mapped to false.
 *
 * @see getInt
 * @see getDouble
 * @see get
 */
bool HdbXSettings::getBool(const char *key) const
{
    if(mMap.count(key) > 0)
    {
        std::string val = mMap.at(key);
        return (val.size() > 0 && (strcasecmp(val.c_str(), "true") == 0 || atoi(val.c_str()) != 0) );
    }
    return false;
}

/** \brief Get the value associated to the key, if present, and transform it in a double.
 *
 * @param key the key whose value is needed as a double.
 * @param ok a pointer to a bool that, if not NULL, will contain the result of the
 *        conversion. If the string associated to the key cannot be converted to a
 *        double, ok will be FALSE and the return value is 0.
 *
 * @return a double, resulting from the conversion of the value associated to the specified key.
 *
 * \note strtol is internally used.
 *
 * \note The returned value is 0 also in the case that key is not found. So remember to
 *       check with hasKey before.
 *
 */
double HdbXSettings::getDouble(const char* key, bool *ok) const
{
    double ret = 0;
    std::string val = mMap.at(key);
    char *endptr;
    if(val.size() > 0)
    {
        const char *c = val.c_str();
        printf("converting \"%s\"\n", c);
        ret = strtod(c, &endptr);
        if(ok != NULL && ret == 0 && endptr == c) /* failed */
        {
            perr("QueryConfiguration::getDouble: error converting \"%s\" to double", c);
            *ok = false;
        }
        else if(ok != NULL)
            *ok = true;
    }
    return ret;
}
