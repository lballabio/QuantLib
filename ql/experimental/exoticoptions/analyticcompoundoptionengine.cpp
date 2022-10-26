/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Dimitri Reiswich

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

#include <ql/experimental/exoticoptions/analyticcompoundoptionengine.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <utility>

namespace QuantLib {

    namespace {

        // Helper Class needed to solve an implicit problem of finding a
        // spot to a corresponding option price.
        class ImpliedSpotHelper {
          public:
            ImpliedSpotHelper(DiscountFactor dividendDiscount,
                              DiscountFactor riskFreeDiscount,
                              Real standardDeviation,
                              ext::shared_ptr<PlainVanillaPayoff> payoff,
                              Real strike)
            : dividendDiscount_(dividendDiscount), riskFreeDiscount_(riskFreeDiscount),
              standardDeviation_(standardDeviation), strike_(strike), payoff_(std::move(payoff)) {}
            Real operator()(Real spot) const {
                Real forwardPrice = spot*dividendDiscount_/riskFreeDiscount_;
                Real value = blackFormula(payoff_, forwardPrice,
                                          standardDeviation_,riskFreeDiscount_);
                return value - strike_;
            }
          private:
            DiscountFactor dividendDiscount_;
            DiscountFactor riskFreeDiscount_;
            Real standardDeviation_;
            Real strike_;
            ext::shared_ptr<PlainVanillaPayoff> payoff_;
        };

    }

    AnalyticCompoundOptionEngine::AnalyticCompoundOptionEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)) {
        registerWith(process_);
    }

    void AnalyticCompoundOptionEngine::calculate() const {

        QL_REQUIRE(strikeDaughter()>0.0,
                   "Daughter strike must be positive");

        QL_REQUIRE(strikeMother()>0.0,
                   "Mother strike must be positive");

        QL_REQUIRE(spot() > 0.0, "negative or null underlying given");

        /* Solver Setup ***************************************************/
        Date helpDate(process_->riskFreeRate()->referenceDate());
        Date helpMaturity=helpDate+(maturityDaughter()-maturityMother())*Days;
        Real vol =process_->blackVolatility()->blackVol(helpMaturity,
                                                        strikeDaughter());

        Time helpTimeToMat=process_->time(helpMaturity);
        vol=vol*std::sqrt(helpTimeToMat);

        DiscountFactor dividendDiscount =
            process_->dividendYield()->discount(helpMaturity);

        DiscountFactor riskFreeDiscount =
            process_->riskFreeRate()->discount(helpMaturity);


        ext::shared_ptr<ImpliedSpotHelper> f(
                new ImpliedSpotHelper(dividendDiscount, riskFreeDiscount,
                                      vol, payoffDaughter(), strikeMother()));

        Brent solver;
        solver.setMaxEvaluations(1000);
        Real accuracy = 1.0e-6;

        Real sSolved=solver.solve(*f, accuracy, strikeDaughter(), 1.0e-6, strikeDaughter()*1000.0);
        Real X=transformX(sSolved); // transform stock to return as in Wystup's book
        /* Solver Setup Finished*****************************************/

        Real phi=typeDaughter(); // -1 or 1
        Real w=typeMother(); // -1 or 1

        Real rho=std::sqrt(residualTimeMother()/residualTimeDaughter());
        BivariateCumulativeNormalDistributionDr78 N2(w*rho) ;

        DiscountFactor ddD=dividendDiscountDaughter();
        DiscountFactor rdD=riskFreeDiscountDaughter();
        //DiscountFactor ddM=dividendDiscountMother();
        DiscountFactor rdM=riskFreeDiscountMother();

        Real XmSM=X-stdDeviationMother();
        Real S=spot();
        Real dP=dPlus();
        Real dPT12=dPlusTau12(sSolved);
        Real vD=volatilityDaughter();

        Real dM=dMinus();
        Real strD=strikeDaughter();
        Real strM=strikeMother();
        Real rTM=residualTimeMother();
        Real rTD=residualTimeDaughter();

        Real rD=riskFreeRateDaughter();
        Real dD=dividendRateDaughter();

        Real N2XmSM=N2(-phi*w*XmSM,phi*dP);
        Real N2X=N2(-phi*w*X,phi*dM);
        Real NeX=N_(-phi*w*e(X));
        Real NX=N_(-phi*w*X);
        Real NT12=N_(phi*dPT12);
        Real ndP=n_(dP);
        Real nXm=n_(XmSM);
        Real invMTime=1/std::sqrt(rTM);
        Real invDTime=1/std::sqrt(rTD);

        Real tempRes=phi*w*S*ddD*N2XmSM-phi*w*strD*rdD*N2X-w*strM*rdM*NX;
        Real tempDelta=phi*w*ddD*N2XmSM;
        Real tempGamma=(ddD/(vD*S))*(invMTime*nXm*NT12+w*invDTime*ndP*NeX);
        Real tempVega=ddD*S*((1/invMTime)*nXm*NT12+w*(1/invDTime)*ndP*NeX);
        Real tempTheta=phi*w*dD*S*ddD*N2XmSM-phi*w*rD*strD*rdD*N2X-w*rD*strM*rdM*NX;
        tempTheta-=0.5*vD*S*ddD*(invMTime*nXm*NT12+w*invDTime*ndP*NeX);

        results_.value=tempRes;
        results_.delta=tempDelta;
        results_.gamma=tempGamma;
        results_.vega=tempVega;
        results_.theta=tempTheta;
    }

    Real AnalyticCompoundOptionEngine::typeDaughter() const {
        // returns -1 or 1 according to put or call
        return (Real) payoffDaughter()->optionType();
    }

    Real AnalyticCompoundOptionEngine::typeMother() const {
        return (Real) payoffMother()->optionType();
    }

    Date AnalyticCompoundOptionEngine::maturityDaughter() const {
        return arguments_.daughterExercise->lastDate();
    }

    Date AnalyticCompoundOptionEngine::maturityMother() const {
        return arguments_.exercise->lastDate();
    }

    Time AnalyticCompoundOptionEngine::residualTimeDaughter() const {
        return process_->time(maturityDaughter());
    }

    Time AnalyticCompoundOptionEngine::residualTimeMother() const {
        return process_->time(maturityMother());
    }

    Time AnalyticCompoundOptionEngine::residualTimeMotherDaughter() const {
        return residualTimeDaughter()-residualTimeMother();
    }


    Real AnalyticCompoundOptionEngine::volatilityDaughter() const {
        return process_->blackVolatility()->blackVol(maturityDaughter(),
                                                     strikeDaughter());
    }


    Real AnalyticCompoundOptionEngine::volatilityMother() const {
        return process_->blackVolatility()->blackVol(maturityMother(),
                                                     strikeMother());
    }

    Real AnalyticCompoundOptionEngine::stdDeviationDaughter() const {
        return volatilityDaughter()*std::sqrt(residualTimeDaughter());
    }

    Real AnalyticCompoundOptionEngine::stdDeviationMother() const {
        return volatilityMother()*std::sqrt(residualTimeMother());
    }


    ext::shared_ptr<PlainVanillaPayoff>
    AnalyticCompoundOptionEngine::payoffDaughter() const {
        ext::shared_ptr<PlainVanillaPayoff> dPayoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(
                                                   arguments_.daughterPayoff);
        QL_REQUIRE(dPayoff, "non-plain payoff given");
        return dPayoff;
    }

    ext::shared_ptr<PlainVanillaPayoff>
    AnalyticCompoundOptionEngine::payoffMother() const {
        ext::shared_ptr<PlainVanillaPayoff> mPayoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(mPayoff, "non-plain payoff given");
        return mPayoff;
    }

    Real AnalyticCompoundOptionEngine::strikeMother() const {
        return payoffMother()->strike();
    }

    Real AnalyticCompoundOptionEngine::strikeDaughter() const {
        return payoffDaughter()->strike();
    }

    DiscountFactor AnalyticCompoundOptionEngine::riskFreeDiscountDaughter() const {
        return process_->riskFreeRate()->discount(residualTimeDaughter());
    }

    DiscountFactor AnalyticCompoundOptionEngine::riskFreeDiscountMother() const {
        return process_->riskFreeRate()->discount(residualTimeMother());
    }

    DiscountFactor AnalyticCompoundOptionEngine::riskFreeDiscountMotherDaughter() const {
        return process_->riskFreeRate()->discount(residualTimeMotherDaughter());
    }

    DiscountFactor AnalyticCompoundOptionEngine::dividendDiscountDaughter() const {
        return process_->dividendYield()->discount(residualTimeDaughter());
    }

    DiscountFactor AnalyticCompoundOptionEngine::dividendDiscountMother() const {
        return process_->dividendYield()->discount(residualTimeMother());
    }

    DiscountFactor AnalyticCompoundOptionEngine::dividendDiscountMotherDaughter() const {
        return process_->dividendYield()->discount(residualTimeMotherDaughter());
    }

    Real AnalyticCompoundOptionEngine::dPlus() const {
        Real forward = spot() * dividendDiscountDaughter() / riskFreeDiscountDaughter();
        Real sd=stdDeviationDaughter();
        return std::log(forward/strikeDaughter())/sd+0.5*sd;
    }

    Real AnalyticCompoundOptionEngine::dMinus() const {
        return dPlus()-stdDeviationDaughter();
    }

    Real AnalyticCompoundOptionEngine::dPlusTau12(Real S) const {
        Real forward = S * dividendDiscountMotherDaughter() / riskFreeDiscountMotherDaughter();
        Real sd=volatilityDaughter()*std::sqrt(residualTimeMotherDaughter());
        return std::log(forward/strikeDaughter())/sd+0.5*sd;
    }

    Real AnalyticCompoundOptionEngine::spot() const {
        return process_->x0();
    }

    Real AnalyticCompoundOptionEngine::riskFreeRateDaughter() const {
        return process_->riskFreeRate()->zeroRate(residualTimeDaughter(),
                                                  Continuous,
                                                  NoFrequency);
    }

    Real AnalyticCompoundOptionEngine::dividendRateDaughter() const {
        return process_->dividendYield()->zeroRate(residualTimeDaughter(),
                                                   Continuous,
                                                   NoFrequency);
    }

    Real AnalyticCompoundOptionEngine::transformX(Real X) const {

        Real sd=stdDeviationMother();
        Real resX=riskFreeDiscountMother()*X/(spot()*dividendDiscountMother());
        resX=resX*std::exp(0.5*sd*sd);
        resX=std::log(resX);

        return resX/sd;
    }

    Real AnalyticCompoundOptionEngine::e(Real X) const {
        Real rtM=residualTimeMother();
        Real rtD=residualTimeDaughter();

        return (X*std::sqrt(rtD)+std::sqrt(rtM)*dMinus())/std::sqrt(rtD-rtM);
    }

}
