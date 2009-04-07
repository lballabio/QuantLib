/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ferdinando Ametrano
 Copyright (C) 2009 StatPro Italia srl

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

/*! \file yield.hpp
    \brief yield-related bond calculations
*/

#ifndef quantlib_yield_hpp
#define quantlib_yield_hpp

#include <ql/types.hpp>
#include <ql/compounding.hpp>
#include <ql/time/frequency.hpp>
#include <ql/time/date.hpp>

namespace QuantLib {

    // forward declarations
    class Bond;
    class DayCounter;

    //! clean bond price given a yield
    /*! Yield compounding, frequency, daycount are taken into account
        The default bond settlement is used if no date is given.
    */
    Real cleanPriceFromYield(
                   const Bond& bond,
                   Rate yield,
                   const DayCounter& dayCounter,
                   Compounding compounding,
                   Frequency frequency,
                   Date settlementDate = Date());

    //! Yield given a clean price
    /*! The default bond settlement is used if no date is given. */
    Rate yieldFromCleanPrice(
                   const Bond& bond,
                   Real cleanPrice,
                   const DayCounter& dayCounter,
                   Compounding compounding,
                   Frequency frequency,
                   Date settlementDate = Date(),
                   Real accuracy = 1.0e-8,
                   Size maxEvaluations = 100);

}

#endif
