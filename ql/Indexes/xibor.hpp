
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
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
