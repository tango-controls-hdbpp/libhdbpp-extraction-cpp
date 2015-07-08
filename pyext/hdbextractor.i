// hdbextractor.i - SWIG interface
%module hdbextractor
%include "typemaps.i"
%include xvariant.i

%{
#include <xvariant.h>
#include <hdbextractor.h>
%}

%apply std::vector<XVariant>& INOUT { std::vector<XVariant>& variantlist } ;
int Hdbextractor::get(std::vector<XVariant>& variantlist);

%apply std::vector<double>& INOUT { std::vector<double>& dv };
int Hdbextractor::testGet4(std::vector<double> &dv);

// Parse the original header file
%include <xvariant.h>
%include <hdbextractor.h>

