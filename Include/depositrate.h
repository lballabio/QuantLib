
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
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
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file depositrate.h
    \brief Deposit rate

    $Source$
    $Log$
    Revision 1.2  2001/03/07 10:34:25  nando
    added ratehelper.cpp and ratehelper.h.
    Borland, Linux and Visual C++ updated
    Also added some missing files to Visual C++

    Revision 1.1  2001/01/18 16:22:05  nando
    deposit file and class renamed to DepositRate

    Revision 1.7  2001/01/17 14:37:54  nando
    tabs removed

    Revision 1.6  2000/12/14 12:32:29  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#ifndef quantlib_depositrate_h
#define quantlib_depositrate_h

#include "qldefines.h"
#include "date.h"
#include "daycounter.h"
#include "calendar.h"
#include "rate.h"

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
