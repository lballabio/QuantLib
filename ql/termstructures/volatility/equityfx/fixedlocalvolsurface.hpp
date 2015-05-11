/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Johannes Goettker-Schnetmann
 Copyright (C) 2015 Klaus Spanderen

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

/*! \file fixedlocalvolsurface.hpp
    \brief Local volatility surface based on fixed values plus interpolation
*/

#ifndef quantlib_fixed_local_vol_surface_hpp
#define quantlib_fixed_local_vol_surface_hpp

#include <ql/math/matrix.hpp>
#include <ql/math/interpolations/interpolation2d.hpp>
#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>

namespace QuantLib {

	class FixedLocalVolSurface : public LocalVolTermStructure {
      public:
        enum Extrapolation { ConstantExtrapolation,
                             InterpolatorDefaultExtrapolation };
        FixedLocalVolSurface(const Date& referenceDate,
                             const std::vector<Date>& dates,
                             const std::vector<Real>& strikes,
                             const Matrix& localVolMatrix,
                             const DayCounter& dayCounter,
                             Extrapolation lowerExtrapolation =
                                InterpolatorDefaultExtrapolation,
                             Extrapolation upperExtrapolation =
                                InterpolatorDefaultExtrapolation);

        FixedLocalVolSurface(const Date& referenceDate,
                             const std::vector<Time>& times,
                             const std::vector<Real>& strikes,
                             const Matrix& localVolMatrix,
                             const DayCounter& dayCounter,
                             Extrapolation lowerExtrapolation =
                                InterpolatorDefaultExtrapolation,
                             Extrapolation upperExtrapolation =
                                InterpolatorDefaultExtrapolation);

        Date maxDate() const;
        Date minDate() const;
        Time maxTime() const;
        Time minTime() const;
        Real minStrike() const;
        Real maxStrike() const;

        template <class Interpolator>
        void setInterpolation(const Interpolator& i = Interpolator()) {
            localVolSurface_ =
                i.interpolate(times_.begin(), times_.end(),
                              strikes_.begin(), strikes_.end(),
                              localVolMatrix_);
            notifyObservers();
        }

        Matrix & matrix() {return localVolMatrix_;}

      protected:
        Volatility localVolImpl(Time t, Real strike) const;

        const Date maxDate_;
        const Date minDate_;
        const std::vector<Real> strikes_;
        Matrix localVolMatrix_;
        std::vector<Time> times_;
        Interpolation2D localVolSurface_;
        Extrapolation lowerExtrapolation_, upperExtrapolation_;

      private:
        void checkSurface();
    };
}

#endif
