
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
    Revision 1.1  2001/04/09 14:03:55  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.3  2001/04/06 18:46:19  nando
    changed Authors, Contributors, Licence and copyright header

*/

/*! \file chf.hpp
    \brief Swiss Franc
*/

#ifndef quantlib_CHF_h
#define quantlib_CHF_h

#include "ql/qldefines.hpp"
#include "ql/currency.hpp"
#include "ql/Calendars/zurich.hpp"

namespace QuantLib {

    namespace Currencies {

        //! Swiss Franc
        class CHF : public Currency {
          public:
            CHF() {}
            std::string name() const { return std::string("CHF"); }
            //! returns a handle to the Zurich calendar
            Handle<Calendar> settlementCalendar() const {
                return Handle<Calendar>(new Calendars::Zurich); }
            //! returns 2
            int settlementDays() const { return 2; }
        };

    }

}


#endif
