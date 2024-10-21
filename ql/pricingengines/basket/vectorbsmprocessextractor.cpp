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

/*! \file bsmprocessesextractor.cpp
*/
#include <ql/math/functional.hpp>
#include <ql/math/comparison.hpp>

#include <ql/pricingengines/basket/vectorbsmprocessextractor.hpp>

namespace QuantLib {
    namespace detail {
        VectorBsmProcessExtractor::VectorBsmProcessExtractor(
            std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > p)
          : processes_(std::move(p)) {
        }

        Array VectorBsmProcessExtractor::extractProcesses(
            const std::function<Real(
                const ext::shared_ptr<GeneralizedBlackScholesProcess>&)>& f) const {

            Array x(processes_.size());
            std::transform(processes_.begin(), processes_.end(), x.begin(), f);

            return x;
        }

        DiscountFactor VectorBsmProcessExtractor::getInterestRateDf(
            const Date& maturityDate) const {
            const Array dr = extractProcesses(
                [maturityDate](const auto& p) -> DiscountFactor {
                    return p->riskFreeRate()->discount(maturityDate);
                }
            );

            QL_REQUIRE(
                std::equal(
                    dr.begin()+1, dr.end(), dr.begin(),
                    [](Real a, Real b) -> bool { return close_enough(a, b);}
                ),
                "interest rates need to be the same for all underlyings"
            );

            return dr[0];
        }

        Array VectorBsmProcessExtractor::getSpot() const {
            return extractProcesses([](const auto& p) -> Real { return p->x0(); });
        }

        Array VectorBsmProcessExtractor::getDividendYieldDf(
            const Date& maturityDate) const {
            return extractProcesses(
                [maturityDate](const auto& p) -> DiscountFactor {
                    return p->dividendYield()->discount(maturityDate);
                }
            );
        }

        Array VectorBsmProcessExtractor::getBlackVariance(
            const Date& maturityDate) const {
            return extractProcesses(
                [maturityDate](const auto& p) -> Volatility {
                    return p->blackVolatility()->blackVariance(maturityDate, p->x0());
                }
            );
        }

        Array VectorBsmProcessExtractor::getBlackStdDev(
            const Date& maturityDate) const {
            return extractProcesses(
                [maturityDate](const auto& p) -> Volatility {
                    const Time maturity = p->blackVolatility()->timeFromReference(maturityDate);
                    return p->blackVolatility()->blackVol(maturityDate, p->x0())*std::sqrt(maturity);
                }
            );
        }
    }
}
