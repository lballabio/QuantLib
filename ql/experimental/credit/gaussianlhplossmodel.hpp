/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2009, 2014 Jose Aparicio

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

#ifndef quantlib_gaussian_lhp_lossmodel_hpp
#define quantlib_gaussian_lhp_lossmodel_hpp

#include <numeric>

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <ql/math/distributions/bivariatenormaldistribution.hpp>
#include <ql/experimental/credit/recoveryratequote.hpp>
#include <ql/quotes/simplequote.hpp>

#include <ql/experimental/credit/defaultlossmodel.hpp>
#include <ql/experimental/credit/basket.hpp>
#include <ql/experimental/math/latentmodel.hpp>

/* Intended to replace GaussianLHPCDOEngine in 
    ql/experimental/credit/syntheticcdoengines.hpp
*/

namespace QuantLib {

    // TO DO: Make avergae prob and average recvoery to be weighted average (by exposure) so they can be used as control variate.
// TO DO: Implement the Student-t version for tail dependnece
// TO DO: Return the distribution and the other metrics.
// TO DO: FIX: SETTING UP A DETACH HIGHER THAN THE MAX ATTAINABLE LOSS SHOULD NOT B A PROBLM
// TO DO: Implement ESF
// TO DO: Implement full distribution....

    /*!
      CDO engine with analytical expected tranche loss for a large homogeneous
      pool with Gaussian one-factor copula. See for example
      "The Normal Inverse Gaussian Distribution for Synthetic CDO pricing.",
      Anna Kalemanova, Bernd Schmid, Ralf Werner,
      Journal of Derivatives, Vol. 14, No. 3, (Spring 2007), pp. 80-93.
      http://www.defaultrisk.com/pp_crdrv_91.htm

      It can be used to price a credit derivative or to provide risk metrics of 
      a portfolio.
     */
    class GaussianLHPLossModel : public DefaultLossModel, 
        public LatentModel<GaussianCopulaPolicy> {
    public:
        GaussianLHPLossModel(
            const Handle<Quote>& correlQuote,
            const std::vector<Handle<RecoveryRateQuote> >& quotes);

        GaussianLHPLossModel(
            Real correlation,
            const std::vector<Real>& recoveries);

        GaussianLHPLossModel(
            const Handle<Quote>& correlQuote,
            const std::vector<Real>& recoveries);

        void update() {
            sqrt1minuscorrel_ = sqrt(1.-correl_->value());
            biphi_ = BivariateCumulativeNormalDistribution(
                -std::sqrt(1.-correl_->value()));
            beta_ = sqrt(correl_->value());
        }

        // basket needs only to initialize once. The basket is lazy triggered on updates ------Still we 
        //   need to init (and lock) the loss model
        //   on every call to a model method since it might be used by other baskets between calls.
        void initialize(const Basket& basket) {
            basket_ = &basket;

            /* Notice that the attach and detach ratios are floating here. The amount limits are fixed (except for past defaults) and the notinal portfolio-basket is floating, giving a floating tranche. In the situation where the underlying amortizes this implies a lowering of the risk with time since this pushes the ratio limits up (even above 100%) or increasing the seniority of the tranche with time.

            */
            remainingAttachAmount_ = basket.remainingAttachmentAmount();
            remainingDetachAmount_ = basket.remainingDetachmentAmount();

            /// CHECK THE EXPOSURES ON THIS BASKET ARE DETERMINISTIC (FIXED OR PROGRAMMED AMORTIZNG) OTHERWISE THIS MODEL IS NOT FIT FOR IT. (needs the exposure in the copula)
        }

    private:
            // @param attachLimit as a fraction of the underlying (live????) portfolio notional... the algo needs it to be live but do I convert before the actual calcs or do I get it converted??????
            /* trying to solve the problem when the notional is time dependent....

        */
        Real expectedTrancheLossImpl(
            Real remainingNot, // << at the given date 'd'
            Real prob, // << at the given date 'd'
            Real averageRR, // << at the given date 'd'
            Real attachLimit, Real detachLimit) const;
    public:
        Real expectedTrancheLoss(const Date& d) const {
            const Real remainingBasktNot = basket_->remainingNotional(d);// CALLED HERE AND CALLED AGAIN IN expectedTrancheLossImpl
                        Real averageRR = averageRecovery(d);
            Probability prob = averageProb(d);

            const Real attach = std::min(remainingAttachAmount_ / remainingBasktNot, 1.);
            const Real detach = std::min(remainingDetachAmount_ / remainingBasktNot, 1.);

            return expectedTrancheLossImpl(remainingBasktNot, prob, averageRR, attach, detach);
        }

/// WRITE DENSITY.....
        /// homogeneize, prob loss over.... and density names as in Saddle....... then VaR search function.......
       //////.... Real densityTrancheLoss(const Date& d, Real trancheLossFraction) const {
            //@param remaining fraction in live tranche units, not portfolio as a fraction of the remaining(live) tranche (i.e. a_remaining=0% and det_remaining=100%) 
        Real probOverLoss(const Date& d, Real remainingLossFraction) const;

        //! Returns the ESF as an absolute amount (rather than a fraction)
        /* The way it is implemented here is a transformation from ETL to ESF
        is a generic algorithm, not specific to this model so it should be moved
        to the Basket/DefaultLossModel class. 
        TO DO: Implement the inverse transformation
        */
        Real expectedShortfall(const Date& d, Probability perctl) const;
    protected:
        // This is wrong, it is not accounting for the current defaults ....
        // returns the loss value in actual loss units, returns the loss value for the underlying portfolio, untranched
        Real percentilePortfolioLossFraction(const Date& d, Real perctl) const;
    public:
            // same as percentilePortfolio but tranched
        Real percentile(const Date& d, Real perctl) const {
            const Real remainingNot = basket_->remainingNotional(d);
            const Real attach = 
                std::min(remainingAttachAmount_ / remainingNot, 1.);
            const Real detach = 
                std::min(remainingDetachAmount_ / remainingNot, 1.);
            return remainingNot * 
                std::min(std::max(percentilePortfolioLossFraction(d, perctl) 
                    - attach, 0.), detach - attach);
        }

        Real recoveryValueImpl(const Date& d, Size iName, 
            const std::vector<DefaultProbKey>& defKeys = 
                std::vector<DefaultProbKey>()) const 
        {
            return averageRecovery(d);
        }

        // should be initalized beforehand, not cacheable because is basket dependent, and basket could be updated at any time
        Probability averageProb(const Date& d) const {
            // weighted average by programmed exposure.
            const std::vector<Probability> probs = 
                basket_->remainingProbabilities(d);//use remaining basket
            const std::vector<Real> remainingNots = 
                basket_->remainingNotionals(d);
            return std::inner_product(probs.begin(), probs.end(), 
                remainingNots.begin(), 0.) / basket_->remainingNotional(d);
        }
/*
It is tempting to define the average recovery without the probability factor, weighting only by notional instead, but that way the expected loss of the average/aggregated and the original portfolio would not coincide. This instroduces however a time dependence in the recovery value.

weighting by notional implies time dependent weighting since the basket might amortize.
*/
        Real averageRecovery(const Date& d) const {// no time dependence in this model
            const std::vector<Probability> probs = basket_->remainingProbabilities(d);
            std::vector<Real> recoveries;
            for(Size i=0; i<basket_->remainingSize(); i++)
                recoveries.push_back(rrQuotes_[i]->value());
            std::vector<Real> notionals = basket_->remainingNotionals(d);
            Real denominator = std::inner_product(notionals.begin(), 
                notionals.end(), probs.begin(), 0.);
            if(denominator == 0.) return 0.;

            std::transform(notionals.begin(), notionals.end(), probs.begin(),
                notionals.begin(), std::multiplies<Real>());

            return std::inner_product(recoveries.begin(), recoveries.end(), 
                notionals.begin(), 0.) / denominator;
        }

    private:
        // cached
        mutable Real sqrt1minuscorrel_;
    ////////////////////////////////////////////////////////////////    mutable Real averageRecovery_;
        Handle<Quote> correl_;
        std::vector<Handle<RecoveryRateQuote> > rrQuotes_;
        // calculation buffers

        /*! The problem with defining a fixed average recovery on a portfolio with uneven exposures 
        is that it does not preserve portfolio
        moments like the expected loss. To achieve it one should define the averarage recovery with a 
        time dependence: $\hat{R}(t) = \frac{\sum_i R_i N_i P_i(t)}{\sum_i N_i P_i(t)}$
        But the date dependence increases significantly the calculations cost.
        Notice that this problem dissapears if the recoveries are the same along the portfolio.
        */
//////////////////        mutable Real averageRecovery_;

        //mutable Basket_N remainingBasket_;
 ////////////////////////////       mutable Real attach_, detach_;
        //remaining underlying portfolio notional
///////////////////////////        mutable Real remainingNotional_;
        mutable Real remainingAttachAmount_;
        mutable Real remainingDetachAmount_;
        
        const Basket* basket_;

        Real beta_;
        BivariateCumulativeNormalDistribution biphi_;
   ////     static InverseCumulativeNormal inverse_;
        static CumulativeNormalDistribution const phi_;
    };

}

#endif
