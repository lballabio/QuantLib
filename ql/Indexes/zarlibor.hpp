/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

This file is part of QuantLib, a free-software/open-source library
for financial quantitative analysts and developers - http://quantlib.org/

QuantLib is free software: you can redistribute it and/or modify it
under the terms of the QuantLib license.  You should have received a
copy of the license along with this program; if not, please email
<quantlib-dev@lists.sf.net>. The license is also available online at
h<ttp://quantlib.org/reference/license.html>.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file zarlibor.hpp
    \brief %ZAR %Libor index (also known as JIBAR)
*/

#ifndef quantlib_zar_libor_hpp
#define quantlib_zar_libor_hpp

#include <ql/Indexes/xibor.hpp>
#include <ql/Calendars/johannesburg.hpp>
#include <ql/DayCounters/actual365fixed.hpp>
#include <ql/Currencies/africa.hpp>

namespace QuantLib {

    //! %ZAR %Libor index, also known as JIBAR
    /*! \todo check settlement days */
    class ZARLibor : public Xibor {
      public:
        ZARLibor(Integer n, TimeUnit units,
                 const Handle<YieldTermStructure>& h,
                 const DayCounter& dc = Actual365Fixed())
        : Xibor("ZARLibor", n, units, 0, ZARCurrency(),
                Johannesburg(), ModifiedFollowing, dc, h) {}
    };

}


#endif
