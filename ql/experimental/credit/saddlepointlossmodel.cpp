/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Jose Aparicio

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

#include <ql/experimental/credit/saddlepointlossmodel.hpp>

namespace QuantLib {

    /* ------------------------------------------------------------------------
                    Conditional Moments and derivatives. 

        Notice that in all this methods the date dependence is implicitly
        present in the unconditional probabilities. But, as in other LMs, it
        is redundant and expensive to perform the call to the probabilities in
        these methods since they are integrands.
       ---------------------------------------------------------------------- */

    Real SaddlePointLossModel::CumulantGeneratingCond(
        const std::vector<Probability>& uncondProbs,
        ////////////////////////const std::vector<Real>& uncondRRs,
        ///const Date& date, 
        Real lossFraction,
        const std::vector<Real>&  mktFactor) const 
    {
        const Size nNames = remainingNotionals_.size();
        Real sum = 0.;

        for(Size iName=0; iName < nNames; iName++) {
            Probability pBuffer = 
                copula_->conditionalDefaultProbability(uncondProbs[iName], 
                    iName, mktFactor);
            sum += std::log(1. - pBuffer + 
                pBuffer * std::exp(remainingNotionals_[iName] * 
                (1.-copula_->conditionalRecovery(uncondProbs[iName], iName, mktFactor)) ////////////////////////////////////////////////////////
                * lossFraction / remainingNotional_));
        }
       return sum;
    }

    Real SaddlePointLossModel::CumGen1stDerivativeCond(
        const std::vector<Probability>& uncondProbs,
        Real saddle,
        const std::vector<Real>&  mktFactor) const 
    {
        const Size nNames = remainingNotionals_.size();
        Real sum = 0.;

        for(Size iName=0; iName < nNames; iName++) {
            Probability pBuffer = 
                copula_->conditionalDefaultProbability(uncondProbs[iName], 
                    iName, mktFactor);
            // loss in fractional units
            Real lossInDef = remainingNotionals_[iName] * 
                (1.-copula_->conditionalRecovery(uncondProbs[iName], iName, mktFactor)) ////////////////////////////////////////////////////////
                / remainingNotional_;
            Real midFactor = pBuffer * std::exp(lossInDef * saddle);
            sum += lossInDef * midFactor / (1.-pBuffer + midFactor);
        }
       return sum;
    }

    Real SaddlePointLossModel::CumGen2ndDerivativeCond(
        const std::vector<Probability>& uncondProbs,
        Real saddle, 
        const std::vector<Real>&  mktFactor) const 
    {
        const Size nNames = remainingNotionals_.size();
        Real sum = 0.;

        for(Size iName=0; iName < nNames; iName++) {
            Probability pBuffer = 
                copula_->conditionalDefaultProbability(uncondProbs[iName], 
                    iName, mktFactor);
            // loss in fractional units
            Real lossInDef = remainingNotionals_[iName] * 
                (1.-copula_->conditionalRecovery(uncondProbs[iName], iName, mktFactor)) ////////////////////////////////////////////////////////
                / remainingNotional_;
            Real midFactor = pBuffer * std::exp(lossInDef * saddle);
            Real denominator = 1.-pBuffer + midFactor;
            sum += lossInDef * lossInDef * midFactor / denominator - 
                std::pow(lossInDef * midFactor / denominator , 2.);
        }
       return sum;
    }

    Real SaddlePointLossModel::CumGen3rdDerivativeCond(
        const std::vector<Probability>& uncondProbs,
        Real saddle, 
        const std::vector<Real>&  mktFactor) const 
    {
        const Size nNames = remainingNotionals_.size();
        Real sum = 0.;

        for(Size iName=0; iName < nNames; iName++) {
            Probability pBuffer = 
                copula_->conditionalDefaultProbability(uncondProbs[iName], 
                    iName, mktFactor);
            Real lossInDef = remainingNotionals_[iName] * 
                (1.-copula_->conditionalRecovery(uncondProbs[iName], iName, mktFactor)) ////////////////////////////////////////////////////////
                / remainingNotional_;

            const Real midFactor = pBuffer * std::exp(lossInDef * saddle);
            const Real denominator = 1.-pBuffer + midFactor;

            const Real& suma0 = denominator;
            const Real suma1  = lossInDef * midFactor;
            const Real suma2  = lossInDef * suma1;
            const Real suma3  = lossInDef * suma2;

            sum += (suma3 + (2.*std::pow(suma1, 3.)/suma0 - 
                3.*suma1*suma2)/suma0)/suma0;
        }
       return sum;
    }

    Real SaddlePointLossModel::CumGen4thDerivativeCond(
        const std::vector<Probability>& uncondProbs,
        Real saddle, 
        const std::vector<Real>&  mktFactor) const 
    {
        const Size nNames = remainingNotionals_.size();
        Real sum = 0.;

        for(Size iName=0; iName < nNames; iName++) {
            Probability pBuffer = 
                copula_->conditionalDefaultProbability(uncondProbs[iName], 
                    iName, mktFactor);
            Real lossInDef = remainingNotionals_[iName] * 
                (1.-copula_->conditionalRecovery(uncondProbs[iName], iName, mktFactor)) ////////////////////////////////////////////////////////
                / remainingNotional_;

            Real midFactor = pBuffer * std::exp(lossInDef * saddle);
            Real denominator = 1.-pBuffer + midFactor;

            const Real& suma0 = denominator;
            const Real suma1  = lossInDef * midFactor;
            const Real suma2  = lossInDef * suma1;
            const Real suma3  = lossInDef * suma2;
            const Real suma4  = lossInDef * suma3;

            sum += (suma4 + (-4.*suma1*suma3 - 3.*suma2*suma2 + 
                (12.*suma1*suma1*suma2 - 
                    6.*std::pow(suma1,4.)/suma0)/suma0)/suma0)/suma0;
        }
       return sum;
    }

    boost::tuples::tuple<Real, Real, Real, Real> /// DISPOSABLE????
        SaddlePointLossModel::CumGen0234DerivCond(
        const std::vector<Probability>& uncondProbs,
        Real saddle, 
        const std::vector<Real>&  mktFactor) const 
    {
        const Size nNames = remainingNotionals_.size();
        Real deriv0 = 0.,
             //deriv1 = 0.,
             deriv2 = 0.,
             deriv3 = 0.,
             deriv4 = 0.;
        for(Size iName=0; iName < nNames; iName++) {
            Probability pBuffer = 
                copula_->conditionalDefaultProbability(uncondProbs[iName], 
                    iName, mktFactor);
            Real lossInDef = remainingNotionals_[iName] * 
                (1.-copula_->conditionalRecovery(uncondProbs[iName], iName, mktFactor)) ////////////////////////////////////////////////////////
                / remainingNotional_;

            Real midFactor = pBuffer * std::exp(lossInDef * saddle);
            Real denominator = 1.-pBuffer + midFactor;

            const Real& suma0 = denominator;
            const Real suma1  = lossInDef * midFactor;
            const Real suma2  = lossInDef * suma1;
            const Real suma3  = lossInDef * suma2;
            const Real suma4  = lossInDef * suma3;

            // To do: optimize these:
            deriv0 += std::log(suma0);
            //deriv1 += suma1 / suma0;
            deriv2 += suma2 / suma0 - std::pow(suma1 / suma0 , 2.);
            deriv3 += (suma3 + (2.*std::pow(suma1, 3.)/suma0 - 
                3.*suma1*suma2)/suma0)/suma0;
            deriv4 += (suma4 + (-4.*suma1*suma3 - 3.*suma2*suma2 + 
                (12.*suma1*suma1*suma2 - 
                    6.*std::pow(suma1,4.)/suma0)/suma0)/suma0)/suma0;
        }
        return boost::tuples::tuple<Real, Real, Real, Real>(deriv0, deriv2, 
            deriv3, deriv4);
    }

    boost::tuples::tuple<Real, Real> /// DISPOSABLE???? 
        SaddlePointLossModel::CumGen02DerivCond(
        const std::vector<Probability>& uncondProbs,
        Real saddle, 
        const std::vector<Real>&  mktFactor) const 
    {
        const Size nNames = remainingNotionals_.size();
        Real deriv0 = 0.,
             //deriv1 = 0.,
             deriv2 = 0.;
        for(Size iName=0; iName < nNames; iName++) {
            Probability pBuffer = 
                copula_->conditionalDefaultProbability(uncondProbs[iName], 
                    iName, mktFactor);
            Real lossInDef = remainingNotionals_[iName] * 
                (1.-copula_->conditionalRecovery(uncondProbs[iName], iName, mktFactor)) ////////////////////////////////////////////////////////
                / remainingNotional_;

            Real midFactor = pBuffer * std::exp(lossInDef * saddle);
            Real denominator = 1.-pBuffer + midFactor;

            const Real& suma0 = denominator;
            const Real suma1  = lossInDef * midFactor;
            const Real suma2  = lossInDef * suma1;

            // To do: optimize these:
            deriv0 += std::log(suma0);
            //deriv1 += suma1 / suma0;
            deriv2 += suma2 / suma0 - std::pow(suma1 / suma0 , 2.);
        }
        return boost::tuples::tuple<Real, Real>(deriv0, deriv2);
    }






    // ----- Saddle point search ----------------------------------------------


    /*! Calculates the mkt-fct-conditional saddle point for the loss level 
        given and the probability passed. 
        The date is implicitly given through the probability. Performance 
        requires to pass the probabilities for that date. Otherwise once we 
        integrate this over the market factor we would be computing the same 
        probabilities over and over. While this works fine here some models of 
        the recovery rate might require the date.
    */
    Real SaddlePointLossModel::findSaddle(
        const std::vector<Probability>& uncondPs,
        Real lossLevel, // in total portfolio loss fractional unit 
        const std::vector<Real>& mktFactor, 
        Real accuracy,
        Natural maxEvaluations
        ) const 
    {
        // \to do:
        // REQUIRE that loss level is below the max loss attainable in 
        //   the portfolio, otherwise theres no solution...
        SaddleObjectiveFunction f(*this, lossLevel, uncondPs, mktFactor);

        Size nNames = remainingNotionals_.size();
        std::vector<Real> lgds;
        for(Size iName=0; iName<nNames; iName++)
            lgds.push_back(remainingNotionals_[iName] * 
            (1.-copula_->conditionalRecovery(uncondPs[iName], iName, 
                mktFactor)) );

        // computed limits:
        // position of the name with the largest relative exposure loss (i.e.:
        //   largest: N_i LGD_i / N_{total})
        Size iNamMax = std::distance(lgds.begin(), 
            std::max_element(lgds.begin(), lgds.end()) );
        // gap to be considered zero at the negative side of the logistic 
        //   inversion:
        static const Real deltaMin = 1.e-5;
        //
        Probability pMaxName = copula_->conditionalDefaultProbability(
            uncondPs[iNamMax], iNamMax, mktFactor);
        // aproximates the  saddle pt corresponding to this minimum; finds 
        //   it by using only the smallest logistic term and thus this is 
        //   smaller than the true value:
        Real saddleMin = 1./(lgds[iNamMax]/remainingNotional_) * 
            std::log(deltaMin*(1.-pMaxName)/
                (pMaxName*lgds[iNamMax]/remainingNotional_-pMaxName*deltaMin));
        // and the associated minimum loss is approximately: (this is thence 
        //   the minimum loss we can resolve/invert)
        Real minLoss = CumGen1stDerivativeCond(uncondPs, saddleMin, mktFactor);

        // If we are below the loss resolution it returns approximating 
        //  by the minimum/maximum attainable point. Typically the functionals
        //  to integrate will have a low dependency on this point.
        if(lossLevel < minLoss) return saddleMin;
        static const Real deltaMax = 1.e-9;
        Real saddleMax = 1./(lgds[iNamMax]/remainingNotional_) * 
            std::log((lgds[iNamMax]/remainingNotional_
                -deltaMin)*(1.-pMaxName)/(pMaxName*deltaMin));
        Real maxLoss = 
            CumGen1stDerivativeCond(uncondPs, saddleMax, mktFactor);
        if(lossLevel > maxLoss) return saddleMax;

        Brent solverBrent;
        solverBrent.setMaxEvaluations(maxEvaluations);
        return solverBrent.solve(f, accuracy, (saddleMin+saddleMax)/2., 
            saddleMin, saddleMax);
    }





    // ----- Statistics -------------------------------------------------------


    Real SaddlePointLossModel::percentile(const Date& d, 
        Probability percentile) const 
    {
        //this test should be in the calling basket...?
        QL_REQUIRE(percentile >=0. && percentile <=1., 
            "Incorrect percentile value.");

        // still this does not tackle the situation where we have cumulated 
        //   losses from previous defaults:
        if(d <= Settings::instance().evaluationDate()) return 0.;

        // Trivial cases when the percentile is outside the prob range 
        //   associated to the tranche limits:
        if(percentile <= 1.-probOverLoss(d, 0.)) return 0.;
        if(percentile >= 1.-probOverLoss(d, 1.)) 
            return basket_->remainingTrancheNotional();

        SaddlePercObjFunction f(*this, percentile, d);
        Brent solver;
        solver.setMaxEvaluations(100);
        Real minVal = QL_EPSILON;

        // PREVIOUS SOLUTION (expensive) 
        //if(probOverLoss(d, 1.e-6) <= 1.-percentile) return 0.;
        // ON A TRANCHED PORTFOLIO I CAN NOT REQUEST ANY PERCENTILE. ATTAINABLE
        //   ONES OSCILATE BETWEEN THE ONES CORRESPONDING TO THE TRANCHE LIMITS.
        Real maxVal = 1.-QL_EPSILON; 
        Real guess = 0.5;

        Real solut = solver.solve(f, 1.e-4, guess, minVal, maxVal);
        return basket_->remainingTrancheNotional() * solut;
    }

    Probability SaddlePointLossModel::probOverLossCond(
        const std::vector<Probability>& uncondPs,
        Real trancheLossFract, 
        const std::vector<Real>& mktFactor) const {
        Real portfFract = attachRatio_ + trancheLossFract * 
            (detachRatio_-attachRatio_);// these are remaining ratios
        
        // for non-equity losses add the probability jump at zero tranche 
        //   losses (since this method returns prob of losing more or 
        //   equal to)
        ////////////////---       if(trancheLossFract <= QL_EPSILON) return 1.;
        return 
            probOverLossPortfCond(uncondPs,
            //below; should substract realized loses. Use remaining amounts??
                portfFract * basket_->basketNotional(),
                mktFactor);
    }

    Disposable<std::map<Real, Probability> > 
        SaddlePointLossModel::lossDistribution(const Date& d) const {
        std::map<Real, Probability> distrib;
        static const Real numPts = 500.;
        for(Real lossFraction=1./numPts; lossFraction<0.45; lossFraction+= 1./numPts) {
            // FOR FIXED RECOVERY MODEL I SHOULD BE USING THE MAX LOSS ATTAINABLE--------------------------
            distrib.insert(std::make_pair<Real, Probability>((lossFraction) * remainingNotional_ , 
                // maybe call the full structure????
            //    1.-probOverLoss(d, lossFraction)));
             // 1.-  probOverPortfLoss(d, lossFraction)));
              1.-  probOverPortfLoss(d, lossFraction* remainingNotional_ )));
        }
        return distrib;
    }

    /*!  NOTICE THIS IS ON THE TOTAL PORTFOLIO ---- UNTRANCHED..............
        Probability of having losses in the portfolio due to default 
        events equal or larger than a given absolute loss value on a 
        given date conditional to the latent model factor.
        The integral expression on the expansion is the first order 
        integration as presented in several references, see for instance; 
        equation 8 in R.Martin, K.Thompson, and C. Browne 's 
        'Taking to the Saddle', Risk Magazine, June 2001, page 91

        @param loss loss in absolute value

        to do: behaves like theres a missing term, e.g. returns 0 for 0 losses and is 
        displaced with respect to the other models.
    */
    Probability SaddlePointLossModel::probOverLossPortfCond(
        const std::vector<Probability>& uncondProbs,
        Real loss, 
        const std::vector<Real>& mktFactor) const 
    {
       // return probOverLossPortfCond1stOrder(d, loss, mktFactor);
        if (loss <= QL_EPSILON) return 1.;

        Real relativeLoss = loss / remainingNotional_;
        if (relativeLoss >= 1.-QL_EPSILON) return 0.;

        const Size nNames = remainingNotionals_.size();

        Real averageRecovery_ = 0.;
        for(Size iName=0; iName < nNames; iName++)
            averageRecovery_ += 
            copula_->conditionalRecovery(uncondProbs[iName], iName, mktFactor);  ////////////////////////////////////////////////////////////////
        averageRecovery_ = averageRecovery_ / nNames;/////////ACCUMULATE + BIND

        Real maxAttLossFract = 1.-averageRecovery_;
        if(relativeLoss > maxAttLossFract) return 0.;

        Real saddlePt = findSaddle(uncondProbs,
            ///d, 
            relativeLoss, mktFactor);

        boost::tuples::tuple<Real, Real, Real, Real> cumulants = 
            CumGen0234DerivCond(uncondProbs, 
                //d, 
                saddlePt, mktFactor);
        Real baseVal = cumulants.get<0>();
        Real secondVal = cumulants.get<1>();
        Real K3Saddle = cumulants.get<2>();
        Real K4Saddle = cumulants.get<3>();

        Real saddleTo2 = saddlePt * saddlePt;
        Real saddleTo3 = saddleTo2 * saddlePt;
        Real saddleTo4 = saddleTo3 * saddlePt;
        Real saddleTo6 = saddleTo4 * saddleTo2;
        Real K3SaddleTo2 = K3Saddle*K3Saddle;

        if(saddlePt > 0.) { // <-> (loss > condEL)
            Real exponent = baseVal - relativeLoss * saddlePt + 
                .5 * saddleTo2 * secondVal;
            if( std::abs(exponent) > 700.) return 0.;
            return 
                std::exp(exponent)
                * CumulativeNormalDistribution()(-std::abs(saddlePt)*
                    std::sqrt(/*saddleTo2 **/secondVal))

                // high order corrections:
                * (1. - saddleTo3*K3Saddle/6. + saddleTo4*K4Saddle/24. + 
                    saddleTo6*K3SaddleTo2/72.) 
                /*
                // FIX ME: this term introduces at times numerical 
                //   instabilty (shows up in percentile computation)
                + (3.*secondVal*(1.-secondVal*saddleTo2)*
                        (saddlePt*K4Saddle-4.*K3Saddle)
                    - saddlePt*K3SaddleTo2*(3.-saddleTo2*secondVal + 
                            saddleTo4*secondVal*secondVal)) 
                     / (72.*M_SQRTPI*M_SQRT_2*std::pow(secondVal, 5./2.) ) 
                 */
                 ;
        }else if(saddlePt==0.){// <-> (loss == condEL)
            return .5;
        }else {// <->(loss < condEL)
            Real exponent = baseVal - relativeLoss * saddlePt + 
                .5 * saddleTo2 * secondVal;
            if( std::abs(exponent) > 700.) return 0.;
            return 
                1.-
                std::exp(exponent)
                * CumulativeNormalDistribution()(-std::abs(saddlePt)
                    * std::sqrt(/*saddleTo2 **/secondVal))// static call?

                // high order corrections:
                * (1. - saddleTo3*K3Saddle/6. + saddleTo4*K4Saddle/24. + 
                    saddleTo6*K3SaddleTo2/72.) 
                /*
                  + (3.*secondVal*(1.-secondVal*saddleTo2)*
                    (saddlePt*K4Saddle-4.*K3Saddle)
                  - saddlePt*K3SaddleTo2*(3.-saddleTo2*secondVal +
                        saddleTo4*secondVal*secondVal)) 
                    / (72.*M_SQRTPI*M_SQRT_2*std::pow(secondVal, 5./2.) ) 
                */
                ;
        }
    }


    Probability SaddlePointLossModel::probOverLossPortfCond1stOrder(
        const std::vector<Probability>& uncondPs,
        Real loss, 
        const std::vector<Real>& mktFactor) const 
    {
        if (loss <= QL_EPSILON) return 1.;
        const Size nNames = remainingNotionals_.size();

        Real relativeLoss = loss / remainingNotional_;
        if(relativeLoss >= 1.-QL_EPSILON) return 0.;

        // only true for constant recovery models......?
        Real averageRecovery_ = 0.;
        for(Size iName=0; iName < nNames; iName++)
            averageRecovery_ += 
            copula_->conditionalRecovery(uncondPs[iName], iName, mktFactor);  ///////////////////////////////////////////////////////////////////////
        averageRecovery_ = averageRecovery_ / nNames;/////////ACCUMULATE + BIND

        Real maxAttLossFract = 1.-averageRecovery_;
        if(relativeLoss > maxAttLossFract) return 0.;

        Real saddlePt = findSaddle(uncondPs,
            ////d, 
            relativeLoss, mktFactor);

        boost::tuples::tuple<Real, Real> cumulants = 
            CumGen02DerivCond(uncondPs,
                ////d, 
                saddlePt, mktFactor);
        Real baseVal = cumulants.get<0>();
        Real secondVal = cumulants.get<1>();

        Real saddleTo2 = saddlePt * saddlePt;

        if(saddlePt > 0.) { // <-> (loss > condEL)
            Real exponent = baseVal - relativeLoss * saddlePt + 
                .5 * saddleTo2 * secondVal;
            if( std::abs(exponent) > 700.) return 0.;
            return 
                // dangerous exponential; fix me
                std::exp(exponent)
                //////  std::exp(baseVal - relativeLoss * saddlePt + .5 * saddleTo2 * secondVal)
                * CumulativeNormalDistribution()(-std::abs(saddlePt)*
                    std::sqrt(/*saddleTo2 **/secondVal));
        }else if(saddlePt==0.){// <-> (loss == condEL)
            return .5;
        }else {// <->(loss < condEL)
            Real exponent = baseVal - relativeLoss * saddlePt + 
                .5 * saddleTo2 * secondVal;
            if( std::abs(exponent) > 700.) return 0.;

            return 
                1.-
               //// std::exp(baseVal - relativeLoss * saddlePt + .5 * saddleTo2 * secondVal)
                std::exp(exponent)
                * CumulativeNormalDistribution()(-std::abs(saddlePt)*
                    std::sqrt(/*saddleTo2 **/secondVal));
        }
    }


    /*!   NOTICE THIS IS ON THE TOTAL PORTFOLIO ---- UNTRANCHED
    Probability density of having losses in the portfolio due to default 
        events equal to a given value on a given date conditional to the w
        latent model factor.
        Based on the integrals of the expected shortfall. See......refernce.
    */
    Probability SaddlePointLossModel::probDensityCond(
        const std::vector<Probability>& uncondPs,
        Real loss,
        const std::vector<Real>& mktFactor) const 
    {
        if (loss <= QL_EPSILON) return 0.;

        Real relativeLoss = loss / remainingNotional_;
        Real saddlePt = findSaddle(uncondPs,
            relativeLoss, mktFactor);

        boost::tuples::tuple<Real, Real, Real, Real> cumulants = 
            CumGen0234DerivCond(uncondPs,
            saddlePt, mktFactor);
        Real K0Saddle = cumulants.get<0>();/// ACCESS THEM DIRECTLY BELOW RATHER THAN THIS COPY!
        Real K2Saddle = cumulants.get<1>();
        Real K3Saddle = cumulants.get<2>();
        Real K4Saddle = cumulants.get<3>();
        /* see, for instance R.Martin "he saddle point method and portfolio 
        optionalities." in Risk December 2006 p.93 */
        Real cum2DerCond = CumGen2ndDerivativeCond(uncondPs,
            saddlePt, mktFactor);
        //\todo the exponentials below are dangerous and agressive, tame them.
        return 
            (
            1.
            + K4Saddle
                /(8.*std::pow(K2Saddle, 2.))
            - 5.*std::pow(K3Saddle,2.)
                /(24.*std::pow(K2Saddle, 3.))
            ) * std::exp(K0Saddle - saddlePt * relativeLoss)
             / (std::sqrt(2. * M_PI * K2Saddle)*remainingNotional_);
    }

    /*! Conditional expected equity-tranche loss. The integration on the Saddle point
      expansion (to first order, see corrections in the reference) to obtain 
      this value is described on equation 57 (section 4.2) of
      'Analytical techniques for synthetic CDOs and credit default risk 
      measures', by A.Antonov, S.Mechkov and T.Misirpashaev, Numerix paper, 
      May 2005

      @param lossRatio In portfolio fractional units.

      FAILING....
    */
////////    Real SaddlePointLossModel::expectedEquityLossCond(
////////        const std::vector<Probability>& uncondPs,
////////        //const Date& d, 
////////        Real lossRatio, 
////////        const std::vector<Real>& mktFactor) const 
////////    {
////////        if(lossRatio < QL_EPSILON) return 0.;
////////        // avoid failing saddle:
////////        Date today = Settings::instance().evaluationDate();
////////        if(d <= today) return 0.;
////////        // failing when the tranche limit lies above the attainable losses..... 
////////                //....??? this->recoveryValueImpl
////////                /*
////////when requesting non attainable losses the saddle finding point will fail to find its root finding limits (not the case for an stochastic recovery rate).
////////                */
////////                // true for a constant RR model:
/////////////calculate();
////////   ////     Basket_N remainingBasket = basket_->remainingBasket(refDate, d);
////////        const Size nNames = remainingNotionals_.size();
////////
////////      //  const std::vector<Real>& nots = remainingBasket_.notionals();
////////        std::vector<Real> recoveries;// should be LGDs
////////        for(Size iName=0; iName<nNames; iName++)
////////            recoveries.push_back(1.-recoveryValueImpl(Date(), iName));// should be LGDs
////////          //23 Aug :   recoveries.push_back(recoveryValueImpl(d, iName));// should be LGDs
////////        Real maxAttainableLoss = // should be LGDs
////////            std::inner_product(recoveries.begin(), recoveries.end(), remainingNotionals_.begin(), 0.) / remainingNotional_;
////////       //     std::accumulate(nots.begin(), nots.end(), 0.);
////////
////////    //    if(lossRatio >= maxAttainableLoss) lossRatio = maxAttainableLoss - QL_EPSILON;
////////    //    Real saddle = findSaddle(/*refDate,*/ d, lossRatio, mktFactor);
////////
////////        lossRatio = 
////////            lossRatio >= maxAttainableLoss ?  maxAttainableLoss - 1.e-2 : lossRatio;
////////        Real saddle = findSaddle(/*refDate,*/ uncondPs,
////////            ////d, 
////////            lossRatio, 
////////           // lossRatio >= maxAttainableLoss ?  maxAttainableLoss - 1.e-2 : lossRatio,// due to this being a saddle expansion to second order we have to take this big a cut
////////            mktFactor);
////////
////////        boost::tuples::tuple<Real, Real, Real, Real> cumulants = 
////////            CumGen0234DerivCond(uncondPs,
////////            ////d, 
////////            saddle, mktFactor);
////////        Real KSaddle  = cumulants.get<0>();//// USE THEM DIRECTLY -- NO COPY
////////        Real K2Saddle = cumulants.get<1>();
////////        Real K3Saddle = cumulants.get<2>();
////////  // not needed here...      Real K4Saddle = cumulants.get<3>();
////////
////////        Real absSaddle = std::abs(saddle);
////////        //Real KSaddle = CumulantGeneratingCond(d, saddle, mktFactor);
////////        //Real K2Saddle = CumGen2ndDerivativeCond(d, saddle, mktFactor);
////////        //
////////    // no need, use saddle pt sign directly    Real condELratio = conditionalExpectedLoss(/*refDate,*/ d, mktFactor) / remainingNotional_;
////////        //
////////
////////
////////    //    Real heavy = saddle > 0. ? 0. : 1.;
////////
////////    //    Real f1 = std::exp(KSaddle-saddle * lossRatio * remainingNotional_);
////////        Real f1 = std::exp(KSaddle-saddle * lossRatio);
////////        Real s1 = std::sqrt(K2Saddle/M_TWOPI);
////////        //Real s2 = CumulativeNormalDistribution()(-absSaddle * std::sqrt(K2Saddle));
////////        Real s2 = CumulativeNormalDistribution()(-absSaddle * std::sqrt(K2Saddle));
////////        Real sf4 ;//= std::exp(0.5 * K2Saddle * saddle * saddle);
////////        // numerical stability:
////////        if(s2<QL_EPSILON) {
////////            sf4 = 0.;
////////        }else{
////////            sf4 = std::exp(0.5 * K2Saddle * saddle * saddle);
////////        }
////////        Real eLequity = f1 * (s1 - K2Saddle * absSaddle * s2 * sf4);
////////        /*
////////                Real eLequity = std::exp(KSaddle-saddle * lossRatio) *
////////                    (std::sqrt(K2Saddle/M_TWOPI) - K2Saddle * absSaddle * 
////////                      CumulativeNormalDistribution()(-absSaddle * std::sqrt(K2Saddle)) * 
////////                        std::exp(0.5 * K2Saddle * saddle * saddle));
////////        */
////////
////////
////////          //      if(lossRatio < condELratio)
////////        if(saddle<0.)
////////            eLequity += conditionalExpectedLoss(uncondPs,
////////                //d, 
////////                mktFactor) - 
////////                lossRatio * remainingNotional_;/// remainingBasket_.basketNotional();
////////        /*
////////                    eLequity += heavy * (condELratio - lossRatio);
////////        */
////////        // first correction (eq. 58 on p.15): (presumably I will have the same problem as before above)
////////        /**/
////////
////////    //    return eLequity;
////////
////////        ///   Real K3Saddle = CumGen3rdDerivativeCond(d, saddle, mktFactor);
////////        eLequity += (1./6. ) * saddle * K3Saddle * f1 * 
////////            (-1./(s1*M_PI) + (3. + K2Saddle)*absSaddle*s2*sf4-saddle*saddle*s1);
////////          //  (-1./(s1*M_PI) + (3. + K2Saddle)*absSaddle*s2*std::exp(.5*K2Saddle*saddle*saddle)-saddle*saddle*s1);
////////
////////
////////        return eLequity;
////////    }
////////


    /*!    NOTICE THIS IS ON THE TOTAL PORTFOLIO ---- UNTRANCHED...........................................
        Sensitivities of the individual names to a given portfolio loss value due to defaults. It returns ratios to the total structure notional, which aggregated add up to the requested loss value.

    see equation 8 in 'VAR: who contributes and how much?' by R.Martin, K.Thompson, and C. Browne in Risk Magazine, August 2001
    @param loss Loss amount level at which we want to request the sensitivity. Equivalent to a percentile.
    */
    Disposable<std::vector<Real> > SaddlePointLossModel::splitLossCond(
        const std::vector<Probability>& uncondProbs,
        Real loss, 
        std::vector<Real> mktFactor) const ///// SHOULD BE RETURNING A FRACTION NOT ABSOLUTE VALUES......
    {
        const Size nNames = remainingNotionals_.size();
        if (loss <= QL_EPSILON) return std::vector<Real>(nNames, 0.);

        Real saddlePt = findSaddle(uncondProbs, loss / remainingNotional_, 
            mktFactor);

        std::vector<Real> condContrib(nNames, 0.);

        for(Size iName=0; iName<nNames; iName++) {
            Probability pBuffer = 
                copula_->conditionalDefaultProbability(uncondProbs[iName], 
                    iName, mktFactor);
            Real lossInDef = remainingNotionals_[iName] * 
                (1.-copula_->conditionalRecovery(uncondProbs[iName], iName,      //////////////////////////////
                    mktFactor));
            Real midFactor = pBuffer * 
                std::exp(lossInDef * saddlePt/ remainingNotional_);
            Real denominator = 1.-pBuffer + midFactor;

            condContrib[iName] = lossInDef * midFactor / denominator; 
        }
        return condContrib;
    }

    Real SaddlePointLossModel::conditionalExpectedLoss(
        const std::vector<Probability>& uncondProbs,
        const std::vector<Real>& mktFactor) const 
    {
        const Size nNames = remainingNotionals_.size();
        Real eloss = 0.;
        /// USE STL.....-------------------
        for(Size iName=0; iName < nNames; iName++) {
            Probability pBuffer = 
                copula_->conditionalDefaultProbability(uncondProbs[iName], 
                    iName, mktFactor);
            eloss += pBuffer * remainingNotionals_[iName] *
                (1.-copula_->conditionalRecovery(uncondProbs[iName], iName, mktFactor)); //////////////////
        }
        return eloss;
    }

    Real SaddlePointLossModel::conditionalExpectedTrancheLoss(
        const std::vector<Probability>& uncondProbs,
        const std::vector<Real>& mktFactor) const 
    {
        const Size nNames = remainingNotionals_.size();
        Real eloss = 0.;
        /// USE STL.....-------------------
        for(Size iName=0; iName < nNames; iName++) {
            Probability pBuffer = 
                copula_->conditionalDefaultProbability(uncondProbs[iName], iName, mktFactor);
            eloss += 
                pBuffer * remainingNotionals_[iName] * 
                (1.-copula_->conditionalRecovery(uncondProbs[iName], iName, mktFactor))//////////////////
                ;
        }
        return std::min(
            std::max(eloss - attachRatio_ * remainingNotional_, 0.), 
                (detachRatio_ - attachRatio_) * remainingNotional_)
                ;
    }

    // disposable?
    std::vector<Real> SaddlePointLossModel::expectedShortfallSplitCond(
        const std::vector<Probability>& uncondProbs,
        Real lossPerc, const std::vector<Real>& mktFactor) const 
    {
        const Size nNames = remainingNotionals_.size();
        std::vector<Real> lgds;
        for(Size iName=0; iName<nNames; iName++)
            lgds.push_back(remainingNotionals_[iName] * 
            (1.-copula_->conditionalRecovery(uncondProbs[iName], iName, mktFactor)) );////////////////////////
        std::vector<Real> vola(nNames, 0.), mu(nNames, 0.);
        Real volaTot = 0., muTot = 0.;
        for(Size iName=0; iName < nNames; iName++) {
            Probability pBuffer = 
                copula_->conditionalDefaultProbability(uncondProbs[iName], 
                    iName, mktFactor);
            mu[iName] = lgds[iName] * pBuffer / remainingNotionals_[iName];
            muTot += lgds[iName] * pBuffer;
            vola[iName] = lgds[iName] * lgds[iName] * pBuffer * (1.-pBuffer) 
                / remainingNotionals_[iName];
            volaTot += lgds[iName] * lgds[iName] * pBuffer * (1.-pBuffer) ;
        }
        std::for_each(vola.begin(), vola.end(), 
            std::bind1st(std::divides<Real>(), volaTot));
        for(Size iName=0; iName < nNames; iName++)
            vola[iName] = vola[iName] / volaTot;

        std::vector<Real> esfPartition(nNames, 0.);
        for(Size iName=0; iName < nNames; iName++) {
            Real uEdisp = (lossPerc-muTot)/std::sqrt(volaTot);
            esfPartition[iName] = mu[iName]
                * CumulativeNormalDistribution()(uEdisp) // static call?
                + vola[iName] * NormalDistribution()(uEdisp);
        }
        return esfPartition;
    }

    Real SaddlePointLossModel::expectedShortfallTrancheCond(
        const std::vector<Probability>& uncondProbs,
        Real lossPerc, // value 
        Probability percentile,
        const std::vector<Real>& mktFactor) const 
    {
        /* TO DO: this is too crude, a general expresion valid for all 
        situations is possible (with no extra cost as long as the loss limits 
        are checked).
        */
        //tranche correction term:
        Real correctionTerm = 0.;
        Real probLOver = probOverLossPortfCond(uncondProbs, 
            basket_->detachmentAmount(), mktFactor);
        if(basket_->attachmentAmount() > QL_EPSILON) {
            if(lossPerc < basket_->attachmentAmount()) {
                correctionTerm = ( (basket_->detachmentAmount() 
                    - 2.*basket_->attachmentAmount())*
                        probOverLossPortfCond(uncondProbs, lossPerc, mktFactor)
                            + basket_->attachmentAmount() * probLOver
                                )/(1.-percentile);
            }else{
                correctionTerm = ( (percentile-1)*basket_->attachmentAmount()
                    + basket_->detachmentAmount() * probLOver
                    )/(1.-percentile);
            }
        }

        return expectedShortfallFullPortfolioCond(uncondProbs, 
            std::max(lossPerc, basket_->attachmentAmount()), mktFactor)
            + expectedShortfallFullPortfolioCond(uncondProbs, 
                basket_->detachmentAmount(), mktFactor)
            - correctionTerm;
    }


    // ESF except by a factor equal to the tail probability.
    Real SaddlePointLossModel::expectedShortfallFullPortfolioCond(
        const std::vector<Probability>& uncondProbs,
        Real lossPerc, // value 
        const std::vector<Real>& mktFactor) const 
    {
        Real lossPercRatio = lossPerc  /remainingNotional_;
        Real elCond = 0.;
        const Size nNames = remainingNotionals_.size();

        /// use stl algorthms
        for(Size iName=0; iName < nNames; iName++) {
            Probability pBuffer = 
                copula_->conditionalDefaultProbability(uncondProbs[iName], 
                    iName, mktFactor);
            elCond += pBuffer * remainingNotionals_[iName] * 
                (1.-copula_->conditionalRecovery(uncondProbs[iName], /////////////////////////////////////
                    iName, mktFactor));
        }

        // calling the EL tranche
        // return elCond - expectedEquityLossCond(d, lossPercRatio, mktFactor);

        /* This version is based on: Martin 2006 paper and on the expression 
        in 'SaddlePoint approximation of expected shortfall for transformed 
        means' S.A. Broda and M.S.Paolella , Amsterdam School of Economics 
        discussion paper, available online.
        */
        Real elCondRatio = elCond / remainingNotional_;
        Real saddlePt = findSaddle(uncondProbs,
            lossPercRatio, mktFactor);

        boost::tuples::tuple<Real, Real, Real, Real> cumulants = 
            CumGen0234DerivCond(uncondProbs, 
                saddlePt, mktFactor);
        Real K0Saddle = cumulants.get<0>();///USE THEM DIRECTLY
        Real K2Saddle = cumulants.get<1>();

        Real wq = std::sqrt(2. * saddlePt * lossPercRatio - 2. * K0Saddle);
        //std::sqrt(-2. * saddlePt * lossPerc + 2. * K0Saddle);????
        Real factor = 1.;
        if(saddlePt<0.) {
            wq = -wq;
            factor = -1.;
        }

        Real numNames = static_cast<Real>(nNames);

        Real term1 = CumulativeNormalDistribution()(wq/* *std::sqrt(numNames)*/)
            * elCond ;
        Real term2 = .5 * M_2_SQRTPI * M_SQRT1_2 * (1./std::sqrt(numNames))
            * exp(-wq*wq * numNames/2.)*(elCond/wq - 
                lossPerc/(saddlePt * std::sqrt(K2Saddle)));
        // Broda and Paolella:
        ////   return term1 + term2;

        // Martin:
        return 
            //  ( 2. * M_SQRT1_2 / M_2_SQRTPI) *
            (elCond * probOverLossPortfCond(uncondProbs, lossPerc, mktFactor)
            + (.5 * M_2_SQRTPI * M_SQRT1_2) * (lossPerc - elCond) 
                * probDensityCond(uncondProbs, lossPerc, mktFactor) /saddlePt);

        return CumulativeNormalDistribution()(wq//*std::sqrt(numNames)
        ) * elCond + .5 * M_2_SQRTPI * M_SQRT1_2 * exp(-wq*wq//*numNames
        /2.)* (elCond/wq - lossPerc/(saddlePt * std::sqrt(K2Saddle)));

        return CumulativeNormalDistribution()(wq) * elCond +
            std::sqrt(1./M_2_PI)* exp(-wq*wq/2.)*(elCond/wq - 
                lossPercRatio/(saddlePt * std::sqrt(K2Saddle)));


/*
        const std::vector<Real> uncondProbs = basket_->remainingProbabilities(d);
        const Size nNames = remainingNotionals_.size();
        std::vector<Real> lgds;
        for(Size i=0; i<nNames; i++)
            lgds.push_back(remainingNotionals_[i] * (1.-recoveryValueImpl(d, i)) );//use RR from method provided to acces model RR...conditional...
        Real vola = 0., mu = 0.;
        for(Size iName=0; iName < nNames; iName++) {
            Probability pBuffer = 
                copula_->conditionalProbability(uncondProbs[iName], iName, mktFactor);
            mu += lgds[iName] * pBuffer;
            vola += lgds[iName] * lgds[iName] * pBuffer * (1.-pBuffer);
        }

return 
        mu * 
        probOverLossPortfCond(d, lossPerc, mktFactor)
        + probDensityCond(d, lossPerc, mktFactor) * (lossPerc-mu)/findSaddle(d, lossPerc/remainingNotional_, mktFactor);
*/


/*
        Real uEdisp = (lossPerc-mu)/std::sqrt(vola);

        return CumulativeNormalDistribution(mu, std::sqrt(vola))(lossPerc) * mu + // + 
            std::sqrt(vola) * NormalDistribution()(uEdisp);
*/

        //return CumulativeNormalDistribution()(uEdisp) * mu + 
        //    std::sqrt(vola) * NormalDistribution()(uEdisp);
    }

    Real SaddlePointLossModel::expectedShortfall(const Date&d, 
        Probability percProb) const 
    {
        //double integration here ....??? correct??
        // assuming I have the tranched one.
        Real lossPerc = percentile(d, percProb);

        // check the trivial case when the loss is over the detachment limit 
        //   to avoid computation:
        Real trancheAmount = basket_->trancheNotional() * 
            (detachRatio_-attachRatio_);
        //assumed the amount includes the realized loses
        if(lossPerc >= trancheAmount) return trancheAmount;
        //SHOULD CHECK NOW THE OPPOSITE LIMIT ("zero" losses)....

//////////        GaussLaguerreIntegration integrtr(20);
//////////ESFIntegrator esfInt(*this, d, lossPerc);
//////////        return integrtr(esfInt) / (1.-percProb);

        const std::vector<Probability> uncondProbs = 
            basket_->remainingProbabilities(d);
             
        // Integrate with the tranche or the portfolio according to the limits.

        return copula_->integratedExpectedValue(
            boost::function<Real (const std::vector<Real>& v1)>(
                boost::bind(
   //                 &SaddlePointLossModel::expectedShortfallCond,
                    &SaddlePointLossModel::expectedShortfallTrancheCond,
                    this,
                    boost::cref(uncondProbs),
                    lossPerc,
                    percProb,
                    _1)
                )
            ) / (1.-percProb);

        /*
        std::vector<Real> integrESFPartition = 
            copula_->integratedExpectedValue(
            boost::function<std::vector<Real> (const std::vector<Real>& v1)>(
                boost::bind(
                    &SaddlePointLossModel::expectedShortfallSplitCond,
                    this,
                    boost::cref(d),
                    lossPerc,
                    _1)
                )
            );

        return std::inner_product(integrESFPartition.begin(), integrESFPartition.end(), remainingNotionals_.begin(), 0.);
        */

    }









}
