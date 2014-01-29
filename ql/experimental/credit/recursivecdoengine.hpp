/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#ifndef recursive_cdo_engine_hpp
#define recursive_cdo_engine_hpp

#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/matrixutilities/factorreduction.hpp>
#include <ql/experimental/credit/syntheticcdoengines.hpp>
#include <ql/experimental/credit/onefactorgaussiancopula.hpp>
#include <ql/experimental/credit/onefactorstudentcopula.hpp>
#include <boost/bind.hpp>
#include <map>
#include <algorithm>

namespace QuantLib {

    /*! Recursive STCDO pricing for a heterogeneous pool of names. The pool
        names are heterogeneous in their default probabilities, notionals
        and recovery rates. Correlations are pairwise. The recursive pricing
        algorithm used here is described in Andersen, Sidenius and Basu;
        "All your hedges in one basket", Risk, November 2003, pages 67-72

        Notice that using copulas other than Gaussian it is only an
        approximation (see remark on p.68).
    */
    template <class CDOEngine, class copulaT>
    class RecursiveCdoEngine : public CDOEngine {
      public:
        // Base constructors call default Handle constructor, the copula is to
        // be relinked by template partial specializations on the copula type

        //! Single correlation construction
        RecursiveCdoEngine(const Handle<Quote>& correl,
                           Size nbuckets  = 1,
                           Size quadOrder = 20)
        : correlQuote_(correl), copula_(), nBuckets_(nbuckets),
          integral_(quadOrder), wk_()
        {
            this->registerWith(correl);
        }

        //! Correlation name to name single factor construction
        RecursiveCdoEngine(const Handle<Quote>& correl,
                           const Matrix& correlMtrx,
                           Size nbuckets  = 1,
                           Size quadOrder = 20)
        : correlQuote_(correl), copula_(), nBuckets_(nbuckets),
          integral_(quadOrder), wk_(),
          oneFactorCorrels_(factorReduction(correlMtrx))
        {
            // at least
            QL_REQUIRE(!oneFactorCorrels_.empty(),
                "Invalid correlation parameter matrix.");
        }
      protected:
        void initialize() const;
      private:
        //! Weights the conditional portfolio loss by the mkt factor
        //    distribtion
        Real integratorLoss(const Date& date, Real mktFactor) const {
            return expectedConditionalLoss(date, mktFactor) *
               copula_->density(mktFactor);
        }
        //! Portfolio loss conditional to the market factor value
        Real expectedConditionalLoss(const Date& date,
                                     Real mktFactor) const;
      public:
        void update();

        /*  Expected tranche Loss calculation.
            This is computed from the first equation on page 70 (not numbered)
            Notice that while we want to compute:
            \f[
            EL(t) = \sum_{l_k}l_k P(l;t) =
              \sum_{l_k}l_k \int P(l_k;t|\omega) d\omega q(\omega)
            \f]
            One can invert the sumation and the integral order to:
            \f[
            EL(t) = \int\,q(\omega)\,d\omega\,\sum_{l_k}\,l_k\,P(l_k;t|\omega) =
              \int\,q(\omega)\,d\omega\,EL(t|\omega)
            \f]
            and this is the way it is integrated here. The recursion formula makes
            it easier this way.
        */
        Real expectedTrancheLoss(const Date& date) const {
            return
                integral_(boost::bind(
                    &RecursiveCdoEngine<CDOEngine, copulaT>::integratorLoss,
                    this,
                    date,
                    _1)
                );
        }
      protected:
        const Handle<Quote> correlQuote_;
        mutable RelinkableHandle<copulaT> copula_;
      private:
        // loss model descriptor members
        Size nBuckets_;
       const GaussHermiteIntegration integral_;
        mutable std::vector<Real> wk_;
        mutable Real loss_unit_;
        //! name to name factor loadings (betas). In the single factor copula:
        //    correl = beta * beta
        // When constructing through a single correlation number the factor is
        //   taken to be the positive swuare root of this number in the copula.
        mutable std::vector<Real> oneFactorCorrels_;
    };


    template <class CDOEngine, class copulaT>
    void RecursiveCdoEngine<CDOEngine, copulaT>::update() {
        oneFactorCorrels_.clear();
        CDOEngine::update();
    }


    template <class CDOEngine, class copulaT>
    void RecursiveCdoEngine<CDOEngine, copulaT>::initialize() const {
        wk_.clear();
        Date today = Settings::instance().evaluationDate();
        Date start = this->arguments_.schedule.startDate();
        boost::shared_ptr<Basket>& basket = this->arguments_.basket;
        /*
          Remove defaulted names and adjust the subordination.
        */
        std::vector<std::string> names =
            basket->remainingNames(start, today);
        std::vector<Real> notionals
            = basket->remainingNotionals(start, today);
        Real a = basket->remainingAttachmentRatio(start, today);
        Real d = basket->remainingDetachmentRatio(start, today);
        const boost::shared_ptr<Pool> pool = basket->pool();
        this->remainingBasket_ =
            boost::shared_ptr<Basket>(new Basket(names, notionals, pool,
                                                basket->remainingDefaultKeys(start, today),
                                                basket->remainingRecModels(start, today),
                                                 a, d));

        this->results_.xMin = this->remainingBasket_->attachmentAmount();
        this->results_.xMax = this->remainingBasket_->detachmentAmount();
        this->results_.remainingNotional =
            this->results_.xMax - this->results_.xMin;
        //----------------------------------------------------------------
        if(oneFactorCorrels_.empty())
            oneFactorCorrels_ = std::vector<Real>(names.size(),
                                    correlQuote_->value());
        // check size of factors:
        QL_REQUIRE(oneFactorCorrels_.size() == names.size(),
            "Size of matrix must match number of names in the basket.");
        //
        std::vector<Real> lgdsTmp, lgds = this->remainingBasket_->LGDs();
        lgdsTmp = lgds;
        lgds.erase(std::remove(lgds.begin(), lgds.end(), 0.), lgds.end());
        loss_unit_ = *(std::min_element(lgds.begin(), lgds.end()))
            / nBuckets_;

        for(Size i = 0; i<names.size(); i++)
            wk_.push_back(std::floor(lgdsTmp[i]/loss_unit_ + .5));

        // Could not check parameters at construction time because we
        //   had no arguments yet, do it now:
        if(oneFactorCorrels_.size() == 1)
            oneFactorCorrels_ =
                std::vector<Real>(pool->size(), oneFactorCorrels_[0]);
        else
            QL_REQUIRE(oneFactorCorrels_.size() == this->remainingBasket_->size(),
                "Incompatible correl matrix, pool size.");
        //----------------------------------------------------------------
        const std::vector<Date>& dates = this->arguments_.schedule.dates();
        for (Size i = 0; i < dates.size(); i++) {
            if (dates[i] <= today)
                this->results_.expectedTrancheLoss.push_back(0.0);
            else {
                Real L = expectedTrancheLoss(dates[i]);
                this->results_.expectedTrancheLoss.push_back(L);
            }
        }

    }


    //! Portfolio loss conditional to the market factor value
    template <class CDOEngine, class copulaT>
    Real RecursiveCdoEngine<CDOEngine, copulaT>::expectedConditionalLoss(
                                 const Date& date,
                                 Real mktFactor) const {
        const std::vector<std::string>& names = this->remainingBasket_->names();

        // eq. 10 p.68
        // attainable losses distribution, recursive algorithm
        std::vector<Probability> uncDefProb =
            this->remainingBasket_->probabilities(date);;
        std::map<Real, Probability> pIndepDistrib;
        // K=0
        pIndepDistrib.insert(std::make_pair(0., 1.));
        for(Size iName=0; iName<names.size(); iName++) {

            // to do: allow for matrix constructor and uncoment this
            // correlQuote_->setValue(oneFactorCorrels_[iName]);

            Probability pDef =
                copula_->conditionalProbability(uncDefProb[iName],
                                                mktFactor);
            // iterate on all possible losses in the distribution:
            std::map<Real, Probability> pDistTemp;
            std::map<Real, Probability>::iterator distIt =
                pIndepDistrib.begin();
            while(distIt != pIndepDistrib.end()) {
                // update prob if this name does not default
                std::map<Real, Probability>::iterator matchIt
                    = pDistTemp.find(distIt->first);
                if(matchIt != pDistTemp.end()) {
                    matchIt->second += distIt->second * (1.-pDef);
                }else{
                    pDistTemp.insert(std::make_pair(distIt->first,
                        distIt->second * (1.-pDef)));
                }
                // and if it does
                matchIt = pDistTemp.find(distIt->first + wk_[iName]);
                if(matchIt != pDistTemp.end()) {
                    matchIt->second += distIt->second * pDef;
                }else{
                    pDistTemp.insert(std::make_pair(
                        distIt->first+wk_[iName], distIt->second * pDef));
                }
                distIt++;
            }
            // copy back
            pIndepDistrib = pDistTemp;
        }

        // get the expected value subject to the value of the market
        //   factor.
        Real expLoss = 0.;
        //---------------------------------------------------------------
        /* This is the original (easy to read) loop which I have partially
             unroll below to take profit of the fact that once we go over
             the tranche top the loss amount is fixed:

        std::map<Real, Probability>::iterator distIt =
            pIndepDistrib.begin();
        while(distIt != pIndepDistrib.end()) {
            Real loss = distIt->first * loss_unit_
                                ;
            loss = std::max(std::min(loss,
                results_.xMax)-results_.xMin, 0.);
            expLoss += loss * distIt->second;
            distIt++;
        }
        return expLoss ;
        */
        //---------------------------------------------------------------
        Real relativeMax = this->results_.xMax / loss_unit_;
        Real relativeMin = this->results_.xMin / loss_unit_;
        Size relativeMinIdx = std::floor(relativeMin);
        std::map<Real, Probability>::iterator
            distIt = pIndepDistrib.lower_bound(relativeMinIdx),
            itTop  = pIndepDistrib.lower_bound(relativeMax);
        for(; distIt != itTop; distIt++)
            expLoss += std::max(std::min(distIt->first, relativeMax)
                                -relativeMin, 0.) * distIt->second;
        Real sumProbs = 0.;
        for(;distIt != pIndepDistrib.end(); distIt++)
            sumProbs += distIt->second;
        return expLoss * loss_unit_ + this->results_.remainingNotional * sumProbs;
    }



    // Partial specializations on the copula type. Kind
    //   of template virtual constructions. Allows to own the correlation
    //   quote and the copula for each specific copula type. It only
    //   needs to know its particular copula constructor.
    // These and the base correlation pricers use a unifactorial copula
    //   which needs to be modified for different parameters of the
    //   correlation per name to name if theres such an structure or
    //   because the correlation parameter has got a time or/and loss
    //   level surface.
    //
    // TO do: Correlation matrix constructors.

    //! Specialization for Gaussian copula, the integration still remains
    //    to be defined by the user out of the available ones in
    //    syntheticcdoengines.
    template <class CDOEngine>
    class GaussianRecursiveCdoEngine : public
        RecursiveCdoEngine<CDOEngine, OneFactorGaussianCopula> {
      public:
        //! quote constructor.
        GaussianRecursiveCdoEngine(
            const Handle<Quote>& correlQuote,
            Size nbuckets  = 1,
            Size quadOrder = 12,
            Real maxval    = 5.,
            Size steps     = 50)
            :
            RecursiveCdoEngine<CDOEngine, OneFactorGaussianCopula>(correlQuote,
                    nbuckets, quadOrder) {
                this->copula_.linkTo(boost::shared_ptr<OneFactorGaussianCopula>(
                new OneFactorGaussianCopula(correlQuote,  maxval, steps)), true);
        }
    };

    template <class CDOEngine>
    class StudentRecursiveCdoEngine : public
        RecursiveCdoEngine<CDOEngine, OneFactorStudentCopula> {
      public:
        //! quote constructor.
        StudentRecursiveCdoEngine(
            const Handle<Quote>& correlQuote,
            Size nz,
            Size nm,
            Size nbuckets  = 1,
            Size quadOrder = 12,
            Real maxval    = 5.,
            Size steps     = 50)
            :
            RecursiveCdoEngine<CDOEngine, OneFactorStudentCopula>(correlQuote,
                    nbuckets, quadOrder) {
                this->copula_.linkTo(boost::shared_ptr<OneFactorStudentCopula>(
                new OneFactorStudentCopula(correlQuote, nz, nm, maxval, steps)), true);
        }
    };



    typedef GaussianRecursiveCdoEngine<MidPointCDOEngine> GaussRecCDOEngine;
    typedef StudentRecursiveCdoEngine<MidPointCDOEngine>  StudentRecCDOEngine;

}

#endif
