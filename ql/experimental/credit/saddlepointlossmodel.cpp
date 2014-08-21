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
        const std::vector<Real>& invUncondProbs,
        Real lossFraction,
        const std::vector<Real>&  mktFactor) const 
    {
        const Size nNames = remainingNotionals_.size();
        Real sum = 0.;

        for(Size iName=0; iName < nNames; iName++) {
            Probability pBuffer = 
                copula_->conditionalDefaultProbabilityInvP(
                    invUncondProbs[iName], iName, mktFactor);
            sum += std::log(1. - pBuffer + 
                pBuffer * std::exp(remainingNotionals_[iName] * 
                (1.-copula_->conditionalRecoveryInvP(invUncondProbs[iName],
                    iName, mktFactor)) * lossFraction / remainingNotional_));
        }
       return sum;
    }

    Real SaddlePointLossModel::CumGen1stDerivativeCond(
        const std::vector<Real>& invUncondProbs,
        Real saddle,
        const std::vector<Real>&  mktFactor) const 
    {
        const Size nNames = remainingNotionals_.size();
        Real sum = 0.;

        for(Size iName=0; iName < nNames; iName++) {
            Probability pBuffer = 
                copula_->conditionalDefaultProbabilityInvP(
                    invUncondProbs[iName], iName, mktFactor);
            // loss in fractional units
            Real lossInDef = remainingNotionals_[iName] * 
                (1.-copula_->conditionalRecoveryInvP(invUncondProbs[iName], 
                    iName, mktFactor)) / remainingNotional_;
            Real midFactor = pBuffer * std::exp(lossInDef * saddle);
            sum += lossInDef * midFactor / (1.-pBuffer + midFactor);
        }
       return sum;
    }

    Real SaddlePointLossModel::CumGen2ndDerivativeCond(
        const std::vector<Real>& invUncondProbs,
        Real saddle, 
        const std::vector<Real>&  mktFactor) const 
    {
        const Size nNames = remainingNotionals_.size();
        Real sum = 0.;

        for(Size iName=0; iName < nNames; iName++) {
            Probability pBuffer = 
                copula_->conditionalDefaultProbabilityInvP(
                    invUncondProbs[iName], iName, mktFactor);
            // loss in fractional units
            Real lossInDef = remainingNotionals_[iName] * 
                (1.-copula_->conditionalRecoveryInvP(invUncondProbs[iName], 
                    iName, mktFactor)) / remainingNotional_;
            Real midFactor = pBuffer * std::exp(lossInDef * saddle);
            Real denominator = 1.-pBuffer + midFactor;
            sum += lossInDef * lossInDef * midFactor / denominator - 
                std::pow(lossInDef * midFactor / denominator , 2.);
        }
       return sum;
    }

    Real SaddlePointLossModel::CumGen3rdDerivativeCond(
        const std::vector<Real>& invUncondProbs,
        Real saddle, 
        const std::vector<Real>&  mktFactor) const 
    {
        const Size nNames = remainingNotionals_.size();
        Real sum = 0.;

        for(Size iName=0; iName < nNames; iName++) {
            Probability pBuffer = 
                copula_->conditionalDefaultProbabilityInvP(
                    invUncondProbs[iName], iName, mktFactor);
            Real lossInDef = remainingNotionals_[iName] * 
                (1.-copula_->conditionalRecoveryInvP(invUncondProbs[iName], 
                    iName, mktFactor)) / remainingNotional_;

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
        const std::vector<Real>& invUncondProbs,
        Real saddle, 
        const std::vector<Real>&  mktFactor) const 
    {
        const Size nNames = remainingNotionals_.size();
        Real sum = 0.;

        for(Size iName=0; iName < nNames; iName++) {
            Probability pBuffer = 
                copula_->conditionalDefaultProbabilityInvP(
                    invUncondProbs[iName], iName, mktFactor);
            Real lossInDef = remainingNotionals_[iName] * 
                (1.-copula_->conditionalRecoveryInvP(invUncondProbs[iName], 
                    iName, mktFactor)) / remainingNotional_;

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
        const std::vector<Real>& invUncondProbs,
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
                copula_->conditionalDefaultProbabilityInvP(
                    invUncondProbs[iName], iName, mktFactor);
            Real lossInDef = remainingNotionals_[iName] * 
                (1.-copula_->conditionalRecoveryInvP(invUncondProbs[iName], 
                    iName, mktFactor)) / remainingNotional_;

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
        const std::vector<Real>& invUncondProbs,
        Real saddle, 
        const std::vector<Real>&  mktFactor) const 
    {
        const Size nNames = remainingNotionals_.size();
        Real deriv0 = 0.,
             //deriv1 = 0.,
             deriv2 = 0.;
        for(Size iName=0; iName < nNames; iName++) {
            Probability pBuffer = 
                copula_->conditionalDefaultProbabilityInvP(
                    invUncondProbs[iName], iName, mktFactor);
            Real lossInDef = remainingNotionals_[iName] * 
                (1.-copula_->conditionalRecoveryInvP(invUncondProbs[iName], 
                    iName, mktFactor)) / remainingNotional_;

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

    Real SaddlePointLossModel::findSaddle(
        const std::vector<Real>& invUncondPs,
        Real lossLevel, // in total portfolio loss fractional unit 
        const std::vector<Real>& mktFactor, 
        Real accuracy,
        Natural maxEvaluations
        ) const 
    {
        // \to do:
        // REQUIRE that loss level is below the max loss attainable in 
        //   the portfolio, otherwise theres no solution...
        SaddleObjectiveFunction f(*this, lossLevel, invUncondPs, mktFactor);

        Size nNames = remainingNotionals_.size();
        std::vector<Real> lgds;
        for(Size iName=0; iName<nNames; iName++)
            lgds.push_back(remainingNotionals_[iName] * 
            (1.-copula_->conditionalRecoveryInvP(invUncondPs[iName], iName,
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
        Probability pMaxName = copula_->conditionalDefaultProbabilityInvP(
            invUncondPs[iNamMax], iNamMax, mktFactor);
        // aproximates the  saddle pt corresponding to this minimum; finds 
        //   it by using only the smallest logistic term and thus this is 
        //   smaller than the true value:
        Real saddleMin = 1./(lgds[iNamMax]/remainingNotional_) * 
            std::log(deltaMin*(1.-pMaxName)/
                (pMaxName*lgds[iNamMax]/remainingNotional_-pMaxName*deltaMin));
        // and the associated minimum loss is approximately: (this is thence 
        //   the minimum loss we can resolve/invert)
        Real minLoss = 
            CumGen1stDerivativeCond(invUncondPs, saddleMin, mktFactor);

        // If we are below the loss resolution it returns approximating 
        //  by the minimum/maximum attainable point. Typically the functionals
        //  to integrate will have a low dependency on this point.
        if(lossLevel < minLoss) return saddleMin;
        static const Real deltaMax = 1.e-6; // 1.e-9;
        Real saddleMax = 1./(lgds[iNamMax]/remainingNotional_) * 
            std::log((lgds[iNamMax]/remainingNotional_
                -deltaMin)*(1.-pMaxName)/(pMaxName*deltaMin));
        Real maxLoss = 
            CumGen1stDerivativeCond(invUncondPs, saddleMax, mktFactor);
        if(lossLevel > maxLoss) return saddleMax;

        Brent solverBrent;
        Real guess = (saddleMin+saddleMax)/2.;
        /*
            (lossLevel - 
                CumGen1stDerivativeCond(invUncondPs, lossLevel, mktFactor))
                /CumGen2ndDerivativeCond(invUncondPs, lossLevel, mktFactor);
        if(guess > saddleMax) guess = (saddleMin+saddleMax)/2.;
        */
        solverBrent.setMaxEvaluations(maxEvaluations);
        return solverBrent.solve(f, accuracy, guess, saddleMin, saddleMax);
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

        Real maxVal = 1.-QL_EPSILON; 
        Real guess = 0.5;

        Real solut = solver.solve(f, 1.e-4, guess, minVal, maxVal);
        return basket_->remainingTrancheNotional() * solut;
    }

    Probability SaddlePointLossModel::probOverLossCond(
        const std::vector<Real>& invUncondPs,
        Real trancheLossFract, 
        const std::vector<Real>& mktFactor) const {
        Real portfFract = attachRatio_ + trancheLossFract * 
            (detachRatio_-attachRatio_);// these are remaining ratios
        
        // for non-equity losses add the probability jump at zero tranche 
        //   losses (since this method returns prob of losing more or 
        //   equal to)
        ////////////////---       if(trancheLossFract <= QL_EPSILON) return 1.;
        return 
            probOverLossPortfCond(invUncondPs,
            //below; should substract realized loses. Use remaining amounts??
                portfFract * basket_->basketNotional(),
                mktFactor);
    }

    Disposable<std::map<Real, Probability> > 
        SaddlePointLossModel::lossDistribution(const Date& d) const {
        std::map<Real, Probability> distrib;
        static const Real numPts = 500.;
        for(Real lossFraction=1./numPts; lossFraction<0.45; 
            lossFraction+= 1./numPts)
            distrib.insert(std::make_pair<Real, Probability>(
                lossFraction * remainingNotional_ , 
                  1.-probOverPortfLoss(d, lossFraction* remainingNotional_ )));
        return distrib;
    }

    /*  NOTICE THIS IS ON THE TOTAL PORTFOLIO ---- UNTRANCHED..............
        Probability of having losses in the portfolio due to default 
        events equal or larger than a given absolute loss value on a 
        given date conditional to the latent model factor.
        The integral expression on the expansion is the first order 
        integration as presented in several references, see for instance; 
        equation 8 in R.Martin, K.Thompson, and C. Browne 's 
        'Taking to the Saddle', Risk Magazine, June 2001, page 91

        @param loss loss in absolute value
    */
    Probability SaddlePointLossModel::probOverLossPortfCond(
        const std::vector<Real>& invUncondProbs,
        Real loss, 
        const std::vector<Real>& mktFactor) const 
    {
        /* This is taking in the unconditional probabilites non inverted. See if
        the callers can be written taking the inversion already; if they are 
        performing it thats a perf hit. At least this can be seen to be true
        for the recovery call (but rand rr are not intended to be used yet)
        */
       // return probOverLossPortfCond1stOrder(d, loss, mktFactor);
        if (loss <= QL_EPSILON) return 1.;

        Real relativeLoss = loss / remainingNotional_;
        if (relativeLoss >= 1.-QL_EPSILON) return 0.;

        const Size nNames = remainingNotionals_.size();

        Real averageRecovery_ = 0.;
        for(Size iName=0; iName < nNames; iName++)
            averageRecovery_ += copula_->conditionalRecoveryInvP(
                invUncondProbs[iName], iName, mktFactor);
        averageRecovery_ = averageRecovery_ / nNames;

        Real maxAttLossFract = 1.-averageRecovery_;
        if(relativeLoss > maxAttLossFract) return 0.;

        Real saddlePt = findSaddle(invUncondProbs,
            relativeLoss, mktFactor);

        boost::tuples::tuple<Real, Real, Real, Real> cumulants = 
            CumGen0234DerivCond(invUncondProbs, 
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

    // cheaper; less terms retained; yet the cost lies in the saddle point calc
    Probability SaddlePointLossModel::probOverLossPortfCond1stOrder(
        const std::vector<Real>& invUncondPs,
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
            copula_->conditionalRecoveryInvP(invUncondPs[iName], iName, 
            mktFactor);  
        averageRecovery_ = averageRecovery_ / nNames;

        Real maxAttLossFract = 1.-averageRecovery_;
        if(relativeLoss > maxAttLossFract) return 0.;

        Real saddlePt = findSaddle(invUncondPs,
            relativeLoss, mktFactor);

        boost::tuples::tuple<Real, Real> cumulants = 
            CumGen02DerivCond(invUncondPs,
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
                /*  std::exp(baseVal - relativeLoss * saddlePt 
                    + .5 * saddleTo2 * secondVal)*/
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
               /* std::exp(baseVal - relativeLoss * saddlePt 
               + .5 * saddleTo2 * secondVal)*/
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
        const std::vector<Real>& invUncondPs,
        Real loss,
        const std::vector<Real>& mktFactor) const 
    {
        if (loss <= QL_EPSILON) return 0.;

        Real relativeLoss = loss / remainingNotional_;
        Real saddlePt = findSaddle(invUncondPs,
            relativeLoss, mktFactor);

        boost::tuples::tuple<Real, Real, Real, Real> cumulants = 
            CumGen0234DerivCond(invUncondPs,
            saddlePt, mktFactor);
        /// access them directly rather than through this copy
        Real K0Saddle = cumulants.get<0>();
        Real K2Saddle = cumulants.get<1>();
        Real K3Saddle = cumulants.get<2>();
        Real K4Saddle = cumulants.get<3>();
        /* see, for instance R.Martin "he saddle point method and portfolio 
        optionalities." in Risk December 2006 p.93 */
        Real cum2DerCond = CumGen2ndDerivativeCond(invUncondPs,
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
             / (std::sqrt(2. * M_PI * K2Saddle));
    }

    /*    NOTICE THIS IS ON THE TOTAL PORTFOLIO ---- UNTRANCHED..
        Sensitivities of the individual names to a given portfolio loss value 
        due to defaults. It returns ratios to the total structure notional, 
        which aggregated add up to the requested loss value.

    see equation 8 in 'VAR: who contributes and how much?' by R.Martin, 
    K.Thompson, and C. Browne in Risk Magazine, August 2001
    @param loss Loss amount level at which we want to request the sensitivity. 
    Equivalent to a percentile.
    */
    Disposable<std::vector<Real> > SaddlePointLossModel::splitLossCond(
        const std::vector<Real>& invUncondProbs,
        Real loss, 
        std::vector<Real> mktFactor) const 
    {
        const Size nNames = remainingNotionals_.size();
        if (loss <= QL_EPSILON) return std::vector<Real>(nNames, 0.);

        Real saddlePt = findSaddle(invUncondProbs, loss / remainingNotional_, 
            mktFactor);

        std::vector<Real> condContrib(nNames, 0.);

        for(Size iName=0; iName<nNames; iName++) {
            Probability pBuffer = 
                copula_->conditionalDefaultProbabilityInvP(
                    invUncondProbs[iName], iName, mktFactor);
            Real lossInDef = remainingNotionals_[iName] * 
                (1.-copula_->conditionalRecoveryInvP(invUncondProbs[iName], 
                    iName, mktFactor));
            Real midFactor = pBuffer * 
                std::exp(lossInDef * saddlePt/ remainingNotional_);
            Real denominator = 1.-pBuffer + midFactor;

            condContrib[iName] = lossInDef * midFactor / denominator; 
        }
        return condContrib;
    }

    Real SaddlePointLossModel::conditionalExpectedLoss(
        const std::vector<Real>& invUncondProbs,
        const std::vector<Real>& mktFactor) const 
    {
        const Size nNames = remainingNotionals_.size();
        Real eloss = 0.;
        /// USE STL.....-------------------
        for(Size iName=0; iName < nNames; iName++) {
            Probability pBuffer = copula_->conditionalDefaultProbabilityInvP(
                invUncondProbs[iName], iName, mktFactor);
            eloss += pBuffer * remainingNotionals_[iName] *
                (1.-copula_->conditionalRecoveryInvP(invUncondProbs[iName], 
                    iName, mktFactor));
        }
        return eloss;
    }

    Real SaddlePointLossModel::conditionalExpectedTrancheLoss(
        const std::vector<Real>& invUncondProbs,
        const std::vector<Real>& mktFactor) const 
    {
        const Size nNames = remainingNotionals_.size();
        Real eloss = 0.;
        /// USE STL.....-------------------
        for(Size iName=0; iName < nNames; iName++) {
            Probability pBuffer = copula_->conditionalDefaultProbabilityInvP(
                invUncondProbs[iName], iName, mktFactor);
            eloss += 
                pBuffer * remainingNotionals_[iName] * 
                (1.-copula_->conditionalRecoveryInvP(invUncondProbs[iName], 
                iName, mktFactor));
        }
        return std::min(
            std::max(eloss - attachRatio_ * remainingNotional_, 0.), 
                (detachRatio_ - attachRatio_) * remainingNotional_);
    }

    Disposable<std::vector<Real> > 
        SaddlePointLossModel::expectedShortfallSplitCond(
            const std::vector<Real>& invUncondProbs,
            Real lossPerc, const std::vector<Real>& mktFactor) const 
    {
        const Size nNames = remainingNotionals_.size();
        std::vector<Real> lgds;
        for(Size iName=0; iName<nNames; iName++)
            lgds.push_back(remainingNotionals_[iName] * 
                (1.-copula_->conditionalRecoveryInvP(invUncondProbs[iName],
                    iName, mktFactor))); 
        std::vector<Real> vola(nNames, 0.), mu(nNames, 0.);
        Real volaTot = 0., muTot = 0.;
        for(Size iName=0; iName < nNames; iName++) {
            Probability pBuffer = copula_->conditionalDefaultProbabilityInvP(
                invUncondProbs[iName], iName, mktFactor);
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
        const std::vector<Real>& invUncondProbs,
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
        Real probLOver = probOverLossPortfCond(invUncondProbs,
            basket_->detachmentAmount(), mktFactor);
        if(basket_->attachmentAmount() > QL_EPSILON) {
            if(lossPerc < basket_->attachmentAmount()) {
                correctionTerm = ( (basket_->detachmentAmount() 
                    - 2.*basket_->attachmentAmount())*
                        probOverLossPortfCond(invUncondProbs, lossPerc, 
                            mktFactor)
                    + basket_->attachmentAmount() * probLOver )/(1.-percentile);
            }else{
                correctionTerm = ( (percentile-1)*basket_->attachmentAmount()
                    + basket_->detachmentAmount() * probLOver
                    )/(1.-percentile);
            }
        }

        return expectedShortfallFullPortfolioCond(invUncondProbs, 
            std::max(lossPerc, basket_->attachmentAmount()), mktFactor)
            + expectedShortfallFullPortfolioCond(invUncondProbs, 
                basket_->detachmentAmount(), mktFactor)
            - correctionTerm;
    }

    Real SaddlePointLossModel::expectedShortfallFullPortfolioCond(
        const std::vector<Real>& invUncondProbs,
        Real lossPerc, // value 
        const std::vector<Real>& mktFactor) const 
    {
        /* This version is based on: Martin 2006 paper and on the expression 
        in 'SaddlePoint approximation of expected shortfall for transformed 
        means' S.A. Broda and M.S.Paolella , Amsterdam School of Economics 
        discussion paper, available online.
        */
        Real lossPercRatio = lossPerc  /remainingNotional_;
        Real elCond = 0.;
        const Size nNames = remainingNotionals_.size();

        /// use stl algorthms
        for(Size iName=0; iName < nNames; iName++) {
            Probability pBuffer = copula_->conditionalDefaultProbabilityInvP(
                invUncondProbs[iName], iName, mktFactor);
            elCond += pBuffer * remainingNotionals_[iName] * 
                (1.-copula_->conditionalRecoveryInvP(invUncondProbs[iName],
                    iName, mktFactor));
        }
        Real saddlePt = findSaddle(invUncondProbs, lossPercRatio, mktFactor);

        // Martin 2006:
        return 
            elCond * probOverLossPortfCond(invUncondProbs, lossPerc, mktFactor)
              + (lossPerc - elCond) * probDensityCond(invUncondProbs, lossPerc,
                    mktFactor) /saddlePt;

        // calling the EL tranche
        // return elCond - expectedEquityLossCond(d, lossPercRatio, mktFactor);

        /*
        // Broda and Paolella:
        Real elCondRatio = elCond / remainingNotional_;

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

        Real term1 = CumulativeNormalDistribution()(wq)// * std::sqrt(numNames)
            * elCond ;
        Real term2 = .5 * M_2_SQRTPI * M_SQRT1_2 * (1./std::sqrt(numNames))
            * exp(-wq*wq * numNames/2.)*(elCond/wq - 
                lossPerc/(saddlePt * std::sqrt(K2Saddle)));
        return term1 + term2;
        */
    }

    Real SaddlePointLossModel::expectedShortfall(const Date&d, 
        Probability percProb) const 
    {
        // assuming I have the tranched one.
        Real lossPerc = percentile(d, percProb);

        // check the trivial case when the loss is over the detachment limit 
        //   to avoid computation:
        Real trancheAmount = basket_->trancheNotional() * 
            (detachRatio_-attachRatio_);
        //assumed the amount includes the realized loses
        if(lossPerc >= trancheAmount) return trancheAmount;
        //SHOULD CHECK NOW THE OPPOSITE LIMIT ("zero" losses)....
        std::vector<Real> invUncondProbs = 
            basket_->remainingProbabilities(d);
        for(Size i=0; i<invUncondProbs.size(); i++)
            invUncondProbs[i] = 
                copula_->inverseCumulativeY(invUncondProbs[i], i);

        // Integrate with the tranche or the portfolio according to the limits.
        return copula_->integratedExpectedValue(
            boost::function<Real (const std::vector<Real>& v1)>(
                boost::bind(
                    &SaddlePointLossModel::expectedShortfallFullPortfolioCond,
                    this,
                    boost::cref(invUncondProbs),
                    lossPerc,
                    _1)
                )
            ) / (1.-percProb);

    /* test:?
        return std::inner_product(integrESFPartition.begin(), 
        integrESFPartition.end(), remainingNotionals_.begin(), 0.);
    */        

    }


}
