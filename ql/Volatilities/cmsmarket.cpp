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


#include <ql/Volatilities/cmsmarket.hpp>
#include <ql/Volatilities/swaptionvolcube.hpp>
#include <ql/Indexes/euriborswapfixa.hpp>

#include <fstream>
#include <string>

namespace QuantLib {

    //===========================================================================//
    //                                CmsMarket                                  //
    //===========================================================================//

    CmsMarket::CmsMarket(
        const std::vector<Period>& expiries,
        const std::vector<Period>& lengths,
        const Matrix& bidsAsks,
        const Matrix& meanReversions,
        const boost::shared_ptr<VanillaCMSCouponPricer>& pricer,
        const Handle<YieldTermStructure>& yieldTermStructure,
        const Handle<SwaptionVolatilityStructure>& volStructure):
    expiries_(expiries),
    lengths_(lengths),
    calendar_(TARGET()),
    tenor_(Period(Quarterly)),
    bdc_(Unadjusted),
    dayCounter_(Actual360()),
    meanReversions_(meanReversions),
    pricer_(pricer),
    floatingIndex_(boost::shared_ptr<Xibor>(
                                          new Euribor3M(yieldTermStructure))),
    yieldTermStructure_(yieldTermStructure),
    volStructure_(volStructure) {

        referenceDate_ = yieldTermStructure_->referenceDate();
        effectiveDate_ = calendar_.advance(referenceDate_,2,Days,Following); //FIXME

        nExercise_ = expiries_.size();
        nLengths_ = lengths_.size();

        QL_REQUIRE(2*nLengths_==bidsAsks.columns(),
                   "2*nLengths_!=bidsAsks.columns()");
        QL_REQUIRE(nExercise_==bidsAsks.rows(),
                   "nExercise_==bidsAsks.rows()");

        bids_ = Matrix(nExercise_, nLengths_, 0.);
        asks_ = Matrix(nExercise_, nLengths_, 0.);
        mids_ = Matrix(nExercise_, nLengths_, 0.);
        prices_= Matrix(nExercise_, nLengths_, 0.);
        impliedCmsSpreads_ = Matrix(nExercise_, nLengths_, 0.);
        spreadErrors_ = Matrix(nExercise_, nLengths_, 0.);
        midPrices_ = Matrix(nExercise_, nLengths_, 0.);
        askPrices_ = Matrix(nExercise_, nLengths_, 0.);
        bidPrices_ = Matrix(nExercise_, nLengths_, 0.);

        swapIndices_.push_back(boost::shared_ptr<SwapIndex>(new EuriborSwapFixA2Y(yieldTermStructure_)));
        swapIndices_.push_back(boost::shared_ptr<SwapIndex>(new EuriborSwapFixA5Y(yieldTermStructure_)));
        swapIndices_.push_back(boost::shared_ptr<SwapIndex>(new EuriborSwapFixA10Y(yieldTermStructure_)));
        swapIndices_.push_back(boost::shared_ptr<SwapIndex>(new EuriborSwapFixA20Y(yieldTermStructure_)));
        swapIndices_.push_back(boost::shared_ptr<SwapIndex>(new EuriborSwapFixA30Y(yieldTermStructure_)));

        for (Size i=0; i<nExercise_; i++) {
            exerciseDates_.push_back(calendar_.advance(effectiveDate_,expiries[i],bdc_));//FIXME
            schedules_.push_back( boost::shared_ptr<Schedule>(
                new Schedule(effectiveDate_, exerciseDates_[i], tenor_,
                             calendar_, bdc_, bdc_, true, false)) // FIXME
            );
            std::vector<Leg> cmsTmp;
            std::vector<Leg> floatingTmp;
            std::vector< boost::shared_ptr<Swap> > swapTmp;
            for (Size j=0; j<nLengths_ ; j++) {
                bids_[i][j] = bidsAsks[i][j*2];
                asks_[i][j] = bidsAsks[i][j*2+1];
                mids_[i][j] = (bids_[i][j]+asks_[i][j])/2;
                Size nCoupons = schedules_[i]->size();
                cmsTmp.push_back(
                    CMSCouponVector(*(schedules_[i].get()),
                                    bdc_,
                                    std::vector<double>(nCoupons,1.),
                                    swapIndices_[j],
                                    2,
                                    dayCounter_,
                                    std::vector<double>(nCoupons, 0.),
                                    std::vector<double>(nCoupons, 1.),
                                    std::vector<double>(nCoupons, 1.),
                                    std::vector<double>(nCoupons, 0.),
                                    std::vector<double>(nCoupons, meanReversions_[i][j]),
                                    pricer_,
                                    volStructure_ )
                );
                floatingTmp.push_back(
                    FloatingRateCouponVector(*(schedules_[i].get()),
                                    floatingIndex_->businessDayConvention(),
                                    std::vector<double>(nCoupons, 1.),
                                    floatingIndex_->settlementDays(),
                                    floatingIndex_,
                                    std::vector<double>(nCoupons, 1.),
                                    std::vector<double>(nCoupons, 0.),
                                    floatingIndex_->dayCounter())
                );
                swapTmp.push_back(
                    boost::shared_ptr<Swap>(new Swap(yieldTermStructure_, cmsTmp.back(), floatingTmp.back()))
                );
                prices_[i][j] = swapTmp.back()->NPV();
                Real PV01 = swapTmp.back()->legBPS(1);
                impliedCmsSpreads_[i][j] = -(prices_[i][j]/PV01)/10000;;
                spreadErrors_[i][j] = impliedCmsSpreads_[i][j]-mids_[i][j];
                midPrices_[i][j] = prices_[i][j]+PV01*mids_[i][j]*10000;
                askPrices_[i][j] = prices_[i][j]+PV01*asks_[i][j]*10000;
                bidPrices_[i][j] = prices_[i][j]+PV01*bids_[i][j]*10000;
            }
            cmsLegs_.push_back(cmsTmp);
            floatingLegs_.push_back(floatingTmp);
            swaps_.push_back(swapTmp);
        }
     }
    void CmsMarket::reprice(const Handle<SwaptionVolatilityStructure>& volStructure){
        volStructure_ = volStructure;
        for (Size i=0; i<nExercise_; i++) {
            for (Size j=0; j<nLengths_ ; j++) {
                Size nCoupons = schedules_[i]->size();
                cmsLegs_[i][j] = CMSCouponVector(*(schedules_[i].get()),
                                    bdc_,
                                    std::vector<double>(nCoupons,1.),
                                    swapIndices_[j],
                                    2,
                                    dayCounter_,
                                    std::vector<double>(nCoupons, 0.),
                                    std::vector<double>(nCoupons, 1.),
                                    std::vector<double>(nCoupons, 1.),
                                    std::vector<double>(nCoupons, 0.),
                                    std::vector<double>(nCoupons, meanReversions_[i][j]),
                                    pricer_,
                                    volStructure_ );
                floatingLegs_[i][j] = FloatingRateCouponVector(*(schedules_[i].get()),
                                    floatingIndex_->businessDayConvention(),
                                    std::vector<double>(nCoupons, 1.),
                                    floatingIndex_->settlementDays(),
                                    floatingIndex_,
                                    std::vector<double>(nCoupons, 1.),
                                    std::vector<double>(nCoupons, 0.),
                                    floatingIndex_->dayCounter());
                swaps_[i][j] = boost::shared_ptr<Swap>(
                    new Swap(yieldTermStructure_, cmsLegs_[i][j], floatingLegs_[i][j]));
                prices_[i][j] = swaps_[i][j]->NPV();
                impliedCmsSpreads_[i][j] = -(prices_[i][j]/swaps_[i][j]->legBPS(1))/10000;
                spreadErrors_[i][j] = impliedCmsSpreads_[i][j]-mids_[i][j];
            }
        }
    }
    Real CmsMarket::weightedError(const Matrix& weights){
        Real error=0.;
        Size count=0;
        for(Size i=0;i<nExercise_;i++){
            for(Size j=0;j<nLengths_;j++){
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
        Real fictitiousNominal = 1.;
        for(Size i=0;i<nExercise_;i++){
            for(Size j=0;j<nLengths_;j++){
                count++;
                error+=fictitiousNominal*weights[i][j]*midPrices_[i][j]*midPrices_[i][j];
            }
        }
        error=std::sqrt(error/count);
        return error;
    }


    Matrix CmsMarket::browse() const{
        Matrix result(nExercise_*nLengths_,14,0.);
	        for(Size j=0;j<nLengths_;j++){
                for(Size i=0;i<nExercise_;i++){
                result[j*nLengths_+i][0]= lengths_[j].length();
                result[j*nLengths_+i][1]= expiries_[i].length();
                result[j*nLengths_+i][2]= bids_[i][j]*10000;
                result[j*nLengths_+i][3]= asks_[i][j]*10000;
                result[j*nLengths_+i][4]= mids_[i][j]*10000;
                result[j*nLengths_+i][5]= impliedCmsSpreads_[i][j]*10000;
                result[j*nLengths_+i][6]= spreadErrors_[i][j]*10000;
                if(impliedCmsSpreads_[i][j]>asks_[i][j]){ 
                    result[j*nLengths_+i][7]= (impliedCmsSpreads_[i][j]-asks_[i][j])*10000;
                }
                else if(impliedCmsSpreads_[i][j]<bids_[i][j]){
                    result[j*nLengths_+i][7]= (bids_[i][j]-impliedCmsSpreads_[i][j])*10000;
                }
                else{ result[j*nLengths_+i][7]= 0.; } 
                result[j*nLengths_+i][8]= swaps_[i][j]->legNPV(0);
                result[j*nLengths_+i][9]= swaps_[i][j]->legNPV(1);
                result[j*nLengths_+i][10]= prices_[i][j];
                result[j*nLengths_+i][11]= bidPrices_[i][j];
                result[j*nLengths_+i][12]= askPrices_[i][j];
                result[j*nLengths_+i][13]= midPrices_[i][j];

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
        bool isSpreadCalibrated):
    volCube_(volCube),
    cmsMarket_(cmsMarket),
    weights_(weights),
    isSpreadCalibrated_(isSpreadCalibrated){
        tranformation_ = boost::shared_ptr<Transformation>(new transformationBeta);
    }

    Real SmileAndCmsCalibrationBySabr::calibration(){

        NoConstraint constraint;
        ObjectiveFunctionJustBeta costFunction(this);

        boost::shared_ptr<LineSearch> lineSearch(
            new ArmijoLineSearch(1e-12, 0.15, 0.55));
        boost::shared_ptr<OptimizationMethod> method =
            boost::shared_ptr<OptimizationMethod>(new ConjugateGradient(lineSearch));
        //boost::shared_ptr<OptimizationMethod> method =
        //    boost::shared_ptr<OptimizationMethod>(new Simplex(.0001,1e-3));
        
        if(isSpreadCalibrated_){ 
            method->setEndCriteria(EndCriteria(1000, 1e-1));
        }
        else{
            method->setEndCriteria(EndCriteria(1000, 1e-6));
        }        

        Array guess(1);

        const boost::shared_ptr<SwaptionVolatilityCubeBySabr> volCubeBySabr =
            boost::dynamic_pointer_cast<SwaptionVolatilityCubeBySabr>(volCube_.currentLink());
        guess[0] = volCubeBySabr->sparseSabrParameters()[0][3];

        guess = tranformation_->inverse(guess);
        method->setInitialValue(guess);

        Problem problem(costFunction, constraint, *method);
        problem.minimize();
        Array result = problem.minimumValue();

        Array y = tranformation_->direct(result);

        error_ = method->functionValue();
        endCriteria_ = method->endCriteria().criteria();

        return y[0];
    }

    //===========================================================================//
    //       SmileAndCmsCalibrationBySabr::ObjectiveFunctionJustBeta             //
    //===========================================================================//

    Real SmileAndCmsCalibrationBySabr::ObjectiveFunctionJustBeta::value(const Array& x) const {
        const Array y = smileAndCms_->tranformation_->direct(x);
        Real beta = y[0];
        const boost::shared_ptr<SwaptionVolatilityCubeBySabr> volCubeBySabr =
               boost::dynamic_pointer_cast<SwaptionVolatilityCubeBySabr>(volCube_.currentLink());
        volCubeBySabr->recalibration(beta);
        cmsMarket_->reprice(volCube_);
        if(isSpreadCalibrated_){ 
            return cmsMarket_->weightedError(weights_);
        }
        else{
            return cmsMarket_->weightedPriceError(weights_);
        }
    }


}
