/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 Klaus Spanderen

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

/*! \file cashdividendeuropeanengine.cpp
*/

#include <ql/payoff.hpp>
#include <ql/exercise.hpp>
#include <ql/instruments/basketoption.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/pricingengines/vanilla/cashdividendeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/analyticdividendeuropeanengine.hpp>
#include <ql/pricingengines/basket/choibasketengine.hpp>


namespace QuantLib {
    CashDividendEuropeanEngine::CashDividendEuropeanEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process,
        DividendSchedule dividends,
        CashDividendModel cashDividendModel)
    : process_(std::move(process)),
      dividends_(std::move(dividends)),
      cashDividendModel_(cashDividendModel) {

        registerWith(process_);
    }

    void CashDividendEuropeanEngine::calculate() const {

        const ext::shared_ptr<EuropeanExercise> exercise =
            ext::dynamic_pointer_cast<EuropeanExercise>(arguments_.exercise);
        QL_REQUIRE(exercise, "not an European option");

        const ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        if (cashDividendModel_ == Escrowed) {
            VanillaOption option(payoff, exercise);
            option.setPricingEngine(
                ext::make_shared<AnalyticDividendEuropeanEngine>(
                    process_, dividends_));

            results_.value = option.NPV();
            return;
        }


        const Real strike = payoff->strike();

        const Handle<YieldTermStructure> rTS = process_->riskFreeRate();
        const Handle<YieldTermStructure> qTS = process_->dividendYield();

        const Date settlementDate = rTS->referenceDate();
        const Date maturityDate = exercise->lastDate();
        const Time maturity = process_->time(maturityDate);

        DividendSchedule dividends;
        std::copy_if(
            dividends_.begin(), dividends_.end(),
            std::back_inserter(dividends),
            [settlementDate, maturityDate](
                const ext::shared_ptr<Dividend>& div) -> bool {
                return div->date() >= settlementDate && div->date() <= maturityDate;
            }
        );
        std::sort(
            dividends.begin(), dividends.end(),
            [](const ext::shared_ptr<Dividend>& d1, const ext::shared_ptr<Dividend>& d2) {
                return d1->date() < d2->date();
            }
        );

        DividendSchedule underlyings(dividends);
        if (!underlyings.empty() && underlyings.back()->date() == maturityDate) {
            underlyings.back() = ext::make_shared<FixedDividend>(
                    underlyings.back()->amount() + strike, maturityDate
            );
        }
        else
            underlyings.emplace_back(ext::make_shared<FixedDividend>(strike, maturityDate));

        const Handle<BlackVolTermStructure> volTS = process_->blackVolatility();
        const Date volRefDate = volTS->referenceDate();
        const DayCounter volDc = volTS->dayCounter();

        const Handle<YieldTermStructure> zeroRateTS(
            ext::make_shared<FlatForward>(settlementDate, 0.0, rTS->dayCounter())
        );

        std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes;
        for (const auto& div: underlyings) {
            const Rate r_mod = std::log(rTS->discount(div->date())) / maturity;
            const Rate q_mod = std::log(qTS->discount(div->date())) / maturity;

            processes.emplace_back(
                ext::make_shared<GeneralizedBlackScholesProcess>(
                    Handle<Quote>(ext::make_shared<SimpleQuote>(div->amount())),
                    Handle<YieldTermStructure>(
                        ext::make_shared<FlatForward>(
                            settlementDate, q_mod - r_mod, rTS->dayCounter()
                        )
                    ),
                    zeroRateTS,
                    Handle<BlackVolTermStructure>(ext::make_shared<BlackConstantVol>(
                        volRefDate, volTS->calendar(),
                        Handle<Quote>(ext::make_shared<SimpleQuote>(
                            std::sqrt(volTS->blackVariance(div->date(), strike) / maturity)
                        )),
                        volDc
                    ))
                )
            );
        }

        Array v(underlyings.size());
        for (Size i=0; i < underlyings.size(); ++i)
            v[i] = volTS->blackVariance(underlyings[i]->date(), strike);

        Matrix rho(underlyings.size(), underlyings.size());
        for (Size i=0; i < underlyings.size(); ++i) {
            rho[i][i] = 1.0;
            for (Size j=0; j < i; ++j)
                rho[i][j] = rho[j][i] = v[j] / std::sqrt(v[i]*v[j]);
        }

        BasketOption basketOption(
            ext::make_shared<AverageBasketPayoff>(
                ext::make_shared<PlainVanillaPayoff>(Option::Put, process_->x0()),
                Array(underlyings.size(), 1.0)
            ),
            ext::make_shared<EuropeanExercise>(maturityDate)
        );

        basketOption.setPricingEngine(
            ext::make_shared<ChoiBasketEngine>(processes, rho, 10, 2000, false, true)
        );

        if (payoff->optionType() == Option::Call)
            results_.value = basketOption.NPV() * qTS->discount(maturityDate);
        else {
            const Real divDiscounted = std::accumulate(
                dividends.begin(), dividends.end(), Real(0),
                [&rTS, &qTS](Real acc, const ext::shared_ptr<Dividend>& div) {
                    return acc + div->amount() * rTS->discount(div->date()) / qTS->discount(div->date());
                }
            );

            const Real fwd = (process_->x0() - divDiscounted)
                * qTS->discount(maturityDate) / rTS->discount(maturityDate);

            results_.value = basketOption.NPV() * qTS->discount(maturityDate)
                - (fwd - payoff->strike()) * rTS->discount(maturityDate);
        }
    }
}
