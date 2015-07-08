#include "configurabledbschemahelper.h"
#include "../../hdbxsettings.h"
#include "../../hdbxmacros.h"

#include <time.h>
#include <string.h>

/** \brief A class used to avoid too much code inside db schema methods that takes care of
 *         examining the HdbXSettings and help determine what to do with the settings
 *         stored into it.
 *
 */
ConfigurableDbSchemaHelper::ConfigurableDbSchemaHelper()
{
}

/** \brief Returns the "data from the past" integration mode according to the HdbXSettings
 *         options and the provided start_date.
 *
 *  @param HdbXSettings a pointer to the HdbXSettings object
 *  @param start_date the start date, as C string
 *
 *  @return true A query to retrieve the first data from the past is required.
 *  @return false There is no need to get past data.
 */
ConfigurableDbSchemaHelper::FillFromThePastMode ConfigurableDbSchemaHelper::fillFromThePastMode(const HdbXSettings* hdbXSettings,
                                                     const char *start_date,
                                                     const char *stop_date,
                                                     const char *first_value_date) const
{
    FillFromThePastMode mode = None;
    if(!hdbXSettings)
        return mode;

    if(!hdbXSettings->hasKey("FillFromThePastMode"))
        ; /* nothing to do: mode is None */
    else if(hdbXSettings->get("FillFromThePastMode").compare("KeepWindow") == 0)
        mode = KeepWindow;
    else if(hdbXSettings->get("FillFromThePastMode").compare("WidenWindow") == 0)
        mode = WidenWindow;
    else
        perr("ConfigurableDbSchemaHelper.fillFromThePastMode: unrecognized FillFromThePastMode \%s\"",
             hdbXSettings->get("FillFromThePastMode").c_str());

    printf("\e[0;35mfill mode from %s is %d\e[0m\n", hdbXSettings->get("FillFromThePastMode").c_str(), mode);

    if(mode != None)
    {
        double windowPercent = 5;
        bool ok = true;
        if(hdbXSettings->hasKey("FillFromThePastThresholdPercent"))
            windowPercent = hdbXSettings->getDouble("FillFromThePastThresholdPercent", &ok);

        if(!ok)
        {
            perr("ConfigurableDbSchemaHelper.fillFromThePastMode: invalid double for");
            perr(" the \"FillFromThePastThresholdPercent\" option: \"%s\": must be a floating point number.",
             hdbXSettings->get("FillFromThePastThresholdPercent").c_str());
            perr("Assuming a value of 5%%");
            windowPercent = 5;
        }

        struct tm mtm;
        time_t start_time_t, stop_time_t, first_value_time_t, first_required_data_time_t;
        double delta_time_t;
        memset(&mtm, 0, sizeof(struct tm));
        strptime(start_date, "%Y-%m-%d %H:%M:%S", &mtm);
        start_time_t = mktime(&mtm);

        memset(&mtm, 0, sizeof(struct tm));
        strptime(stop_date, "%Y-%m-%d %H:%M:%S", &mtm);
        stop_time_t = mktime(&mtm);

        memset(&mtm, 0, sizeof(struct tm));
        if(strptime(first_value_date, "%Y-%m-%d %H:%M:%S", &mtm) != NULL)
            first_value_time_t = mktime(&mtm);
        else /* no data at all */
            first_value_time_t = 0;

        delta_time_t = difftime(stop_time_t, start_time_t);

        first_required_data_time_t = start_time_t + (time_t) (delta_time_t * windowPercent / 100.0);

        printf("ConfigurableDbSchemaHelper.fillFromThePastMode: \e[0;7;36m t1 %s (%ld) "
               "\nt2 %s (%ld) delta %f percent %f required %s (%ld)\n"
               "first value at %s (%ld) ===> mode %d\e[0m\n",
               start_date, start_time_t, stop_date, stop_time_t, delta_time_t,   windowPercent,
              ctime(&first_required_data_time_t), first_required_data_time_t, ctime(&first_value_time_t),
               first_value_time_t, mode);
        printf("\e[1;33m first value date time %s\e[0m\n", ctime(&first_value_time_t));

        if(first_value_time_t == 0 || first_required_data_time_t < first_value_time_t)
        {
            printf("\e[1;32m returning mode %d\e[0m\n", mode);
            return mode;
        }
    }

    printf("ConfigurableDbSchemaHelper.fillFromThePastMode: \e[0;7;36m returnin' None!\e[0m\n");
    return None;
}
