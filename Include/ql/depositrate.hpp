
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
    Revision 1.1  2001/04/09 14:03:54  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.2  2001/04/06 18:46:19  nando
    changed Authors, Contributors, Licence and copyright header

*/

/*! \file depositrate.hpp
    \brief Deposit rate
*/

#ifndef quantlib_depositrate_h
#define quantlib_depositrate_h

#include "ql/qldefines.hpp"
#include "ql/date.hpp"
#include "ql/daycounter.hpp"
#include "ql/calendar.hpp"
#include "ql/rate.hpp"

namespace QuantLib {

    //! %deposit rate
    class DepositRate {
      public:
        DepositRate() {}
        DepositRate(const Date& maturity,
                    Rate rate,
                    const Handle<DayCounter>& dayCounter)
        : maturity_(maturity), rate_(rate), dayCounter_(dayCounter) {}
        //! \name Inspectors
        //@{
        Date maturity() const { return maturity_; }
        Rate rate() const { return rate_; }
        Handle<DayCounter> dayCounter() const { return dayCounter_; }
        //@}
      private:
        Date maturity_;
        Rate rate_;
        Handle<DayCounter> dayCounter_;
    };

}


#endif
