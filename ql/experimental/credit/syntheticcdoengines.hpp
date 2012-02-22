/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2009 Jose Aparicio

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

/*! \file syntheticcdoengines.hpp
    \brief Pricing engines for the Synthetic CDO instrument
    \todo Add further engines for analytical expected tranche loss
          cases - large homogeneous pool with Normal Inverse Gaussian,
          Gamma copula
*/

#ifndef quantlib_synthetic_cdo_engines_hpp
#define quantlib_synthetic_cdo_engines_hpp

#include <ql/experimental/credit/syntheticcdo.hpp>
#include <ql/experimental/credit/randomdefaultmodel.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/distributions/bivariatenormaldistribution.hpp>

namespace QuantLib {

    //--------------------------------------------------------------------------
    //! CDO base engine
    class SyntheticCDO::engine : public GenericEngine<SyntheticCDO::arguments,
                                                      SyntheticCDO::results> {
    protected:
        virtual Real expectedTrancheLoss(const Date&) const = 0;
        virtual void initialize() const {
            Date today = Settings::instance().evaluationDate();
            Date start = this->arguments_.schedule.startDate();
            boost::shared_ptr<Basket>& basket = this->arguments_.basket;
            /*
              Remove defaulted names and adjust the subordination.
            */
            std::vector<std::string> names = basket->remainingNames(start, today);
            std::vector<Real> notionals
                = basket->remainingNotionals(start, today);
            Real a = basket->remainingAttachmentRatio(start, today);
            Real d = basket->remainingDetachmentRatio(start, today);
            const boost::shared_ptr<Pool> pool = basket->pool();
            remainingBasket_ =
                boost::shared_ptr<Basket>(new Basket(names, notionals, pool,
                                                    basket->remainingDefaultKeys(start, today),
                                                    basket->remainingRecModels(start, today),
                                                     a, d));

            this->results_.xMin = remainingBasket_->attachmentAmount();
            this->results_.xMax = remainingBasket_->detachmentAmount();
            this->results_.remainingNotional = results_.xMax - results_.xMin;

            const std::vector<Date>& dates = arguments_.schedule.dates();
            for (Size i = 0; i < dates.size(); i++) {
                if (dates[i] <= today)
                    results_.expectedTrancheLoss.push_back(0.0);
                else {
                    Real L = expectedTrancheLoss(dates[i]);
                    results_.expectedTrancheLoss.push_back(L);
                }
            }
        }
        mutable boost::shared_ptr<Basket> remainingBasket_;
    };

    //--------------------------------------------------------------------------
    //! CDO base engine taking schedule steps
    class MidPointCDOEngine : public SyntheticCDO::engine {
      public:
        MidPointCDOEngine() {}
        void calculate() const;
      private:
        virtual Real expectedTrancheLoss(const Date&) const = 0;
    };

    //--------------------------------------------------------------------------
    //! CDO base engine taking (possibly) small time steps
    class IntegralCDOEngine : public SyntheticCDO::engine {
    public:
        void calculate() const;
        IntegralCDOEngine(Period stepSize = 3*Months) : stepSize_(stepSize) {}
    private:
        virtual Real expectedTrancheLoss(const Date&) const = 0;
    protected:
        Period stepSize_;
    };

    //--------------------------------------------------------------------------
    //! CDO engine, Monte Carlo for the exptected tranche loss distribution
    class MonteCarloCDOEngine1 : public MidPointCDOEngine {
    public:
        MonteCarloCDOEngine1 (boost::shared_ptr<RandomDefaultModel> rdm,
                              Size samples)
            : rdm_(rdm), samples_(samples) {}
    private:
        void defaultScenarios() const;

        void initialize() const {
            Date today = Settings::instance().evaluationDate();
            Date start = this->arguments_.schedule.startDate();
            boost::shared_ptr<Basket>& basket = this->arguments_.basket;
            std::vector<std::string> names = basket->remainingNames(start, today);
            std::vector<Real> notionals
                = basket->remainingNotionals(start, today);
            Real a = basket->remainingAttachmentRatio(start, today);
            Real d = basket->remainingDetachmentRatio(start, today);
            const boost::shared_ptr<Pool> pool = basket->pool();
            remainingBasket_ =
                boost::shared_ptr<Basket>(new Basket(names, notionals,
                                                     pool,
                                                     basket->remainingDefaultKeys(start, today),
                                                     basket->remainingRecModels(start, today),
                                                     a, d));

            this->results_.xMin = remainingBasket_->attachmentAmount();
            this->results_.xMax = remainingBasket_->detachmentAmount();

            this->results_.remainingNotional = results_.xMax - results_.xMin;

            // This sets the vector results_.expectedTrancheLoss for each
            // schedule date by Monte Carlo simulation
            defaultScenarios();
        }

        Real expectedTrancheLoss(const Date& d) const {
            const std::vector<Date>& dates = arguments_.schedule.dates();
            Real L = 0.0;
            for (Size i = 0; i < dates.size(); i++)
                if (d >= dates[i])
                    L = results_.expectedTrancheLoss[i];
                else
                    break;
            return L;
        }

        boost::shared_ptr<RandomDefaultModel> rdm_;
        Size samples_;
    };

    //--------------------------------------------------------------------------
    //! CDO engine, Monte Carlo for the sample payoff
    class MonteCarloCDOEngine2 : public SyntheticCDO::engine {
      public:
        MonteCarloCDOEngine2 (boost::shared_ptr<RandomDefaultModel> rdm,
                              Size samples)
            : rdm_(rdm), samples_(samples) {}
        void calculate() const;
      private:
        // not needed here
        Real expectedTrancheLoss(const Date&) const { return 0.0; }

        boost::shared_ptr<RandomDefaultModel> rdm_;
        Size samples_;
    };

    //--------------------------------------------------------------------------
    //! CDO engine, loss distribution convolution for finite homogeneous pool
    template <class CDOEngine>
    class HomogeneousPoolCDOEngine : public CDOEngine {
    public:
        HomogeneousPoolCDOEngine(const Handle<OneFactorCopula> copula,
                                 Size nBuckets)
            : copula_(copula), nBuckets_(nBuckets) {
            this->registerWith(copula_);
        }
    private:
        Real expectedTrancheLoss(const Date& d) const {
            LossDistHomogeneous op(nBuckets_, this->results_.xMax);
            std::vector<Real> lgd = this->remainingBasket_->LGDs();
            std::vector<Real> prob = this->remainingBasket_->probabilities(d);
            Distribution dist = copula_->integral (op, lgd, prob);
            return dist.cumulativeExcessProbability (this->results_.xMin,
                                                     this->results_.xMax);
        };
    protected:
        const Handle<OneFactorCopula> copula_;
        Size nBuckets_;
    };

    //--------------------------------------------------------------------------
    //! CDO engine, loss disctribution bucketing for finite inhomogeneous pool
    template <class CDOEngine>
    class InhomogeneousPoolCDOEngine : public CDOEngine {
    public:
        InhomogeneousPoolCDOEngine(const Handle<OneFactorCopula> copula,
                                   Size nBuckets)
            : copula_(copula), nBuckets_(nBuckets) {
            this->registerWith(copula_);
        }
    private:
        Real expectedTrancheLoss(const Date& d) const{
            LossDistBucketing op (nBuckets_, this->results_.xMax);
            std::vector<Real> lgd = this->remainingBasket_->LGDs();
            std::vector<Real> prob = this->remainingBasket_->probabilities(d);
            Distribution dist = copula_->integral (op, lgd, prob);
            return dist.cumulativeExcessProbability (this->results_.xMin,
                                                     this->results_.xMax);
    };
    protected:
        const Handle<OneFactorCopula> copula_;
        Size nBuckets_;
    };

    //--------------------------------------------------------------------------
    /*!
      CDO engine with analytical expected tranche loss for a large homogeneous
      pool with Gaussian one-factor copula. See for example
      "The Normal Inverse Gaussian Distribution for Synthetic CDO pricing.",
      Anna Kalemanova, Bernd Schmid, Ralf Werner,
      Journal of Derivatives, Vol. 14, No. 3, (Spring 2007), pp. 80-93.
      http://www.defaultrisk.com/pp_crdrv_91.htm
     */
    template <class CDOEngine>
    class GaussianLHPCDOEngine : public CDOEngine {
    public:
        GaussianLHPCDOEngine(const Handle<OneFactorCopula>& copula)
            : copula_(copula) {
            this->registerWith(copula_);
        }
    private:
        Real expectedTrancheLoss(const Date& d) const {
            Date today = Settings::instance().evaluationDate();
            Date start = this->arguments_.schedule.startDate();
            boost::shared_ptr<Basket>& basket = this->arguments_.basket;
            std::vector<std::string> names =
                basket->remainingNames(start, today);
            const boost::shared_ptr<Pool> pool = basket->pool();

            Issuer name = pool->get(names[0]);
            Real prob = name.defaultProbability(basket->defaultKeys()[0])->defaultProbability(d);
            //what if the name has defualted? The homogeneity in the model concerns the default state too...
            Real rec = basket->recoveryModels()[0]->recoveryValue(d,
                basket->defaultKeys()[0]);
            Real ntl = this->results_.remainingNotional;
            Real attach = basket->remainingAttachmentRatio(start, today);
            Real detach = basket->remainingDetachmentRatio(start, today);
            Real one = 1.0 - 1.0e-12;  // FIXME
            Real k1 = std::min(one, attach / (1.0 - rec));
            Real k2 = std::min(one, detach / (1.0 - rec));

            Real y = sqrt(1.0 - copula_->correlation());
            BivariateCumulativeNormalDistribution biphi(-y);
            InverseCumulativeNormal inverse;
            CumulativeNormalDistribution phi;

            if (ntl == 0.) return 0.;

            if (prob > 0) {
                Real ip = inverse(prob);
                if (k1 > 0)
                    return ntl
                        * (biphi(-inverse(k1), ip) - biphi(-inverse(k2), ip))
                        / (k2 - k1);
                else return ntl * (prob - biphi(-inverse(k2), ip)) / (k2 - k1);
            }
            else return 0.0;
        };
        const Handle<OneFactorCopula> copula_;
    };

    typedef HomogeneousPoolCDOEngine<MidPointCDOEngine> HPMidPointCDOEngine;
    typedef HomogeneousPoolCDOEngine<IntegralCDOEngine> HPIntegralCDOEngine;

    typedef InhomogeneousPoolCDOEngine<MidPointCDOEngine> IHPMidPointCDOEngine;
    typedef InhomogeneousPoolCDOEngine<IntegralCDOEngine> IHPIntegralCDOEngine;

    typedef GaussianLHPCDOEngine<MidPointCDOEngine> GLHPMidPointCDOEngine;
    typedef GaussianLHPCDOEngine<IntegralCDOEngine> GLHPIntegralCDOEngine;

}

#endif
