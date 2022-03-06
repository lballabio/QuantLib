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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file hestonblackvolsurface.hpp
    \brief Black volatility surface back by Heston model
*/

#ifndef quantlib_heston_black_vol_surface_hpp
#define quantlib_heston_black_vol_surface_hpp


#include <ql/models/equity/hestonmodel.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>
#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>

namespace QuantLib {
    class HestonBlackVolSurface : public BlackVolTermStructure {
      public:
        explicit HestonBlackVolSurface(
            const Handle<HestonModel>& hestonModel,
            AnalyticHestonEngine::ComplexLogFormula cpxLogFormula = AnalyticHestonEngine::Gatheral,
            AnalyticHestonEngine::Integration integration =
                AnalyticHestonEngine::Integration::gaussLaguerre(164));

        DayCounter dayCounter() const override;
        Date maxDate() const override;
        Real minStrike() const override;
        Real maxStrike() const override;

      protected:
        Real blackVarianceImpl(Time t, Real strike) const override;
        Volatility blackVolImpl(Time t, Real strike) const override;

      private:
        const Handle<HestonModel> hestonModel_;
        const AnalyticHestonEngine::ComplexLogFormula cpxLogFormula_;
        const AnalyticHestonEngine::Integration integration_;
    };
}

#endif




#ifndef id_0b2021109ccf2684b00033c83911576e
#define id_0b2021109ccf2684b00033c83911576e
inline bool test_0b2021109ccf2684b00033c83911576e(int* i) { return i != 0; }
#endif
