/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Chiara Fornarola
 Copyright (C) 2007, 2008, 2009 Ferdinando Ametrano
 Copyright (C) 2009 StatPro Italia srl
 Copyright (C) 2009 Nathan Abbott

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

/*! \file zspread.hpp
    \brief z-spread-related bond calculations
*/

#ifndef quantlib_z_spread_hpp
#define quantlib_z_spread_hpp

#include <ql/types.hpp>
#include <ql/compounding.hpp>
#include <ql/time/frequency.hpp>
#include <ql/time/date.hpp>
#include <boost/shared_ptr.hpp>

namespace QuantLib {

    // forward declarations
    class Bond;
    class DayCounter;
    class YieldTermStructure;

    //! clean bond price given a discount curve and a z-spread
    /*! Z-spread compounding, frequency, daycount are taken into account
        The default bond settlement is used if no date is given.
        For details on z-spread refer to:
        "Credit Spreads Explained", Lehman Brothers European Fixed
        Income Research - March 2004, D. O'Kane
    */
    Real cleanPriceFromZSpread(
                   const Bond& bond,
                   const boost::shared_ptr<YieldTermStructure>& discountCurve,
                   Spread zSpread,
                   const DayCounter& dayCounter,
                   Compounding compounding,
                   Frequency frequency,
                   Date settlementDate = Date());

    //! Z-spread given a discount curve and a clean price
    /*! The default bond settlement is used if no date is given. */
    Spread zSpreadFromCleanPrice(
                   const Bond& bond,
                   const boost::shared_ptr<YieldTermStructure>& discountCurve,
                   Real cleanPrice,
                   const DayCounter& dayCounter,
                   Compounding compounding,
                   Frequency frequency,
                   Date settlementDate = Date(),
                   Real accuracy = 1.0e-10,
                   Size maxEvaluations = 100);

}

#endif
