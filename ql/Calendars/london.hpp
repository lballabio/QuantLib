
/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file london.hpp
    \brief London calendar
*/

#ifndef quantlib_london_calendar_h
#define quantlib_london_calendar_h

#include <ql/Calendars/unitedkingdom.hpp>

namespace QuantLib {

    #if !defined(QL_DISABLE_DEPRECATED)
    //! \deprecated use UnitedKingdom with the Exchange market
    class London : public UnitedKingdom {
      public:
        London() : UnitedKingdom(UnitedKingdom::Exchange) {}
    };
    #endif

}


#endif
