/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Johannes Göttker-Schnetmann
 Copyright (C) 2015 Klaus Spanderen

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

/*! \file fixedlocalvolsurface.hpp
    \brief Local volatility surface based on fixed values plus interpolation
*/

#ifndef quantlib_fixed_local_vol_surface_hpp
#define quantlib_fixed_local_vol_surface_hpp

#include <ql/math/matrix.hpp>
#include <ql/math/interpolation.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>

#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>

namespace QuantLib {

    class FixedLocalVolSurface : public LocalVolTermStructure {
      public:
        enum Extrapolation { ConstantExtrapolation,
                             InterpolatorDefaultExtrapolation };
        FixedLocalVolSurface(const Date& referenceDate,
                             const std::vector<Date>& dates,
                             const std::vector<Real>& strikes,
                             ext::shared_ptr<Matrix> localVolMatrix,
                             const DayCounter& dayCounter,
                             Extrapolation lowerExtrapolation = ConstantExtrapolation,
                             Extrapolation upperExtrapolation = ConstantExtrapolation);

        FixedLocalVolSurface(const Date& referenceDate,
                             const std::vector<Time>& times,
                             const std::vector<Real>& strikes,
                             ext::shared_ptr<Matrix> localVolMatrix,
                             const DayCounter& dayCounter,
                             Extrapolation lowerExtrapolation = ConstantExtrapolation,
                             Extrapolation upperExtrapolation = ConstantExtrapolation);

        FixedLocalVolSurface(const Date& referenceDate,
                             const std::vector<Time>& times,
                             const std::vector<ext::shared_ptr<std::vector<Real> > >& strikes,
                             ext::shared_ptr<Matrix> localVolMatrix,
                             const DayCounter& dayCounter,
                             Extrapolation lowerExtrapolation = ConstantExtrapolation,
                             Extrapolation upperExtrapolation = ConstantExtrapolation);


        Date maxDate() const override;
        Time maxTime() const override;
        Real minStrike() const override;
        Real maxStrike() const override;

        template <class Interpolator>
        void setInterpolation(const Interpolator& i = Interpolator()) {
            for (Size j=0; j < times_.size(); ++j) {
                localVolInterpol_[j] = i.interpolate(
                    strikes_[j]->begin(), strikes_[j]->end(),
                    localVolMatrix_->column_begin(j));
            }
            notifyObservers();
        }

      protected:
        Volatility localVolImpl(Time t, Real strike) const override;

        const Date maxDate_;
        std::vector<Time> times_;
        const ext::shared_ptr<Matrix> localVolMatrix_;
        const std::vector<ext::shared_ptr<std::vector<Real> > > strikes_;

        std::vector<Interpolation> localVolInterpol_;
        Extrapolation lowerExtrapolation_, upperExtrapolation_;

      private:
        void checkSurface();
    };
}

#endif
