/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Klaus Spanderen

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

/*! \file fdmblackscholesmesher.cpp
    \brief 1-d mesher for the Black-Scholes process (in ln(S))
*/

#include <ql/processes/blackscholesprocess.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/yield/quantotermstructure.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/methods/finitedifferences/utilities/fdmquantohelper.hpp>
#include <ql/methods/finitedifferences/meshers/uniform1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/concentrating1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmblackscholesmesher.hpp>

namespace QuantLib {

    FdmBlackScholesMesher::FdmBlackScholesMesher(
        Size size,
        const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
        Time maturity, Real strike,
        Real xMinConstraint, Real xMaxConstraint,
        Real eps, Real scaleFactor,
        const std::pair<Real, Real>& cPoint,
        const DividendSchedule& dividendSchedule,
        const ext::shared_ptr<FdmQuantoHelper>& fdmQuantoHelper,
        Real spotAdjustment)
    : Fdm1dMesher(size) {

        const Real S = process->x0();
        QL_REQUIRE(S > 0.0, "negative or null underlying given");

        std::vector<std::pair<Time, Real> > intermediateSteps;
        for (const auto& i : dividendSchedule) {
            const Time t = process->time(i->date());
            if (t <= maturity && t >= 0.0)
                intermediateSteps.emplace_back(process->time(i->date()), i->amount());
        }

        const Size intermediateTimeSteps = std::max<Size>(2, Size(24.0*maturity));
        for (Size i=0; i < intermediateTimeSteps; ++i)
            intermediateSteps.emplace_back((i + 1) * (maturity / intermediateTimeSteps), 0.0);

        std::sort(intermediateSteps.begin(), intermediateSteps.end());

        const Handle<YieldTermStructure> rTS = process->riskFreeRate();

        const Handle<YieldTermStructure> qTS =
            (fdmQuantoHelper) != nullptr ?
                Handle<YieldTermStructure>(ext::make_shared<QuantoTermStructure>(
                    process->dividendYield(), process->riskFreeRate(),
                    Handle<YieldTermStructure>(fdmQuantoHelper->fTS_), process->blackVolatility(),
                    strike, Handle<BlackVolTermStructure>(fdmQuantoHelper->fxVolTS_),
                    fdmQuantoHelper->exchRateATMlevel_, fdmQuantoHelper->equityFxCorrelation_)) :
                process->dividendYield();

        Time lastDivTime = 0.0;
        Real fwd = S + spotAdjustment;
        Real mi = fwd, ma = fwd;

        for (auto& intermediateStep : intermediateSteps) {
            const Time divTime = intermediateStep.first;
            const Real divAmount = intermediateStep.second;

            fwd = fwd / rTS->discount(divTime) * rTS->discount(lastDivTime)
                      * qTS->discount(divTime) / qTS->discount(lastDivTime);

            mi  = std::min(mi, fwd); ma = std::max(ma, fwd);

            fwd-= divAmount;

            mi  = std::min(mi, fwd); ma = std::max(ma, fwd);

            lastDivTime = divTime;
        }

        // Set the grid boundaries
        const Real normInvEps = InverseCumulativeNormal()(1-eps);
        const Real sigmaSqrtT 
            = process->blackVolatility()->blackVol(maturity, strike)
                                                        *std::sqrt(maturity);
        
        Real xMin = std::log(mi) - sigmaSqrtT*normInvEps*scaleFactor;
        Real xMax = std::log(ma) + sigmaSqrtT*normInvEps*scaleFactor;

        if (xMinConstraint != Null<Real>()) {
            xMin = xMinConstraint;
        }
        if (xMaxConstraint != Null<Real>()) {
            xMax = xMaxConstraint;
        }

        ext::shared_ptr<Fdm1dMesher> helper;
        if (   cPoint.first != Null<Real>() 
            && std::log(cPoint.first) >=xMin && std::log(cPoint.first) <=xMax) {
            
            helper = ext::shared_ptr<Fdm1dMesher>(
                new Concentrating1dMesher(xMin, xMax, size, 
                    std::pair<Real,Real>(std::log(cPoint.first),
                                         cPoint.second)));
        }
        else {
            helper = ext::shared_ptr<Fdm1dMesher>(
                                        new Uniform1dMesher(xMin, xMax, size));
            
        }
        
        locations_ = helper->locations();
        for (Size i=0; i < locations_.size(); ++i) {
            dplus_[i]  = helper->dplus(i);
            dminus_[i] = helper->dminus(i);
        }
    }
            
    ext::shared_ptr<GeneralizedBlackScholesProcess> 
    FdmBlackScholesMesher::processHelper(const Handle<Quote>& s0,
                                         const Handle<YieldTermStructure>& rTS,
                                         const Handle<YieldTermStructure>& qTS,
                                         Volatility vol) {
        
        return ext::make_shared<GeneralizedBlackScholesProcess>(
            
                s0, qTS, rTS,
                Handle<BlackVolTermStructure>(
                    ext::shared_ptr<BlackVolTermStructure>(
                        new BlackConstantVol(rTS->referenceDate(),
                                             Calendar(),
                                             vol,
                                             rTS->dayCounter()))));
    }
}

