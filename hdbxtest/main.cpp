#include <stdio.h>
#include <stdlib.h>
#include "../src/utils/datasiever.h"
#include "myhdbextractorimpl.h"
#include "hdbxsettings.h"
#include "../src/hdbextractor.h"
#include "../src/configurationparser.h"
#include "../src/utils/xvariantprinter.h"
#include <map>

using namespace std;

int main(int argc, char **argv)
{
    if(argc < 5)
    {
        printf("\e[1;31mUsage\e[0m \"%s configfile.dat domain/family/member/attribute \"2014-07-20 10:00:00\" \"2014-07-20 12:00:00\"\n",
               argv[0]);
        exit(EXIT_FAILURE);
    }
    else
    {
        const char* start_date = argv[argc - 2];
        const char* stop_date = argv[argc - 1];

        std::vector<std::string> sources;
        for(int i = 2; i < argc - 2; i++)
            sources.push_back(std::string(argv[i]));

        HdbXSettings *qc = new HdbXSettings();
        qc->loadFromFile(argv[1]);

        MyHdbExtractorImpl *hdbxi = new MyHdbExtractorImpl(qc->get("dbuser").c_str(),
                qc->get("dbpass").c_str(), qc->get("dbhost").c_str(), qc->get("dbname").c_str());

        hdbxi->getHdbExtractor()->setHdbXSettings(qc);
        hdbxi->getData(sources, start_date, stop_date);

        const std::vector<XVariant> & valuelist = hdbxi->getValuelistRef();

        DataSiever siever;
        siever.divide(valuelist);
        siever.fill();
        std::vector<std::string> srcs = siever.getSources();
        for(size_t i = 0; i < srcs.size(); i++)
        {
           std::vector<XVariant > values = siever.getData(srcs.at(i));
           XVariantPrinter().printValueList(values, 2);
        }


        printf("main.cpp: deleting hdbxsettings\n");
        delete qc;
        printf("main.cpp: deleting MyHdbExtractorImpl\n");
        delete hdbxi;
    }
    return 0;
}

