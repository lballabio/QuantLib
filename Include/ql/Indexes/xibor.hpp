
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

/*! \file xibor.hpp
    \fullpath Include/ql/Indexes/%xibor.hpp
    \brief purely virtual base class for libor indexes

*/

// $Id$
// $Log$
// Revision 1.12  2001/08/09 14:59:46  sigmud
// header modification
//
// Revision 1.11  2001/08/08 11:07:48  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.10  2001/08/07 11:25:53  sigmud
// copyright header maintenance
//
// Revision 1.9  2001/07/25 15:47:27  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.8  2001/06/18 11:52:37  lballabio
// Fixed name used for history lookup
//
// Revision 1.7  2001/06/18 08:05:59  lballabio
// Reworked indexes and floating rate coupon
//
// Revision 1.6  2001/06/15 13:52:07  lballabio
// Reworked indexes
//
// Revision 1.5  2001/06/01 16:50:16  lballabio
// Term structure on deposits and swaps
//
// Revision 1.4  2001/05/29 09:24:06  lballabio
// Using relinkable handle to term structure
//
// Revision 1.3  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_xibor_hpp
#define quantlib_xibor_hpp

#include "ql/index.hpp"
#include "ql/termstructure.hpp"

namespace QuantLib {

    namespace Indexes {

        //! base class for libor indexes
        class Xibor : public Index {
          public:
            Xibor(const std::string& familyName, 
                int n, TimeUnit units, Currency currency, 
                const Handle<Calendar>& calendar, bool isAdjusted, 
                RollingConvention rollingConvention,
                const Handle<DayCounter>& dayCounter,
                const RelinkableHandle<TermStructure>& h)
            : familyName_(familyName), n_(n), units_(units), 
              currency_(currency), calendar_(calendar), 
              isAdjusted_(isAdjusted), 
              rollingConvention_(rollingConvention), 
              dayCounter_(dayCounter), termStructure_(h) {}
            //! \name Index interface
            //@{
            Rate fixing(const Date& fixingDate) const;
            //@}
            //! \name Inspectors
            //@{
            std::string name() const;
            Period tenor() const { return Period(n_,units_); }
            Currency currency() const { return currency_; }
            Handle<Calendar> calendar() const { return calendar_; }
            bool isAdjusted() const { return isAdjusted_; }
            RollingConvention rollingConvention() const { 
                return rollingConvention_; }
            Handle<DayCounter> dayCounter() const { return dayCounter_; }
            //@}
          private:
            std::string familyName_;
            int n_;
            TimeUnit units_;
            Currency currency_;
            Handle<Calendar> calendar_;
            bool isAdjusted_;
            RollingConvention rollingConvention_;
            Handle<DayCounter> dayCounter_;
            RelinkableHandle<TermStructure> termStructure_;
        };

    }

}


#endif
