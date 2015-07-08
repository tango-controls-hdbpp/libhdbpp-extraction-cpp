#ifndef CONFIGURATIONPARSER_H
#define CONFIGURATIONPARSER_H

#define MAXERRORLEN 128

#include <map>
#include <string>

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
