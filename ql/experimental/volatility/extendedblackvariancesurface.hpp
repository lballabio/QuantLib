/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Frank Hövermann

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

/*! \file extendedblackvariancesurface.hpp
    \brief Black volatility surface modelled as variance surface
*/

#ifndef quantlib_extended_black_variance_surface_hpp
#define quantlib_extended_black_variance_surface_hpp

#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <ql/math/matrix.hpp>
#include <ql/math/interpolations/interpolation2d.hpp>
#include <ql/handle.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    //! Black volatility surface modelled as variance surface
    /*! This class is similar to BlackVarianceSurface, but extends it
        to use quotes for the input volatilities.
    */
    class ExtendedBlackVarianceSurface : public BlackVarianceTermStructure {
      public:
        enum Extrapolation { ConstantExtrapolation,
                             InterpolatorDefaultExtrapolation };
        ExtendedBlackVarianceSurface(
            const Date& referenceDate,
            const Calendar& calendar,
            const std::vector<Date>& dates,
            std::vector<Real> strikes,
            const std::vector<Handle<Quote> >& volatilities,
            DayCounter dayCounter,
            Extrapolation lowerExtrapolation = InterpolatorDefaultExtrapolation,
            Extrapolation upperExtrapolation = InterpolatorDefaultExtrapolation);
        DayCounter dayCounter() const override { return dayCounter_; }
        Date maxDate() const override { return maxDate_; }
        Real minStrike() const override { return strikes_.front(); }
        Real maxStrike() const override { return strikes_.back(); }
        template <class Interpolator>
        void setInterpolation(const Interpolator& i = Interpolator()) {
            varianceSurface_ =
                i.interpolate(times_.begin(), times_.end(),
                              strikes_.begin(), strikes_.end(),
                              variances_);
            varianceSurface_.update();
            notifyObservers();
        }
        void accept(AcyclicVisitor&) override;
        void update() override;

      private:
        Real blackVarianceImpl(Time t, Real strike) const override;
        void setVariances();
        DayCounter dayCounter_;
        Date maxDate_;
        const std::vector<Handle<Quote> >& volatilities_;
        std::vector<Real> strikes_;
        std::vector<Time> times_;
        Matrix variances_;
        Interpolation2D varianceSurface_;
        Extrapolation lowerExtrapolation_, upperExtrapolation_;
    };

    inline void ExtendedBlackVarianceSurface::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<ExtendedBlackVarianceSurface>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            BlackVarianceTermStructure::accept(v);
    }

}

#endif


#ifndef id_b7627ff36d72ace75804e266006cea1b
#define id_b7627ff36d72ace75804e266006cea1b
inline bool test_b7627ff36d72ace75804e266006cea1b(int* i) { return i != 0; }
#endif
