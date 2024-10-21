/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 Klaus Spanderen

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

/*! \file choibasketengine.cpp
*/

#include <ql/exercise.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/math/matrixutilities/svd.hpp>
#include <ql/math/matrixutilities/householder.hpp>
#include <ql/math/matrixutilities/getcovariance.hpp>
#include <ql/math/matrixutilities/choleskydecomposition.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/pricingengines/basket/choibasketengine.hpp>
#include <ql/pricingengines/basket/vectorbsmprocessextractor.hpp>
#include <ql/pricingengines/basket/singlefactorbsmbasketengine.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>

#include <boost/math/special_functions/sign.hpp>
#include <iostream>

namespace QuantLib {

    ChoiBasketEngine::ChoiBasketEngine(
        std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes,
        Matrix rho, Real lambda,
        Size maxNrIntegrationSteps,
        bool calcFwdDelta, bool controlVariate)
    : n_(processes.size()),
      processes_(std::move(processes)),
      rho_(std::move(rho)),
      lambda_(lambda),
      maxNrIntegrationSteps_(maxNrIntegrationSteps),
      calcFwdDelta_(calcFwdDelta || controlVariate),
      controlVariate_(controlVariate) {

        QL_REQUIRE(n_ > 0, "No Black-Scholes process is given.");
        QL_REQUIRE(n_ == rho_.size1() && rho_.size1() == rho_.size2(),
            "process and correlation matrix must have the same size.");

        QL_REQUIRE(lambda_ > 0.0, "lambda must be positive");

        std::for_each(processes_.begin(), processes_.end(),
            [this](const auto& p) { registerWith(p); });
    }

    void ChoiBasketEngine::calculate() const {
        const ext::shared_ptr<EuropeanExercise> exercise =
            ext::dynamic_pointer_cast<EuropeanExercise>(arguments_.exercise);
        QL_REQUIRE(exercise, "not an European exercise");

        const Date maturityDate = exercise->lastDate();

        const detail::VectorBsmProcessExtractor pExtractor(processes_);
        const Array s = pExtractor.getSpot();
        const Array dq = pExtractor.getDividendYieldDf(maturityDate);
        const Array stdDev = pExtractor.getBlackStdDev(maturityDate);
        const DiscountFactor dr0 = pExtractor.getInterestRateDf(maturityDate);

        const Array fwd = s * dq/dr0;

        const ext::shared_ptr<AverageBasketPayoff> avgPayoff =
            ext::dynamic_pointer_cast<AverageBasketPayoff>(arguments_.payoff);
        QL_REQUIRE(avgPayoff, "average basket payoff expected");

        const Array weights = avgPayoff->weights();
        QL_REQUIRE(n_ == weights.size() && n_ > 1,
             "wrong number of weights arguments in payoff");

        const Array g = weights*fwd / Norm2(weights*fwd);

        const Matrix Sigma = getCovariance(stdDev.begin(), stdDev.end(), rho_);
        Array vStar1 = Sigma*g;
        vStar1 /= std::sqrt(DotProduct(g, vStar1));

        const Matrix C = CholeskyDecomposition(Sigma);

        const Real eps = 100*std::sqrt(QL_EPSILON);
        // publication sets tol=0, pyfeng implementation sets tol=0.01
        const Real tol = 100*std::sqrt(QL_EPSILON);

        bool flip = false;
        for (Size i=0; i < n_; ++i)
            if (boost::math::sign(g[i])*vStar1[i] < tol*stdDev[i]) {
                flip = true;
                vStar1[i] = eps * boost::math::sign(g[i]) * stdDev[i];
            }

        Array q1(n_);
        if (flip) {
            //q1 = inverse(C)*vStar1;
            for (Size i=0; i < n_; ++i)
                q1[i] = (vStar1[i] - std::inner_product(
                    C.row_begin(i), C.row_begin(i) + i, q1.begin(), 0.0))/C[i][i];

            vStar1 /= Norm2(q1);
        }
        else {
            q1 = transpose(C)*g;
        }
        q1 /= Norm2(q1);

        Array e1(n_, 0.0);
        e1[0] = 1.0;

        const Matrix R = HouseholderTransformation(
            HouseholderReflection(e1).reflectionVector(q1)).getMatrix();
        Matrix R_2_n = Matrix(n_, n_-1);
        for (Size i=0; i < n_; ++i)
            std::copy(R.row_begin(i)+1, R.row_end(i), R_2_n.row_begin(i));

        const SVD svd(C*R_2_n);
        const Matrix U = svd.U();
        const Array sv = svd.singularValues();

        Matrix v(n_, n_-1);
        for (Size i=0; i < n_-1; ++i)
            std::transform(
                U.column_begin(i), U.column_end(i), v.column_begin(i),
                [i, &sv](Real x) -> Real { return sv[i]*x; }
            );

        std::vector<Size> nIntOrder(n_-1);
        Real lambda = lambda_;
        const Real alpha = 1/std::abs(DotProduct(g, vStar1));
        do {
            const Real intScale = lambda * alpha;
            for (Size i=0; i < n_-1; ++i)
                nIntOrder[i] = Size(std::lround(1 + intScale*sv[i]));

            lambda*=0.9;
            QL_REQUIRE(lambda/lambda_ > 1e-10,
                "can not rescale lambda to fit max integration order");
        } while (std::accumulate(
                     nIntOrder.begin(), nIntOrder.end(), 1.0, std::multiplies<>())
                > Real(maxNrIntegrationSteps_));

        std::vector<ext::shared_ptr<SimpleQuote> > quotes;
        std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > p;
        for (Size i=0; i < n_; ++i) {
            quotes.push_back(ext::make_shared<SimpleQuote>(fwd[i]));

            const Handle<BlackVolTermStructure> bv = processes_[i]->blackVolatility();
            const Volatility vol = vStar1[i] / std::sqrt(
                bv->dayCounter().yearFraction(bv->referenceDate(), maturityDate)
            );
            p.push_back(
                ext::make_shared<BlackProcess>(
                    Handle<Quote>(quotes[i]),
                    processes_[i]->riskFreeRate(),
                    Handle<BlackVolTermStructure>(
                       ext::make_shared<BlackConstantVol>(
                          bv->referenceDate(), bv->calendar(),
                          Handle<Quote>(ext::make_shared<SimpleQuote>(vol)),
                          bv->dayCounter()
                       )
                    )
                )
            );
        }

        BasketOption option(avgPayoff, exercise);
        option.setPricingEngine(ext::make_shared<SingleFactorBsmBasketEngine>(p));

        Array vq(n_);
        for (Size i=0; i < n_; ++i)
            vq[i] = 0.5*std::accumulate(
                v.row_begin(i), v.row_end(i), 0.0,
                [](Real acc, Real x) -> Real { return acc + x*x; }
            );

        MulitDimGaussianIntegration ghq(
            nIntOrder,
            [](const Size n) { return ext::make_shared<GaussHermiteIntegration>(n); }
        );
        const Real normFactor = std::pow(M_PI, -0.5*nIntOrder.size());

        std::vector<Real> dStore;
        dStore.reserve(ghq.weights().size());
        const auto bsm1dPricer = [&](const Array& z) -> Real {
            const Array f = Exp(-M_SQRT2*(v*z) - vq) * fwd;

            for (Size i=0; i < f.size(); ++i)
                quotes[i]->setValue(f[i]);

            dStore.push_back(ext::any_cast<Real>(option.additionalResults().at("d")));
            return std::exp(-DotProduct(z, z)) * option.NPV();
        };

        results_.value = ghq(bsm1dPricer) * normFactor;

        if (calcFwdDelta_) {
            const ext::shared_ptr<PlainVanillaPayoff> payoff =
                 ext::dynamic_pointer_cast<PlainVanillaPayoff>(avgPayoff->basePayoff());
            QL_REQUIRE(payoff, "non-plain vanilla payoff given");
            const Real putIndicator = (payoff->optionType() == Option::Call) ? 0.0 : -1.0;

            Size dStoreCounter;
            const CumulativeNormalDistribution N;

            Array fwdDelta(n_), fHat(n_);
            for (Size k=0; k < n_; ++k) {
                dStoreCounter = 0;

                const auto deltaPricer = [&](const Array& z) -> Real {
                    const Real d = dStore[dStoreCounter++];
                    const Real vz = std::inner_product(
                        v.row_begin(k), v.row_end(k), z.begin(), 0.0);
                    const Real f = std::exp(-M_SQRT2*vz - vq[k]);

                    return std::exp(-DotProduct(z, z)) * f * N(d + vStar1[k]);
                };

                fwdDelta[k] = dr0*weights[k]*(ghq(deltaPricer) * normFactor + putIndicator);

                const std::string deltaName = "forwardDelta " + std::to_string(k);
                results_.additionalResults[deltaName] = fwdDelta[k];
            }

            if (controlVariate_) {
                for (Size k=0; k < n_; ++k) {
                    const auto fHatPricer =  [&](const Array& z) -> Real {
                        const Real vz = std::inner_product(
                            v.row_begin(k), v.row_end(k), z.begin(), 0.0);
                        const Real f = std::exp(-M_SQRT2*vz - vq[k]);

                        return std::exp(-DotProduct(z, z)) * f;
                    };
                fHat[k] = ghq(fHatPricer) * normFactor;
                }
                const Array cv = fwdDelta*fwd*(fHat-1.0);
                results_.value -= std::accumulate(cv.begin(), cv.end(), 0.0);
            }
        }
    }
}
