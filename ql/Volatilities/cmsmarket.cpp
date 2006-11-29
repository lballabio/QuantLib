/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Giorgio Facchinetti

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


#include <ql/CashFlows/cashflowvectors.hpp>
#include <ql/Volatilities/cmsmarket.hpp>
#include <ql/Volatilities/swaptionvolcube.hpp>
#include <ql/Indexes/euriborswapfixa.hpp>
#include <ql/CashFlows/cashflowvectors.hpp>
#include <ql/Instruments/makecms.hpp>
#include <ql/Optimization/armijo.hpp>

namespace QuantLib {

    //===========================================================================//
    //                                CmsMarket                                  //
    //===========================================================================//

    CmsMarket::CmsMarket(
        const std::vector<Period>& expiries,
        const std::vector< boost::shared_ptr<SwapIndex> >& swapIndices,
        const std::vector<std::vector<Handle<Quote> > >& bidAskSpreads,
        const std::vector< boost::shared_ptr<VanillaCMSCouponPricer> >& pricers,
        const Handle<YieldTermStructure>& yieldTermStructure):
    expiries_(expiries),
    pricers_(pricers),
    swapIndices_(swapIndices),
    yieldTermStructure_(yieldTermStructure){

        nExercise_ = expiries_.size();
        nSwapTenors_ = swapIndices_.size();
        for (Size j=0; j<nSwapTenors_ ; j++) {
            swapTenors_.push_back(swapIndices_[j]->tenor());
        }

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
                bids_[i][j] = bidAskSpreads[i][j*2]->value();
                asks_[i][j] = bidAskSpreads[i][j*2+1]->value();
                mids_[i][j] = (bids_[i][j]+asks_[i][j])/2;

				const boost::shared_ptr<ConundrumPricer> pricer =
					boost::dynamic_pointer_cast<ConundrumPricer>(pricers_[j]);
				meanReversions_[i][j] = pricer->meanReversion();

                swapTmp.push_back(
                    MakeCMS(expiries_[i], swapIndices_[j], 0.,
						pricers_[j],Period()).operator boost::shared_ptr<Swap>()
               );

                // Spread errors valuation
                prices_[i][j]= swapTmp.back()->NPV();
                Real PV01 = swapTmp.back()->legBPS(1);
                modelCmsSpreads_[i][j] = -(prices_[i][j]/PV01)/10000;

                spreadErrors_[i][j] = modelCmsSpreads_[i][j]-mids_[i][j];

                // Price errors valuation
                Real floatingLegValueWithoutSpread = swapTmp.back()->legNPV(1);
                marketBidCmsLegValues_[i][j] = -(floatingLegValueWithoutSpread + PV01*bids_[i][j]*10000);
                marketAskCmsLegValues_[i][j] = -(floatingLegValueWithoutSpread + PV01*asks_[i][j]*10000);
                marketMidCmsLegValues_[i][j] = -(floatingLegValueWithoutSpread + PV01*mids_[i][j]*10000);

                modelCmsLegValues_[i][j] = swapTmp.back()->legNPV(0);

                priceErrors_[i][j] = modelCmsLegValues_[i][j]-marketMidCmsLegValues_[i][j];

            }
            swaps_.push_back(swapTmp);
        }
        createForwardStartingCms();
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
            for (Size j=0; j<nSwapTenors_ ; j++) {
                QL_REQUIRE(expiries_[i].units()==startingCmsTenor.units(),
                    "CmsMarket::createForwardStartingCms: Attenzione");
                Period tenorOfForwardCms =
                    Period(expiries_[i].length()-startingCmsTenor.length(),expiries_[i].units());
                boost::shared_ptr<Swap> forwardSwap =
                     MakeCMS(tenorOfForwardCms, swapIndices_[j], 0., 
                        pricers_[j],startingCmsTenor).operator boost::shared_ptr<Swap>();

                // ForwardPrice errors valuation
                if(i==0){
                    marketBidForwardCmsLegValues_[i][j] =
                        -(swaps_[i][j]->legNPV(1) + swaps_[i][j]->legBPS(1)*bids_[i][j]*10000);
                    marketAskForwardCmsLegValues_[i][j] =
                        -(swaps_[i][j]->legNPV(1) + swaps_[i][j]->legBPS(1)*asks_[i][j]*10000);
                    marketMidForwardCmsLegValues_[i][j] =
                        -(swaps_[i][j]->legNPV(1) + swaps_[i][j]->legBPS(1)*mids_[i][j]*10000);
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
                modelForwardCmsLegValues_[i][j] = forwardSwap->legNPV(0);

                forwardPriceErrors_[i][j]= modelForwardCmsLegValues_[i][j]-marketMidForwardCmsLegValues_[i][j];
            }
        }
     }

     void CmsMarket::reprice(const Handle<SwaptionVolatilityStructure>& volStructure,
                             Real meanReversion){

        for (Size j=0; j<nSwapTenors_ ; j++) {
			// set new volatility structure and new mean reversion
			pricers_[j]->setSwaptionVolatility(volStructure);
			const boost::shared_ptr<ConundrumPricer> pricer =
				boost::dynamic_pointer_cast<ConundrumPricer>(pricers_[j]);
			pricer->setMeanReversion(meanReversion);
		}
    
		for (Size i=0; i<nExercise_; i++) {
            for (Size j=0; j<nSwapTenors_ ; j++) {
	            swaps_[i][j] = MakeCMS(expiries_[i], swapIndices_[j], 0.,
                    pricers_[j],Period()).operator boost::shared_ptr<Swap>();

                // Spread errors valuation
                prices_[i][j] = swaps_[i][j]->NPV();
                Real PV01 = swaps_[i][j]->legBPS(1);
                modelCmsSpreads_[i][j] = -(prices_[i][j]/PV01)/10000;

                spreadErrors_[i][j] = modelCmsSpreads_[i][j]-mids_[i][j];

                // Price errors valuation
                Real floatingLegValueWithoutSpread = swaps_[i][j]->legNPV(1);
                marketBidCmsLegValues_[i][j] = -(floatingLegValueWithoutSpread + PV01*bids_[i][j]*10000);
                marketAskCmsLegValues_[i][j] = -(floatingLegValueWithoutSpread + PV01*asks_[i][j]*10000);
                marketMidCmsLegValues_[i][j] = -(floatingLegValueWithoutSpread + PV01*mids_[i][j]*10000);

                modelCmsLegValues_[i][j] = swaps_[i][j]->legNPV(0);

                priceErrors_[i][j] = modelCmsLegValues_[i][j]-marketMidCmsLegValues_[i][j];
            }
        }
        createForwardStartingCms();
    }
    Real CmsMarket::weightedError(const Matrix& weights){
        Real error=0.;
        Size count=0;
        for(Size i=0;i<nExercise_;i++){
            for(Size j=0;j<nSwapTenors_;j++){
                count++;
                error+=10000.*weights[i][j]*spreadErrors_[i][j]*spreadErrors_[i][j];
            }
        }
        error=std::sqrt(error/count);
        return error;
    }

    Real CmsMarket::weightedPriceError(const Matrix& weights){
        Real error=0.;
        Size count=0;
        for(Size i=0;i<nExercise_;i++){
            for(Size j=0;j<nSwapTenors_;j++){
                count++;
                error+=weights[i][j]*priceErrors_[i][j]*priceErrors_[i][j];
            }
        }
        error=std::sqrt(error/count);
        return error;
    }

    Real CmsMarket::weightedForwardPriceError(const Matrix& weights){
        Real error=0.;
        Size count=0;
        for(Size i=0;i<nExercise_;i++){
            for(Size j=0;j<nSwapTenors_;j++){
                count++;
                error+=weights[i][j]*forwardPriceErrors_[i][j]*forwardPriceErrors_[i][j];
            }
        }
        error=std::sqrt(error/count);
        return error;
    }

    Matrix CmsMarket::browse() const{
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

    //===========================================================================//
    //                       SmileAndCmsCalibrationBySabr                        //
    //===========================================================================//

    SmileAndCmsCalibrationBySabr::SmileAndCmsCalibrationBySabr(
        Handle<SwaptionVolatilityStructure>& volCube,
        boost::shared_ptr<CmsMarket>& cmsMarket,
        const Matrix& weights,
        CalibrationType calibrationType):
    volCube_(volCube),
    cmsMarket_(cmsMarket),
    weights_(weights),
    calibrationType_(calibrationType){
    }

	Array SmileAndCmsCalibrationBySabr::calibration(
            const Array& guess,
			SmileAndCmsCalibrationBySabr::OptimMethod optimizationMethod){

        ParametersConstraint constraint;
        ObjectiveFunction costFunction(this);
        boost::shared_ptr<OptimizationMethod> method;
		switch (optimizationMethod) {
            case DownHillSimplex:
                method = boost::shared_ptr<OptimizationMethod>(new Simplex(0.01));
                break;
			case ConjugateGrad:{
                boost::shared_ptr<LineSearch> lineSearch(new
                    ArmijoLineSearch(1e-12, 0.05, 0.65));
				method = boost::shared_ptr<OptimizationMethod>(new
                    ConjugateGradient(Array(), EndCriteria(), lineSearch));}
				break;
            default:
                QL_FAIL("unknown/illegal optimization method");
		}
       
        switch (calibrationType_) {
            case OnSpread:
                method->setEndCriteria(EndCriteria(30, 1e-1));
                break;
            case OnPrice:
                method->setEndCriteria(EndCriteria(30, 300.));
                break;
            case OnForwardCmsPrice:
                method->setEndCriteria(EndCriteria(30, 300.));
                break;
            default:
                QL_FAIL("unknown/illegal calibration type");
        }
        //Array guess(2);

        const boost::shared_ptr<SwaptionVolatilityCubeBySabr> volCubeBySabr =
            boost::dynamic_pointer_cast<SwaptionVolatilityCubeBySabr>(volCube_.currentLink());
        //guess[0] = volCubeBySabr->sparseSabrParameters()[0][3];
        //guess[1] = cmsMarket_->meanReversions()[0][0];

        method->setInitialValue(guess);

        Problem problem(costFunction, constraint, *method);
        problem.minimize();
        Array result = problem.minimumValue();

        Array y = result;

        error_ = method->functionValue();
        endCriteria_ = method->endCriteria().criteria();

        return y;
    }

    //===========================================================================//
    //           SmileAndCmsCalibrationBySabr::ObjectiveFunction                 //
    //===========================================================================//

    Real SmileAndCmsCalibrationBySabr::ObjectiveFunction::value(const Array& x) const {
        const Array y = x;
        Real beta = y[0];
        Real meanReversion = y[1];
        const boost::shared_ptr<SwaptionVolatilityCubeBySabr> volCubeBySabr =
               boost::dynamic_pointer_cast<SwaptionVolatilityCubeBySabr>(volCube_.currentLink());

        const std::vector<Period>& swapTenors = cmsMarket_->swapTenors();
        for (Size i=0; i<swapTenors.size(); i++){
            volCubeBySabr->recalibration(beta, swapTenors[i]);
        }
        cmsMarket_->reprice(volCube_, meanReversion);
        switch (calibrationType_) {
            case OnSpread:
                return cmsMarket_->weightedError(weights_);
            case OnPrice:
                return cmsMarket_->weightedPriceError(weights_);
            case OnForwardCmsPrice:
                return cmsMarket_->weightedForwardPriceError(weights_);
            default:
                QL_FAIL("unknown/illegal calibration type");
        }
    }


}
