
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file depositrate.hpp
    \brief Deposit rate

    \fullpath
    ql/%depositrate.hpp
*/

// $Id$

#ifndef quantlib_depositrate_h
#define quantlib_depositrate_h

#include "ql/daycounter.hpp"
#include "ql/calendar.hpp"

namespace QuantLib {

    //! %deposit rate
    /*! \ingroup deprecated
        \deprecated
        this class was used for building the PiecewiseConstantForwards term
        structure. The DepositRateHelper and PiecewiseFlatForward classes
        should be used instead.
    */
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
