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
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Processes/merton76process.hpp>

namespace QuantLib {

    Merton76Process::Merton76Process(
             const Handle<Quote>& stateVariable,
             const Handle<YieldTermStructure>& dividendTS,
             const Handle<YieldTermStructure>& riskFreeTS,
             const Handle<BlackVolTermStructure>& blackVolTS,
             const Handle<Quote>& jumpInt,
             const Handle<Quote>& logJMean,
             const Handle<Quote>& logJVol,
             const boost::shared_ptr<StochasticProcess::discretization>& disc)
    : blackProcess_(new BlackScholesProcess(stateVariable, dividendTS,
                                            riskFreeTS, blackVolTS, disc)),
      jumpIntensity_(jumpInt), logMeanJump_(logJMean),
      logJumpVolatility_(logJVol) {
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

    const boost::shared_ptr<Quote>& Merton76Process::stateVariable() const {
        return blackProcess_->stateVariable();
    }

    const boost::shared_ptr<YieldTermStructure>&
    Merton76Process::dividendYield() const {
        return blackProcess_->dividendYield();
    }

    const boost::shared_ptr<YieldTermStructure>&
    Merton76Process::riskFreeRate() const {
        return blackProcess_->riskFreeRate();
    }

    const boost::shared_ptr<BlackVolTermStructure>&
    Merton76Process::blackVolatility() const {
        return blackProcess_->blackVolatility();
    }

    const boost::shared_ptr<Quote>& Merton76Process::jumpIntensity() const {
        return jumpIntensity_.currentLink();
    }

    const boost::shared_ptr<Quote>& Merton76Process::logMeanJump() const {
        return logMeanJump_.currentLink();
    }

    const boost::shared_ptr<Quote>&
    Merton76Process::logJumpVolatility() const {
        return logJumpVolatility_.currentLink();
    }

}
