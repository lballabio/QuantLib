
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
    \fullpath Include/ql/%depositrate.hpp
    \brief Deposit rate

*/

// $Id$
// $Log$
// Revision 1.10  2001/08/09 14:59:45  sigmud
// header modification
//
// Revision 1.9  2001/08/08 11:07:48  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.8  2001/08/07 11:25:53  sigmud
// copyright header maintenance
//
// Revision 1.7  2001/07/25 15:47:27  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.6  2001/07/24 08:49:32  sigmud
// pruned redundant header inclusions
//
// Revision 1.5  2001/07/05 15:57:22  lballabio
// Collected typedefs in a single file
//
// Revision 1.4  2001/06/25 10:04:01  nando
// R019-branch-merge5 merged into trunk
//
// Revision 1.3  2001/05/24 15:38:07  nando
// smoothing #include xx.hpp and cutting old Log messages
//

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
