/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009, 2014 Jose Aparicio

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_recursive_loss_model_hpp
#define quantlib_recursive_loss_model_hpp

#include <ql/experimental/credit/constantlosslatentmodel.hpp>
#include <ql/experimental/credit/defaultlossmodel.hpp>
#include <map>
#include <algorithm>

namespace QuantLib {

    /*! Recursive STCDO default loss model for a heterogeneous pool of names. 
    The pool names are heterogeneous in their default probabilities, notionals
    and recovery rates. Correlations are given by the latent model.
    The recursive pricing algorithm used here is described in Andersen, Sidenius
    and Basu; "All your hedges in one basket", Risk, November 2003, pages 67-72

        Notice that using copulas other than Gaussian it is only an
        approximation (see remark on p.68).

        \todo Make the loss unit equal to some small fraction depending on the
        portfolio loss weights (notionals and recoveries). As it is now this
        is ok for pricing but not for risk metrics. See the discussion in O'Kane
        18.3.2
        \todo Intengrands should all use the inverted probabilities for 
        performance instead of calling the copula inversion with the same vals.
    */
    template<class copulaPolicy> 
    class RecursiveLossModel : public DefaultLossModel {
    public:
      explicit RecursiveLossModel(
          const ext::shared_ptr<ConstantLossLatentmodel<copulaPolicy> >& m,
          // nope! use max common divisor. See O'Kane. Or give both options at least.
          Size nbuckets = 1)
      : copula_(m), nBuckets_(nbuckets) {}

    private:
      /*!
      @param pDefDate Vector of unconditional default probabilities for each
      live name (at the current evaluation date). This is passed instead of
      the date for performance reasons (if in the future other magnitudes
      -e.g. lgd- are contingent on the date they shouldd be passed too).
      */
      std::map<Real, Probability> conditionalLossDistrib(const std::vector<Probability>& pDefDate,
                                                         const std::vector<Real>& mktFactor) const;
      Real expectedConditionalLoss(const std::vector<Probability>& pDefDate, //<< never used!!
                                   const std::vector<Real>& mktFactor) const;
      std::vector<Real> conditionalLossProb(const std::vector<Probability>& pDefDate,
                                            // const Date& date,
                                            const std::vector<Real>& mktFactor) const;
      // versions using the P-inverse, deprecate the former
      std::map<Real, Probability> conditionalLossDistribInvP(const std::vector<Real>& pDefDate,
                                                             // const Date& date,
                                                             const std::vector<Real>& mktFactor) const;
      Real expectedConditionalLossInvP(const std::vector<Real>& pDefDate,
                                       // const Date& date,
                                       const std::vector<Real>& mktFactor) const;
    protected:
      void resetModel() override;

    public:
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
            and this is the way it is integrated here. The recursion formula 
            makes it easier this way.
        */
      Real expectedTrancheLoss(const Date& date) const override;
      std::vector<Real> lossProbability(const Date& date) const;
      // REMEBER THIS HAS TO BE MOVED TO A DISTRIBUTION OBJECT.............
      std::map<Real, Probability> lossDistribution(const Date& d) const override;
      // INTEGRATE THEN SEARCH RATHER THAN SEARCH AND THEN INTEGRATE:
      // Here I am not using a search because the point might not be attainable
      //  (loss distrib is not continuous)
      Real percentile(const Date& d, Real percentile) const override;
      Real expectedShortfall(const Date& d, Real perctl) const override;

    protected:
        const ext::shared_ptr<ConstantLossLatentmodel<copulaPolicy> > copula_;
    private:
        // loss model descriptor members
        const Size nBuckets_;
        mutable std::vector<Real> wk_;
        mutable Real lossUnit_;
        //! name to name factor. In the single factor copula:
        //    correl = beta * beta
        // When constructing through a single correlation number the factor is
        //   taken to be the positive swuare root of this number in the copula.
        ////////in the latent model now: mutable std::vector<Real> oneFactorCorrels_;
        // cached remaining basket magnitudes:
        mutable Real attachAmount_, 
            detachAmount_,
            notional_;
        mutable Size remainingBsktSize_;
        mutable std::vector<Real> notionals_;
    };


    typedef RecursiveLossModel<GaussianCopulaPolicy> RecursiveGaussLossModel;

    // Inlines ------------------------------------------------

    template<class CP>
    inline Real RecursiveLossModel<CP>::expectedTrancheLoss(
        const Date& date) const 
    {
/*
        std::map<Real, Probability> dist = lossDistribution(date);

        Real expLoss = 0.;
        std::map<Real, Probability>::iterator distIt = dist.begin();

        while(distIt != dist.end()) {
            Real loss = distIt->first * lossUnit_;
            loss = std::max(std::min(loss, detachAmount_)-attachAmount_, 0.);
            // MIN MAX BUGS ....??
            expLoss += loss * distIt->second;
            distIt++;
        }
        return expLoss ;




    ///////////////////////////////////////////////////////////////////////

        // calculate inverted unconditional Ps first so we save the inversion:
        // TO DO : turn to STL algorithm code
        std::vector<Probability> uncDefProb = 
            basket_->remainingProbabilities(date);

        return copula_->integratedExpectedValue(
            [&](const std::vector<Real>& v1) {
                return expectedConditionalLoss(uncDefProb, v1);
            });
            */

        std::vector<Probability> uncDefProb = 
            basket_->remainingProbabilities(date);
        std::vector<Real> invProb;
        for(Size i=0; i<uncDefProb.size(); ++i)
           invProb.push_back(copula_->inverseCumulativeY(uncDefProb[i], i));
           ///  invProb.push_back(CP::inverseCumulativeY(uncDefProb[i], i));//<-static call
        return copula_->integratedExpectedValue(
            [&](const std::vector<Real>& v1) {
                return expectedConditionalLossInvP(invProb, v1);
            });
    }

    template<class CP>
    inline std::vector<Real> RecursiveLossModel<CP>::lossProbability(const Date& date) const {

        std::vector<Probability> uncDefProb = 
            basket_->remainingProbabilities(date);
        return copula_->integratedExpectedValueV(
            [&](const std::vector<Real>& v1) {
                return conditionalLossProb(uncDefProb, v1);
            });
    }

    // -------------------------------------------------------------------

    template<class CP>
    void RecursiveLossModel<CP>::resetModel() {
        // basket update:
        notionals_ = basket_->remainingNotionals();
        notional_  = basket_->remainingNotional();
        attachAmount_ = basket_->remainingAttachmentAmount();
        detachAmount_ = basket_->remainingDetachmentAmount();
        // model parameters:
        remainingBsktSize_ = notionals_.size();

        copula_->resetBasket(basket_.currentLink());

        std::vector<Real> lgdsTmp, lgds;
        for(Size i=0; i<remainingBsktSize_; ++i)
            lgds.push_back(notionals_[i]*(1.-copula_->recoveries()[i]));
        lgdsTmp = lgds;
        ///////////////std::remove(lgds.begin(), lgds.end(), 0.);
        lgds.erase(std::remove(lgds.begin(), lgds.end(), 0.), lgds.end());
        lossUnit_ = *(std::min_element(lgds.begin(), lgds.end()))
            / nBuckets_;
        for(Size i=0; i<remainingBsktSize_; ++i)
            wk_.push_back(std::floor(lgdsTmp[i]/lossUnit_ + .5));
    }

    // make it return a distribution object?
    template<class CP>
    std::map<Real, Probability> RecursiveLossModel<CP>::lossDistribution(const Date& d) const 
    {
        std::map<Real, Probability> distrib;
        std::vector<Real> values  = lossProbability(d);
        Real sum = 0.;
        for(Size i=0; i<values.size(); ++i) {
            distrib.insert(std::make_pair<Real, Probability>(i * lossUnit_, 
                sum + values[i]));
            sum += values[i];
        }
        return distrib;
    }

    // Integrate then search rather than search and then integrate?
    // Here I am not using a search because the point might be not attainable 
    //   (loss distrib is not continuous) 
    template<class CP>
    Real RecursiveLossModel<CP>::percentile(const Date& d, 
        Real percentile) const 
    {
        std::map<Real, Probability> dist = lossDistribution(d);

        if(dist.begin()->second >=1.) return dist.begin()->first;

        // deterministic case (e.g. date requested is todays date)
        if(dist.size() == 1) return dist.begin()->first;

        if(percentile == 1.) return dist.rbegin()->second;
        if(percentile == 0.) return dist.begin()->second;
        std::map<Real, Probability>::const_iterator itdist = dist.begin();
        while (itdist->second <= percentile) ++itdist;
        Real valPlus = itdist->second;
        Real xPlus   = itdist->first;
        --itdist;  //we're never 1st or last, because of tests above
        Real valMin  = itdist->second;
        Real xMin    = itdist->first;

        // return xPlus-(xPlus-xMin)*(valPlus-percentile)/(valPlus-valMin);
        Real portfLoss =  xPlus-(xPlus-xMin)*(valPlus-percentile)
            /(valPlus-valMin);
        return //remainingNotional_ * 
            std::min(std::max(portfLoss - attachAmount_, 0.), 
                detachAmount_ - attachAmount_);/////(detach_ - attach_);
    }

    template<class CP>
    Real RecursiveLossModel<CP>::expectedShortfall(const Date& d, 
        Real perctl) const 
    {
        if(d == Settings::instance().evaluationDate()) return 0.;
        std::map<Real, Probability> distrib = lossDistribution(d);

        std::map<Real, Probability>::iterator itNxt, itDist = 
            distrib.begin();
        for(; itDist != distrib.end(); ++itDist)
            if(itDist->second >= perctl) break;
        itNxt = itDist;
        --itDist; // what if we are on the first one?!!!

        // One could linearly triangulate the exact point and get extra 
        // precission on the first(broken) period.
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
                suma += .5 * (lossHere + lossNxt) * (itNxt->second - 
                    itDist->second);
                ++itDist; ++itNxt;
            }while(itNxt != distrib.end());
            return suma / (1.-perctl);
        }
        return 0.;// well, we are in error....  fix: FAIL
    }

    template<class CP>
    std::map<Real, Probability> RecursiveLossModel<CP>::conditionalLossDistrib(
            const std::vector<Probability>& pDefDate, 
            //const Date& date,
            const std::vector<Real>& mktFactor) const 
    {
        //eq. 10 p.68
        //attainable losses distribution, recursive algorithm
        const std::vector<Probability>& uncDefProb = pDefDate;// alias, remove

        std::map<Real, Probability> pIndepDistrib;
        ////////  K=0
        pIndepDistrib.insert(std::make_pair(0., 1.));
        for(Size iName=0; iName<remainingBsktSize_; ++iName) {
            Probability pDef =
                copula_->conditionalDefaultProbability(uncDefProb[iName], iName,
                                                mktFactor);
            ////// iterate on all possible losses in the distribution:
            std::map<Real, Probability> pDistTemp;
            auto distIt = pIndepDistrib.begin();
            while(distIt != pIndepDistrib.end()) {
              ///   update prob if this name does not default
              auto matchIt = pDistTemp.find(distIt->first);
              if (matchIt != pDistTemp.end()) {
                  matchIt->second += distIt->second * (1. - pDef);
                }else{
                    pDistTemp.insert(std::make_pair(distIt->first,
                        distIt->second * (1.-pDef)));
                }
              ////   and if it does
                matchIt = pDistTemp.find(distIt->first + wk_[iName]);
                if(matchIt != pDistTemp.end()) {
                    matchIt->second += distIt->second * pDef;
                }else{
                    pDistTemp.insert(std::make_pair(
                        distIt->first+wk_[iName], distIt->second * pDef));
                }
                ++distIt;
            }
           /////  copy back
            pIndepDistrib = pDistTemp;
        }
        /* Apply tranche limits now .... mind you this could be done outside*/
        ////  to be done....
        return pIndepDistrib;
    }

    // twice?! rewrite one in terms of the other, this is a duplicate!
    template<class CP>
    std::map<Real, Probability> RecursiveLossModel<CP>::conditionalLossDistribInvP(
            const std::vector<Real>& invpDefDate, 
            //const Date& date,
            const std::vector<Real>& mktFactor) const 
    {
        // eq. 10 p.68
        // attainable losses distribution, recursive algorithm

        std::map<Real, Probability> pIndepDistrib;
        // K=0
        pIndepDistrib.insert(std::make_pair(0., 1.));
        for(Size iName=0; iName<remainingBsktSize_; ++iName) {
            Probability pDef =
                copula_->conditionalDefaultProbabilityInvP(invpDefDate[iName], 
                    iName, mktFactor);

            // iterate on all possible losses in the distribution:
            std::map<Real, Probability> pDistTemp;
            auto distIt = pIndepDistrib.begin();
            while(distIt != pIndepDistrib.end()) {
                // update prob if this name does not default
                auto matchIt = pDistTemp.find(distIt->first);
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
                ++distIt;
            }
            // copy back
            pIndepDistrib = pDistTemp;
        }
        /* Apply tranche limits now .... mind you this could be done outside*/
        return pIndepDistrib;
    }




    /*
    Bugs here???. The max min on the tranche looks 
    wrong. It is better to have a tranche function since that way we can avoid 
    adding up losses over all the posible losses rather than just over the 
    tranche limits.
    */
    //! Portfolio loss conditional to the market factor value
    template<class CP>
    Real RecursiveLossModel<CP>::expectedConditionalLoss(
        const std::vector<Probability>& pDefDate, 
        //const Date& date,
        const std::vector<Real>& mktFactor) const 
    {
        std::map<Real, Probability> pIndepDistrib =
            conditionalLossDistrib(pDefDate, mktFactor);

        // get the expected value subject to the value of the market
        //   factor.
        Real expLoss = 0.;
        //---------------------------------------------------------------
        /* This is the original (easy to read) loop which I have partially
             unroll below to take profit of the fact that once we go over
             the tranche top the loss amount is fixed:
        */
        auto distIt = pIndepDistrib.begin();

        while(distIt != pIndepDistrib.end()) {
            Real loss = distIt->first * lossUnit_;
     //       loss = std::max(std::min(loss, detachAmount_)-attachAmount_, 0.);
            loss = std::min(std::max(loss - attachAmount_, 0.), 
                detachAmount_ - attachAmount_);
            // MIN MAX BUGS ....??
            expLoss += loss * distIt->second;
            ++distIt;
        }
        return expLoss ;
    }

    template<class CP>
    // again, I am duplicating code.
    Real RecursiveLossModel<CP>::expectedConditionalLossInvP(
                                 const std::vector<Real>& invPDefDate, 
                                 //const Date& date,
                                 const std::vector<Real>& mktFactor) const 
    {
        std::map<Real, Probability> pIndepDistrib =
            conditionalLossDistribInvP(invPDefDate, mktFactor);

        // get the expected value subject to the value of the market
        //   factor.
        Real expLoss = 0.;
        //---------------------------------------------------------------
        /* This is the original (easy to read) loop which I have partially
             unroll below to take profit of the fact that once we go over
             the tranche top the loss amount is fixed:
        */
        auto distIt = pIndepDistrib.begin();

        while(distIt != pIndepDistrib.end()) {
            Real loss = distIt->first * lossUnit_;
   //         loss = std::max(std::min(loss, detachAmount_)-attachAmount_, 0.);
            loss = std::min(std::max(loss - attachAmount_, 0.), 
                detachAmount_ - attachAmount_);
            // MIN MAX BUGS ....???
            expLoss += loss * distIt->second;
            ++distIt;
        }
        return expLoss ;
    }

    template<class CP>
    std::vector<Real> RecursiveLossModel<CP>::conditionalLossProb(
        const std::vector<Probability>& pDefDate, 
        //const Date& date,
        const std::vector<Real>& mktFactor) const 
    {
        std::map<Real, Probability> pIndepDistrib =
            conditionalLossDistrib(pDefDate, mktFactor);

        std::vector<Real> results;
        auto distIt = pIndepDistrib.begin();
        while(distIt != pIndepDistrib.end()) {
            //Real loss = distIt->first * loss_unit_
            //                    ;
            //loss = std::max(std::min(loss,
            //    results_.xMax)-results_.xMin, 0.);
            //expLoss += loss * distIt->second;

            results.push_back(distIt->second);
             ++distIt;
        }
        return results;
    }

}

#endif
