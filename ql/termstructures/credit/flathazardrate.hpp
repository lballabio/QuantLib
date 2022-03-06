/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2008 StatPro Italia srl

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

/*! \file flathazardrate.hpp
    \brief flat hazard-rate term structure
*/

#ifndef quantlib_flat_hazard_rate_hpp
#define quantlib_flat_hazard_rate_hpp

#include <ql/termstructures/credit/hazardratestructure.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    //! Flat hazard-rate curve
    /*! \ingroup defaultprobabilitytermstructures */
    class FlatHazardRate : public HazardRateStructure {
      public:
        //! \name Constructors
        //@{
        FlatHazardRate(const Date& referenceDate, Handle<Quote> hazardRate, const DayCounter&);
        FlatHazardRate(const Date& referenceDate,
                       Rate hazardRate,
                       const DayCounter&);
        FlatHazardRate(Natural settlementDays,
                       const Calendar& calendar,
                       Handle<Quote> hazardRate,
                       const DayCounter&);
        FlatHazardRate(Natural settlementDays,
                       const Calendar& calendar,
                       Rate hazardRate,
                       const DayCounter&);
        //@}
        //! \name TermStructure interface
        //@{
        Date maxDate() const override { return Date::maxDate(); }
        //@}
      private:
        //! \name HazardRateStructure interface
        //@{
        Rate hazardRateImpl(Time) const override { return hazardRate_->value(); }
        //@}

        //! \name DefaultProbabilityTermStructure interface
        //@{
        Probability survivalProbabilityImpl(Time) const override;
        //@}

        Handle<Quote> hazardRate_;
    };

    // inline definitions

    inline Probability FlatHazardRate::survivalProbabilityImpl(Time t) const {
        return std::exp(-hazardRate_->value()*t);
    }

}

#endif


#ifndef id_abe9d901afee2e114396951b720a92f7
#define id_abe9d901afee2e114396951b720a92f7
inline bool test_abe9d901afee2e114396951b720a92f7(int* i) { return i != 0; }
#endif
