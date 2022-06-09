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

#ifndef quantlib_binomial_loss_model_hpp
#define quantlib_binomial_loss_model_hpp

#include <ql/experimental/credit/basket.hpp>
#include <ql/experimental/credit/constantlosslatentmodel.hpp>
#include <ql/experimental/credit/defaultlossmodel.hpp>
#include <ql/handle.hpp>
#include <algorithm>
#include <numeric>
#include <utility>

namespace QuantLib {

    /*! Binomial Defaultable Basket Loss Model\par
    Models the portfolio loss distribution by approximatting it to an adjusted 
    binomial. Fits the two moments of the loss distribution through an adapted 
    binomial approximation. This simple model allows for portfolio inhomogeneity
    with no excesive cost over the LHP.\par
    See:\par
    <b>Approximating Independent Loss Distributions with an Adjusted Binomial 
    Distribution</b> , Dominic O'Kane, 2007 EDHEC RISK AND ASSET MANAGEMENT 
    RESEARCH CENTRE \par
    <b>Modelling single name and multi-name credit derivatives</b> Chapter 
    18.5.2, Dominic O'Kane, Wiley Finance, 2008 \par
    The version presented here is adaptated to the multifactorial case
    by computing a conditional binomial approximation; notice that the Binomial
    is stable. This way the model can be used also in risk management models
    rather than only in pricing. The copula is also left 
    undefined/arbitrary. \par
    LLM: Loss Latent Model template parameter able to model default and 
    loss.\par
    The model is allowed and arbitrary copula, although initially designed for
    a Gaussian setup. If these exotic versions were not allowed the template 
    parameter can then be dropped but the use of random recoveries should be
    added in some other way.

    \todo untested/wip for the random recovery models.
    \todo integrate with the previously computed probability inversions of
    the cumulative functions.
    */
    template<class LLM>
    class BinomialLossModel : public DefaultLossModel {
    public:
        typedef typename LLM::copulaType copulaType;
        explicit BinomialLossModel(ext::shared_ptr<LLM> copula) : copula_(std::move(copula)) {}

      private:
        void resetModel() override {
            /* say there are defaults and these havent settled... and this is
            the engine to compute them.... is this the wrong place?:*/
            attachAmount_ = basket_->remainingAttachmentAmount();
            detachAmount_ = basket_->remainingDetachmentAmount();

            copula_->resetBasket(basket_.currentLink()); // forces interface
      }

    protected:
        /*! Returns the probability of the default loss values given by the 
            method lossPoints.
        */
        std::vector<Real> expectedDistribution(const Date& date) const {
            // precal date conditional magnitudes:
            std::vector<Real> notionals = basket_->remainingNotionals(date);
            std::vector<Probability> invProbs = 
                basket_->remainingProbabilities(date);
            for(Size iName=0; iName<invProbs.size(); iName++)
                invProbs[iName] = 
                    copula_->inverseCumulativeY(invProbs[iName], iName);

            return copula_->integratedExpectedValueV(
                [&](const std::vector<Real>& v1) {
                    return lossProbability(date, notionals, invProbs, v1);
                });
        }
        //! attainable loss points this model provides
        std::vector<Real> lossPoints(const Date&) const;
        //! Returns the cumulative full loss distribution
        std::map<Real, Probability> lossDistribution(const Date& d) const override;
        //! Loss level for this percentile
        Real percentile(const Date& d, Real percentile) const override;
        Real expectedShortfall(const Date& d, Real percentile) const override;
        Real expectedTrancheLoss(const Date& d) const override;

        // Model internal workings ----------------
        //! Average loss per credit.
        Real averageLoss(const Date&, const std::vector<Real>& reminingNots, 
            const std::vector<Real>&) const;
        Real condTrancheLoss(const Date&, const std::vector<Real>& lossVals, 
            const std::vector<Real>& bsktNots,
            const std::vector<Probability>& uncondDefProbs, 
            const std::vector<Real>&) const;
        // expected as in time-value, not average, see literature
        std::vector<Real> expConditionalLgd(const Date& d,
                                            const std::vector<Real>& mktFactors) const
        {
            std::vector<Real> condLgds;
            const std::vector<Size>& evalDateLives = basket_->liveList();
            condLgds.reserve(evalDateLives.size());
            for (unsigned long evalDateLive : evalDateLives)
                condLgds.push_back(1. - copula_->conditionalRecovery(d, evalDateLive, mktFactors));
            return condLgds;
        }

        //! Loss probability density conditional on the market factor value.
        // Heres where the burden of the algorithm setup lies.
        std::vector<Real> lossProbability(      
                const Date& date,
                // expected exposures at the passed date, no wrong way means
                //  no dependence of the exposure with the mkt factor 
                const std::vector<Real>& bsktNots,
                const std::vector<Real>& uncondDefProbInv, 
                            const std::vector<Real>&  mktFactor) const;

        const ext::shared_ptr<LLM> copula_;

        // cached arguments:
        // remaining basket magnitudes:
        mutable Real attachAmount_, detachAmount_;
    };

    //-------------------------------------------------------------------------

    /* The algorithm to compute the prob. of n defaults in the basket is 
        recursive. For this reason theres no sense in returning the prob 
        distribution of a given number of defaults.
    */
    template< class LLM>
    std::vector<Real> BinomialLossModel<LLM>::lossProbability(
        const Date& date, 
        const std::vector<Real>& bsktNots,
        const std::vector<Real>& uncondDefProbInv, 
        const std::vector<Real>& mktFactors) const 
    {   // the model as it is does not model the exposures conditional to the 
        //   mkt factr, otherwise this needs revision
        /// model does not take the unconditional rr
        Size bsktSize = basket_->remainingSize();
        /* The conditional loss per unit notional of each name at time 'date'
            The spot recovery model is returning for all i's:
            \frac{\int_0^t  [1-rr_i(\tau; \xi)] P_{def-i}(0, \tau; \xi) d\tau}
                 {P_{def-i}(0,t;\xi)}
            and the constant recovery model is simply returning: 
            1-RR_i
        */
        // conditional fractional LGD expected as given by the recovery model 
        //   for the ramaining(live) names at the current eval date.
        std::vector<Real> fractionalEL = expConditionalLgd(date, mktFactors);
        std::vector<Real> lgdsLeft;
        std::transform(fractionalEL.begin(), fractionalEL.end(), 
            bsktNots.begin(), std::back_inserter(lgdsLeft), 
            std::multiplies<Real>());
        Real avgLgd = 
            std::accumulate(lgdsLeft.begin(), lgdsLeft.end(), Real(0.)) /
                bsktSize;

        std::vector<Probability> condDefProb(bsktSize, 0.);
        for(Size j=0; j<bsktSize; j++)//transform
            condDefProb[j] = 
                copula_->conditionalDefaultProbabilityInvP(uncondDefProbInv[j],
                    j, mktFactors);
        // of full portfolio:
        Real avgProb = avgLgd <= QL_EPSILON ? 0. : // only if all are 0
                std::inner_product(condDefProb.begin(), 
                    condDefProb.end(), lgdsLeft.begin(), 0.)
                / (avgLgd * bsktSize);
        // model parameters:
        Real m = avgProb * bsktSize;
        Real floorAveProb = std::min(Real(bsktSize-1), std::floor(Real(m)));
        Real ceilAveProb = floorAveProb + 1.;
        // nu_A
        Real varianceBinom = avgProb * (1. - avgProb)/bsktSize;
        // nu_E
        std::vector<Probability> oneMinusDefProb;//: 1.-condDefProb[j]
        std::transform(condDefProb.begin(), condDefProb.end(), 
                       std::back_inserter(oneMinusDefProb), 
                       [](Real x){ return 1.0-x; });

        //breaks condDefProb and lgdsLeft to spare memory
        std::transform(condDefProb.begin(), condDefProb.end(), 
            oneMinusDefProb.begin(), condDefProb.begin(), 
            std::multiplies<Real>());
        std::transform(lgdsLeft.begin(), lgdsLeft.end(), 
            lgdsLeft.begin(), lgdsLeft.begin(), std::multiplies<Real>());
        Real variance = std::inner_product(condDefProb.begin(), 
            condDefProb.end(), lgdsLeft.begin(), 0.);

        variance = avgLgd <= QL_EPSILON ? 0. : 
            variance / (bsktSize * bsktSize * avgLgd * avgLgd );
        Real sumAves = -std::pow(ceilAveProb-m, 2) 
            - (std::pow(floorAveProb-m, 2) - std::pow(ceilAveProb,2.)) 
                * (ceilAveProb-m);
        Real alpha = (variance * bsktSize + sumAves) 
            / (varianceBinom * bsktSize + sumAves);
        // Full distribution: 
        // ....DO SOMETHING CHEAPER at least go up to the loss tranche limit.
        std::vector<Probability> lossProbDensity(bsktSize+1, 0.); 
        if(avgProb >= 1.-QL_EPSILON) {
           lossProbDensity[bsktSize] = 1.;
        }else if(avgProb <= QL_EPSILON) {
           lossProbDensity[0] = 1.;
        }else{
            /* FIX ME: With high default probabilities one only gets tiny values
            at the end and the sum of probabilities in the 
            conditional distribution does not add up to one. It might be due to 
            the fact that recursion should be done in the other direction as 
            pointed out in the book. This is numerical.
            */
            Probability probsRatio = avgProb/(1.-avgProb);
            lossProbDensity[0] = std::pow(1.-avgProb, 
                static_cast<Real>(bsktSize));
            for(Size i=1; i<bsktSize+1; i++) // recursive to avoid factorial
                lossProbDensity[i] = lossProbDensity[i-1] * probsRatio 
                    * (bsktSize-i+1.)/i;
            // redistribute probability:
            for(Size i=0; i<bsktSize+1; i++)
                lossProbDensity[i] *= alpha;
            // adjust average
            Real epsilon = (1.-alpha)*(ceilAveProb-m);
            Real epsilonPlus = 1.-alpha-epsilon;
            lossProbDensity[ql_cast<Size>(floorAveProb)] += epsilon;
            lossProbDensity[ql_cast<Size>(ceilAveProb)] += epsilonPlus;
        }
        return lossProbDensity;
    }

    //-------------------------------------------------------------------------

    template< class LLM>
    Real BinomialLossModel<LLM>::averageLoss(
        const Date& d, 
        const std::vector<Real>& reminingNots,
        const std::vector<Real>& mktFctrs) const 
    {
        Size bsktSize = basket_->remainingSize();
        /* The conditional loss per unit notional of each name at time 'date'
            The spot recovery model is returning for all i's:
            \frac{\int_0^t  [1-rr_i(\tau; \xi)] P_{def-i}(0, \tau; \xi) d\tau}
                 {P_{def-i}(0,t;\xi)}
            and the constant recovery model is simply returning: 
            1-RR_i
        */
        std::vector<Real> fractionalEL = expConditionalLgd(d, mktFctrs);
        Real notBskt = std::accumulate(reminingNots.begin(), 
                                       reminingNots.end(), Real(0.));
        std::vector<Real> lgdsLeft;
        std::transform(fractionalEL.begin(), fractionalEL.end(), 
                       reminingNots.begin(), std::back_inserter(lgdsLeft),
                       std::multiplies<Real>());
        return std::accumulate(lgdsLeft.begin(), lgdsLeft.end(), Real(0.)) 
            / (bsktSize*notBskt);
    }

    template< class LLM>
    std::vector<Real> BinomialLossModel<LLM>::lossPoints(const Date& d) const 
    {
        std::vector<Real> notionals = basket_->remainingNotionals(d);

        Real aveLossFrct = copula_->integratedExpectedValue(
            [&](const std::vector<Real>& v1) {
                return averageLoss(d, notionals, v1);
            });

        std::vector<Real> data;
        Size dataSize = basket_->remainingSize() + 1;
        data.reserve(dataSize);
        // use std::algorithm
        Real outsNot = basket_->remainingNotional(d);
        for(Size i=0; i<dataSize; i++)
            data.push_back(i * aveLossFrct * outsNot);
        return data;
    }

    template< class LLM>
    Real BinomialLossModel<LLM>::condTrancheLoss(
        const Date& d, 
        const std::vector<Real>& lossVals, 
        const std::vector<Real>& bsktNots,
        const std::vector<Real>& uncondDefProbsInv,
        const std::vector<Real>& mkf) const {

        std::vector<Real> condLProb = 
            lossProbability(d, bsktNots, uncondDefProbsInv, mkf);
        // \to do: move to a do-while over attach to detach
        Real suma = 0.;
        for(Size i=0; i<lossVals.size(); i++) { 
            suma += condLProb[i] * 
                std::min(std::max(lossVals[i]
                 - attachAmount_, 0.), detachAmount_ - attachAmount_);
        }
        return suma;
    }

    template< class LLM>
    Real BinomialLossModel<LLM>::expectedTrancheLoss(const Date& d) const {
        std::vector<Real> lossVals  = lossPoints(d);
        std::vector<Real> notionals = basket_->remainingNotionals(d);
        std::vector<Probability> invProbs = 
            basket_->remainingProbabilities(d);
        for(Size iName=0; iName<invProbs.size(); iName++)
            invProbs[iName] = 
                copula_->inverseCumulativeY(invProbs[iName], iName);
            
        return copula_->integratedExpectedValue(
            [&](const std::vector<Real>& v1) {
                return condTrancheLoss(d, lossVals, notionals, invProbs, v1);
            });
    }


    template< class LLM>
    std::map<Real, Probability> BinomialLossModel<LLM>::lossDistribution(const Date& d) const 
    {
        std::map<Real, Probability> distrib;
        std::vector<Real> lossPts = lossPoints(d);
        std::vector<Real> values  = expectedDistribution(d);
        Real sum = 0.;
        for(Size i=0; i<lossPts.size(); i++) {
            distrib.insert(std::make_pair(lossPts[i], 
                //capped, some situations giving a very small probability over 1
                std::min(sum+values[i],1.)
                ));
            sum+= values[i];
        }
        return distrib;
    }

    template< class LLM>
    Real BinomialLossModel<LLM>::percentile(const Date& d, Real perc) const {
        std::map<Real, Probability> dist = lossDistribution(d);
        // \todo: Use some of the library interpolators instead
        if(// included in test below-> (dist.begin()->second >=1.) ||
            (dist.begin()->second >= perc))return dist.begin()->first;

        // deterministic case (e.g. date requested is todays date)
        if(dist.size() == 1) return dist.begin()->first;

        if(perc == 1.) return dist.rbegin()->first;
        if(perc == 0.) return dist.begin()->first;
        std::map<Real, Probability>::const_iterator itdist = dist.begin();
        while (itdist->second <= perc) ++itdist;
        Real valPlus = itdist->second;
        Real xPlus   = itdist->first;
        --itdist; //we're never 1st or last, because of tests above
        Real valMin  = itdist->second;
        Real xMin    = itdist->first;

        Real portfLoss = xPlus-(xPlus-xMin)*(valPlus-perc)/(valPlus-valMin);

        return 
            std::min(std::max(portfLoss - attachAmount_, 0.), 
                detachAmount_ - attachAmount_);
    }

    template< class LLM>
    Real BinomialLossModel<LLM>::expectedShortfall(const Date&d, 
        Real perctl) const 
    {
        //taken from recursive since we have the distribution in both cases.
        if(d == Settings::instance().evaluationDate()) return 0.;
            std::map<Real, Probability> distrib = lossDistribution(d);

            std::map<Real, Probability>::iterator 
                itNxt, itDist = distrib.begin();
            for(; itDist != distrib.end(); ++itDist)
                if(itDist->second >= perctl) break;
            itNxt = itDist;
            --itDist;

            // \todo: I could linearly triangulate the exact point and get 
            //    extra precission on the first(broken) period.
            if(itNxt != distrib.end()) { 
                Real lossNxt = std::min(std::max(itNxt->first - attachAmount_, 
                    0.), detachAmount_ - attachAmount_);
                Real lossHere = std::min(std::max(itDist->first - attachAmount_,
                    0.), detachAmount_ - attachAmount_);

                Real val =  lossNxt - (itNxt->second - perctl) * 
                    (lossNxt - lossHere) / (itNxt->second - itDist->second); 
                Real suma = (itNxt->second - perctl) * (lossNxt + val) * .5;
                ++itDist; ++itNxt;
                do{
                    lossNxt = std::min(std::max(itNxt->first - attachAmount_, 
                        0.), detachAmount_ - attachAmount_);
                    lossHere = std::min(std::max(itDist->first - attachAmount_, 
                        0.), detachAmount_ - attachAmount_);
                    suma += .5 * (lossHere + lossNxt) 
                        * (itNxt->second - itDist->second);
                    ++itDist; ++itNxt;
                }while(itNxt != distrib.end());
                return suma / (1.-perctl);
            }
            QL_FAIL("Binomial model fails to calculate ESF.");
    }

    // The standard use:
    typedef BinomialLossModel<GaussianConstantLossLM> GaussianBinomialLossModel;
    typedef BinomialLossModel<TConstantLossLM> TBinomialLossModel;

}

#endif
