/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#include <ql/termstructures/volatility/interestrate/swaption/cmsmarket.hpp>
#include <ql/instruments/makecms.hpp>
#include <ql/cashflows/conundrumpricer.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/termstructures/volatility/interestrate/swaption/swaptionvolcube1.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/instruments/swap.hpp>

namespace QuantLib {
    
           
    //===========================================================================//
    //                                CmsMarket                                  //
    //===========================================================================//

    CmsMarket::CmsMarket(
        const std::vector<Period>& expiries,
        const std::vector< boost::shared_ptr<SwapIndex> >& swapIndices,
        const std::vector<std::vector<Handle<Quote> > >& bidAskSpreads,
        const std::vector< boost::shared_ptr<CmsCouponPricer> >& pricers,
        const Handle<YieldTermStructure>& yieldTermStructure):
    expiries_(expiries),
    swapFloatingLegsPrices_(expiries.size(), swapIndices.size()),
    swapFloatingLegsBps_(expiries.size(), swapIndices.size()),
    pricers_(pricers),
    swapIndices_(swapIndices),
    bidAskSpreads_(bidAskSpreads),
    yieldTermStructure_(yieldTermStructure) {

        nExercise_ = expiries_.size();
        nSwapTenors_ = swapIndices_.size();
        swapTenors_.reserve(nSwapTenors_);
        for (Size j=0; j<nSwapTenors_ ; ++j)
            swapTenors_.push_back(swapIndices_[j]->tenor());

        QL_REQUIRE(2*nSwapTenors_==bidAskSpreads[0].size(),
                   "2*nSwapTenors_!=bidAskSpreads columns()");
        QL_REQUIRE(nExercise_==bidAskSpreads.size(),
                   "nExercise_==bidAskSpreads rows()");

        bids_ = Matrix(nExercise_, nSwapTenors_, 0.);
        asks_ = Matrix(nExercise_, nSwapTenors_, 0.);
        mids_ = Matrix(nExercise_, nSwapTenors_, 0.);
        modelCmsSpreads_ = Matrix(nExercise_, nSwapTenors_, 0.);
        spreadErrors_ = Matrix(nExercise_, nSwapTenors_, 0.);

        prices_= Matrix(nExercise_, nSwapTenors_, 0.);
        marketBidCmsLegValues_ = Matrix(nExercise_, nSwapTenors_, 0.);
        marketAskCmsLegValues_ = Matrix(nExercise_, nSwapTenors_, 0.);
        marketMidCmsLegValues_ = Matrix(nExercise_, nSwapTenors_, 0.);
        modelCmsLegValues_ = Matrix(nExercise_, nSwapTenors_, 0.);
        priceErrors_ = Matrix(nExercise_, nSwapTenors_, 0.);

        marketBidForwardCmsLegValues_ = Matrix(nExercise_, nSwapTenors_, 0.);
        marketAskForwardCmsLegValues_ = Matrix(nExercise_, nSwapTenors_, 0.);
        marketMidForwardCmsLegValues_ = Matrix(nExercise_, nSwapTenors_, 0.);
        modelForwardCmsLegValues_ = Matrix(nExercise_, nSwapTenors_, 0.);
        forwardPriceErrors_ = Matrix(nExercise_, nSwapTenors_, 0.);
        meanReversions_ = Matrix(nExercise_, nSwapTenors_, 0.);
        for (Size i=0; i<nExercise_; i++) {
            std::vector< boost::shared_ptr<Swap> > swapTmp;
            for (Size j=0; j<nSwapTenors_ ; j++) {
                swapTmp.push_back(
                    MakeCms(expiries_[i], swapIndices_[j], 0.,
                        Period()).operator boost::shared_ptr<Swap>()
               );
            }
            swaps_.push_back(swapTmp);
        }
        registerWithMarketData();
        createForwardStartingCms();
        performCalculations();
     }


    void CmsMarket::registerWithMarketData() {
        // register with Market Cms Spread
        for (Size i=0; i<nExercise_; i++) {
            for (Size j=0; j<nSwapTenors_ ; j++) {
                registerWith(bidAskSpreads_[i][j*2]);
                registerWith(bidAskSpreads_[i][j*2+1]);
            }
        }
        // register with pricers
        for (Size j=0; j<nSwapTenors_ ; j++) {
            registerWith(pricers_[j]);
        }
        // register with yieldTermStructure
        registerWith(yieldTermStructure_);
    }

    void CmsMarket::performCalculations() const{
        Size cmsIndex = 0;
        Size floatIndex = 1;

        for (Size i = 0; i< nExercise_;++i)
            for (Size j = 0; j< nSwapTenors_;++j){

                bids_[i][j] = bidAskSpreads_[i][j*2]->value();
                asks_[i][j] = bidAskSpreads_[i][j*2+1]->value();
                mids_[i][j] = (bids_[i][j]+asks_[i][j])/2;

                const boost::shared_ptr<ConundrumPricer> pricer =
                    boost::dynamic_pointer_cast<ConundrumPricer>(pricers_[j]);
                meanReversions_[i][j] = pricer->meanReversion();

                setCouponPricer(swaps_[i][j]->leg(cmsIndex), pricer);
                setCouponPricer(forwardSwaps_[i][j]->leg(cmsIndex), pricer);

                swapFloatingLegsBps_[i][j] = swaps_[i][j]->legBPS(floatIndex);
                swapFloatingLegsPrices_[i][j] = swaps_[i][j]->legNPV(floatIndex);
                // Price errors valuation
                Real floatingLegValueWithoutSpread = swaps_[i][j]->legNPV(floatIndex);
                Real PV01 = swapFloatingLegsBps_[i][j];
                marketBidCmsLegValues_[i][j] = -(floatingLegValueWithoutSpread + PV01*bids_[i][j]*10000);
                marketAskCmsLegValues_[i][j] = -(floatingLegValueWithoutSpread + PV01*asks_[i][j]*10000);
                marketMidCmsLegValues_[i][j] = -(floatingLegValueWithoutSpread + PV01*mids_[i][j]*10000);

                // ForwardPrice errors valuation
                if(i==0){
                    marketBidForwardCmsLegValues_[i][j] =
                        -(swapFloatingLegsPrices_[i][j] + swaps_[i][j]->legBPS(1)*bids_[i][j]*10000);
                    marketAskForwardCmsLegValues_[i][j] =
                        -(swapFloatingLegsPrices_[i][j] + swaps_[i][j]->legBPS(1)*asks_[i][j]*10000);
                    marketMidForwardCmsLegValues_[i][j] =
                        -(swapFloatingLegsPrices_[i][j] + swaps_[i][j]->legBPS(1)*mids_[i][j]*10000);
                }
                else{
                    marketBidForwardCmsLegValues_[i][j] =
                        -((swaps_[i][j]->legNPV(1) + swaps_[i][j]->legBPS(1)*bids_[i][j]*10000)-
                        (swaps_[i-1][j]->legNPV(1) + swaps_[i-1][j]->legBPS(1)*bids_[i-1][j]*10000));
                    marketAskForwardCmsLegValues_[i][j] =
                        -((swaps_[i][j]->legNPV(1) + swaps_[i][j]->legBPS(1)*asks_[i][j]*10000)-
                        (swaps_[i-1][j]->legNPV(1) + swaps_[i-1][j]->legBPS(1)*asks_[i-1][j]*10000));
                    marketMidForwardCmsLegValues_[i][j] =
                        -((swaps_[i][j]->legNPV(1) + swaps_[i][j]->legBPS(1)*mids_[i][j]*10000)-
                        (swaps_[i-1][j]->legNPV(1) + swaps_[i-1][j]->legBPS(1)*mids_[i-1][j]*10000));
                }
            }
        priceForwardStartingCms();
        priceSpotFromForwardStartingCms();
    }

    void CmsMarket::createForwardStartingCms(){
        for (Size i=0; i<nExercise_; i++) {
            Period startingCmsTenor;
            if(i==0){
                startingCmsTenor = Period(0,Years);
            }
            else{
                startingCmsTenor = expiries_[i-1];
            }
            std::vector< boost::shared_ptr<Swap> > forwardSwapTmp;
            for (Size j=0; j<nSwapTenors_ ; j++) {
                QL_REQUIRE(expiries_[i].units()==startingCmsTenor.units(),
                    "CmsMarket::createForwardStartingCms: Attenzione");
                Period tenorOfForwardCms =
                    Period(expiries_[i].length()-startingCmsTenor.length(),expiries_[i].units());
                forwardSwapTmp.push_back(
                     MakeCms(tenorOfForwardCms, swapIndices_[j], 0.,
                        startingCmsTenor).operator boost::shared_ptr<Swap>()
                     );
            }
            forwardSwaps_.push_back(forwardSwapTmp);
        }
    }

    void CmsMarket::reprice(const Handle<SwaptionVolatilityStructure>& volStructure,
                             Real meanReversion){
        Handle<Quote> meanReversionQuote = Handle<Quote>(boost::shared_ptr<Quote>(new
                SimpleQuote(meanReversion)));
        for (Size j=0; j<nSwapTenors_ ; j++) {
            // set new volatility structure and new mean reversion
            pricers_[j]->setSwaptionVolatility(volStructure);
            const boost::shared_ptr<ConundrumPricer> pricer =
                boost::dynamic_pointer_cast<ConundrumPricer>(pricers_[j]);
            pricer->setMeanReversion(meanReversionQuote);
        }

      priceForwardStartingCms();
    }

    void CmsMarket::priceForwardStartingCms() const {
          for (Size i=0; i<nExercise_; i++) {
            for (Size j=0; j<nSwapTenors_ ; j++) {
                Real modelForwardCmsLegValue = forwardSwaps_[i][j]->legNPV(0);
                modelForwardCmsLegValues_[i][j] = modelForwardCmsLegValue;
                forwardPriceErrors_[i][j]= modelForwardCmsLegValue
                                            - marketMidForwardCmsLegValues_[i][j];
            }
        }
    }

    void CmsMarket::priceSpotFromForwardStartingCms() const {
          for (Size i=0; i<nExercise_; i++) {
            for (Size j=0; j<nSwapTenors_ ; j++) {
                modelCmsLegValues_[i][j] = modelForwardCmsLegValues_[i][j];
                if (i>0)
                    modelCmsLegValues_[i][j] += modelCmsLegValues_[i-1][j];
                priceErrors_[i][j] = modelCmsLegValues_[i][j]
                                     - marketMidCmsLegValues_[i][j];

                // Spread errors valuation
                prices_[i][j]= swapFloatingLegsPrices_[i][j]+ modelCmsLegValues_[i][j];
                Real PV01 = swapFloatingLegsBps_[i][j];
                modelCmsSpreads_[i][j] = -(prices_[i][j]/PV01)/10000;

                spreadErrors_[i][j] = modelCmsSpreads_[i][j]-mids_[i][j];
            }
        }
    }


    Real CmsMarket::weightedError(const Matrix& weights){
        priceSpotFromForwardStartingCms();
        return weightedMean(spreadErrors_,weights);
    }

    Real CmsMarket::weightedPriceError(const Matrix& weights){
        priceSpotFromForwardStartingCms();
        return weightedMean(priceErrors_,weights);
    }

    Real CmsMarket::weightedForwardPriceError(const Matrix& weights){
        return weightedMean(forwardPriceErrors_,weights);
    }

    // return an array of errors to be used for Levenberg-Marquardt optimization.

    Disposable<Array> CmsMarket::weightedErrors(const Matrix& weights){
        priceSpotFromForwardStartingCms();
        return weightedMeans(spreadErrors_,weights);
    }

    Disposable<Array> CmsMarket::weightedPriceErrors(const Matrix& weights){
        priceSpotFromForwardStartingCms();
        return weightedMeans(priceErrors_,weights);
    }

    Disposable<Array> CmsMarket::weightedForwardPriceErrors(
                                                        const Matrix& weights){
        return weightedMeans(forwardPriceErrors_,weights);
    }

    Real CmsMarket::weightedMean(const Matrix& var, const Matrix& weights){
        Real mean=0.;
        for(Size i=0;i<nExercise_;i++){
            for(Size j=0;j<nSwapTenors_;j++){
                mean += weights[i][j]*var[i][j]*var[i][j];
            }
        }
        mean=std::sqrt(mean/(nExercise_*nSwapTenors_));
        return mean;
    }

    Disposable<Array> CmsMarket::weightedMeans(const Matrix& var,
                                                       const Matrix& weights){
        Array  weightedVars(nExercise_*nSwapTenors_);
        for(Size i=0; i<nExercise_; i++) {
            for(Size j=0; j<nSwapTenors_; j++) {
                weightedVars[i*nSwapTenors_+j] = std::sqrt(weights[i][j])*var[i][j];
            }
        }
        return weightedVars;
    }

    Matrix CmsMarket::browse() const{
        calculate();
        Matrix result(nExercise_*nSwapTenors_,19,0.);
            for(Size j=0;j<nSwapTenors_;j++){
                for(Size i=0;i<nExercise_;i++){
                result[j*nSwapTenors_+i][0]= swapTenors_[j].length();
                result[j*nSwapTenors_+i][1]= expiries_[i].length();

                // Spreads
                result[j*nSwapTenors_+i][2]= bids_[i][j]*10000;
                result[j*nSwapTenors_+i][3]= asks_[i][j]*10000;
                result[j*nSwapTenors_+i][4]= mids_[i][j]*10000;
                result[j*nSwapTenors_+i][5]= modelCmsSpreads_[i][j]*10000;
                result[j*nSwapTenors_+i][6]= spreadErrors_[i][j]*10000;
                if(modelCmsSpreads_[i][j]>asks_[i][j]){
                    result[j*nSwapTenors_+i][7]= (modelCmsSpreads_[i][j]-asks_[i][j])*10000;
                }
                else if(modelCmsSpreads_[i][j]<bids_[i][j]){
                    result[j*nSwapTenors_+i][7]= (bids_[i][j]-modelCmsSpreads_[i][j])*10000;
                }
                else{ result[j*nSwapTenors_+i][7]= 0.; }

                // Prices of cms
                result[j*nSwapTenors_+i][8]= marketBidCmsLegValues_[i][j];
                result[j*nSwapTenors_+i][9]= marketAskCmsLegValues_[i][j];
                result[j*nSwapTenors_+i][10]= marketMidCmsLegValues_[i][j];
                result[j*nSwapTenors_+i][11]= modelCmsLegValues_[i][j];
                result[j*nSwapTenors_+i][12]= priceErrors_[i][j];

                // Prices of forward cms
                result[j*nSwapTenors_+i][13]= marketBidForwardCmsLegValues_[i][j];
                result[j*nSwapTenors_+i][14]= marketAskForwardCmsLegValues_[i][j];
                result[j*nSwapTenors_+i][15]= marketMidForwardCmsLegValues_[i][j];
                result[j*nSwapTenors_+i][16]= modelForwardCmsLegValues_[i][j];
                result[j*nSwapTenors_+i][17]= forwardPriceErrors_[i][j];

                // mean reversions
                result[j*nSwapTenors_+i][18]= meanReversions_[i][j];

            }
        }
        return result;
    }
}
