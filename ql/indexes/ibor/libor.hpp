/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Chiara Fornarola
 Copyright (C) 2005, 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file libor.hpp
    \brief base class for BBA LIBOR indexes
*/

#ifndef quantlib_libor_hpp
#define quantlib_libor_hpp

#include <ql/indexes/iborindex.hpp>

namespace QuantLib {

    //! base class for all BBA LIBOR indexes but the EUR ones
    /*! LIBOR fixed by BBA.

        See <http://www.bba.org.uk/bba/jsp/polopoly.jsp?d=225&a=1414>.

        \warning This is not a valid base class for the O/N, S/N index
    */
    class Libor : public IborIndex {
      public:
        Libor(const std::string& familyName,
              const Period& tenor,
              Natural settlementDays,
              const Currency& currency,
              const Calendar& financialCenterCalendar,
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
        Calendar jointCalendar_;
    };

}

#endif
