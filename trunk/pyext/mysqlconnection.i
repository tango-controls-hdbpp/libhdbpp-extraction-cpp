// mysqlconnection.i - SWIG interface
%module hdbextractor

%{
#include <mysqlconnection.h>
%}

// Parse the original header file
%include <mysqlconnection.h>
