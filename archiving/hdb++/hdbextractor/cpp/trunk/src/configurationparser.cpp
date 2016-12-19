#include "configurationparser.h"
#include <errno.h>
#include <cstdio>
#include <string.h>
#include <stdlib.h>
#include "hdbxmacros.h"

/** \brief The class constructor.
 *
 * No paramters are required to instantiate an object of this type.
 *
 * @see read
 */
ConfigurationParser::ConfigurationParser()
{


}

/** \brief Read a configuration file and insert into the given map the detected key/value pairs.
 *
 * @return if read is successful, false otherwise.
 *
 * @param filepath the full path to the configuration file
 * @param params a reference to a std::map<std::string, std::string> where the pairs are inserted by read.
 *
 * \par Note
 * This method does not clear the map passed as input argument before parsing the configuration file.
 *
 */
bool ConfigurationParser::read(const char *filepath, std::map<std::string, std::string>& params)
{
    bool ret = false;
    errno = 0;
    FILE *fp = fopen(filepath, "r");
    if(fp == NULL)
    {
        perr("ConfigurationParser.read: error reading \"%s\": %s", filepath, strerror(errno));
        strncpy(m_error, strerror(errno), MAXERRORLEN);
    }
    else
    {
        char *line = NULL;
        char *conf = NULL;
        char *param = NULL;
        char *conf_save = NULL;
        char *line_save = NULL;
        char key[32];
        int res;
        size_t len = 0;
        ssize_t read;
        int conf_len;
        ret = true;
        /* reset error */
        strncpy(m_error, "", 2);

        while ((read = getline(&line, &len, fp)) != -1)
        {
            param = NULL;
            line_save = line; /* save pointer to buffer allocated by getline() */
            if(strchr(line, '#') == NULL)
                conf_len = read;
            else
                conf_len = strchr(line, '#') - line;

            if(conf_len > 0)
            {
                conf = (char *) realloc(conf_save, conf_len + 1);
                 /* save pointer to buffer allocated by realloc() */
                conf_save = conf;

                memset(conf, 0, conf_len + 1);

                /* in conf, put a copy of line without any space, tab, \n, \r */
                while(*line)
                {
                    if (!isspace(*line))
                    {
                        *conf = *line;
                        conf++;
                    }
                    line++;
                }
                line = line_save;
                conf = conf_save;
           //     printf("parsing line \"%s\" -> trimmed \"%s\"\n", line, conf);
                if(strlen(conf) > 1)
                {
                    param = (char *) realloc(param, conf_len + 1);
                    memset(param, 0, conf_len + 1);
                    res = sscanf(conf, "%[^=]=%s", key, param);
         //           printf("ConfigurationParser.read: \e[1;32m\"%s\" -> \"%s\"\e[0m\n",  key, param);

                    params[std::string(key)] = std::string(param);

                    if(param != NULL)
                        free(param);
                }
                conf = conf_save;
            }
        }

        if(conf)
            free(conf);
        if(line)
            free(line);

        fclose(fp);
    }
    return ret;
}

/** \brief Returns the error message according to the last ConfigurationParser::read failure reason
 *
 * @return The last error message, if read returns false
 *
 * @see read
 */
const char *ConfigurationParser::getError() const
{
    return m_error;
}
