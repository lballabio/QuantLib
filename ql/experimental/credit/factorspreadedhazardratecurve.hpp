/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters

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

/*! \file factorspreadedhazardratecurve.hpp
    \brief Default-probability structure with a multiplicative spread on hazard rates
*/

#ifndef quantlib_factor_spreaded_hazard_rate_curve_hpp
#define quantlib_factor_spreaded_hazard_rate_curve_hpp

#include <ql/quote.hpp>
#include <ql/termstructures/credit/hazardratestructure.hpp>
#include <utility>

namespace QuantLib {

    //! Default-probability structure with a multiplicative spread on hazard rates
    /*! \note This term structure will remain linked to the original
              structure, i.e., any changes in the latter will be
              reflected in this structure as well.

        \ingroup defaultprobabilitytermstructures
    */
    class FactorSpreadedHazardRateCurve : public HazardRateStructure {
      public:
        FactorSpreadedHazardRateCurve(Handle<DefaultProbabilityTermStructure> originalCurve,
                                      Handle<Quote> spread);
        //! \name DefaultTermStructure interface
        //@{
        DayCounter dayCounter() const override;
        Calendar calendar() const override;
        const Date& referenceDate() const override;
        Date maxDate() const override;
        Time maxTime() const override;
        //@}
      protected:
        Real hazardRateImpl(Time t) const override;

      private:
        Handle<DefaultProbabilityTermStructure> originalCurve_;
        Handle<Quote> spread_;
    };


    // inline definitions

    inline FactorSpreadedHazardRateCurve::FactorSpreadedHazardRateCurve(
        Handle<DefaultProbabilityTermStructure> h, Handle<Quote> spread)
    : originalCurve_(std::move(h)), spread_(std::move(spread)) {
        registerWith(originalCurve_);
        registerWith(spread_);
    }

    inline DayCounter FactorSpreadedHazardRateCurve::dayCounter() const {
        return originalCurve_->dayCounter();
    }

    inline Calendar FactorSpreadedHazardRateCurve::calendar() const {
        return originalCurve_->calendar();
    }

    inline const Date& FactorSpreadedHazardRateCurve::referenceDate() const {
        return originalCurve_->referenceDate();
    }

    inline Date FactorSpreadedHazardRateCurve::maxDate() const {
        return originalCurve_->maxDate();
    }

    inline Time FactorSpreadedHazardRateCurve::maxTime() const {
        return originalCurve_->maxTime();
    }

    inline Real FactorSpreadedHazardRateCurve::hazardRateImpl(Time t) const {
        return originalCurve_->hazardRate(t, true) * (1.0 + spread_->value());
    }

}

#endif
