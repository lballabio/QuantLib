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

#ifndef quantlib_saddle_point_lossmodel_hpp
#define quantlib_saddle_point_lossmodel_hpp

#include <ql/tuple.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/math/solvers1d/newton.hpp>
#include <ql/experimental/credit/basket.hpp>
#include <ql/experimental/credit/defaultlossmodel.hpp>
#include <ql/experimental/credit/constantlosslatentmodel.hpp>

namespace QuantLib {

    /*! \brief Saddle point portfolio credit default loss model.\par
      Default Loss model implementing the Saddle point expansion 
      integrations on several default risk metrics. Codepence is dealt 
      through a latent model making the integrals conditional to the latent 
      model factor. Latent variables are integrated indirectly.\par
    See:\par
    <b>Taking to the saddle</b> by R.Martin, K.Thompson and C.Browne; RISK JUNE 
        2001; p.91\par
    <b>The saddlepoint method and portfolio optionalities</b> R.Martin in Risk 
        December 2006\par
    <b>VAR: who contributes and how much?</b> R.Martin, K.Thompson and 
        C.Browne RISK AUGUST 2001\par
    <b>Shortfall: Who contributes and how much?</b> R. J. Martin, Credit Suisse 
        January 3, 2007 \par
    <b>Don't Fall from the Saddle: the Importance of Higher Moments of Credit 
        Loss Distributions</b> J.Annaert, C.Garcia Joao Batista, J.Lamoot, 
        G.Lanine February 2006, Gent University\par
    <b>Analytical techniques for synthetic CDOs and credit default risk 
        measures</b> A. Antonov, S. Mechkovy, and T. Misirpashaevz; 
        NumeriX May 23, 2005 \par
    <b>Computation of VaR and VaR contribution in the Vasicek portfolio credit 
        loss model: a comparative study</b> X.Huang, C.W.Oosterlee, M.Mesters
        Journal of Credit Risk (75-96) Volume 3/ Number 3, Fall 2007 \par
    <b>Higher-order saddlepoint approximations in the Vasicek portfolio credit 
        loss model</b> X.Huang, C.W.Oosterlee, M.Mesters  Journal of 
        Computational Finance (93-113) Volume 11/Number 1, Fall 2007 \par
    While more expensive, a high order expansion is used here; see the paper by 
    Antonov et al for the terms retained.\par
    For a discussion of an alternative to fix the error at low loss levels 
    (more relevant to pricing than risk metrics) see: \par
    <b>The hybrid saddlepoint method for credit portfolios</b> by A.Owen, 
    A.McLeod and K.Thompson; in Risk, August 2009. This is not implemented here
    though (yet?...)\par
    For the more general context mathematical theory see: <b>Saddlepoint 
    approximations with applications</b> by R.W. Butler, Cambridge series in 
    statistical and probabilistic mathematics. 2007 \par
    \todo Some portfolios show instabilities in the high order expansion terms.
    \todo Methods here are calling and integrating using the unconditional 
        probabilities without inverting them first; quite a lot of calls to 
        the copula inversion can be avoided; this should improve performance.
    \todo Revise the model for stability of the saddle point calculation. The
        search for the point does not convege in extreme cases; e.g. very high
        value of all the factors; factors for each variable not ordered from 
        high to low,...
    */

    /* The treatment of recovery wont work with random recoveries, they should
    be passed to the conditional methods in the same way as the probabilities.
    */

    /*
    TO DO:
    -> Failing when the tranche upper loss limit goes over the max attainable 
        loss.

    - With 15 quadrature points things are OK but 25 gives me -1#IND000 errors 
            (over region around the EL I think) 
    - Silly bug when calling some methods on todays date (zero time). 
            ProbDef = 0 there
    - VaR <- tranched?????!
    - ESF <- tranched????!!
    - VaR split
    - ESF split?

    When introducing defaults; somewhere, (after an update?) there should be 
    a check that: copula_->basketSize() EQUALS remainingBasket_.size()
    */
    template<class CP> 
    class SaddlePointLossModel : public DefaultLossModel {
    public:
        explicit SaddlePointLossModel(
            const ext::shared_ptr<ConstantLossLatentmodel<CP> >& m)
            : copula_(m) { }
    protected:
        // ----------- Cumulants and derivatives auxiliary functions ---------

        /*! Returns the cumulant generating function (zero-th order 
        expansion term) conditional to the mkt factor:
            \f$ K = \sum_j ln(1-p_j + p_j e^{N_j \times lgd_j \times s}) \f$
        */
        Real CumulantGeneratingCond(
            const std::vector<Real>& invUncondProbs,
            Real lossFraction,// saddle pt
            const std::vector<Real>&  mktFactor) const;
        /*! Returns the first derivative of the cumulant generating function 
        (first order expansion term) conditional to the mkt factor:
           \f$ K1 = \sum_j \frac{p_j \times N_j \times LGD_j \times 
                e^{N_j \times LGD_j \times s}} \
                             {1-p_j + p_j e^{N_j \times LGD_j \times s}} \f$
           One of its properties is that its value at zero is the portfolio 
           expected loss (in fractional units). Its value at infinity is the 
           max attainable portfolio loss. To be understood conditional to the 
           market factor.
        */
        Real CumGen1stDerivativeCond(
            const std::vector<Real>& invUncondProbs,
            Real saddle, // in fract loss units... humm not really
            const std::vector<Real>&  mktFactor) const;
        /*! Returns the second derivative of the cumulant generating function 
        (first order expansion term) conditional to the mkt factor:
            \f$ K2 = \sum_j \frac{p_j \times (N_j \times LGD_j)^2 \times 
                e^{N_j \times LGD_j \times s}}
                             {1-p_j + p_j e^{N_j \times LGD_j \times s}}
                      - (\frac{p_j \times N_j \times LGD_j \times e^{N_j \times 
                      LGD_j \times s}}
                             {1-p_j + p_j e^{N_j \times LGD_j \times s}})^2 \f$
        */
        Real CumGen2ndDerivativeCond(
            const std::vector<Real>& invUncondProbs,
            Real saddle, 
            const std::vector<Real>&  mktFactor) const;
        Real CumGen3rdDerivativeCond(
            const std::vector<Real>& invUncondProbs,
            Real saddle, 
            const std::vector<Real>&  mktFactor) const;
        Real CumGen4thDerivativeCond(
            const std::vector<Real>& invUncondProbs,
            Real saddle, 
            const std::vector<Real>&  mktFactor) const ;
        /*! Returns the cumulant and second to fourth derivatives together.
          Included for optimization, most methods work on expansion of these 
          terms.
          Alternatively use a local private buffer member? */
        ext::tuple<Real, Real, Real, Real> CumGen0234DerivCond(
            const std::vector<Real>& invUncondProbs,
            Real saddle, 
            const std::vector<Real>&  mktFactor) const;
        ext::tuple<Real, Real> CumGen02DerivCond(
            const std::vector<Real>& invUncondProbs,
            Real saddle, 
            const std::vector<Real>&  mktFactor) const;

        /* Unconditional cumulants. Because this class integrates the various
          statistics it provides in indirect mode they are never used. 
          Provided for completeness/extendability */
        /*! Returns the cumulant generating function (zero-th order expansion
          term) weighting the conditional value by the prob density of the 
          market factor, called by integrations */
        Real CumulantGenerating(const Date& date, Real s) const;
        Real CumGen1stDerivative(const Date& date, Real s) const;
        Real CumGen2ndDerivative(const Date& date, Real s) const;
        Real CumGen3rdDerivative(const Date& date, Real s) const;
        Real CumGen4thDerivative(const Date& date, Real s) const;
        
        // -------- Saddle point search functions ---------------------------
        class SaddleObjectiveFunction {
            const SaddlePointLossModel& me_;
            Real targetValue_;
            const std::vector<Real>& mktFactor_;
            const std::vector<Real>& invUncondProbs_;
        public:
            //! The passed target is in fractional loss units
            SaddleObjectiveFunction(const SaddlePointLossModel& me,
                                    const Real target,
                                    const std::vector<Real>& invUncondProbs,
                                    const std::vector<Real>& mktFactor
                                    )
            : me_(me), 
              targetValue_(target), 
              mktFactor_(mktFactor), 
              invUncondProbs_(invUncondProbs)
            {}
            Real operator()(const Real x) const {
                return me_.CumGen1stDerivativeCond(invUncondProbs_, x, 
                    mktFactor_) - targetValue_;
            }
            Real derivative(Real x) const {
                return me_.CumGen2ndDerivativeCond(invUncondProbs_, x, 
                    mktFactor_);
            }
        };

        /*! Calculates the mkt-fct-conditional saddle point for the loss level 
            given and the probability passed. 
            The date is implicitly given through the probability. Performance 
            requires to pass the probabilities for that date. Otherwise once we
            integrate this over the market factor we would be computing the same
            probabilities over and over. While this works fine here some models
            of the recovery rate might require the date.

            The passed lossLevel is in total portfolio loss fractional units.

            \todo Improve convergence speed (which is bad at the moment).See 
            discussion in several places; references above and The Oxford 
            Handbook of CD, sect 2.9
        */
        Real findSaddle(
            const std::vector<Real>& invUncondProbs,
            Real lossLevel,
            const std::vector<Real>& mktFactor, 
            Real accuracy = 1.0e-3,//1.e-4
            Natural maxEvaluations = 50
            ) const;

        class SaddlePercObjFunction {
            const SaddlePointLossModel& me_;
            Real targetValue_;
            Date date_;
        public:
            SaddlePercObjFunction(
                const SaddlePointLossModel& me,
                const Real target,
                const Date& date)
            : me_(me), targetValue_(1.-target), date_(date) {}
            /*! The passed x is the _tranche_ loss fraction */
            Real operator()(const Real x) const {
                return me_.probOverLoss(date_, x) - targetValue_;
            }
        };
        // Functionality, Provides various portfolio statistics---------------
    public:
        /*! Returns the loss amount at the requested date for which the 
        probability of lossing that amount or less is equal to the value passed.
        */
      Real percentile(const Date& d, Probability percentile) const override;

    protected:
        /*! Conditional (on the mkt factor) prob of a loss fraction of
            the the tranched portfolio.

            The trancheLossFract parameter is the fraction over the
            tranche notional and must be in [0,1].
        */
        Probability probOverLossCond( 
            const std::vector<Real>& invUncondProbs,
            Real trancheLossFract, 
            const std::vector<Real>& mktFactor) const;
        Probability probOverLossPortfCond1stOrder(
            const std::vector<Real>& invUncondProbs,
            Real loss, 
            const std::vector<Real>& mktFactor) const;
    public:
      Probability probOverLoss(const Date& d, Real trancheLossFract) const override;

      Disposable<std::map<Real, Probability> > lossDistribution(const Date& d) const override;

    protected:
        /*! 
            Probability of having losses in the portfolio due to default 
            events equal or larger than a given absolute loss value on a 
            given date conditional to the latent model factor.
            The integral expression on the expansion is the first order 
            integration as presented in several references, see for instance; 
            equation 8 in R.Martin, K.Thompson, and C. Browne 's 
            'Taking to the Saddle', Risk Magazine, June 2001, page 91

            The passed loss is in absolute value.
        */
        Probability probOverLossPortfCond(
            const std::vector<Real>& invUncondProbs,
            Real loss, 
            const std::vector<Real>& mktFactor) const;
    public:
        Probability probOverPortfLoss(const Date& d, Real loss) const;
        Real expectedTrancheLoss(const Date& d) const override;

      protected:
        /*!
        Probability density of having losses in the total portfolio (untranched)
        due to default events equal to a given value on a given date conditional
        to the latent model factor.
        Based on the integrals of the expected shortfall. 
        */
        Probability probDensityCond(const std::vector<Real>& invUncondProbs,
            Real loss, const std::vector<Real>& mktFactor) const;
    public:
        Probability probDensity(const Date& d, Real loss) const;
    protected:
        Disposable<std::vector<Real> > splitLossCond(
            const std::vector<Real>& invUncondProbs,
            Real loss, std::vector<Real> mktFactor) const;
        Real expectedShortfallFullPortfolioCond(
            const std::vector<Real>& invUncondProbs,
            Real lossPerc, const std::vector<Real>& mktFactor) const;
        Real expectedShortfallTrancheCond(
            const std::vector<Real>& invUncondProbs,
            Real lossPerc, Probability percentile, 
            const std::vector<Real>& mktFactor) const;
        Disposable<std::vector<Real> > expectedShortfallSplitCond(
            const std::vector<Real>& invUncondProbs,
            Real lossPerc, const std::vector<Real>& mktFactor) const;
    public:
        /*! Sensitivities of the individual names to a given portfolio loss 
            value due to defaults. It returns ratios to the total structure 
            notional, which aggregated add up to the requested loss value.
            Notice then that it refers to the total portfolio, not the tranched
            basket.
            \todo  Fix this.
            \par
            see equation 8 in <b>VAR: who contributes and how much?</b> by 
            R.Martin, K.Thompson, and C. Browne in Risk Magazine, August 2001

            The passed loss is the loss amount level at which we want
            to request the sensitivity. Equivalent to a percentile.
        */
      Disposable<std::vector<Real> > splitVaRLevel(const Date& date, Real loss) const override;
      Real expectedShortfall(const Date& d, Probability percentile) const override;

    protected:
        Real conditionalExpectedLoss(
            const std::vector<Real>& invUncondProbs,
            const std::vector<Real>& mktFactor) const;
        Real conditionalExpectedTrancheLoss(
            const std::vector<Real>& invUncondProbs,
            const std::vector<Real>& mktFactor) const;

        void resetModel() override {
            remainingNotionals_ = basket_->remainingNotionals();
            remainingNotional_  = basket_->remainingNotional();
            attachRatio_ = std::min(basket_->remainingAttachmentAmount() 
                / basket_->remainingNotional(), 1.);
            detachRatio_ = std::min(basket_->remainingDetachmentAmount() 
                / basket_->remainingNotional(), 1.);
            copula_->resetBasket(basket_.currentLink());
        }

        const ext::shared_ptr<ConstantLossLatentmodel<CP> > copula_;
        // cached todays arguments values
        mutable Size remainingSize_;
        mutable std::vector<Real> remainingNotionals_;
        mutable Real remainingNotional_;
        // remaining basket levels:
        mutable Real attachRatio_, detachRatio_;
        /*
        // Just for testing the ESF direct integration, not for release, 
        //   this is very inneficient:
        class ESFIntegrator {
        public:
            ESFIntegrator(const SaddlePointLossModel& me,
                const Date& date,
                Real lossPercentileFract//,
                //const std::vector<Real>& mktFactor
                )
                : me_(me), date_(date),lossPercentileFract_(lossPercentileFract)
            {}


            Real operator()(Real x) const {
                return me_.densityTrancheLoss(date_, x + lossPercentileFract_) 
                    * (x + lossPercentileFract_);
            }

            Real lossPercentileFract_;
            Date date_;
            //  const std::vector<Real>& mktFactor_;
            const SaddlePointLossModel& me_;
        };
        */
    };


    // -- Inlined integrations------------------------------------------------

    // Unconditional Moments and derivatives. --------------------------------
    template<class CP>
    inline Real SaddlePointLossModel<CP>::CumulantGenerating(
        const Date& date, Real s) const 
    {
        std::vector<Real> invUncondProbs = 
            basket_->remainingProbabilities(date);
        for(Size i=0; i<invUncondProbs.size(); i++)
            invUncondProbs[i] = 
            copula_->inverseCumulativeY(invUncondProbs[i], i);

        return copula_->integratedExpectedValue(
            [&](const std::vector<Real>& v1) {
                return CumulantGeneratingCond(invUncondProbs, s, v1);
            });
    }

    template<class CP>
    inline Real SaddlePointLossModel<CP>::CumGen1stDerivative(
        const Date& date, Real s) const 
    {
        std::vector<Real> invUncondProbs = 
            basket_->remainingProbabilities(date);
        for(Size i=0; i<invUncondProbs.size(); i++)
            invUncondProbs[i] = 
            copula_->inverseCumulativeY(invUncondProbs[i], i);

       return copula_->integratedExpectedValue(
           [&](const std::vector<Real>& v1) {
               return CumGen1stDerivativeCond(invUncondProbs, s, v1);
           });
    }

    template<class CP>
    inline Real SaddlePointLossModel<CP>::CumGen2ndDerivative(
        const Date& date, Real s) const 
    {
        std::vector<Real> invUncondProbs = 
            basket_->remainingProbabilities(date);
        for(Size i=0; i<invUncondProbs.size(); i++)
            invUncondProbs[i] = 
            copula_->inverseCumulativeY(invUncondProbs[i], i);

        return copula_->integratedExpectedValue(
           [&](const std::vector<Real>& v1) {
               return CumGen2ndDerivativeCond(invUncondProbs, s, v1);
           });
    }

    template<class CP>
    inline Real SaddlePointLossModel<CP>::CumGen3rdDerivative(
        const Date& date, Real s) const 
    {
        std::vector<Real> invUncondProbs = 
            basket_->remainingProbabilities(date);
        for(Size i=0; i<invUncondProbs.size(); i++)
            invUncondProbs[i] = 
            copula_->inverseCumulativeY(invUncondProbs[i], i);

        return copula_->integratedExpectedValue(
           [&](const std::vector<Real>& v1) {
               return CumGen3rdDerivativeCond(invUncondProbs, s, v1);
           });
    }

    template<class CP>
    inline Real SaddlePointLossModel<CP>::CumGen4thDerivative(
        const Date& date, Real s) const 
    {
        std::vector<Real> invUncondProbs = 
            basket_->remainingProbabilities(date);
        for(Size i=0; i<invUncondProbs.size(); i++)
            invUncondProbs[i] = 
            copula_->inverseCumulativeY(invUncondProbs[i], i);

        return copula_->integratedExpectedValue(
           [&](const std::vector<Real>& v1) {
               return CumGen4thDerivativeCond(invUncondProbs, s, v1);
           });
    }

    template<class CP>
    inline Probability SaddlePointLossModel<CP>::probOverLoss(
        const Date& d, Real trancheLossFract) const 
    {
        // avoid computation:
        if (trancheLossFract >= 
            // time dependent soon:
            basket_->detachmentAmount()) return 0.;

        std::vector<Real> invUncondProbs = 
            basket_->remainingProbabilities(d);
        for(Size i=0; i<invUncondProbs.size(); i++)
            invUncondProbs[i] = 
            copula_->inverseCumulativeY(invUncondProbs[i], i);

        return copula_->integratedExpectedValue(
           [&](const std::vector<Real>& v1) {
               return probOverLossCond(invUncondProbs, trancheLossFract, v1);
           });
    }

    template<class CP>
    inline Probability SaddlePointLossModel<CP>::probOverPortfLoss(
        const Date& d, Real loss) const 
    {
        std::vector<Probability> invUncondProbs = 
            basket_->remainingProbabilities(d);
        for(Size i=0; i<invUncondProbs.size(); i++)
            invUncondProbs[i] = 
            copula_->inverseCumulativeY(invUncondProbs[i], i);

        return copula_->integratedExpectedValue(
           [&](const std::vector<Real>& v1) {
               return probOverLossPortfCond(invUncondProbs, loss, v1);
           });
    }

    template<class CP>
    inline Real SaddlePointLossModel<CP>::expectedTrancheLoss(
        const Date& d) const 
    {
        std::vector<Real> invUncondProbs = 
            basket_->remainingProbabilities(d);
        for(Size i=0; i<invUncondProbs.size(); i++)
            invUncondProbs[i] = 
            copula_->inverseCumulativeY(invUncondProbs[i], i);

        return copula_->integratedExpectedValue(
           [&](const std::vector<Real>& v1) {
               return conditionalExpectedTrancheLoss(invUncondProbs, v1);
           });
    }

    template<class CP>
    inline Probability SaddlePointLossModel<CP>::probDensity(
        const Date& d, Real loss) const 
    {
        std::vector<Real> invUncondProbs = 
            basket_->remainingProbabilities(d);
        for(Size i=0; i<invUncondProbs.size(); i++)
            invUncondProbs[i] = 
            copula_->inverseCumulativeY(invUncondProbs[i], i);

        return copula_->integratedExpectedValue(
           [&](const std::vector<Real>& v1) {
               return probDensityCond(invUncondProbs, loss, v1);
           });
    }

    template<class CP>
    inline Disposable<std::vector<Real> > 
    SaddlePointLossModel<CP>::splitVaRLevel(const Date& date, Real s) const 
    {
        std::vector<Real> invUncondProbs = 
            basket_->remainingProbabilities(date);
        for(Size i=0; i<invUncondProbs.size(); i++)
            invUncondProbs[i] = 
            copula_->inverseCumulativeY(invUncondProbs[i], i);

        return copula_->integratedExpectedValueV(
           [&](const std::vector<Real>& v1) {
               return splitLossCond(invUncondProbs, s, v1);
           });
    }







    /* ------------------------------------------------------------------------
                    Conditional Moments and derivatives. 

        Notice that in all this methods the date dependence is implicitly
        present in the unconditional probabilities. But, as in other LMs, it
        is redundant and expensive to perform the call to the probabilities in
        these methods since they are integrands.
       ---------------------------------------------------------------------- */

    template<class CP>
    Real SaddlePointLossModel<CP>::CumulantGeneratingCond(
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

    template<class CP>
    Real SaddlePointLossModel<CP>::CumGen1stDerivativeCond(
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

    template<class CP>
    Real SaddlePointLossModel<CP>::CumGen2ndDerivativeCond(
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

    template<class CP>
    Real SaddlePointLossModel<CP>::CumGen3rdDerivativeCond(
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

    template<class CP>
    Real SaddlePointLossModel<CP>::CumGen4thDerivativeCond(
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

    template<class CP>
    ext::tuple<Real, Real, Real, Real> /// DISPOSABLE????
        SaddlePointLossModel<CP>::CumGen0234DerivCond(
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
        return {deriv0, deriv2, deriv3, deriv4};
    }

    template<class CP>
    ext::tuple<Real, Real> /// DISPOSABLE???? 
        SaddlePointLossModel<CP>::CumGen02DerivCond(
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
        return {deriv0, deriv2};
    }

    // ----- Saddle point search ----------------------------------------------

    template<class CP>
    Real SaddlePointLossModel<CP>::findSaddle(
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


    template<class CP>
    Real SaddlePointLossModel<CP>::percentile(const Date& d, 
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

    template<class CP>
    Probability SaddlePointLossModel<CP>::probOverLossCond(
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

    template<class CP>
    Disposable<std::map<Real, Probability> > 
        SaddlePointLossModel<CP>::lossDistribution(const Date& d) const {
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

        The loss is passed in absolute value.
    */
    template<class CP>
    Probability SaddlePointLossModel<CP>::probOverLossPortfCond(
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

        ext::tuple<Real, Real, Real, Real> cumulants = 
            CumGen0234DerivCond(invUncondProbs, 
                saddlePt, mktFactor);
        Real baseVal = ext::get<0>(cumulants);
        Real secondVal = ext::get<1>(cumulants);
        Real K3Saddle = ext::get<2>(cumulants);
        Real K4Saddle = ext::get<3>(cumulants);

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

    template<class CP>
    // cheaper; less terms retained; yet the cost lies in the saddle point calc
    Probability SaddlePointLossModel<CP>::probOverLossPortfCond1stOrder(
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

        ext::tuple<Real, Real> cumulants = 
            CumGen02DerivCond(invUncondPs,
                saddlePt, mktFactor);
        Real baseVal = ext::get<0>(cumulants);
        Real secondVal = ext::get<1>(cumulants);

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
    template<class CP>
    Probability SaddlePointLossModel<CP>::probDensityCond(
        const std::vector<Real>& invUncondPs,
        Real loss,
        const std::vector<Real>& mktFactor) const 
    {
        if (loss <= QL_EPSILON) return 0.;

        Real relativeLoss = loss / remainingNotional_;
        Real saddlePt = findSaddle(invUncondPs,
            relativeLoss, mktFactor);

        ext::tuple<Real, Real, Real, Real> cumulants = 
            CumGen0234DerivCond(invUncondPs,
            saddlePt, mktFactor);
        /// access them directly rather than through this copy
        Real K0Saddle = ext::get<0>(cumulants);
        Real K2Saddle = ext::get<1>(cumulants);
        Real K3Saddle = ext::get<2>(cumulants);
        Real K4Saddle = ext::get<3>(cumulants);
        /* see, for instance R.Martin "he saddle point method and portfolio 
        optionalities." in Risk December 2006 p.93 */
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

    The passed loss is the loss amount level at which we want to
    request the sensitivity.  Equivalent to a percentile.
    */
    template<class CP>
    Disposable<std::vector<Real> > SaddlePointLossModel<CP>::splitLossCond(
        const std::vector<Real>& invUncondProbs,
        Real loss, 
        std::vector<Real> mktFactor) const 
    {
        const Size nNames = remainingNotionals_.size();
        std::vector<Real> condContrib(nNames, 0.);
        if (loss <= QL_EPSILON) return condContrib;

        Real saddlePt = findSaddle(invUncondProbs, loss / remainingNotional_, 
            mktFactor);

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

    template<class CP>
    Real SaddlePointLossModel<CP>::conditionalExpectedLoss(
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

    template<class CP>
    Real SaddlePointLossModel<CP>::conditionalExpectedTrancheLoss(
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

    template<class CP>
    Disposable<std::vector<Real> > 
        SaddlePointLossModel<CP>::expectedShortfallSplitCond(
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
        for (Size iName=0; iName < nNames; iName++)
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

    template<class CP>
    Real SaddlePointLossModel<CP>::expectedShortfallTrancheCond(
        const std::vector<Real>& invUncondProbs,
        Real lossPerc, // value 
        Probability percentile,
        const std::vector<Real>& mktFactor) const 
    {
        /* TO DO: this is too crude, a general expression valid for all 
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

    template<class CP>
    Real SaddlePointLossModel<CP>::expectedShortfallFullPortfolioCond(
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

        ext::tuple<Real, Real, Real, Real> cumulants = 
            CumGen0234DerivCond(uncondProbs, 
                saddlePt, mktFactor);
        Real K0Saddle = ext::get<0>(cumulants);///USE THEM DIRECTLY
        Real K2Saddle = ext::get<1>(cumulants);

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

    template<class CP>
    Real SaddlePointLossModel<CP>::expectedShortfall(const Date&d, 
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
            [&](const std::vector<Real>& v1) {
                return expectedShortfallFullPortfolioCond(invUncondProbs, lossPerc, v1);
            }) / (1.-percProb);

    /* test:?
        return std::inner_product(integrESFPartition.begin(), 
        integrESFPartition.end(), remainingNotionals_.begin(), 0.);
    */        

    }



}

#endif


#ifndef id_813d272c0ee12aacc13cc38f853a8378
#define id_813d272c0ee12aacc13cc38f853a8378
inline bool test_813d272c0ee12aacc13cc38f853a8378(int* i) { return i != 0; }
#endif
