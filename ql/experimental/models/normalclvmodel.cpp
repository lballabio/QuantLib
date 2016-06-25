/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Klaus Spanderen

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

/*! \file normalclvmodel.cpp
*/

#include <ql/instruments/vanillaoption.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>

#include <ql/experimental/models/normalclvmodel.hpp>

#include <boost/make_shared.hpp>

namespace QuantLib {

    NormalCLVMCModel::NormalCLVMCModel(
        const boost::shared_ptr<GeneralizedBlackScholesProcess>& bsProcess,
        const std::vector<Date>& maturityDates)
    : bsProcess_(bsProcess),
      maturityDates_(maturityDates),
      pricingEngine_(boost::make_shared<AnalyticEuropeanEngine>(bsProcess_)) {
        registerWith(bsProcess_);
    }

    void NormalCLVMCModel::update() {
    }

    Real NormalCLVMCModel::F(const Date& maturityDate, Real k) const {

        const DiscountFactor df
            = bsProcess_->riskFreeRate()->discount(maturityDate);

        VanillaOption option(
            boost::make_shared<PlainVanillaPayoff>(Option::Call, k),
            boost::make_shared<EuropeanExercise>(maturityDate));

        option.setPricingEngine(pricingEngine_);

        const Handle<BlackVolTermStructure> volTS
            = bsProcess_->blackVolatility();

        const Real dk = 1e-4*k;
        const Real dvol_dk
            = ( volTS->blackVol(maturityDate, k+dk)
               -volTS->blackVol(maturityDate, k-dk)) / (2*dk);

        return 1.0 + (  option.strikeSensitivity()
                      + option.vega() * dvol_dk) /df;
    }


    boost::function<Real(Time, Real)> NormalCLVMCModel::g() const {
        return boost::function<Real(Time, Real)>();
    }

    const boost::shared_ptr<GeneralizedBlackScholesProcess>&
    NormalCLVMCModel::process() const {
        return bsProcess_;
    }

    void NormalCLVMCModel::performCalculations() const {
    }
}
