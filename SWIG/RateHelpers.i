
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
    Revision 1.1  2001/05/16 09:57:27  lballabio
    Added indexes and piecewise flat forward curve

*/

#ifndef quantlib_rate_helpers_i
#define quantlib_rate_helpers_i

%include Date.i
%include DayCounters.i
%include Financial.i

%{
    using QuantLib::Handle;
    using QuantLib::TermStructures::RateHelper;
    using QuantLib::TermStructures::DepositRateHelper;
    typedef Handle<RateHelper> RateHelperHandle;
    typedef Handle<RateHelper> DepositRateHelperHandle;
%}

%name(RateHelper) class RateHelperHandle {
  public:
    ~RateHelperHandle();
};

%addmethods RateHelperHandle {
    Date maturity() {
        return (*self)->maturity();
    }
}

// actually they are the same class, but I'll fake inheritance...
%name(DepositRateHelper) class DepositRateHelperHandle 
: public RateHelperHandle {
  public:
    // constructor redefined below
    ~DepositRateHelperHandle();
};

%addmethods DepositRateHelperHandle {
    DepositRateHelperHandle(int n, TimeUnit units, bool modified, 
      Rate rate, DayCounterHandle dayCounter) {
        return new DepositRateHelperHandle(
            new DepositRateHelper(n,units,modified,rate,dayCounter));
    }
}

// typedef Python list of rate helpers to std::vector<Handle<RateHelper> >

%{
typedef std::vector<Handle<RateHelper> > RateHelperHandleList;
%}

%typemap(python,in) RateHelperHandleList (RateHelperHandleList temp),
                    RateHelperHandleList * (RateHelperHandleList temp),
                    const RateHelperHandleList & (RateHelperHandleList temp) {
	if (PyTuple_Check($source) || PyList_Check($source)) {
		int size = (PyTuple_Check($source) ?
		            PyTuple_Size($source) :
		            PyList_Size($source));
        temp = RateHelperHandleList(size);
        $target = &temp;
		for (int i=0; i<size; i++) {
			RateHelperHandle* h;
			PyObject* o = PySequence_GetItem($source,i);
			if ((SWIG_ConvertPtr(o,(void **) &h,
			 (swig_type_info *)SWIG_TypeQuery("RateHelperHandle *"),1)) != -1) {
				temp[i] = *h;
                Py_DECREF(o);
			} else {
				PyErr_SetString(PyExc_TypeError,
				    "the sequence must contain rate helpers");
                Py_DECREF(o);
				return NULL;
			}
		}
	} else {
		PyErr_SetString(PyExc_TypeError,"not a sequence");
		return NULL;
	}
};


#endif
