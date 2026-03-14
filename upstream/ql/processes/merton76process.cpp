/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004, 2005 StatPro Italia srl

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

#include <ql/processes/merton76process.hpp>
#include <utility>

namespace QuantLib {

    Merton76Process::Merton76Process(const Handle<Quote>& stateVariable,
                                     const Handle<YieldTermStructure>& dividendTS,
                                     const Handle<YieldTermStructure>& riskFreeTS,
                                     const Handle<BlackVolTermStructure>& blackVolTS,
                                     Handle<Quote> jumpInt,
                                     Handle<Quote> logJMean,
                                     Handle<Quote> logJVol,
                                     const ext::shared_ptr<discretization>& disc)
    : StochasticProcess1D(disc), blackProcess_(
          new BlackScholesMertonProcess(stateVariable, dividendTS, riskFreeTS, blackVolTS, disc)),
      jumpIntensity_(std::move(jumpInt)), logMeanJump_(std::move(logJMean)),
      logJumpVolatility_(std::move(logJVol)) {
        registerWith(blackProcess_);
        registerWith(jumpIntensity_);
        registerWith(logMeanJump_);
        registerWith(logJumpVolatility_);
    }

    Real Merton76Process::x0() const {
        return blackProcess_->x0();
    }

    Time Merton76Process::time(const Date& d) const {
        return blackProcess_->time(d);
    }

    const Handle<Quote>& Merton76Process::stateVariable() const {
        return blackProcess_->stateVariable();
    }

    const Handle<YieldTermStructure>& Merton76Process::dividendYield() const {
        return blackProcess_->dividendYield();
    }

    const Handle<YieldTermStructure>& Merton76Process::riskFreeRate() const {
        return blackProcess_->riskFreeRate();
    }

    const Handle<BlackVolTermStructure>&
    Merton76Process::blackVolatility() const {
        return blackProcess_->blackVolatility();
    }

    const Handle<Quote>& Merton76Process::jumpIntensity() const {
        return jumpIntensity_;
    }

    const Handle<Quote>& Merton76Process::logMeanJump() const {
        return logMeanJump_;
    }

    const Handle<Quote>& Merton76Process::logJumpVolatility() const {
        return logJumpVolatility_;
    }

}
