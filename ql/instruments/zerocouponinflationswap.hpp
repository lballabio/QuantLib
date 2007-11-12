/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Chris Kenyon

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

/*! \file zciis.hpp
    \brief Zero-coupon inflation-indexed swap
*/

#ifndef quantlib_zciis_hpp
#define quantlib_zciis_hpp

#include <ql/instruments/inflationswap.hpp>

namespace QuantLib {

    //! Zero-coupon inflation-indexed swap
    /*! A ZCIIS pays a fixed rate and receives the inflation rate at
        date \f$ d2 \f$ relative to inflation at date \f$ d1 \f$,
        where \f$ d1 \f$ is a lag period before start date and \f$ d2 \f$
        is a lag period before maturity.
    */
    class ZeroCouponInflationSwap : public InflationSwap {
      public:
        ZeroCouponInflationSwap(
                       const Date& start,
                       const Date& maturity,
                       const Period& lag,
                       Rate fixedRate,
                       const Calendar& calendar,
                       BusinessDayConvention convention,
                       const DayCounter& dayCounter,
                       const Handle<YieldTermStructure>& yieldTS,
                       const Handle<ZeroInflationTermStructure>& inflationTS);

        //! \name Instrument interface
        //@{
        bool isExpired() const;
        //@}

        //! \name InflationSwap interface
        //@{
        Rate fairRate() const;
        //@}

        //! \name Inspectors
        //@{
        Rate fixedRate() const;
        //@}

      protected:
        //! \name Instrument interface
        //@{
        void performCalculations() const;
        //@}

        Rate fixedRate_;
        Handle<ZeroInflationTermStructure> inflationTS_;
    };

}


#endif
