
/*
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

/*! \file gbplibor.hpp
    \brief %GBP %Libor index
*/

#ifndef quantlib_gbp_libor_hpp
#define quantlib_gbp_libor_hpp

#include <ql/Indexes/xibor.hpp>
#include <ql/Calendars/london.hpp>
#include <ql/DayCounters/actual365.hpp>

namespace QuantLib {

    //! %GBP %Libor index
    class GBPLibor : public Xibor {
      public:
        GBPLibor(Integer n, TimeUnit units,
                 const RelinkableHandle<TermStructure>& h,
                 const DayCounter& dc = Actual365())
        : Xibor("GBPLibor", n, units, 0, GBP,
                GreatBritain(), true, ModifiedFollowing, dc, h) {}
    };

}


#endif
