#ifndef QUERYCONFIGURATION_H
#define QUERYCONFIGURATION_H

#include <map>
#include <string>

class QueryConfiguration
{
public:
    QueryConfiguration();

    void loadFromFile(const char *filename);

    void add(const char* key, const char *value);

    bool hasKey(const char *key) const;

    long getInt(const char *key, bool *ok) const;

    std::string get(const char *key) const;

    bool getBool(const char *key) const;

    double getDouble(const char* key, bool *ok) const;

private:
    std::map<std::string, std::string> mMap;
};

#endif // QUERYCONFIGURATION_H
