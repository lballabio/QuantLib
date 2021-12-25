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

/*! \file spreadedhazardratecurve.hpp
    \brief Default-probability structure with an additive spread on hazard rates
*/

#ifndef quantlib_spreaded_hazard_rate_curve_hpp
#define quantlib_spreaded_hazard_rate_curve_hpp

#include <ql/quote.hpp>
#include <ql/termstructures/credit/hazardratestructure.hpp>
#include <utility>

namespace QuantLib {

    //! Default-probability structure with an additive spread on hazard rates
    /*! \note This term structure will remain linked to the original
              structure, i.e., any changes in the latter will be
              reflected in this structure as well.

        \ingroup termstructures
    */
    class SpreadedHazardRateCurve : public HazardRateStructure {
      public:
        SpreadedHazardRateCurve(Handle<DefaultProbabilityTermStructure> originalCurve,
                                Handle<Quote> spread);
        //! \name DefaultProbabilityTermStructure interface
        //@{
        DayCounter dayCounter() const override;
        Calendar calendar() const override;
        const Date& referenceDate() const override;
        Date maxDate() const override;
        Time maxTime() const override;
        //@}
      protected:
        //! \name DefaultProbabilityTermStructure interface
        //@{
        Real hazardRateImpl(Time t) const override;
        //@}
      private:
        Handle<DefaultProbabilityTermStructure> originalCurve_;
        Handle<Quote> spread_;
    };


    // inline definitions

    inline SpreadedHazardRateCurve::SpreadedHazardRateCurve(
        Handle<DefaultProbabilityTermStructure> h, Handle<Quote> spread)
    : originalCurve_(std::move(h)), spread_(std::move(spread)) {
        registerWith(originalCurve_);
        registerWith(spread_);
    }

    inline DayCounter SpreadedHazardRateCurve::dayCounter() const {
        return originalCurve_->dayCounter();
    }

    inline Calendar SpreadedHazardRateCurve::calendar() const {
        return originalCurve_->calendar();
    }

    inline const Date& SpreadedHazardRateCurve::referenceDate() const {
        return originalCurve_->referenceDate();
    }

    inline Date SpreadedHazardRateCurve::maxDate() const {
        return originalCurve_->maxDate();
    }

    inline Time SpreadedHazardRateCurve::maxTime() const {
        return originalCurve_->maxTime();
    }

    inline Real SpreadedHazardRateCurve::hazardRateImpl(Time t) const {
        return originalCurve_->hazardRate(t, true) + spread_->value();
    }

}

#endif
