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

/*! \file svismilesection.hpp
    \brief svi smile section
*/

#ifndef quantlib_svi_smile_section_hpp
#define quantlib_svi_smile_section_hpp

#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <vector>

namespace QuantLib {

class SviSmileSection : public SmileSection {

  public:
    //! \name Constructors
    //@{
    /*! @param timeToExpiry Time to expiry
        @param forward Forward price corresponding to the expiry date
        @param sviParameters Expects SVI parameters as a vector composed of a, b, sigma, rho, s, m
    */
    SviSmileSection(Time timeToExpiry, Rate forward, std::vector<Real> sviParameters);
    /*! @param d Date of expiry
        @param forward Forward price corresponding to the expiry date
        @param sviParameters Expects SVI parameters as a vector composed of a, b, sigma, rho, s, m
        @param dc Day count method used to compute the time to expiry
    */
    SviSmileSection(const Date& d,
                    Rate forward,
                    std::vector<Real> sviParameters,
                    const DayCounter& dc = Actual365Fixed());
    //@}
    Real minStrike() const override { return 0.0; }
    Real maxStrike() const override { return QL_MAX_REAL; }
    Real atmLevel() const override { return forward_; }

  protected:
    Volatility volatilityImpl(Rate strike) const override;

  private:
    void init();
    Rate forward_;
    std::vector<Real> params_;
};
}

#endif
