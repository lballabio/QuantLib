/*
 Copyright (C) 2017 Quaternion Risk Management Ltd
 All rights reserved.

 This file is part of ORE, a free-software/open-source library
 for transparent pricing and risk analysis - http://opensourcerisk.org

 ORE is free software: you can redistribute it and/or modify it
 under the terms of the Modified BSD License.  You should have received a
 copy of the license along with this program.
 The license is also available online at <http://opensourcerisk.org>

 This program is distributed on the basis that it will form a useful
 contribution to risk analytics and model standardisation, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
*/

/*! \file fxvannavolgasmilesection.hpp
    \brief FX smile section assuming a strike/volatility space using vanna volga method
    \ingroup termstructures
*/

#ifndef quantext_fx_vanna_volga_smile_section_hpp
#define quantext_fx_vanna_volga_smile_section_hpp

#include <qle/termstructures/fxsmilesection.hpp>

using namespace QuantLib;

namespace QuantExt {

/*! Vanna Volga Smile section
 *
 *  Consistent Pricing of FX Options
 *  Castagna & Mercurio (2006)
 *  http://papers.ssrn.com/sol3/papers.cfm?abstract_id=873788
 \ingroup termstructures
 */
class VannaVolgaSmileSection : public FxSmileSection {
public:
    VannaVolgaSmileSection(Real spot, Real rd, Real rf, Time t, Volatility atmVol, Volatility rr25d, Volatility bf25d);

    //! getters for unit test
    Real k_atm() const { return k_atm_; }
    Real k_25c() const { return k_25c_; }
    Real k_25p() const { return k_25p_; }
    Volatility vol_atm() const { return atmVol_; }
    Volatility vol_25c() const { return vol_25c_; }
    Volatility vol_25p() const { return vol_25p_; }

    //! \name FxSmileSection interface
    //@{
    Volatility volatility(Real strike) const;
    //}@

private:
    Real d1(Real x) const;
    Real d2(Real x) const;

    Real k_atm_, k_25c_, k_25p_;
    Volatility vol_25c_, vol_25p_;
};

} // namespace QuantExt

#endif
