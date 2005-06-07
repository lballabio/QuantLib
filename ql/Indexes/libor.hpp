/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file libor.hpp
    \brief base class for BBA LIBOR indexes
*/

#ifndef quantlib_libor_hpp
#define quantlib_libor_hpp

#include <ql/Indexes/xibor.hpp>

namespace QuantLib {

    //! base class for BBA LIBOR indexes
    class Libor : public Xibor {
      public:
        Libor(const std::string& familyName,
              Integer n, TimeUnit units, Integer settlementDays,
              const Currency& currency,
              const Calendar& localCalendar,
              const Calendar& currencyCalendar,
              BusinessDayConvention convention,
              const DayCounter& dayCounter,
              const Handle<YieldTermStructure>& h);
        /*! \name Date calculations

            see http://www.bba.org.uk/bba/jsp/polopoly.jsp?d=225&a=1412
            @{
        */
        Date valueDate(const Date& fixingDate) const;
        Date maturityDate(const Date& valueDate) const;
        // @}
      private:
        Calendar localCalendar_;
        Calendar currencyCalendar_;
    };

}


#endif
