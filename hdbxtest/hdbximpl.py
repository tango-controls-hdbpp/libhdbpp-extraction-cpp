from hdbextractor import *
import sys

class Hdbximpl(HdbExtractorListener):
    def __init__(self, dbu, dbh, dbn, dbp):
        HdbExtractorListener.__init__(self)
        self.dbuser = dbu
        self.dbhost = dbh
        self.dbname =  dbn
        self.dbpass = dbp

        self.inttest = 0;

        self.valuelist = []

        self.ex = Hdbextractor(self)
        if dbn == "hdbpp":
            type = Hdbextractor.HDBPPMYSQL
        else:
            type = Hdbextractor.HDBMYSQL

        res = self.ex.connect(type, dbh, dbn, dbu, dbp, 3306)
        print ("Connected", res)

        
    def onSourceProgressUpdate(self, name, percent):
        print(name + " data extraction: " + str(percent) + "%\n")
        self.extractData()

    def onExtractionFinished(self, totalrows, elapsed):
        print("extraction completed: got " + str(totalrows) + " rows in " + str(elapsed) + "s\n")

    def onSourceExtractionFinished(self, name, totalrows, elapsed):
        print(name + " data extraction completed in " + str(elapsed) + "s [" + str(totalrows) + " rows]\n")
        self.extractData()

    def addToTest(self, inpu):
        inpu = inpu + 1
        return inpu


    def getHdbExtractor(self):
        return self.ex

    def extractData(self):
        ret = self.ex.get(self.valuelist)[1]
#        print("extractData: got" , ret);
#        print("______ VALUELIST _____")
        self.valuelist = ret
#        print(self.valuelist)

    def getValueList(self):
        return self.valuelist

    def getData(self, sources, start_date, stop_date):
        res = self.ex.getData(sources, start_date, stop_date);
        if res == False:
            for s in sources:
                print("Error fetching data: " + s + " -> " + self.ex.getErrorMessage() )


# scalar_double_ro:  ./hdbxtest  db++config.dat
# "2013-01-10 11:14:10.000000" "2013-01-11 11:14:10"
#
# scalar_double_rw: ./hdbxtest  db++config.dat kg13/mod/llrf_kg13.01/RfReverse  "2011-11-06 21:05:43.000000" "2011-11-07 21:05:43.000000"
#
# scalar_double_rw: ./hdbxtest  db++config.dat kg10/mod/llrf_kg10.01/RfReverse  "2011-11-06 21:05:43.000000" "2011-11-07 21:05:43.000000"
#
# array_double_ro: ./hdbxtest db++config.dat f/radiation_protection/blm_master_uh.01/BlmIntData  "2013-01-18 01:38:20" "2013-01-19 11:38:20"
#
# HDB, spectrum, same start-stop date will fetch the first available data in the past
# ./hdbxtest  dbconfig.dat f/radiation_protection/blm_master_uh.01/BlmIntData "2013-05-29 19:09:10" "2013-05-29 19:09:10"
#
# HDBPP, scalar, same start-stop date will fetch the first available data in the past
# ./hdbxtest db++config.dat kg10/mod/llrf_kg10.01/RfReverse  "2011-11-07 21:05:43.000000" "2011-11-07 21:05:43.000000"

# HDB, two sources, short time window, to use to check data filling.
# /hdbxtest  dbconfig.dat f/radiation_protection/blm_master_linac.01/BlmIntData f/radiation_protection/blm_master_uh.01/BlmIntData "2013-01-19 19:07:10" "2013-01-19 19:08:10"

if __name__ == "__main__":
    argc = len(sys.argv)
    conf = sys.argv[1];
    att = sys.argv[2]

    settings = HdbXSettings()
    settings.loadFromFile(conf)

    sources = []
    for i in range (2, len(sys.argv) - 2):
        print("+ adding source " + sys.argv[i])
        sources.append(sys.argv[i])

    d1 = sys.argv[argc - 2]
    d2 = sys.argv[argc - 1]
    hdbximpl = Hdbximpl(settings.get("dbuser"), settings.get("dbhost"),  settings.get("dbname"), settings.get("dbpass"))
    hdbximpl.getHdbExtractor().setHdbXSettings(settings);
    hdbximpl.getData(sources, d1, d2);

    valuelist = hdbximpl.getValueList();
#    print(valuelist)
    print("There are " + str(len(valuelist)) + " datas");

    siever = DataSiever()
    siever.divide(valuelist);
    siever.fill();
    srcs = siever.getSources();
    print("Siever has " + str(siever.getSize())  + " sources");
    for i in range(0, siever.getSize()):
        values = siever.getData(srcs[i])
        XVariantPrinter().printValueList(values, 2)






