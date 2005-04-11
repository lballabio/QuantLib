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

/*! \file zibor.hpp
    \brief %CHF %ZIBOR rate
*/

#ifndef quantlib_zibor_hpp
#define quantlib_zibor_hpp

#include <ql/Indexes/xibor.hpp>
#include <ql/Calendars/zurich.hpp>
#include <ql/DayCounters/actual360.hpp>
#include <ql/Currencies/europe.hpp>

namespace QuantLib {

    //! %CHF %ZIBOR rate
    /*! Zurich Interbank Offered Rate.

        \warning This is the rate fixed in Zurich by BBA. Use CHFLibor if
                 you're interested in the London fixing by BBA.

        \todo check settlement days and day-count.
    */
    class Zibor : public Xibor {
      public:
        Zibor(Integer n, TimeUnit units,
              const Handle<YieldTermStructure>& h,
              const DayCounter& dc = Actual360())
        : Xibor("Zibor", n, units, 2, CHFCurrency(),
                Zurich(), ModifiedFollowing, dc, h) {}
    };

}


#endif
