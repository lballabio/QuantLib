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
    NoArbSabrSmileSection(Time timeToExpiry, Rate forward,
                          const std::vector<Real> &sabrParameters,
                          const Real shift = 0.0);
    NoArbSabrSmileSection(const Date &d, Rate forward,
                          const std::vector<Real> &sabrParameters,
                          const DayCounter &dc = Actual365Fixed(),
                          const Real shift = 0.0);
    Real minStrike() const { return 0.0; }
    Real maxStrike() const { return QL_MAX_REAL; }
    Real atmLevel() const { return forward_; }
    Real optionPrice(Rate strike, Option::Type type = Option::Call,
                     Real discount = 1.0) const;
    Real digitalOptionPrice(Rate strike, Option::Type type = Option::Call,
                            Real discount = 1.0, Real gap = 1.0e-5) const;
    Real density(Rate strike, Real discount = 1.0, Real gap = 1.0E-4) const;

    boost::shared_ptr<NoArbSabrModel> model() { return model_; }

  protected:
    Volatility volatilityImpl(Rate strike) const;

  private:
    void init();
    boost::shared_ptr<NoArbSabrModel> model_;
    Rate forward_;
    std::vector<Real> params_;
    Real shift_;
};
}

#endif
