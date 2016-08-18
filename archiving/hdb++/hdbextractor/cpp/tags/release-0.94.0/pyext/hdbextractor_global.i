%module(directors="1") hdbextractor
%feature("director") HdbExtractorListener; // generate directors for all classes that have virtual methods
%feature("director") DataSieverProgressListener;
%feature("director") ResultListenerInterface;

%include "std_string.i"
%include "std_vector.i"
%include "std_map.i"
%include "typemaps.i"

%include xvariant.i

namespace std {
    %template(IntVector) vector<int>;
    %template(LongIntVector) vector<long int>;
    %template(ULongIntVector) vector<unsigned long int>;
    %template(BoolVector) vector<bool>;
    %template(DoubleVector) vector<double>;
    %template(XVariantVector) vector<XVariant>;
    %template(StringVector) vector<string>;
    %template(StringStringMap) map<string,string>;
}

%include datasieverprogresslistener.i 
%include hdbxsettings.i 
%include result.i
%include resultlistenerinterface.i 
%include xvariantdatainfo.i 
%include xvariantlist.i
%include datasiever.i
%include datetimeutils.i 
%include hdbextractorlistener.i 
%include connection.i 
%include mysqlconnection.i
%include configurationparser.i
%include timeinterval.i
%include xvariantprinter.i
%include hdbextractor.i
