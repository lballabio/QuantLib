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

#include <ql/qldefines.hpp>

#ifndef QL_PATCH_SOLARIS

#include <ql/math/distributions/bivariatenormaldistribution.hpp>
#include <ql/experimental/credit/recoveryratequote.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/experimental/credit/defaultlossmodel.hpp>
#include <ql/experimental/credit/basket.hpp>
#include <ql/experimental/math/latentmodel.hpp>
#include <ql/functional.hpp>
#include <numeric>

/* Intended to replace GaussianLHPCDOEngine in 
    ql/experimental/credit/syntheticcdoengines.hpp
   Moved from an engine to a loss model, CDO engines might refer to it.
*/

namespace QuantLib {

    /*!
      Portfolio loss model with analytical expected tranche loss for a large 
      homogeneous pool with Gaussian one-factor copula. See for example
      "The Normal Inverse Gaussian Distribution for Synthetic CDO pricing.",
      Anna Kalemanova, Bernd Schmid, Ralf Werner,
      Journal of Derivatives, Vol. 14, No. 3, (Spring 2007), pp. 80-93.
      http://www.defaultrisk.com/pp_crdrv_91.htm

      It can be used to price a credit derivative or to provide risk metrics of 
      a portfolio.

      \todo It should be checking that basket exposures are deterministic (fixed
      or programmed amortizing) otherwise the model is not fit for the basket.

      \todo Bugging on tranched baskets with upper limit over maximum 
        attainable loss?
     */
    class GaussianLHPLossModel : public DefaultLossModel, 
        public LatentModel<GaussianCopulaPolicy> {
    public:
        typedef GaussianCopulaPolicy copulaType;

        GaussianLHPLossModel(
            const Handle<Quote>& correlQuote,
            const std::vector<Handle<RecoveryRateQuote> >& quotes);

        GaussianLHPLossModel(
            Real correlation,
            const std::vector<Real>& recoveries);

        GaussianLHPLossModel(
            const Handle<Quote>& correlQuote,
            const std::vector<Real>& recoveries);

        void update() override {
            sqrt1minuscorrel_ = std::sqrt(1.-correl_->value());
            beta_ = std::sqrt(correl_->value());
            biphi_ = BivariateCumulativeNormalDistribution(
                -beta_);
            // tell basket to notify instruments, etc, we are invalid
            if(!basket_.empty()) basket_->notifyObservers();
        }

    private:
      void resetModel() override {}
      /*! @param attachLimit as a fraction of the underlying live portfolio
      notional
      */
      Real expectedTrancheLossImpl(Real remainingNot, // << at the given date 'd'
                                   Real prob,         // << at the given date 'd'
                                   Real averageRR,    // << at the given date 'd'
                                   Real attachLimit,
                                   Real detachLimit) const;
    public:
      Real expectedTrancheLoss(const Date& d) const override {
          // can calls to Basket::remainingNotional(d) be cached?<<<<<<<<<<<<<
          const Real remainingfullNot = basket_->remainingNotional(d);
          Real averageRR = averageRecovery(d);
          Probability prob = averageProb(d);
          Real remainingAttachAmount = basket_->remainingAttachmentAmount();
          Real remainingDetachAmount = basket_->remainingDetachmentAmount();


          // const Real attach = std::min(remainingAttachAmount
          //    / remainingfullNot, 1.);
          // const Real detach = std::min(remainingDetachAmount
          //    / remainingfullNot, 1.);
          const Real attach = remainingAttachAmount / remainingfullNot;
          const Real detach = remainingDetachAmount / remainingfullNot;

          return expectedTrancheLossImpl(remainingfullNot, prob, averageRR, attach, detach);
      }

        /*! The passed remainingLossFraction is in live tranche units,
            not portfolio as a fraction of the remaining(live) tranche
            (i.e. a_remaining=0% and det_remaining=100%)
        */
      Real probOverLoss(const Date& d, Real remainingLossFraction) const override;

      //! Returns the ESF as an absolute amount (rather than a fraction)
      /* The way it is implemented here is a transformation from ETL to ESF
      is a generic algorithm, not specific to this model so it should be moved
      to the Basket/DefaultLossModel class.
      TO DO: Implement the inverse transformation
      */
      Real expectedShortfall(const Date& d, Probability perctl) const override;

    protected:
        // This is wrong, it is not accounting for the current defaults ....
        // returns the loss value in actual loss units, returns the loss value 
        // for the underlying portfolio, untranched
        Real percentilePortfolioLossFraction(const Date& d, Real perctl) const;
        Real expectedRecovery(const Date& d, Size iName, const DefaultProbKey& ik) const override {
            return rrQuotes_[iName].currentLink()->value();
        }

    public:
        // same as percentilePortfolio but tranched
      Real percentile(const Date& d, Real perctl) const override {
          const Real remainingNot = basket_->remainingNotional(d);
          Real remainingAttachAmount = basket_->remainingAttachmentAmount();
          Real remainingDetachAmount = basket_->remainingDetachmentAmount();
          const Real attach = std::min(remainingAttachAmount / remainingNot, 1.);
          const Real detach = std::min(remainingDetachAmount / remainingNot, 1.);
          return remainingNot *
                 std::min(std::max(percentilePortfolioLossFraction(d, perctl) - attach, 0.),
                          detach - attach);
      }

        Probability averageProb(const Date& d) const {// not an overload of Deflossmodel ???<<<<<???
            // weighted average by programmed exposure.
            const std::vector<Probability> probs = 
                basket_->remainingProbabilities(d);//use remaining basket
            const std::vector<Real> remainingNots = 
                basket_->remainingNotionals(d);
            return std::inner_product(probs.begin(), probs.end(), 
                remainingNots.begin(), 0.) / basket_->remainingNotional(d);
        }

        /* One could define the average recovery without the probability
        factor, weighting only by notional instead, but that way the expected 
        loss of the average/aggregated and the original portfolio would not 
        coincide. This introduces however a time dependence in the recovery 
        value.
        Weighting by notional implies time dependent weighting since the basket 
        might amortize.
        */
        Real averageRecovery(
            const Date& d) const //no explicit time dependence in this model
        {
            const std::vector<Probability> probs = 
                basket_->remainingProbabilities(d);
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

        Handle<Quote> correl_;
        std::vector<Handle<RecoveryRateQuote> > rrQuotes_;
        // calculation buffers

        /* The problem with defining a fixed average recovery on a portfolio 
        with uneven exposures is that it does not preserve portfolio
        moments like the expected loss. To achieve it one should define the 
        averarage recovery with a time dependence: 
        $\hat{R}(t) = \frac{\sum_i R_i N_i P_i(t)}{\sum_i N_i P_i(t)}$
        But the date dependence increases significantly the calculations cost.
        Notice that this problem dissapears if the recoveries are all equal.
        */
        
        Real beta_;
        BivariateCumulativeNormalDistribution biphi_;
        static CumulativeNormalDistribution const phi_;
    };

}

#endif

#endif
