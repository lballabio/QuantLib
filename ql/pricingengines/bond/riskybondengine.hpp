/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Lew Wei Hao

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

/*! \file riskybondengine.hpp
    \brief risky bond engine
*/

#ifndef quantlib_risky_bond_engine_hpp
#define quantlib_risky_bond_engine_hpp

#include <ql/instruments/bond.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/handle.hpp>
#include <ql/experimental/credit/riskybond.hpp>

namespace QuantLib {

    class RiskyBondEngine : public Bond::engine {
      public:
        RiskyBondEngine(Handle<DefaultProbabilityTermStructure> defaultTS,
                        Real recoveryRate,
                        Handle<YieldTermStructure> yieldTS);
        void calculate() const override;
        Handle<DefaultProbabilityTermStructure> defaultTS() const;
        Real recoveryRate() const;
        Handle<YieldTermStructure> yieldTS() const;
      private:
        Handle<DefaultProbabilityTermStructure> defaultTS_;
        Real recoveryRate_;
        Handle<YieldTermStructure> yieldTS_;
    };


    inline Handle<DefaultProbabilityTermStructure> RiskyBondEngine::defaultTS() const {
        return defaultTS_;
    }

    inline Real RiskyBondEngine::recoveryRate() const { return recoveryRate_; }

    inline Handle<YieldTermStructure> RiskyBondEngine::yieldTS() const { return yieldTS_; }

}

#endif
