#ifndef CONFIGURABLEDBSCHEMAHELPER_H
#define CONFIGURABLEDBSCHEMAHELPER_H

class HdbXSettings;

class ConfigurableDbSchemaHelper
{
public:

    /** \brief This enumeration reflects the options from the "fill from the past" feature
     *
     */
    enum FillFromThePastMode { None = 0, KeepWindow, WidenWindow };

    ConfigurableDbSchemaHelper();

    FillFromThePastMode fillFromThePastMode(const HdbXSettings* hdbXSettings,
                             const char *start_date, const char *stop_date,
                             const char *first_value_date) const;

};

#endif // CONFIGURABLEDBSCHEMAHELPER_H
