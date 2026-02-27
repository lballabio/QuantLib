/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2003 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file blackvariancecurve.hpp
    \brief Black volatility curve modelled as variance curve
*/

#ifndef quantlib_black_variance_curve_hpp
#define quantlib_black_variance_curve_hpp

#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <ql/math/interpolation.hpp>

namespace QuantLib {

    //! Black volatility curve modelled as variance curve
    /*! This class calculates time-dependent Black volatilities using
        as input a vector of (ATM) Black volatilities observed in the
        market.

        The calculation is performed interpolating on the variance curve.
        Linear interpolation is used as default; this can be changed
        by the setInterpolation() method.

        For strike dependence, see BlackVarianceSurface.

        \todo check time extrapolation

    */
    class BlackVarianceCurve : public BlackVarianceTermStructure {
      public:
        BlackVarianceCurve(const Date& referenceDate,
                           const std::vector<Date>& dates,
                           const std::vector<Volatility>& blackVolCurve,
                           DayCounter dayCounter,
                           bool forceMonotoneVariance = true,
                           BlackVolTimeExtrapolation timeExtrapolation = BlackVolTimeExtrapolation::FlatVolatility);
        //! \name TermStructure interface
        //@{
        DayCounter dayCounter() const override { return dayCounter_; }
        Date maxDate() const override;
        //@}
        //! \name VolatilityTermStructure interface
        //@{
        Real minStrike() const override;
        Real maxStrike() const override;
        //@}
        //! \name Modifiers
        //@{
        template <class Interpolator>
        void setInterpolation(const Interpolator& i = Interpolator()) {
            varianceCurve_ = i.interpolate(times_.begin(), times_.end(),
                                           variances_.begin());
            varianceCurve_.update();
            notifyObservers();
        }
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      protected:
        Real blackVarianceImpl(Time t, Real) const override;

      private:
        DayCounter dayCounter_;
        Date maxDate_;
        std::vector<Time> times_;
        std::vector<Real> variances_;
        Interpolation varianceCurve_;
        BlackVolTimeExtrapolation timeExtrapolation_;
    };


    // inline definitions

    inline Date BlackVarianceCurve::maxDate() const {
        return maxDate_;
    }

    inline Real BlackVarianceCurve::minStrike() const {
        return QL_MIN_REAL;
    }

    inline Real BlackVarianceCurve::maxStrike() const {
        return QL_MAX_REAL;
    }

    inline void BlackVarianceCurve::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<BlackVarianceCurve>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            BlackVarianceTermStructure::accept(v);
    }

}

#endif
