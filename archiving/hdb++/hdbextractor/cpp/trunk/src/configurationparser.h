#ifndef CONFIGURATIONPARSER_H
#define CONFIGURATIONPARSER_H

#define MAXERRORLEN 128

#define _WITH_GETLINE // freebsd complains without this
#include <map>
#include <string>

/** This utility class reads a configuration file containing key/value pairs and fills in a
 *  std::map<std::string, std::string> that contains the results.
 *
 * \par Format of the configuration file
 * The file is made up of couples <em>key = value</em>. Each value is interpreted as a string.
 * It's up to you to perform the right conversion according to the expected data type for the
 * given key. Spaces are ignored. As a consequence, keys cannot contain spaces.<br/>
 * Comments are introduced by the <em>#</em> character. They can be inlined. Empty lines are
 * allowed (and ignored).
 *
 * \par Example of configuration file
 * \code
 *
# This file is parsed by the ConfigurationParser class.
# Lines are ignored after the '#' character.
# key - value pairs must be separated by a '=' character.
# White spaces and tabs are ignored.
#
dbuser  = hdbbrowser # a tab is ignored. This comment is inlined
dbpass =hdbbrowser
dbhost = fcsproxy
dbname=hdb

FillFromThePastMode = WidenWindow

 * \endcode
 *
 * To get the configuration pairs, instantiate a ConfigurationParser and call read
 * with the path of the file and a reference to a std::map<std::string, std::string>
 *
 * When read returns true, the pairs are stored into the map.<br/>
 * Since read does not clear the map passed as parameter, read can ideally be called subsequently
 * on multiple files.
 *
 * getError returns the error message if read returns false.
 */
class ConfigurationParser
{
public:
    ConfigurationParser();

    bool read(const char *filepath, std::map<std::string, std::string> &params);

    const char *getError() const;

private:
    char m_error[MAXERRORLEN];
};

#endif // CONFIGURATIONPARSER_H
