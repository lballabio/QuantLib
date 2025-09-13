/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file gridmodellocalvolsurface.hpp
    \brief Parameterized volatility surface useful for model calibration
*/

#ifndef quantlib_grid_model_local_vol_surface_hpp
#define quantlib_grid_model_local_vol_surface_hpp

#include <ql/models/model.hpp>
#include <ql/termstructures/volatility/equityfx/fixedlocalvolsurface.hpp>

namespace QuantLib {

    class GridModelLocalVolSurface
         : public LocalVolTermStructure,
           public CalibratedModel {
      public:
        typedef FixedLocalVolSurface::Extrapolation Extrapolation;

        GridModelLocalVolSurface(
            const Date& referenceDate,
            const std::vector<Date>& dates,
            const std::vector<ext::shared_ptr<std::vector<Real> > >& strikes,
            const DayCounter& dayCounter,
            Extrapolation lowerExtrapolation
                = FixedLocalVolSurface::ConstantExtrapolation,
            Extrapolation upperExtrapolation
                = FixedLocalVolSurface::ConstantExtrapolation);

        void update() override;

        Date maxDate() const override;
        Time maxTime() const override;
        Real minStrike() const override;
        Real maxStrike() const override;

      protected:
        void generateArguments() override;
        Volatility localVolImpl(Time t, Real strike) const override;

        const Date referenceDate_;
        std::vector<Time> times_;
        const std::vector<ext::shared_ptr<std::vector<Real> > > strikes_;
        const DayCounter dayCounter_;
        Extrapolation lowerExtrapolation_, upperExtrapolation_;

        ext::shared_ptr<LocalVolTermStructure> localVol_;
    };
}

#endif
