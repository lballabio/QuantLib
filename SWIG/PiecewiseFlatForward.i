
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

#ifndef quantlib_piecewise_flat_forward_i
#define quantlib_piecewise_flat_forward_i

%include TermStructures.i
%include RateHelpers.i

%{
using QuantLib::TermStructures::PiecewiseFlatForward;
typedef Handle<TermStructure> PiecewiseFlatForwardHandle;
%}

// actually they are the same class, but I'll fake inheritance...
%name(PiecewiseFlatForward) class PiecewiseFlatForwardHandle
: public TermStructureHandle {
  public:
    // constructor redefined below
    ~PiecewiseFlatForwardHandle();
};

%addmethods PiecewiseFlatForwardHandle {
    PiecewiseFlatForwardHandle(CurrencyHandle currency,
        DayCounterHandle dayCounter, Date today, 
        RateHelperHandleList instruments) {
	        return new PiecewiseFlatForwardHandle(
	            new PiecewiseFlatForward(currency, dayCounter, today, 
	                instruments));
    }
}


#endif
