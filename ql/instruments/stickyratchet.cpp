/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Marco Bianchetti
 Copyright (C) 2007 Giorgio Facchinetti

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

#include <ql/instruments/stickyratchet.hpp>

namespace QuantLib {

    // Double Sticky/Ratchet payoffs
    Real DoubleStickyRatchetPayoff::operator()(Real forward) const {
        QL_REQUIRE((std::fabs(type1_)==1.0 || type1_==0.0),
            "unknown/illegal type1 value (only 0.0 and +/-1,0 are allowed))");
        QL_REQUIRE((std::fabs(type2_)==1.0 || type2_==0.0),
            "unknown/illegal type2 value(only 0.0 and +/-1,0 are allowed)");
        Real swaplet = gearing3_ * forward + spread3_;
        Real effStrike1 = gearing1_ * initialValue1_ + spread1_;
        Real effStrike2 = gearing2_ * initialValue2_ + spread2_;
        Real effStrike3 = type1_*type2_*std::max<Real>(type2_*(swaplet-effStrike2),0.0);
        Real price = accrualFactor_ * (swaplet -
                    type1_*std::max<Real>(type1_*(swaplet-effStrike1),effStrike3));
        return price;
    }

    std::string DoubleStickyRatchetPayoff::name() const {
        return "DoubleStickyRatchetPayoff";
    }

    std::string DoubleStickyRatchetPayoff::description() const {
        std::ostringstream result;
        result << name();
        return result.str();
    }

    void DoubleStickyRatchetPayoff::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<DoubleStickyRatchetPayoff>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            Payoff::accept(v);
    }

/*---------------------------------------------------------------------------

    // Old code for single sticky/ratchet payoffs,
    // superated by DoubleStickyRatchetPayoff class above

    // Single Sticky/Ratchet payoffs
    Real StickyRatchetPayoff::operator()(Real forward) const {
        QL_REQUIRE(abs(type_)==1.0, "unknown/illegal option type");
        Real swaplet = gearing2_ * forward + spread2_;
        Real effStrike = gearing2_ * initialValue_ + spread2_;
        Real price = accrualFactor_ * (swaplet -
                    type_*std::max<Real>(type_*(swaplet-effStrike),0.0));
        return price;
    }

    std::string StickyRatchetPayoff::description() const {
        std::ostringstream result;
        result << name();
        return result.str();
    }

    void StickyRatchetPayoff::accept(AcyclicVisitor& v) {
        Visitor<StickyRatchetPayoff>* v1 =
            dynamic_cast<Visitor<StickyRatchetPayoff>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            Payoff::accept(v);
    }
-----------------------------------------------------------------------------*/

}
