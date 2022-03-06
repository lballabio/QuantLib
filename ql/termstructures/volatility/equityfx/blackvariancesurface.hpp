/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2003, 2004 StatPro Italia srl

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

/*! \file blackvariancesurface.hpp
    \brief Black volatility surface modelled as variance surface
*/

#ifndef quantlib_black_variance_surface_hpp
#define quantlib_black_variance_surface_hpp

#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <ql/math/matrix.hpp>
#include <ql/math/interpolations/interpolation2d.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>

namespace QuantLib {

    //! Black volatility surface modelled as variance surface
    /*! This class calculates time/strike dependent Black volatilities
        using as input a matrix of Black volatilities observed in the
        market.

        The calculation is performed interpolating on the variance
        surface.  Bilinear interpolation is used as default; this can
        be changed by the setInterpolation() method.

        \todo check time extrapolation

    */
    class BlackVarianceSurface : public BlackVarianceTermStructure {
      public:
        enum Extrapolation { ConstantExtrapolation,
                             InterpolatorDefaultExtrapolation };
        BlackVarianceSurface(const Date& referenceDate,
                             const Calendar& cal,
                             const std::vector<Date>& dates,
                             std::vector<Real> strikes,
                             const Matrix& blackVolMatrix,
                             DayCounter dayCounter,
                             Extrapolation lowerExtrapolation = InterpolatorDefaultExtrapolation,
                             Extrapolation upperExtrapolation = InterpolatorDefaultExtrapolation);
        //! \name TermStructure interface
        //@{
        DayCounter dayCounter() const override { return dayCounter_; }
        Date maxDate() const override { return maxDate_; }
        //@}
        //! \name VolatilityTermStructure interface
        //@{
        Real minStrike() const override { return strikes_.front(); }
        Real maxStrike() const override { return strikes_.back(); }
        //@}
        //! \name Modifiers
        //@{
        template <class Interpolator>
        void setInterpolation(const Interpolator& i = Interpolator()) {
            varianceSurface_ =
                i.interpolate(times_.begin(), times_.end(),
                              strikes_.begin(), strikes_.end(),
                              variances_);
            notifyObservers();
        }
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      protected:
        Real blackVarianceImpl(Time t, Real strike) const override;

      private:
        DayCounter dayCounter_;
        Date maxDate_;
        std::vector<Real> strikes_;
        std::vector<Time> times_;
        Matrix variances_;
        Interpolation2D varianceSurface_;
        Extrapolation lowerExtrapolation_, upperExtrapolation_;
    };


    // inline definitions

    inline void BlackVarianceSurface::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<BlackVarianceSurface>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            BlackVarianceTermStructure::accept(v);
    }

}


#endif


#ifndef id_f3173b6a0d2a7825ed52647e00f7cd60
#define id_f3173b6a0d2a7825ed52647e00f7cd60
inline bool test_f3173b6a0d2a7825ed52647e00f7cd60(int* i) { return i != 0; }
#endif
