
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*
    $Id$
    $Source$
    $Log$
    Revision 1.21  2001/04/10 07:54:33  lballabio
    Ruby histories (the Ruby way)

    Revision 1.20  2001/04/09 12:24:58  nando
    updated copyright notice header and improved CVS tags

*/

#ifndef quantlib_currencies_i
#define quantlib_currencies_i

%include Date.i
%include Calendars.i
%include String.i

%{
using QuantLib::Currency;
using QuantLib::Handle;
typedef Handle<Currency> CurrencyHandle;
using QuantLib::Currencies::EUR;
using QuantLib::Currencies::USD;
using QuantLib::Currencies::GBP;
using QuantLib::Currencies::DEM;
using QuantLib::Currencies::ITL;
using QuantLib::Currencies::AUD;
using QuantLib::Currencies::CAD;
using QuantLib::Currencies::CHF;
using QuantLib::Currencies::DKK;
using QuantLib::Currencies::JPY;
using QuantLib::Currencies::SEK;
%}

// export Handle<Currency>
%name(Currency) class CurrencyHandle {
    %pragma(ruby) include = "Comparable";
  public:
    // constructor redefined below as string-based factory
    ~CurrencyHandle();
};

// replicate the Currency interface
%addmethods CurrencyHandle {
    #if defined(SWIGRUBY)
    void crash() {}
    #endif
    CurrencyHandle(const String& name) {
        String s = StringFormatter::toUppercase(name);
        if (s == "EUR")      return new CurrencyHandle(new EUR);
        else if (s == "USD") return new CurrencyHandle(new USD);
        else if (s == "GBP") return new CurrencyHandle(new GBP);
        else if (s == "DEM") return new CurrencyHandle(new DEM);
        else if (s == "ITL") return new CurrencyHandle(new ITL);
        else if (s == "AUD") return new CurrencyHandle(new AUD);
        else if (s == "CAD") return new CurrencyHandle(new CAD);
        else if (s == "CHF") return new CurrencyHandle(new CHF);
        else if (s == "JPY") return new CurrencyHandle(new JPY);
        else if (s == "DKK") return new CurrencyHandle(new DKK);
        else if (s == "SEK") return new CurrencyHandle(new SEK);
        else                 throw Error("Unknown currency");
        QL_DUMMY_RETURN(new CurrencyHandle)
    }
    CalendarHandle settlementCalendar() {
        return (*self)->settlementCalendar();
    }
    int settlementDays() {
        return (*self)->settlementDays();
    }
    Date settlementDate(const Date& d) {
        return (*self)->settlementDate(d);
    }
    #if defined (SWIGPYTHON) || defined (SWIGRUBY)
    String __str__() {
        if (!self->isNull())
            return (*self)->name()+" currency";
        else
            return "Null currency";
    }
    int __cmp__(const CurrencyHandle& other) {
        return ((*self) == other ? 0 : 1);
    }
    #endif
    #if defined (SWIGPYTHON)
    int __nonzero__() {
        return (self->isNull() ? 0 : 1);
    }
    #endif
}

#if defined (SWIGPYTHON)
%{
CurrencyHandle NewEUR()        { return CurrencyHandle(new EUR); }
CurrencyHandle NewUSD()        { return CurrencyHandle(new USD); }
CurrencyHandle NewGBP()        { return CurrencyHandle(new GBP); }
CurrencyHandle NewDEM()        { return CurrencyHandle(new DEM); }
CurrencyHandle NewITL()        { return CurrencyHandle(new ITL); }
CurrencyHandle NewAUD()        { return CurrencyHandle(new AUD); }
CurrencyHandle NewCAD()        { return CurrencyHandle(new CAD); }
CurrencyHandle NewCHF()        { return CurrencyHandle(new CHF); }
CurrencyHandle NewDKK()        { return CurrencyHandle(new DKK); }
CurrencyHandle NewJPY()        { return CurrencyHandle(new JPY); }
CurrencyHandle NewSEK()        { return CurrencyHandle(new SEK); }
%}

%name(EUR)    CurrencyHandle NewEUR();
%name(USD)    CurrencyHandle NewUSD();
%name(GBP)    CurrencyHandle NewGBP();
%name(DEM)    CurrencyHandle NewDEM();
%name(ITL)    CurrencyHandle NewITL();
%name(AUD)    CurrencyHandle NewAUD();
%name(CAD)    CurrencyHandle NewCAD();
%name(CHF)    CurrencyHandle NewCHF();
%name(DKK)    CurrencyHandle NewDKK();
%name(JPY)    CurrencyHandle NewJPY();
%name(SEK)    CurrencyHandle NewSEK();
#endif

// typemap Python sequence of currencies to std::vector<Handle<Currency> >

%{
typedef std::vector<Handle<Currency> > CurrencyHandleVector;
%}

%typemap(python,in) CurrencyHandleVector, CurrencyHandleVector *,
  const CurrencyHandleVector & {
    if (PyTuple_Check($source)) {
        int size = PyTuple_Size($source);
        $target = new std::vector<CurrencyHandle>(size);
        for (int i=0; i<size; i++) {
            CurrencyHandle* d;
            PyObject* o = PyTuple_GetItem($source,i);
            if ((SWIG_ConvertPtr(o,(void **) &d,
              (swig_type_info *)SWIG_TypeQuery("CurrencyHandle *"),1)) != -1) {
                (*$target)[i] = *d;
            } else {
                PyErr_SetString(PyExc_TypeError,
                  "tuple must contain currencies");
                delete $target;
                return NULL;
            }
        }
    } else if (PyList_Check($source)) {
        int size = PyList_Size($source);
        $target = new std::vector<CurrencyHandle>(size);
        for (int i=0; i<size; i++) {
            CurrencyHandle* d;
            PyObject* o = PyList_GetItem($source,i);
            if ((SWIG_ConvertPtr(o,(void **) &d,
              (swig_type_info *)SWIG_TypeQuery("CurrencyHandle *"),1)) != -1) {
                (*$target)[i] = *d;
            } else {
                PyErr_SetString(PyExc_TypeError,"list must contain currencies");
                delete $target;
                return NULL;
            }
        }
    } else {
        PyErr_SetString(PyExc_TypeError,"not a sequence");
        return NULL;
    }
};

%typemap(python,freearg) CurrencyHandleVector, CurrencyHandleVector *,
  const CurrencyHandleVector & {
    delete $source;
};


#endif
