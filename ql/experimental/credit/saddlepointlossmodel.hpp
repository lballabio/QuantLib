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
    <b>Don’t Fall from the Saddle: the Importance of Higher Moments of Credit 
        Loss Distributions</b> J.Annaert, C.Garcia Joao Batista, J.Lamoot, 
        G.Lanine February 2006, Gent University\par
    <b>Analytical techniques for synthetic CDOs and credit default risk 
        measures</b> A. Antonov, S. Mechkovy, and T. Misirpashaevz; 
        NumeriX May 23, 2005 \par
    <b>Computation of VaR and VaR contribution in the Vasicek portfolio credit 
        loss model: a comparative study</b> X.Huang, C.W.Oosterlee, M.Mesters
        Journal of Credit Risk (75–96) Volume 3/ Number 3, Fall 2007 \par
    <b>Higher-order saddlepoint approximations in the Vasicek portfolio credit 
        loss model</b> X.Huang, C.W.Oosterlee, M.Mesters  Journal of 
        Computational Finance (93–113) Volume 11/Number 1, Fall 2007 \par
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
    \todo Restricted by now to gaussian constant loss model. Open it.
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
    class SaddlePointLossModel : public DefaultLossModel {
    public:
        SaddlePointLossModel(const boost::shared_ptr<GaussianConstantLossLM>& m)
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
        boost::tuples::tuple<Real, Real, Real, Real> CumGen0234DerivCond(
            const std::vector<Real>& invUncondProbs,
            Real saddle, 
            const std::vector<Real>&  mktFactor) const;
        boost::tuples::tuple<Real, Real> CumGen02DerivCond(
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
        class SaddleObjectiveFunction : 
            public std::unary_function<Real, Real> {
            const SaddlePointLossModel& me_;
            Real targetValue_;
            const std::vector<Real>& mktFactor_;
            const std::vector<Real>& invUncondProbs_;
        public:
            //! @param target in fractional loss units
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

            @param lossLevel in total portfolio loss fractional unit
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

        class SaddlePercObjFunction : public std::unary_function<Real, Real> {
            const SaddlePointLossModel& me_;
            Real targetValue_;
            Date date_;
        public:
            SaddlePercObjFunction(
                const SaddlePointLossModel& me,
                const Real target,
                const Date& date)
            : me_(me), targetValue_(1.-target), date_(date) {}
            /*!
                @param x Is the _tranche_ loss fraction
            */
            Real operator()(const Real x) const {
                return me_.probOverLoss(date_, x) - targetValue_;
            }
        };
        // Functionality, Provides various portfolio statistics---------------
    public:
        /*! Returns the loss amount at the requested date for which the 
        probability of lossing that amount or less is equal to the value passed.
        */
        Real percentile(const Date& d, Probability percentile) const;
    protected:
        /*! Conditional (on the mkt factor) prob of a loss fraction of the the 
                tranched portfolio.

            @param lossFraction Fraction over the tranche notional. In [0,1]
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
        Probability probOverLoss(const Date& d, Real trancheLossFract) const;

        Disposable<std::map<Real, Probability> > 
            lossDistribution(const Date& d) const;
    protected:
        /*! 
            Probability of having losses in the portfolio due to default 
            events equal or larger than a given absolute loss value on a 
            given date conditional to the latent model factor.
            The integral expression on the expansion is the first order 
            integration as presented in several references, see for instance; 
            equation 8 in R.Martin, K.Thompson, and C. Browne 's 
            'Taking to the Saddle', Risk Magazine, June 2001, page 91

            @param loss loss in absolute value
        */
        Probability probOverLossPortfCond(
                        const std::vector<Real>& invUncondProbs,

            Real loss, 
            const std::vector<Real>& mktFactor) const;
    public:
        Probability probOverPortfLoss(const Date& d, Real loss) const;
        Real expectedTrancheLoss(const Date& d) const;
    protected:
        /*!
        Probability density of having losses in the total portfolio (untranched)
        due to default events equal to a given value on a given date conditional
        to the latent model factor.
        Based on the integrals of the expected shortfall. 
        */
        Probability probDensityCond( 
                        const std::vector<Real>& invUncondProbs,

            Real loss, 
            const std::vector<Real>& mktFactor) const;
    public:
        Probability probDensity(const Date& d, Real loss) const;
    protected:
        Disposable<std::vector<Real> > splitLossCond(
                        const std::vector<Real>& invUncondProbs,

            Real loss, 
            std::vector<Real> mktFactor) const;
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
            \to do  Fix this.
            \par
            see equation 8 in <b>VAR: who contributes and how much?</b> by 
            R.Martin, K.Thompson, and C. Browne in Risk Magazine, August 2001

        @param loss Loss amount level at which we want to request the 
                        sensitivity. Equivalent to a percentile.
        */
        Disposable<std::vector<Real> > 
            splitVaRLevel(const Date& date, Real loss) const;
        Real expectedShortfall(const Date&d, Probability percentile) const;
    protected:
        Real conditionalExpectedLoss(
            const std::vector<Real>& invUncondProbs,
            const std::vector<Real>& mktFactor) const;
        Real conditionalExpectedTrancheLoss(
            const std::vector<Real>& invUncondProbs,
            const std::vector<Real>& mktFactor) const;

        void resetModel() {
            remainingNotionals_ = basket_->remainingNotionals();
            remainingNotional_  = basket_->remainingNotional();
            attachRatio_ = std::min(basket_->remainingAttachmentAmount() 
                / basket_->remainingNotional(), 1.);
            detachRatio_ = std::min(basket_->remainingDetachmentAmount() 
                / basket_->remainingNotional(), 1.);
            copula_->resetBasket(basket_.currentLink());
        }
    protected:
        const boost::shared_ptr<GaussianConstantLossLM> copula_;
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
    inline Real SaddlePointLossModel::CumulantGenerating(
        const Date& date, Real s) const 
    {
        std::vector<Real> invUncondProbs = 
            basket_->remainingProbabilities(date);
        for(Size i=0; i<invUncondProbs.size(); i++)
            invUncondProbs[i] = 
            copula_->inverseCumulativeY(invUncondProbs[i], i);

        return copula_->integratedExpectedValue(
            boost::function<Real (const std::vector<Real>& v1)>(
                boost::bind(
                    &SaddlePointLossModel::CumulantGeneratingCond,
                    this,
                    boost::cref(invUncondProbs),
                   s,
                    _1)
                )
            );
    }

    inline Real SaddlePointLossModel::CumGen1stDerivative(
        const Date& date, Real s) const 
    {
        std::vector<Real> invUncondProbs = 
            basket_->remainingProbabilities(date);
        for(Size i=0; i<invUncondProbs.size(); i++)
            invUncondProbs[i] = 
            copula_->inverseCumulativeY(invUncondProbs[i], i);

       return copula_->integratedExpectedValue(
            boost::function<Real (const std::vector<Real>& v1)>(
                boost::bind(
                    &SaddlePointLossModel::CumGen1stDerivativeCond,
                    this,
                    boost::cref(invUncondProbs),
                    s,
                    _1)
                )
            );
    }

    inline Real SaddlePointLossModel::CumGen2ndDerivative(
        const Date& date, Real s) const 
    {
        std::vector<Real> invUncondProbs = 
            basket_->remainingProbabilities(date);
        for(Size i=0; i<invUncondProbs.size(); i++)
            invUncondProbs[i] = 
            copula_->inverseCumulativeY(invUncondProbs[i], i);

        return copula_->integratedExpectedValue(
            boost::function<Real (const std::vector<Real>& v1)>(
                boost::bind(
                    &SaddlePointLossModel::CumGen2ndDerivativeCond,
                    this,
                    boost::cref(invUncondProbs),
                    s,
                    _1)
                )
            );
    }

    inline Real SaddlePointLossModel::CumGen3rdDerivative(
        const Date& date, Real s) const 
    {
        std::vector<Real> invUncondProbs = 
            basket_->remainingProbabilities(date);
        for(Size i=0; i<invUncondProbs.size(); i++)
            invUncondProbs[i] = 
            copula_->inverseCumulativeY(invUncondProbs[i], i);

        return copula_->integratedExpectedValue(
            boost::function<Real (const std::vector<Real>& v1)>(
                boost::bind(
                    &SaddlePointLossModel::CumGen3rdDerivativeCond,
                    this,
                    boost::cref(invUncondProbs),
                    s,
                    _1)
                )
            );
    }

    inline Real SaddlePointLossModel::CumGen4thDerivative(
        const Date& date, Real s) const 
    {
        std::vector<Real> invUncondProbs = 
            basket_->remainingProbabilities(date);
        for(Size i=0; i<invUncondProbs.size(); i++)
            invUncondProbs[i] = 
            copula_->inverseCumulativeY(invUncondProbs[i], i);

        return copula_->integratedExpectedValue(
            boost::function<Real (const std::vector<Real>& v1)>(
                boost::bind(
                    &SaddlePointLossModel::CumGen4thDerivativeCond,
                    this,
                    boost::cref(invUncondProbs),
                    s,
                    _1)
                )
            );
    }

    inline Probability SaddlePointLossModel::probOverLoss(
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
            boost::function<Real (const std::vector<Real>& v1)>(
                boost::bind(
                    &SaddlePointLossModel::probOverLossCond,
                    this,
                    boost::cref(invUncondProbs),
                    trancheLossFract,
                    _1)
                )
            );
        }

    inline Probability SaddlePointLossModel::probOverPortfLoss(
        const Date& d, Real loss) const 
    {
        const std::vector<Probability> uncondProbs = 
            basket_->remainingProbabilities(d);

        std::vector<Real> invUncondProbs = 
            basket_->remainingProbabilities(d);
        for(Size i=0; i<invUncondProbs.size(); i++)
            invUncondProbs[i] = 
            copula_->inverseCumulativeY(invUncondProbs[i], i);

        return copula_->integratedExpectedValue(
            boost::function<Real (const std::vector<Real>& v1)>(
                boost::bind(
                    &SaddlePointLossModel::probOverLossPortfCond,
                    this,
                    boost::cref(invUncondProbs),
                    loss,
                    _1)
                )
            );
    }

    inline Real SaddlePointLossModel::expectedTrancheLoss(
        const Date& d) const 
    {
        std::vector<Real> invUncondProbs = 
            basket_->remainingProbabilities(d);
        for(Size i=0; i<invUncondProbs.size(); i++)
            invUncondProbs[i] = 
            copula_->inverseCumulativeY(invUncondProbs[i], i);

        return copula_->integratedExpectedValue(
            boost::function<Real (const std::vector<Real>& v1)>(
                boost::bind(
                    &SaddlePointLossModel::conditionalExpectedTrancheLoss,
                    this,
                    boost::cref(invUncondProbs),
                    _1)
                )
            );
    }

    inline Probability SaddlePointLossModel::probDensity(
        const Date& d, Real loss) const 
    {
        std::vector<Real> invUncondProbs = 
            basket_->remainingProbabilities(d);
        for(Size i=0; i<invUncondProbs.size(); i++)
            invUncondProbs[i] = 
            copula_->inverseCumulativeY(invUncondProbs[i], i);

        return copula_->integratedExpectedValue(
            boost::function<Real (const std::vector<Real>& v1)>(
                boost::bind(
                    &SaddlePointLossModel::probDensityCond,
                    this,
                    boost::cref(invUncondProbs),
                    loss,
                    _1)
                )
            );
    }

    inline Disposable<std::vector<Real> > 
    SaddlePointLossModel::splitVaRLevel(const Date& date, Real s) const 
    {
        std::vector<Real> invUncondProbs = 
            basket_->remainingProbabilities(date);
        for(Size i=0; i<invUncondProbs.size(); i++)
            invUncondProbs[i] = 
            copula_->inverseCumulativeY(invUncondProbs[i], i);

        return copula_->integratedExpectedValue(
            boost::function<Disposable<std::vector<Real> > (
                const std::vector<Real>& v1)>(
                    boost::bind(
                        &SaddlePointLossModel::splitLossCond,
                        this,
                    boost::cref(invUncondProbs),
                        s,
                        _1)
                )
            );
    }

}

#endif
