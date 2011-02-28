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

/*! \file extendedblackvariancecurve.hpp
    \brief Black volatility curve modelled as variance curve
*/

#ifndef quantlib_extended_black_variance_curve_hpp
#define quantlib_extended_black_variance_curve_hpp

#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <ql/math/interpolation.hpp>
#include <ql/handle.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    //! Black volatility curve modelled as variance curve
    /*! This class is similar to BlackVarianceCurve, but extends it to
        use quotes for the input volatilities.
    */
    class ExtendedBlackVarianceCurve : public BlackVarianceTermStructure {
      public:
        ExtendedBlackVarianceCurve(
                              const Date& referenceDate,
                              const std::vector<Date>& dates,
                              const std::vector<Handle<Quote> >& volatilities,
                              const DayCounter& dayCounter,
                              bool forceMonotoneVariance = true);

        DayCounter dayCounter() const { return dayCounter_; }
        Date maxDate() const;
        Real minStrike() const;
        Real maxStrike() const;

        template <class Interpolator>
        void setInterpolation(const Interpolator& i = Interpolator()) {
            varianceCurve_ = i.interpolate(times_.begin(), times_.end(),
                                           variances_.begin());
            varianceCurve_.update();
            notifyObservers();
        }

        void accept(AcyclicVisitor&);
        void update();
      private:
        Real blackVarianceImpl(Time t, Real) const;
        void setVariances();
        DayCounter dayCounter_;
        Date maxDate_;
        std::vector<Handle<Quote> > volatilities_;
        std::vector<Time> times_;
        std::vector<Real> variances_;
        Interpolation varianceCurve_;
        bool forceMonotoneVariance_;
    };

    inline Date ExtendedBlackVarianceCurve::maxDate() const {
        return maxDate_;
    }

    inline Real ExtendedBlackVarianceCurve::minStrike() const {
        return QL_MIN_REAL;
    }

    inline Real ExtendedBlackVarianceCurve::maxStrike() const {
        return QL_MAX_REAL;
    }

    inline void ExtendedBlackVarianceCurve::accept(AcyclicVisitor& v) {
        Visitor<ExtendedBlackVarianceCurve>* v1 =
            dynamic_cast<Visitor<ExtendedBlackVarianceCurve>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            BlackVarianceTermStructure::accept(v);
    }
}


#endif
