
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
    \brief purely virtual base class for libor indexes

    \fullpath
    ql/Indexes/%xibor.hpp
*/

// $Id$

#ifndef quantlib_xibor_hpp
#define quantlib_xibor_hpp

#include <ql/index.hpp>
#include <ql/termstructure.hpp>

namespace QuantLib {

    namespace Indexes {

        //! base class for libor indexes
        class Xibor : public Index {
          public:
            Xibor(const std::string& familyName,
                int n, TimeUnit units, int settlementDays,
                Currency currency,
                const Calendar& calendar, bool isAdjusted,
                RollingConvention rollingConvention,
                const DayCounter& dayCounter,
                const RelinkableHandle<TermStructure>& h)
            : familyName_(familyName), n_(n), units_(units),
              settlementDays_(settlementDays),
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
            int settlementDays() const { return settlementDays_; }
            Currency currency() const { return currency_; }
            Calendar calendar() const { return calendar_; }
            bool isAdjusted() const { return isAdjusted_; }
            RollingConvention rollingConvention() const {
                return rollingConvention_; }
            DayCounter dayCounter() const { return dayCounter_; }
            //@}
          private:
            std::string familyName_;
            int n_;
            TimeUnit units_;
            int settlementDays_;
            Currency currency_;
            Calendar calendar_;
            bool isAdjusted_;
            RollingConvention rollingConvention_;
            DayCounter dayCounter_;
            RelinkableHandle<TermStructure> termStructure_;
        };

    }

}


#endif
