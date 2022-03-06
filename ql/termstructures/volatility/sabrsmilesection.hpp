/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2015 Peter Caspers

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

/*! \file sabrsmilesection.hpp
    \brief sabr smile section class
*/

#ifndef quantlib_sabr_smile_section_hpp
#define quantlib_sabr_smile_section_hpp

#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <vector>

namespace QuantLib {

    class SabrSmileSection : public SmileSection {
      public:
        SabrSmileSection(Time timeToExpiry,
                         Rate forward,
                         const std::vector<Real>& sabrParameters,
                         Real shift = 0.0,
                         VolatilityType volatilityType = VolatilityType::ShiftedLognormal);
        SabrSmileSection(const Date& d,
                         Rate forward,
                         const std::vector<Real>& sabrParameters,
                         const DayCounter& dc = Actual365Fixed(),
                         Real shift = 0.0,
                         VolatilityType volatilityType = VolatilityType::ShiftedLognormal);
        Real minStrike() const override { return -shift_; }
        Real maxStrike() const override { return QL_MAX_REAL; }
        Real atmLevel() const override { return forward_; }
        Real alpha() const { return alpha_; }
        Real beta() const { return beta_; }
        Real nu() const { return nu_; }
        Real rho() const { return rho_; }
      protected:
        Real varianceImpl(Rate strike) const override;
        Volatility volatilityImpl(Rate strike) const override;

      private:
        Real alpha_, beta_, nu_, rho_, forward_, shift_;
        void initialise(const std::vector<Real>& sabrParameters);
    };


}

#endif


#ifndef id_1b79ff2c1cc8c9074530c2f4c6045dc9
#define id_1b79ff2c1cc8c9074530c2f4c6045dc9
inline bool test_1b79ff2c1cc8c9074530c2f4c6045dc9(const int* i) {
    return i != nullptr;
}
#endif
