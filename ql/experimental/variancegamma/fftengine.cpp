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

#include <ql/exercise.hpp>
#include <ql/experimental/variancegamma/fftengine.hpp>
#include <ql/math/fastfouriertransform.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <complex>
#include <utility>

namespace QuantLib {

    FFTEngine::FFTEngine(ext::shared_ptr<StochasticProcess1D> process, Real logStrikeSpacing)
    : process_(std::move(process)), lambda_(logStrikeSpacing) {
        registerWith(process_);
    }

    void FFTEngine::calculate() const
    {
        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
            "not an European Option");

        ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        auto r1 = resultMap_.find(arguments_.exercise->lastDate());
        if (r1 != resultMap_.end())
        {
            auto r2 = r1->second.find(payoff);
            if (r2 != r1->second.end())
            {
                results_.value = r2->second;
                return;
            }
        }
        
        // Option not precalculated - do entire FFT for one option.  Not very efficient - call precalculate!
        calculateUncached(payoff, arguments_.exercise);
    }

    void FFTEngine::update()
    {
        // Process has changed so cached values may no longer be correct
        resultMap_.clear();

        // Call base class implementation
        VanillaOption::engine::update();
    }

    void FFTEngine::calculateUncached(const ext::shared_ptr<StrikedTypePayoff>& payoff,
                                      const ext::shared_ptr<Exercise>& exercise) const {
        ext::shared_ptr<VanillaOption> option(new VanillaOption(payoff, exercise));
        std::vector<ext::shared_ptr<Instrument> > optionList;
        optionList.push_back(option);

        ext::shared_ptr<FFTEngine> tempEngine(clone().release());
        tempEngine->precalculate(optionList);
        option->setPricingEngine(tempEngine);
        results_.value = option->NPV();
    }

    void FFTEngine::precalculate(const std::vector<ext::shared_ptr<Instrument> >& optionList) {
        // Group payoffs by expiry date
        // as with FFT we can compute a bunch of these at once
        resultMap_.clear();

        typedef std::vector<ext::shared_ptr<StrikedTypePayoff> > PayoffList;
        typedef std::map<Date, PayoffList> PayoffMap;
        PayoffMap payoffMap;

        for (const auto& optIt : optionList) {
            ext::shared_ptr<VanillaOption> option = ext::dynamic_pointer_cast<VanillaOption>(optIt);
            QL_REQUIRE(option, "instrument must be option");
            QL_REQUIRE(option->exercise()->type() == Exercise::European,
                "not an European Option");

            ext::shared_ptr<StrikedTypePayoff> payoff =
                ext::dynamic_pointer_cast<StrikedTypePayoff>(option->payoff());
            QL_REQUIRE(payoff, "non-striked payoff given");

            payoffMap[option->exercise()->lastDate()].push_back(payoff);
        }

        std::complex<Real> i1(0, 1);
        Real alpha = 1.25;

        for (auto & payIt : payoffMap)
        {
            Date expiryDate = payIt.first;

            // Calculate n large enough for maximum strike, and round up to a power of 2
            Real maxStrike = 0.0;
            for (const auto& payoff : payIt.second) {
                if (payoff->strike() > maxStrike)
                    maxStrike = payoff->strike();
            }
            Real nR = 2.0 * (std::log(maxStrike) + lambda_) / lambda_;
      Size log2_n = (static_cast<Size>((std::log(nR) / std::log(2.0))) + 1);
            Size n = static_cast<std::size_t>(1) << log2_n;

            // Strike range (equation 19,20)
            Real b = n * lambda_ / 2.0;

            // Grid spacing (equation 23)
            Real eta = 2.0 * M_PI / (lambda_ * n);

            // Discount factor
            Real df = discountFactor(expiryDate);
            Real div = dividendYield(expiryDate);

            // Input to fourier transform
            std::vector<std::complex<Real> > fti;
            fti.resize(n);

            // Precalculate any discount factors etc.
            precalculateExpiry(expiryDate);

            for (Size i=0; i<n; i++)
            {
                Real v_j = eta * i;
                Real sw = eta * (3.0 + ((i % 2) == 0 ? -1.0 : 1.0) - ((i == 0) ? 1.0 : 0.0)) / 3.0; 

                std::complex<Real> psi = df * complexFourierTransform(v_j - (alpha + 1)* i1);
                psi = psi / (alpha*alpha + alpha - v_j*v_j + i1 * (2 * alpha + 1.0) * v_j);

                fti[i] = std::exp(i1 * b * v_j)  * sw * psi;
            }

            // Perform fft
            std::vector<std::complex<Real> > results(n);
            FastFourierTransform fft(log2_n);
            fft.transform(fti.begin(), fti.end(), results.begin());

            // Call prices
            std::vector<Real> prices, strikes;
            prices.resize(n);
            strikes.resize(n);
            for (Size i=0; i<n; i++)
            {
                Real k_u = -b + lambda_ * i;
                prices[i] = (std::exp(-alpha * k_u) / M_PI) * results[i].real();
                strikes[i] = std::exp(k_u);
            }

            for (const auto& payoff : payIt.second) {
                Real callPrice = LinearInterpolation(strikes.begin(), strikes.end(),
                                                     prices.begin())(payoff->strike());
                switch (payoff->optionType())
                {
                case Option::Call:
                    resultMap_[expiryDate][payoff] = callPrice;
                    break;
                case Option::Put:
                    resultMap_[expiryDate][payoff] = callPrice - process_->x0() * div + payoff->strike() * df;
                    break;
                default:
                    QL_FAIL("Invalid option type");
                }
            }
        }
    }

}

