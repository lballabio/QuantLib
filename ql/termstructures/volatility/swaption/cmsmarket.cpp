/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Marco Bianchetti
 Copyright (C) 2006, 2007 Giorgio Facchinetti

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

#include <ql/termstructures/volatility/swaption/cmsmarket.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/instruments/makecms.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/instruments/swap.hpp>

using std::vector;
using boost::shared_ptr;

namespace QuantLib {

    CmsMarket::CmsMarket(
        const vector<Period>& swapLengths,
        const vector<shared_ptr<SwapIndex> >& swapIndexes,
        const shared_ptr<IborIndex>& iborIndex,
        const vector<vector<Handle<Quote> > >& bidAskSpreads,
        const vector<shared_ptr<HaganPricer> >& pricers,
        const Handle<YieldTermStructure>& discountingTS)
    : swapLengths_(swapLengths),
      swapIndexes_(swapIndexes),
      iborIndex_(iborIndex),
      bidAskSpreads_(bidAskSpreads),
      pricers_(pricers),
      discTS_(discountingTS),

      nExercise_(swapLengths_.size()),
      nSwapIndexes_(swapIndexes_.size()),
      swapTenors_(nSwapIndexes_),

      spotFloatLegNPV_(nExercise_, nSwapIndexes_),
      spotFloatLegBPS_(nExercise_, nSwapIndexes_),

      mktBidSpreads_(nExercise_, nSwapIndexes_),
      mktAskSpreads_(nExercise_, nSwapIndexes_),

      mktSpreads_(nExercise_, nSwapIndexes_),
      mdlSpreads_(nExercise_, nSwapIndexes_),
      errSpreads_(nExercise_, nSwapIndexes_),

      mktSpotCmsLegNPV_(nExercise_, nSwapIndexes_),
      mdlSpotCmsLegNPV_(nExercise_, nSwapIndexes_),
      errSpotCmsLegNPV_(nExercise_, nSwapIndexes_),

      mktFwdCmsLegNPV_(nExercise_, nSwapIndexes_),
      mdlFwdCmsLegNPV_(nExercise_, nSwapIndexes_),
      errFwdCmsLegNPV_(nExercise_, nSwapIndexes_),

      spotSwaps_(nExercise_, vector<shared_ptr<Swap> >(nSwapIndexes_)),
      fwdSwaps_(nExercise_, vector<shared_ptr<Swap> >(nSwapIndexes_))
    {
        QL_REQUIRE(2*nSwapIndexes_==bidAskSpreads[0].size(),
                   "2*nSwapIndexes_!=bidAskSpreads columns()");
        QL_REQUIRE(nExercise_==bidAskSpreads.size(),
                   "nExercise_==bidAskSpreads rows()");

        for (Size j=0; j<nSwapIndexes_; ++j) {
            swapTenors_[j] = swapIndexes_[j]->tenor();
            // pricers
            registerWith(pricers_[j]);
            for (Size i=0; i<nExercise_; ++i) {
                // market Spread
                registerWith(bidAskSpreads_[i][j*2]);
                registerWith(bidAskSpreads_[i][j*2+1]);
            }
        }

        Period start(0, Years);
        for (Size i=0; i<nExercise_; ++i) {
            if (i>0) start = swapLengths_[i-1];
            for (Size j=0; j<nSwapIndexes_; ++j) {
                // never evaluate the spot swap, only its ibor floating leg
                spotSwaps_[i][j] = MakeCms(swapLengths_[i],
                                           swapIndexes_[j],
                                           iborIndex_, 0.0,
                                           Period())
                                   .operator shared_ptr<Swap>();
                fwdSwaps_[i][j]  = MakeCms(swapLengths_[i]-start,
                                           swapIndexes_[j],
                                           iborIndex_, 0.0,
                                           start)
                                   .withCmsCouponPricer(pricers_[j])
                                   .withDiscountingTermStructure(discTS_)
                                   .operator shared_ptr<Swap>();
            }
        }
        // probably useless
        performCalculations();
     }

    void CmsMarket::performCalculations() const {
        for (Size j=0; j<nSwapIndexes_; ++j) {
          Real mktPrevPart = 0.0, mdlPrevPart = 0.0;
          for (Size i=0; i<nExercise_; ++i) {

            // **** market

            mktBidSpreads_[i][j] = bidAskSpreads_[i][j*2]->value();
            mktAskSpreads_[i][j] = bidAskSpreads_[i][j*2+1]->value();
            mktSpreads_[i][j] = (mktBidSpreads_[i][j]+mktAskSpreads_[i][j])/2;

            const Leg& spotFloatLeg = spotSwaps_[i][j]->leg(1);
            spotFloatLegNPV_[i][j] = CashFlows::npv(spotFloatLeg,
                                                    **discTS_,
                                                    false, discTS_->referenceDate());
            spotFloatLegBPS_[i][j] = CashFlows::bps(spotFloatLeg,
                                                    **discTS_,
                                                    false, discTS_->referenceDate());

            // imply the spot CMS leg NPV from the spot ibor floating leg NPV
            mktSpotCmsLegNPV_[i][j] = -(spotFloatLegNPV_[i][j] +
                                spotFloatLegBPS_[i][j]*mktSpreads_[i][j]/1e-4);
            // fwd CMS legs can be computed as differences between spot legs
            mktFwdCmsLegNPV_[i][j] = mktSpotCmsLegNPV_[i][j] - mktPrevPart;
            mktPrevPart = mktSpotCmsLegNPV_[i][j];

            // **** model

            // calculate the forward swap (the time consuming part)
            mdlFwdCmsLegNPV_[i][j] = fwdSwaps_[i][j]->legNPV(0);
            errFwdCmsLegNPV_[i][j] = mdlFwdCmsLegNPV_[i][j] -
                                                mktFwdCmsLegNPV_[i][j];

            // spot CMS legs can be computed as incremental sum of forward legs
            mdlSpotCmsLegNPV_[i][j] = mdlPrevPart + mdlFwdCmsLegNPV_[i][j];
            mdlPrevPart = mdlSpotCmsLegNPV_[i][j];
            errSpotCmsLegNPV_[i][j] = mdlSpotCmsLegNPV_[i][j] -
                                                mktSpotCmsLegNPV_[i][j];

            // equilibriums spread over ibor leg
            Real npv = spotFloatLegNPV_[i][j] + mdlSpotCmsLegNPV_[i][j];
            mdlSpreads_[i][j] = - npv/spotFloatLegBPS_[i][j]*1e-4;
            errSpreads_[i][j] = mdlSpreads_[i][j] - mktSpreads_[i][j];
          }
        }
    }

    void CmsMarket::reprice(const Handle<SwaptionVolatilityStructure>& v,
                            Real meanReversion) {
        Handle<Quote> meanReversionQuote(shared_ptr<Quote>(new
                                                SimpleQuote(meanReversion)));
        for (Size j=0; j<nSwapIndexes_; ++j) {
            // ??
            // set new volatility structure and new mean reversion
            pricers_[j]->setSwaptionVolatility(v);
            pricers_[j]->setMeanReversion(meanReversionQuote);
        }
        performCalculations();
    }

    Real CmsMarket::weightedFwdNpvError(const Matrix& w) {
        performCalculations();
        return weightedMean(errFwdCmsLegNPV_, w);
    }

    Real CmsMarket::weightedSpotNpvError(const Matrix& w) {
        performCalculations();
        return weightedMean(errSpotCmsLegNPV_, w);
    }

    Real CmsMarket::weightedSpreadError(const Matrix& w) {
        performCalculations();
        return weightedMean(errSpreads_, w);
    }

    // array of errors to be used by Levenberg-Marquardt optimization

    Disposable<Array> CmsMarket::weightedFwdNpvErrors(const Matrix& w) {
        performCalculations();
        return weightedMeans(errFwdCmsLegNPV_, w);
    }

    Disposable<Array> CmsMarket::weightedSpotNpvErrors(const Matrix& w) {
        performCalculations();
        return weightedMeans(errSpotCmsLegNPV_, w);
    }

    Disposable<Array> CmsMarket::weightedSpreadErrors(const Matrix& w) {
        performCalculations();
        return weightedMeans(errSpreads_, w);
    }

    Real CmsMarket::weightedMean(const Matrix& var,
                                 const Matrix& w) {
        Real mean = 0.0;
        for (Size i=0; i<nExercise_; ++i) {
            for (Size j=0; j<nSwapIndexes_; ++j) {
                mean += w[i][j]*var[i][j]*var[i][j];
            }
        }
        mean = std::sqrt(mean/(nExercise_*nSwapIndexes_));
        return mean;
    }

    Disposable<Array> CmsMarket::weightedMeans(const Matrix& var,
                                               const Matrix& w) {
        Array weightedVars(nExercise_*nSwapIndexes_);
        for (Size i=0; i<nExercise_; ++i) {
            for (Size j=0; j<nSwapIndexes_; ++j) {
                weightedVars[i*nSwapIndexes_+j] = std::sqrt(w[i][j])*var[i][j];
            }
        }
        return weightedVars;
    }

    Matrix CmsMarket::browse() const {
        calculate();
        //Matrix result(nExercise_*nSwapIndexes_, 15);
        Matrix result(nExercise_*nSwapIndexes_, 14);
            for (Size j=0; j<nSwapIndexes_; ++j) {
                for (Size i=0; i<nExercise_; ++i) {
                result[j*nSwapIndexes_+i][0] = swapTenors_[j].length();
                result[j*nSwapIndexes_+i][1] = swapLengths_[i].length();

                // Spreads
                result[j*nSwapIndexes_+i][2] = mktBidSpreads_[i][j]*10000;
                result[j*nSwapIndexes_+i][3] = mktAskSpreads_[i][j]*10000;
                result[j*nSwapIndexes_+i][4] = mktSpreads_[i][j]*10000;
                result[j*nSwapIndexes_+i][5] = mdlSpreads_[i][j]*10000;
                result[j*nSwapIndexes_+i][6] = errSpreads_[i][j]*10000;
                if (mdlSpreads_[i][j]>mktAskSpreads_[i][j])
                    result[j*nSwapIndexes_+i][7] = (mdlSpreads_[i][j] -
                                                mktAskSpreads_[i][j])*10000;
                else if (mdlSpreads_[i][j]<mktBidSpreads_[i][j])
                    result[j*nSwapIndexes_+i][7] = (mktBidSpreads_[i][j] -
                                                mdlSpreads_[i][j])*10000;
                else
                    result[j*nSwapIndexes_+i][7] = 0.0;

                // spot CMS Leg NPVs
                result[j*nSwapIndexes_+i][ 8] = mktSpotCmsLegNPV_[i][j];
                result[j*nSwapIndexes_+i][ 9] = mdlSpotCmsLegNPV_[i][j];
                result[j*nSwapIndexes_+i][10] = errSpotCmsLegNPV_[i][j];

                // forward CMS Leg NPVs
                result[j*nSwapIndexes_+i][11] = mktFwdCmsLegNPV_[i][j];
                result[j*nSwapIndexes_+i][12] = mdlFwdCmsLegNPV_[i][j];
                result[j*nSwapIndexes_+i][13] = errFwdCmsLegNPV_[i][j];
            }
        }
        return result;
    }
}
