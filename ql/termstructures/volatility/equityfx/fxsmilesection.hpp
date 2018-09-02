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

/*! \file fxsmilesection.hpp
    \brief FX smile section assuming a strike/volatility space
    \ingroup termstructures
*/

#ifndef quantext_fx_smile_section_hpp
#define quantext_fx_smile_section_hpp

#include <ql/types.hpp>

using namespace QuantLib;

namespace QuantExt {

//! FX SmileSection
//! \ingroup termstructures
class FxSmileSection {
public:
    FxSmileSection(Real spot, Real rd, Real rf, Time t, Volatility atmVol, Volatility rr25d, Volatility bf25d)
        : spot_(spot), rd_(rd), rf_(rf), t_(t), atmVol_(atmVol), rr25d_(rr25d), bf25d_(bf25d) {}
    virtual ~FxSmileSection(){};

    virtual Volatility volatility(Real strike) const = 0;

protected:
    Real spot_;
    Real rd_;
    Real rf_;
    Time t_;
    Volatility atmVol_;
    Volatility rr25d_;
    Volatility bf25d_;
};

} // namespace QuantExt

#endif
