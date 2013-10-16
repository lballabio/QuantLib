/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2012 Ferdinando Ametrano
 Copyright (C) 2006 Mark Joshi
 Copyright (C) 2006 StatPro Italia srl
 Copyright (C) 2007 Cristina Duminuco
 Copyright (C) 2007 Chiara Fornarola

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

#include <ql/pricingengines/blackformula.hpp>
#include <ql/math/solvers1d/newtonsafe.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <boost/math/special_functions/atanh.hpp>

namespace {
    void checkParameters(QuantLib::Real strike,
                         QuantLib::Real forward,
                         QuantLib::Real displacement)
    {
        QL_REQUIRE(displacement >= 0.0, "displacement ("
                                            << displacement
                                            << ") must be non-negative");
        QL_REQUIRE(strike + displacement >= 0.0,
                   "strike + displacement (" << strike << " + " << displacement
                                             << ") must be non-negative");
        QL_REQUIRE(forward + displacement > 0.0, "forward + displacement ("
                                                     << forward << " + "
                                                     << displacement
                                                     << ") must be positive");
    }
}

namespace QuantLib {

    Real blackFormula(Option::Type optionType,
                      Real strike,
                      Real forward,
                      Real stdDev,
                      Real discount,
                      Real displacement)
    {
        checkParameters(strike, forward, displacement);
        QL_REQUIRE(stdDev>=0.0,
                   "stdDev (" << stdDev << ") must be non-negative");
        QL_REQUIRE(discount>0.0,
                   "discount (" << discount << ") must be positive");

        if (stdDev==0.0)
            return std::max((forward-strike)*optionType, Real(0.0))*discount;

        forward = forward + displacement;
        strike = strike + displacement;

        // since displacement is non-negative strike==0 iff displacement==0
        // so returning forward*discount is OK
        if (strike==0.0)
            return (optionType==Option::Call ? forward*discount : 0.0);

        Real d1 = std::log(forward/strike)/stdDev + 0.5*stdDev;
        Real d2 = d1 - stdDev;
        CumulativeNormalDistribution phi;
        Real nd1 = phi(optionType*d1);
        Real nd2 = phi(optionType*d2);
        Real result = discount * optionType * (forward*nd1 - strike*nd2);
        QL_ENSURE(result>=0.0,
                  "negative value (" << result << ") for " <<
                  stdDev << " stdDev, " <<
                  optionType << " option, " <<
                  strike << " strike , " <<
                  forward << " forward");
        return result;
    }

    Real blackFormula(const boost::shared_ptr<PlainVanillaPayoff>& payoff,
                      Real forward,
                      Real stdDev,
                      Real discount,
                      Real displacement) {
        return blackFormula(payoff->optionType(),
            payoff->strike(), forward, stdDev, discount, displacement);
    }

    Real blackFormulaImpliedStdDevApproximation(Option::Type optionType,
                                                Real strike,
                                                Real forward,
                                                Real blackPrice,
                                                Real discount,
                                                Real displacement)
    {
        checkParameters(strike, forward, displacement);
        QL_REQUIRE(blackPrice>=0.0,
                   "blackPrice (" << blackPrice << ") must be non-negative");
        QL_REQUIRE(discount>0.0,
                   "discount (" << discount << ") must be positive");

        Real stdDev;
        forward = forward + displacement;
        strike = strike + displacement;
        if (strike==forward)
            // Brenner-Subrahmanyan (1988) and Feinstein (1988) ATM approx.
            stdDev = blackPrice/discount*std::sqrt(2.0 * M_PI)/forward;
        else {
            // Corrado and Miller extended moneyness approximation
            Real moneynessDelta = optionType*(forward-strike);
            Real moneynessDelta_2 = moneynessDelta/2.0;
            Real temp = blackPrice/discount - moneynessDelta_2;
            Real moneynessDelta_PI = moneynessDelta*moneynessDelta/M_PI;
            Real temp2 = temp*temp-moneynessDelta_PI;
            if (temp2<0.0) // approximation breaks down, 2 alternatives:
                // 1. zero it
                temp2=0.0;
                // 2. Manaster-Koehler (1982) efficient Newton-Raphson seed
                //return std::fabs(std::log(forward/strike))*std::sqrt(2.0);
            temp2 = std::sqrt(temp2);
            temp += temp2;
            temp *= std::sqrt(2.0 * M_PI);
            stdDev = temp/(forward+strike);
        }
        QL_ENSURE(stdDev>=0.0,
                  "stdDev (" << stdDev << ") must be non-negative");
        return stdDev;
    }

    Real blackFormulaImpliedStdDevApproximation(
                      const boost::shared_ptr<PlainVanillaPayoff>& payoff,
                      Real forward,
                      Real blackPrice,
                      Real discount,
                      Real displacement) {
        return blackFormulaImpliedStdDevApproximation(payoff->optionType(),
            payoff->strike(), forward, blackPrice, discount, displacement);
    }


    class BlackImpliedStdDevHelper {
      public:
        BlackImpliedStdDevHelper(Option::Type optionType,
                                 Real strike,
                                 Real forward,
                                 Real undiscountedBlackPrice,
                                 Real displacement = 0.0)
        : halfOptionType_(0.5*optionType), signedStrike_(optionType*(strike+displacement)),
          signedForward_(optionType*(forward+displacement)),
          undiscountedBlackPrice_(undiscountedBlackPrice)
        {
            checkParameters(strike, forward, displacement);
            QL_REQUIRE(undiscountedBlackPrice>=0.0,
                       "undiscounted Black price (" <<
                       undiscountedBlackPrice << ") must be non-negative");
            signedMoneyness_ = optionType*std::log((forward+displacement)/(strike+displacement));
        }
        Real operator()(Real stdDev) const {
            #if defined(QL_EXTRA_SAFETY_CHECKS)
            QL_REQUIRE(stdDev>=0.0,
                       "stdDev (" << stdDev << ") must be non-negative");
            #endif
            if (stdDev==0.0)
                return std::max(signedForward_-signedStrike_, Real(0.0))
                                                   - undiscountedBlackPrice_;
            Real temp = halfOptionType_*stdDev;
            Real d = signedMoneyness_/stdDev;
            Real signedD1 = d + temp;
            Real signedD2 = d - temp;
            Real result = signedForward_ * N_(signedD1)
                - signedStrike_ * N_(signedD2);
            // numerical inaccuracies can yield a negative answer
            return std::max(Real(0.0), result) - undiscountedBlackPrice_;
        }
        Real derivative(Real stdDev) const {
            #if defined(QL_EXTRA_SAFETY_CHECKS)
            QL_REQUIRE(stdDev>=0.0,
                       "stdDev (" << stdDev << ") must be non-negative");
            #endif
            Real signedD1 = signedMoneyness_/stdDev + halfOptionType_*stdDev;
            return signedForward_*N_.derivative(signedD1);
        }
      private:
        Real halfOptionType_;
        Real signedStrike_, signedForward_;
        Real undiscountedBlackPrice_, signedMoneyness_;
        CumulativeNormalDistribution N_;
    };


    Real blackFormulaImpliedStdDev(Option::Type optionType,
                                   Real strike,
                                   Real forward,
                                   Real blackPrice,
                                   Real discount,
                                   Real displacement,
                                   Real guess,
                                   Real accuracy,
                                   Natural maxIterations)
    {
        checkParameters(strike, forward, displacement);

        QL_REQUIRE(discount>0.0,
                   "discount (" << discount << ") must be positive");

        QL_REQUIRE(blackPrice>=0.0,
                   "option price (" << blackPrice << ") must be non-negative");
        // check the price of the "other" option implied by put-call paity
        Real otherOptionPrice = blackPrice - optionType*(forward-strike)*discount;
        QL_REQUIRE(otherOptionPrice>=0.0,
                   "negative " << Option::Type(-1*optionType) <<
                   " price (" << otherOptionPrice <<
                   ") implied by put-call parity. No solution exists for " <<
                   optionType << " strike " << strike <<
                   ", forward " << forward <<
                   ", price " << blackPrice <<
                   ", deflator " << discount);

        // solve for the out-of-the-money option which has
        // greater vega/price ratio, i.e.
        // it is numerically more robust for implied vol calculations
        if (optionType==Option::Put && strike>forward) {
            optionType = Option::Call;
            blackPrice = otherOptionPrice;
        }
        if (optionType==Option::Call && strike<forward) {
            optionType = Option::Put;
            blackPrice = otherOptionPrice;
        }

        strike = strike + displacement;
        forward = forward + displacement;

        if (guess==Null<Real>())
            guess = blackFormulaImpliedStdDevApproximation(
                optionType, strike, forward, blackPrice, discount, displacement);
        else
            QL_REQUIRE(guess>=0.0,
                       "stdDev guess (" << guess << ") must be non-negative");
        BlackImpliedStdDevHelper f(optionType, strike, forward,
                                   blackPrice/discount);
        NewtonSafe solver;
        solver.setMaxEvaluations(maxIterations);
        Real minSdtDev = 0.0, maxStdDev = 24.0; // 24 = 300% * sqrt(60)
        Real stdDev = solver.solve(f, accuracy, guess, minSdtDev, maxStdDev);
        QL_ENSURE(stdDev>=0.0,
                  "stdDev (" << stdDev << ") must be non-negative");
        return stdDev;
    }

    Real blackFormulaImpliedStdDev(
                        const boost::shared_ptr<PlainVanillaPayoff>& payoff,
                        Real forward,
                        Real blackPrice,
                        Real discount,
                        Real displacement,
                        Real guess,
                        Real accuracy,
                        Natural maxIterations) {
        return blackFormulaImpliedStdDev(payoff->optionType(), payoff->strike(),
            forward, blackPrice, discount, displacement, guess, accuracy, maxIterations);
    }

    Real blackFormulaCashItmProbability(Option::Type optionType,
                                        Real strike,
                                        Real forward,
                                        Real stdDev,
                                        Real displacement) {
        checkParameters(strike, forward, displacement);
        if (stdDev==0.0)
            return (forward*optionType > strike*optionType ? 1.0 : 0.0);

        forward = forward + displacement;
        strike = strike + displacement;
        if (strike==0.0)
            return (optionType==Option::Call ? 1.0 : 0.0);
        Real d2 = std::log(forward/strike)/stdDev - 0.5*stdDev;
        CumulativeNormalDistribution phi;
        return phi(optionType*d2);
    }


    Real blackFormulaCashItmProbability(
                        const boost::shared_ptr<PlainVanillaPayoff>& payoff,
                        Real forward,
                        Real stdDev,
                        Real displacement) {
        return blackFormulaCashItmProbability(payoff->optionType(),
            payoff->strike(), forward, stdDev , displacement);
    }


    Real blackFormulaVolDerivative(Rate strike,
                                      Rate forward,
                                      Real stdDev,
                                      Real expiry,
                                      Real discount,
                                      Real displacement)
    {
        return  blackFormulaStdDevDerivative(strike,
                                     forward,
                                     stdDev,
                                     discount,
                                     displacement)*std::sqrt(expiry);
    }

    Real blackFormulaStdDevDerivative(Rate strike,
                                      Rate forward,
                                      Real stdDev,
                                      Real discount,
                                      Real displacement)
    {
        checkParameters(strike, forward, displacement);
        QL_REQUIRE(stdDev>=0.0,
                   "stdDev (" << stdDev << ") must be non-negative");
        QL_REQUIRE(discount>0.0,
                   "discount (" << discount << ") must be positive");

        forward = forward + displacement;
        strike = strike + displacement;

        if (stdDev==0.0 || strike==0.0)
            return 0.0;

        Real d1 = std::log(forward/strike)/stdDev + .5*stdDev;
        return discount * forward *
            CumulativeNormalDistribution().derivative(d1);
    }

    Real blackFormulaStdDevDerivative(
                        const boost::shared_ptr<PlainVanillaPayoff>& payoff,
                        Real forward,
                        Real stdDev,
                        Real discount,
                        Real displacement) {
        return blackFormulaStdDevDerivative(payoff->strike(), forward,
                                     stdDev, discount, displacement);
    }


    Real bachelierBlackFormula(Option::Type optionType,
                               Real strike,
                               Real forward,
                               Real stdDev,
                               Real discount)
    {
        QL_REQUIRE(stdDev>=0.0,
                   "stdDev (" << stdDev << ") must be non-negative");
        QL_REQUIRE(discount>0.0,
                   "discount (" << discount << ") must be positive");
        Real d = (forward-strike)*optionType, h = d/stdDev;
        if (stdDev==0.0)
            return discount*std::max(d, 0.0);
        CumulativeNormalDistribution phi;
        Real result = discount*(stdDev*phi.derivative(h) + d*phi(h));
        QL_ENSURE(result>=0.0,
                  "negative value (" << result << ") for " <<
                  stdDev << " stdDev, " <<
                  optionType << " option, " <<
                  strike << " strike , " <<
                  forward << " forward");
        return result;
    }

    Real bachelierBlackFormula(
                        const boost::shared_ptr<PlainVanillaPayoff>& payoff,
                        Real forward,
                        Real stdDev,
                        Real discount) {
        return bachelierBlackFormula(payoff->optionType(),
            payoff->strike(), forward, stdDev, discount);
    }

    static Real h(Real eta) {

        const static Real  A0          = 3.994961687345134e-1;
        const static Real  A1          = 2.100960795068497e+1;
        const static Real  A2          = 4.980340217855084e+1;
        const static Real  A3          = 5.988761102690991e+2;
        const static Real  A4          = 1.848489695437094e+3;
        const static Real  A5          = 6.106322407867059e+3;
        const static Real  A6          = 2.493415285349361e+4;
        const static Real  A7          = 1.266458051348246e+4;

        const static Real  B0          = 1.000000000000000e+0;
        const static Real  B1          = 4.990534153589422e+1;
        const static Real  B2          = 3.093573936743112e+1;
        const static Real  B3          = 1.495105008310999e+3;
        const static Real  B4          = 1.323614537899738e+3;
        const static Real  B5          = 1.598919697679745e+4;
        const static Real  B6          = 2.392008891720782e+4;
        const static Real  B7          = 3.608817108375034e+3;
        const static Real  B8          = -2.067719486400926e+2;
        const static Real  B9          = 1.174240599306013e+1;

        QL_REQUIRE(eta>=0.0,
                       "eta (" << eta << ") must be non-negative");

        const Real num = A0 + eta * (A1 + eta * (A2 + eta * (A3 + eta * (A4 + eta
                    * (A5 + eta * (A6 + eta * A7))))));

        const Real den = B0 + eta * (B1 + eta * (B2 + eta * (B3 + eta * (B4 + eta
                    * (B5 + eta * (B6 + eta * (B7 + eta * (B8 + eta * B9))))))));

        return std::sqrt(eta) * (num / den);

    }

    Real bachelierBlackFormulaImpliedVol(Option::Type optionType,
                                   Real strike,
                                   Real forward,
                                   Real tte,
                                   Real bachelierPrice,
                                   Real discount) {

        const static Real SQRT_QL_EPSILON = std::sqrt(QL_EPSILON);

        QL_REQUIRE(tte>0.0,
                   "tte (" << tte << ") must be positive");

        Real forwardPremium = bachelierPrice/discount;

        Real straddlePremium;
        if (optionType==Option::Call){
            straddlePremium = 2.0 * forwardPremium - (forward - strike);
        } else {
            straddlePremium = 2.0 * forwardPremium + (forward - strike);
        }

        Real nu = (forward - strike) / straddlePremium;
        QL_REQUIRE(nu<=1.0,
                   "nu (" << nu << ") must be <= 1.0");
        QL_REQUIRE(nu>=-1.0,
                     "nu (" << nu << ") must be >= -1.0");

        nu = std::max(-1.0 + QL_EPSILON, std::min(nu,1.0 - QL_EPSILON));

        // nu / arctanh(nu) -> 1 as nu -> 0
        Real eta = (std::fabs(nu) < SQRT_QL_EPSILON) ? 1.0 : nu / boost::math::atanh(nu);

        Real heta = h(eta);

        Real impliedBpvol = std::sqrt(M_PI / (2 * tte)) * straddlePremium * heta;

        return impliedBpvol;
    }
}
