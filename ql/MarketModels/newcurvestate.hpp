/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Mark Joshi

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/


#ifndef quantlib_newcurvestate_hpp
#define quantlib_newcurvestate_hpp

#include <ql/Math/array.hpp>
#include <vector>

namespace QuantLib {

    /*! This class stores the state of the yield curve associated to the
        fixed calendar times within the simulation.
        This is the workhorse discounting object associated to the rate times
        of the simulation. It's important to pass the rates via an object like
        this to the product rather than directly to make it easier to switch
        to other engines such as a coterminal swap rate engine.
        Many products will not need expired rates and others will only require
        the first rate.
    */
    class NewCurveState {
    /* There will n+1 rate times expressing payment and reset times
        of forward rates.

                |-----|-----|-----|-----|-----|      (size = 6)
                t0    t1    t2    t3    t4    t5     rateTimes
                f0    f1    f2    f3    f4           forwardRates
                tau0  tau1  tau2  tau3  tau4         taus
                d0    d1    d2    d3    d4    d5     discountBonds
                d0/d0 d1/d0 d2/d0 d3/d0 d4/d0 d5/d0  discountRatios
                sr0   sr1   sr2   sr3   sr4          cotSwaps
    */
      public:
        NewCurveState(const std::vector<Time>& rateTimes)
        : rateTimes_(rateTimes.begin(), rateTimes.end()),
          taus_(rateTimes_.size()-1),
          nRates_(rateTimes_.size()-1) {
            for (Size i=0; i<nRates_; ++i)
                taus_[i] = rateTimes_[i+1] - rateTimes_[i];
        }

        //! \name Inspectors
        //@{
        Size numberOfRates() const { return nRates_; }

        const std::vector<Time>& rateTimes() const { return rateTimes_; }
        const std::vector<Time>& rateTaus() const { return taus_; }

        virtual Real discountRatio(Size i,
                                 Size j) const = 0;
        virtual Rate forwardRate(Size i) const = 0;
        virtual Rate coterminalSwapAnnuity(Size numeraire,
                                        Size i) const = 0;
        virtual Rate coterminalSwapRate(Size i) const = 0;
        virtual Rate cmSwapAnnuity(Size numeraire,
                                 Size i,
                                 Size spanningForwards) const = 0;
        virtual Rate cmSwapRate(Size i,
                              Size spanningForwards) const = 0;

        //@}
      protected:
        std::vector<Time> rateTimes_, taus_;
        Size nRates_;
    };


    void forwardsFromDiscountRatios(Size firstValidIndex,
                                  const std::vector<DiscountFactor>& ds,
                                  const std::vector<Time>& taus,
                                  std::vector<Rate>& fwds){
        QL_REQUIRE(taus.size()==fwds.size(),
                   "taus.size()!=fwds.size()");
        QL_REQUIRE(ds.size()==fwds.size()+1,
                   "ds.size()!=fwds.size()+1");

        for (Size i=firstValidIndex; i<fwds.size(); ++i)
            fwds[i] = (ds[i]-ds[i+1])/(ds[i]*taus[i]);
    };


    void coterminalFromDiscountRatios(Size firstValidIndex,
                                   const std::vector<DiscountFactor>& ds,
                                   const std::vector<Time>& taus,
                                   std::vector<Rate>& cotSwapRates,
                                   std::vector<Rate>& cotSwapAnnuities){
        Size nCotSwapRates = cotSwapRates.size();
        QL_REQUIRE(taus.size()==nCotSwapRates,
                   "taus.size()!=cotSwapRates.size()");
        QL_REQUIRE(cotSwapAnnuities.size()==nCotSwapRates,
                   "cotSwapAnnuities.size()!=cotSwapRates.size()");
        QL_REQUIRE(ds.size()==nCotSwapRates+1,
                   "ds.size()!=cotSwapRates.size()+1");
        
        cotSwapAnnuities[nCotSwapRates-1] = taus[nCotSwapRates-1]*ds[nCotSwapRates];
        cotSwapRates[nCotSwapRates-1] = (ds[nCotSwapRates-1]-ds[nCotSwapRates])/cotSwapAnnuities[nCotSwapRates-1];
        
        for (Size i=nCotSwapRates-1; i>firstValidIndex; --i) {
            cotSwapAnnuities[i-1] = cotSwapAnnuities[i] + taus[i-1] * ds[i];
            cotSwapRates[i-1] = (ds[i-1]-ds[nCotSwapRates])/cotSwapAnnuities[i-1];
        }
    };


    void constantMaturityFromDiscountRatios(// Size i, // to be added later
                                         Size spanningForwards,
                                         Size firstValidIndex,
                                         const std::vector<DiscountFactor>& ds,
                                         const std::vector<Time>& taus,
                                         std::vector<Rate>& constMatSwapRates,
                                         std::vector<Rate>& constMatSwapAnnuities){
        Size nConstMatSwapRates = constMatSwapRates.size();
        QL_REQUIRE(taus.size()==nConstMatSwapRates,
                   "taus.size()!=nConstMatSwapRates");
        QL_REQUIRE(constMatSwapAnnuities.size()==nConstMatSwapRates,
                   "constMatSwapAnnuities.size()!=nConstMatSwapRates");
        QL_REQUIRE(ds.size()==nConstMatSwapRates+1,
                   "ds.size()!=nConstMatSwapRates+1");
        // compute the first cmsrate and cmsannuity     
        constMatSwapAnnuities[firstValidIndex]=0.;
        Size lastIndex = std::min(firstValidIndex+spanningForwards,nConstMatSwapRates);
        for (Size i=firstValidIndex; i<lastIndex; ++i) {
            constMatSwapAnnuities[firstValidIndex]+= taus[i] * ds[i+1];
        }  
        constMatSwapRates[firstValidIndex] = 
            (ds[firstValidIndex]-ds[lastIndex])/
                constMatSwapAnnuities[firstValidIndex];

        // compute all the other cmas rates and cms annuities
        for (Size i=firstValidIndex+1; i<nConstMatSwapRates; ++i) {
            Size lastIndex = std::min(i+spanningForwards,nConstMatSwapRates);
            constMatSwapAnnuities[i] = constMatSwapAnnuities[i-1] 
                                       - taus[i-1] * ds[i];
            if(lastIndex<nConstMatSwapRates)
               constMatSwapAnnuities[i] += taus[lastIndex-1] * ds[lastIndex];
            constMatSwapRates[i] = 
                (ds[i]-ds[lastIndex])/
                    constMatSwapAnnuities[i];
        }
    };
}

#endif
