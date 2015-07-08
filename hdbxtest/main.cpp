#include <stdio.h>
#include <stdlib.h>
#include "../utils/datasiever.h"
#include "myhdbextractorimpl.h"
#include "queryconfiguration.h"
#include "../src/hdbextractor.h"
#include "../src/configurationparser.h"
#include "../utils/xvariantprinter.h"
#include <map>

using namespace std;

int main(int argc, char **argv)
{
    if(argc < 5)
    {
        printf("\e[1;31mUsage\e[0m \"%s configfile.dat domain/family/member/attribute 2014-07-20 10:00:00 2014-07-20 12:00:00\n",
               argv[0]);
        exit(EXIT_FAILURE);
    }
    else
    {
        std::map<std::string, std::string> confmap ;

        confmap["dbuser"] = "hdbbrowser";
        confmap["dbpass"] = "hdbbrowser";
        confmap["dbhost"] = "fcsproxy";
        confmap["dbname"] = "hdb";
        confmap["dbport"] = "3306";

        const char* start_date = argv[argc - 2];
        const char* stop_date = argv[argc - 1];

        std::vector<std::string> sources;
        for(int i = 2; i < argc - 2; i++)
            sources.push_back(std::string(argv[i]));

        ConfigurationParser cp;
        cp.read(argv[1], confmap);

        QueryConfiguration *qc = new QueryConfiguration();
        qc->loadFromFile(argv[1]);

        MyHdbExtractorImpl *hdbxi = new MyHdbExtractorImpl(confmap["dbuser"].c_str(),
                confmap["dbpass"].c_str(), confmap["dbhost"].c_str(), confmap["dbname"].c_str());

        hdbxi->getHdbExtractor()->setQueryConfiguration(qc);
        hdbxi->getData(sources, start_date, stop_date);

        const std::vector<XVariant> & valuelist = hdbxi->getValuelistRef();


        DataSiever siever;
        printf("\e[1;36mSIEVING DATA....\e[0m\n");
        siever.divide(valuelist);

        siever.fill();

        printf("\e[1;32msources:\e[0m\n");
        std::vector<std::string> srcs = siever.getSources();
        for(size_t i = 0; i < srcs.size(); i++)
        {
            printf("\t* %s\n", srcs.at(i).c_str());

           std::vector<XVariant > values = siever.getData(srcs.at(i));
           XVariantPrinter().printValueList(values, 2);
        }


        delete qc;
    }
    return 0;
}

