/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2010 Adrian O' Neill

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

#include <ql/errors.hpp>
#include <ql/experimental/variancegamma/variancegammaprocess.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/processes/eulerdiscretization.hpp>
#include <utility>

namespace QuantLib {

    VarianceGammaProcess::VarianceGammaProcess(Handle<Quote> s0,
                                               Handle<YieldTermStructure> dividendYield,
                                               Handle<YieldTermStructure> riskFreeRate,
                                               Real sigma,
                                               Real nu,
                                               Real theta)
    : StochasticProcess1D(ext::shared_ptr<discretization>(new EulerDiscretization)),
      s0_(std::move(s0)), dividendYield_(std::move(dividendYield)),
      riskFreeRate_(std::move(riskFreeRate)), sigma_(sigma), nu_(nu), theta_(theta) {
        registerWith(riskFreeRate_);
        registerWith(dividendYield_);
        registerWith(s0_);
    }

    Real VarianceGammaProcess::x0() const
    {
        return s0_->value();
    }

    Real VarianceGammaProcess::drift(Time /*t*/, Real /*x*/) const
    {
        QL_FAIL("not implemented yet");
    }

    Real VarianceGammaProcess::diffusion(Time /*t*/, Real /*x*/) const
    {
        QL_FAIL("not implemented yet");
    }

    const Handle<Quote>& VarianceGammaProcess::s0() const {
        return s0_;
    }

    const Handle<YieldTermStructure>& VarianceGammaProcess::dividendYield() const {
        return dividendYield_;
    }

    const Handle<YieldTermStructure>& VarianceGammaProcess::riskFreeRate() const {
        return riskFreeRate_;
    }

}
