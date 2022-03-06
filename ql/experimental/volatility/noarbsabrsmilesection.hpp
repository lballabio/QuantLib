/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Peter Caspers

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

/*! \file noarbsabrsmilesection.hpp
    \brief no arbitrage sabr smile section
*/

#ifndef quantlib_noarbsabr_smile_section_hpp
#define quantlib_noarbsabr_smile_section_hpp

#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/experimental/volatility/noarbsabr.hpp>
#include <vector>

namespace QuantLib {

class NoArbSabrSmileSection : public SmileSection {

  public:
    NoArbSabrSmileSection(Time timeToExpiry,
                          Rate forward,
                          std::vector<Real> sabrParameters,
                          Real shift = 0.0,
                          VolatilityType volatilityType = VolatilityType::ShiftedLognormal);
    NoArbSabrSmileSection(const Date& d,
                          Rate forward,
                          std::vector<Real> sabrParameters,
                          const DayCounter& dc = Actual365Fixed(),
                          Real shift = 0.0,
                          VolatilityType volatilityType = VolatilityType::ShiftedLognormal);
    Real minStrike() const override { return 0.0; }
    Real maxStrike() const override { return QL_MAX_REAL; }
    Real atmLevel() const override { return forward_; }
    Real
    optionPrice(Rate strike, Option::Type type = Option::Call, Real discount = 1.0) const override;
    Real digitalOptionPrice(Rate strike,
                            Option::Type type = Option::Call,
                            Real discount = 1.0,
                            Real gap = 1.0e-5) const override;
    Real density(Rate strike, Real discount = 1.0, Real gap = 1.0E-4) const override;

    ext::shared_ptr<NoArbSabrModel> model() { return model_; }

  protected:
    Volatility volatilityImpl(Rate strike) const override;

  private:
    void init();
    ext::shared_ptr<NoArbSabrModel> model_;
    Rate forward_;
    std::vector<Real> params_;
    Real shift_;
};
}

#endif


#ifndef id_6ce7b91a830704274f87ac1ba4e5ca9f
#define id_6ce7b91a830704274f87ac1ba4e5ca9f
inline bool test_6ce7b91a830704274f87ac1ba4e5ca9f(const int* i) {
    return i != nullptr;
}
#endif
