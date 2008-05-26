/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Stamm

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

/*! \file cdsoption.hpp
    \brief CDS option
*/

#ifndef quantlib_cds_option_hpp
#define quantlib_cds_option_hpp

#include <ql/instrument.hpp>
#include <ql/issuer.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    class Quote;
    class YieldTermStructure;

    //! CDS option
    /*! \warning the current implementation does not take premium
                 accrual into account

        \warning the current implementation quietly assumes that the
                 expiry equals the start date of the underlying CDS

        \todo take premium accrual into account

        \todo allow expiry to be different from CDS start date
    */
    class CdsOption : public Instrument {
      public:
        CdsOption (const Date& expiry,
                   Rate strike,
                   const Handle<Quote>& volatility,
                   const Issuer& issuer,
                   Protection::Side side,
                   Real nominal,
                   const Schedule& premiumSchedule,
                   const DayCounter& dayCounter,
                   bool settlePremiumAccrual,
                   const Handle<YieldTermStructure>& yieldTS);

        Real forward() const;
        Real riskyAnnuity() const;
        bool isExpired() const;

    private:
        void setupExpired() const;
        void performCalculations() const;

        Date expiry_;
        Rate strike_;
        Handle<Quote> volatility_;
        Issuer issuer_;
        Protection::Side side_;
        Real nominal_;
        Schedule premiumSchedule_;
        DayCounter dayCounter_;
        bool settlePremiumAccrual_;
        Handle<YieldTermStructure> yieldTS_;

        mutable Real forward_;
        mutable Real riskyAnnuity_;
    };

}

#endif
