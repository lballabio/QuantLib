/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
  Copyright (C) 2014 Bernd Lewerenz

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

#include <ql/exercise.hpp>
#include <ql/experimental/exoticoptions/continuousarithmeticasianvecerengine.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/rounding.hpp>
#include <ql/methods/finitedifferences/dminus.hpp>
#include <ql/methods/finitedifferences/dplus.hpp>
#include <ql/methods/finitedifferences/dplusdminus.hpp>
#include <ql/methods/finitedifferences/tridiagonaloperator.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <utility>

namespace QuantLib {

    ContinuousArithmeticAsianVecerEngine::ContinuousArithmeticAsianVecerEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process,
        Handle<Quote> currentAverage,
        Date startDate,
        Size timeSteps,
        Size assetSteps,
        Real z_min,
        Real z_max)
    : process_(std::move(process)), currentAverage_(std::move(currentAverage)),
      startDate_(startDate), z_min_(z_min), z_max_(z_max), timeSteps_(timeSteps),
      assetSteps_(assetSteps) {
        registerWith(process_);
        registerWith(currentAverage_);
    }

    void ContinuousArithmeticAsianVecerEngine::calculate() const {
        Real expectedAverage;

        QL_REQUIRE(arguments_.averageType == Average::Arithmetic,
                   "not an Arithmetic average option");
        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European Option");

        DayCounter rfdc  = process_->riskFreeRate()->dayCounter();
        DayCounter divdc = process_->dividendYield()->dayCounter();
        DayCounter voldc = process_->blackVolatility()->dayCounter();
        Real S_0 = process_->stateVariable()->value();

        // payoff
        ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        // original time to maturity
        Date maturity = arguments_.exercise->lastDate();

        Real X = payoff->strike();
        QL_REQUIRE(z_min_<=0 && z_max_>=0,
                   "strike (0 for vecer fixed strike asian)  not on Grid");

        Volatility sigma =
            process_->blackVolatility()->blackVol(maturity, X);

        Rate r = process_->riskFreeRate()->
            zeroRate(maturity, rfdc, Continuous, NoFrequency);
        Rate q = process_->dividendYield()->
            zeroRate(maturity, divdc, Continuous, NoFrequency);

        Date today(Settings::instance().evaluationDate());

        QL_REQUIRE(startDate_>=today,
                   "Seasoned Asian not yet implemented");

        // Expiry in Years
        Time T = rfdc.yearFraction(today,
                                   arguments_.exercise->lastDate());
        Time T1 = rfdc.yearFraction(today,
                                    startDate_ );            // Average Begin
        Time T2 = T;            // Average End (In this version only Maturity...)

        if ((T2 - T1) < 0.001) {
            // its a vanilla option. Use vanilla engine
            VanillaOption europeanOption(payoff, arguments_.exercise);
            europeanOption.setPricingEngine(
                        ext::make_shared<AnalyticEuropeanEngine>(process_));
            results_.value = europeanOption.NPV();

        } else {
            Real Theta = 0.5;        // Mixed Scheme: 0.5 = Crank Nicolson
            Real Z_0 = cont_strategy(0,T1,T2,q,r) - std::exp(-r*T) * X /S_0;

            QL_REQUIRE(Z_0>=z_min_ && Z_0<=z_max_,
                       "spot not on grid");

            Real h = (z_max_ - z_min_) / assetSteps_; // Space step size
            Real k = T / timeSteps_;         // Time Step size

            Real sigma2 = sigma * sigma, vecerTerm;

            Array SVec(assetSteps_+1),u_initial(assetSteps_+1),
                  u(assetSteps_+1),rhs(assetSteps_+1);

            for (Natural i= 0; i<= SVec.size()-1;i++) {
                SVec[i] = z_min_ + i * h;     // Value of Underlying on the grid
            }

            // Begin gamma construction
            TridiagonalOperator gammaOp = DPlusDMinus(assetSteps_+1,h);

            Array upperD = gammaOp.upperDiagonal();
            Array lowerD = gammaOp.lowerDiagonal();
            Array Dia    = gammaOp.diagonal();

            // Construct Vecer operator
            TridiagonalOperator explicit_part(gammaOp.size());
            TridiagonalOperator implicit_part(gammaOp.size());

            for (Natural i= 0; i<= SVec.size()-1;i++) {
                u_initial[i] = std::max<Real>(SVec[i] , 0.0); // Call Payoff
            }

            u = u_initial;

            // Start Time Loop

            for (Natural j = 1; j<=timeSteps_;j++) {
                if (Theta != 1.0) { // Explicit Part
                    for (Natural i = 1; i<= SVec.size()-2;i++) {
                        vecerTerm = SVec[i] - std::exp(-q * (T-(j-1)*k))
                                  * cont_strategy(T-(j-1)*k,T1,T2,q,r);
                        gammaOp.setMidRow(i,
                            0.5 * sigma2 * vecerTerm * vecerTerm  * lowerD[i-1],
                            0.5 * sigma2 * vecerTerm * vecerTerm  * Dia[i],
                            0.5 * sigma2 *  vecerTerm * vecerTerm * upperD[i]);
                    }
                    explicit_part = TridiagonalOperator::identity(gammaOp.size()) +
                                    (1 - Theta) * k * gammaOp;
                    explicit_part.setFirstRow(1.0,0.0); // Apply before applying
                    explicit_part.setLastRow(-1.0,1.0); // Neumann BC

                    u = explicit_part.applyTo(u);

                    // Apply after applying (Neumann BC)
                    u[assetSteps_] = u[assetSteps_-1] + h;
                    u[0] = 0;
                } // End Explicit Part

                if (Theta != 0.0) {  // Implicit Part
                    for (Natural i = 1; i<= SVec.size()-2;i++) {
                        vecerTerm = SVec[i] - std::exp(-q * (T-j*k)) *
                                    cont_strategy(T-j*k,T1,T2,q,r);
                        gammaOp.setMidRow(i,
                            0.5 * sigma2 * vecerTerm * vecerTerm * lowerD[i-1],
                            0.5 * sigma2 * vecerTerm * vecerTerm  * Dia[i],
                            0.5 * sigma2 * vecerTerm * vecerTerm * upperD[i]);
                    }

                    implicit_part = TridiagonalOperator::identity(gammaOp.size()) -
                                    Theta * k * gammaOp;

                    // Apply before solving
                    implicit_part.setFirstRow(1.0,0.0);
                    implicit_part.setLastRow(-1.0,1.0);
                    rhs = u;
                    rhs[0] = 0; // Lower BC
                    rhs[assetSteps_] = h; // Upper BC (Neumann) Delta=1
                    u = implicit_part.solveFor(rhs);
                } // End implicit Part
            } // End Time Loop

            DownRounding Rounding(0);
            auto lowerI = Integer(Rounding((Z_0 - z_min_) / h));
            // Interpolate solution
            Real pv;

            pv = u[lowerI] + (u[lowerI+1] - u[lowerI]) * (Z_0 - SVec[lowerI])/h;
            results_.value = S_0 * pv;

            if (payoff->optionType()==Option::Put) {
                // Apply Call Put Parity for Asians
                if (r == q) {
                    expectedAverage = S_0;
                } else {
                    expectedAverage =
                        S_0 * (std::exp( (r-q) * T2) -
                               std::exp( (r-q) * T1)) / ((r-q) * (T2-T1));
                }

                Real asianForward = std::exp(-r * T2) * (expectedAverage -  X);
                results_.value = results_.value - asianForward;
            }
        }
    }

    // Replication of Average by holding this amount in Assets
    Real ContinuousArithmeticAsianVecerEngine::cont_strategy(Time t,
                                                             Time T1,
                                                             Time T2,
                                                             Real v,
                                                             Real r) const {
        Real const eps= 0.00001;

        QL_REQUIRE(T1 <= T2, "Average Start must be before Average End");
        if (std::fabs(t-T2) < eps) {
            return 0.0;
        } else {
            if (t<T1) {
                if (std::fabs(r-v) >= eps) {
                    return (std::exp(v * (t-T2)) *
                           (1 - std::exp((v-r) * (T2-T1) ))  /
                           (( r - v) * (T2 - T1) ));
                } else {
                    return std::exp(v*(t-T2));
                } // end else v-r ==0
            } else { // t<T1
                if (std::fabs(r-v) >= eps) {
                    return std::exp(v * (t-T2)) *
                           (1 - std::exp( (v - r) * (T2-t) )) /
                           (( r - v) * (T2 - T1)  );
                } else {
                    return std::exp(v * (t-T2)) * (T2 - t) / (T2 - T1);
                }
            }
        }
    }

}
