/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

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

#include <ql/experimental/models/markovfunctional.hpp>
#include <ql/experimental/models/smilesectionutils.hpp>

namespace QuantLib {

    MarkovFunctional::MarkovFunctional(const Handle<YieldTermStructure>& termStructure,
                        const Real reversion,
                        const std::vector<Date>& volstepdates,
                        const std::vector<Real>& volatilities,
                        const Handle<SwaptionVolatilityStructure>& swaptionVol,
                        const std::vector<Date>& swaptionExpiries,
                        const std::vector<Period>& swaptionTenors,
                        const boost::shared_ptr<SwapIndex>& swapIndexBase,
                        const MarkovFunctional::ModelSettings& modelSettings) :
      TermStructureConsistentModel(termStructure), CalibratedModel(1), modelSettings_(modelSettings), capletCalibrated_(false),
      reversion_(ConstantParameter(reversion, NoConstraint())),sigma_(arguments_[0]), volstepdates_(volstepdates),
      volatilities_(volatilities), swaptionVol_(swaptionVol),
      capletVol_(Handle<OptionletVolatilityStructure>()),
      swaptionExpiries_(swaptionExpiries), capletExpiries_(std::vector<Date>()), swaptionTenors_(swaptionTenors),
      swapIndexBase_(swapIndexBase), iborIndex_(swapIndexBase->iborIndex())
       {

        QL_REQUIRE(swaptionExpiries.size()==swaptionTenors.size(),"number of swaption expiries (" << swaptionExpiries.size() << ") is differnt from number of swaption tenors (" << swaptionTenors.size() << ")");
        QL_REQUIRE(swaptionExpiries.size()>=1,"need at least one swaption expiry to calibrate numeraire");
        QL_REQUIRE(!termStructure.empty(),"yield term structure handle is empty");
        QL_REQUIRE(!swaptionVol.empty(),"swaption volatility structure is empty");
        modelSettings.validate();

        initialize();

    }

    MarkovFunctional::MarkovFunctional(const Handle<YieldTermStructure>& termStructure,
                        const Real reversion,
                        const std::vector<Date>& volstepdates,
                        const std::vector<Real>& volatilities,
                        const Handle<OptionletVolatilityStructure>& capletVol,
                        const std::vector<Date>& capletExpiries,
                        const boost::shared_ptr<IborIndex>& iborIndex,
                        const MarkovFunctional::ModelSettings& modelSettings) :
      TermStructureConsistentModel(termStructure), CalibratedModel(1),
      modelSettings_(modelSettings), capletCalibrated_(true),
      reversion_(ConstantParameter(reversion, NoConstraint())),sigma_(arguments_[0]),
      volstepdates_(volstepdates), volatilities_(volatilities),
      swaptionVol_(Handle<SwaptionVolatilityStructure>()), capletVol_(capletVol),
      swaptionExpiries_(std::vector<Date>()), capletExpiries_(capletExpiries), swaptionTenors_(std::vector<Period>()),
      iborIndex_(iborIndex)
      {

        QL_REQUIRE(capletExpiries.size()>=1,"need at least one caplet expiry to calibrate numeraire");
        QL_REQUIRE(!termStructure.empty(),"yield term structure handle is empty");
        QL_REQUIRE(!capletVol.empty(),"caplet volatility structure is empty");
        modelSettings.validate();

        initialize();

    }

    void MarkovFunctional::initialize() {

        QL_MFMESSAGE(modelOutputs_,"initializing");
        modelOutputs_.dirty_=true;

        modelOutputs_.settings_ = modelSettings_;

        GaussHermiteIntegration gaussHermite(modelSettings_.gaussHermitePoints_);
        normalIntegralX_ = gaussHermite.x();
        normalIntegralW_ = gaussHermite.weights();
        for(Size i=0; i < normalIntegralX_.size(); i++) {
            normalIntegralW_[i] *= exp(-normalIntegralX_[i]*normalIntegralX_[i]) * M_1_SQRTPI;
            normalIntegralX_[i] *= M_SQRT2;
        }
        
        volsteptimes_.clear();
        volsteptimesArray_=Array(volstepdates_.size());
        int j=0;
        for(std::vector<Date>::const_iterator i = volstepdates_.begin(); i!= volstepdates_.end(); i++, j++) {
            volsteptimes_.push_back(termStructure()->timeFromReference(*i));
            volsteptimesArray_[j]=volsteptimes_[j];
            if(j==0) QL_REQUIRE(volsteptimes_[0] > 0.0,"volsteptimes must be positive (" << volsteptimes_[0] << ")");
            else QL_REQUIRE(volsteptimes_[j] > volsteptimes_[j-1],"volsteptimes must be strictly increasing (" << volsteptimes_[j-1] << "@" << (j-1) << ", " << volsteptimes_[j] << "@" << j << ")");
        }

        std::vector<Date>::const_iterator i;
        if(capletCalibrated_) {
            for(i = capletExpiries_.begin() ; i != capletExpiries_.end() ; i++) {
                makeCapletCalibrationPoint(*i);
            }
        }
        else {
            std::vector<Period>::const_iterator j;
            for(i = swaptionExpiries_.begin(), j = swaptionTenors_.begin() ; i != swaptionExpiries_.end() ; i++,j++) {
                makeSwaptionCalibrationPoint(*i,*j);
            }
        }

        bool done;
        numeraireDate_ = Date::minDate();
        do {
            Date numeraireKnown = numeraireDate_;
            done = true;
            for(std::map<Date,CalibrationPoint>::reverse_iterator i = calibrationPoints_.rbegin(); i != calibrationPoints_.rend() && done; i++) {
                if(i->second.paymentDates_.back() > numeraireDate_) {
                      numeraireDate_ = i->second.paymentDates_.back();
                      numeraireKnown = i->second.paymentDates_.back();
                      if(i != calibrationPoints_.rbegin()) {
                          done=false;
                      }
                }
                // Inlining this into the loop condition causes
                // a bogus compilation error wih g++ 4.0.1 on Mac OS X
                std::vector<Date>::const_reverse_iterator rend =
                    i->second.paymentDates_.rend();
                for(std::vector<Date>::const_reverse_iterator j = i->second.paymentDates_.rbegin(); j != rend && done; j++) {
                    if(*j < numeraireKnown) {
                        if(capletCalibrated_) {
                            makeCapletCalibrationPoint(*j);
                            done = false;
                            break;
                        }
                        else {
                            UpRounding rounder(0);
                            makeSwaptionCalibrationPoint(*j,Period(static_cast<Integer>(rounder((swapIndexBase_->dayCounter().yearFraction(*j,numeraireKnown)-0.5/365)*12.0)), Months));
                            done = false;
                            break;
                        }
                    }
                }
                if(done) {
                    numeraireKnown = i->first;
                }
            }
        } while(!done);

        numeraireTime_ = termStructure()->timeFromReference(numeraireDate_);

        times_.clear();
        times_.push_back(0.0);

        modelOutputs_.expiries_.clear();
        modelOutputs_.tenors_.clear();
        for(std::map<Date,CalibrationPoint>::iterator k=calibrationPoints_.begin(); k != calibrationPoints_.end() ; k++) {
            times_.push_back(termStructure()->timeFromReference(k->first));
            modelOutputs_.expiries_.push_back(k->first);
            modelOutputs_.tenors_.push_back(k->second.tenor_);
        }
        times_.push_back(numeraireTime_);

        QL_REQUIRE(volatilities_.size() == volsteptimes_.size()+1,"there must be n+1 volatilities (" << volatilities_.size() << ") for n volatility step times (" << volsteptimes_.size() << ")");
        sigma_ = PiecewiseConstantParameter(volsteptimes_,PositiveConstraint());   
        for(Size i=0;i<sigma_.size();i++) {
            sigma_.setParam(i,volatilities_[i]);
        }

        stateProcess_ = boost::shared_ptr<MfStateProcess>(new MfStateProcess(reversion_(0.0),volsteptimesArray_,sigma_.params()));

        y_ = yGrid(modelSettings_.yStdDevs_,modelSettings_.yGridPoints_);

        discreteNumeraire_ = boost::shared_ptr<Matrix>(new Matrix(times_.size(),2*modelSettings_.yGridPoints_+1,1.0));
        for(Size i=0;i<times_.size();i++) {
            boost::shared_ptr<Interpolation> numInt(new CubicInterpolation(y_.begin(), y_.end(), discreteNumeraire_->row_begin(i), CubicInterpolation::Spline, true,
                                                                                                    CubicInterpolation::Lagrange,0.0,CubicInterpolation::Lagrange,0.0));
            numInt->enableExtrapolation();
            numeraire_.push_back(numInt);
        }


        LazyObject::registerWith(termStructure());
        if(!swaptionVol_.empty()) LazyObject::registerWith(swaptionVol_);
        if(!capletVol_.empty()) LazyObject::registerWith(capletVol_);

    }

    void MarkovFunctional::makeSwaptionCalibrationPoint(const Date& expiry, const Period& tenor) {

        QL_REQUIRE(calibrationPoints_.count(expiry)==0, "swaption expiry (" << expiry << ") occurs more than once in calibration set");
        
        CalibrationPoint p;
        p.isCaplet_ = false;
        p.tenor_ = tenor;
        
        SwapIndex tmpIndex(swapIndexBase_->familyName(),tenor,swapIndexBase_->fixingDays(), 
                            swapIndexBase_->currency(), swapIndexBase_->fixingCalendar(), swapIndexBase_->fixedLegTenor(),
                            swapIndexBase_->fixedLegConvention(), swapIndexBase_->dayCounter(),
                            swapIndexBase_->iborIndex());
        boost::shared_ptr<VanillaSwap> underlying = tmpIndex.underlyingSwap(expiry);
        Schedule sched = underlying->fixedSchedule();
        Calendar cal = sched.calendar();
        BusinessDayConvention bdc = underlying->paymentConvention();
        
        std::vector<Date> paymentDates;
        std::vector<Real> yearFractions;
        for(unsigned int k=1; k<sched.size(); k++) {
            yearFractions.push_back(swapIndexBase_->dayCounter().yearFraction( k==1 ? expiry : sched.date(k-1),sched.date(k))); // adjust the first period to start on the expiry
            paymentDates.push_back(cal.adjust(sched.date(k),bdc));
        }
        p.yearFractions_ = yearFractions;
        p.paymentDates_ = paymentDates;
        calibrationPoints_[expiry]=p;

    }

    void MarkovFunctional::makeCapletCalibrationPoint(const Date& expiry) {

        QL_REQUIRE(calibrationPoints_.count(expiry)==0, "caplet expiry (" << expiry << ") occurs more than once in calibration set");

        CalibrationPoint p;
        p.isCaplet_ = true;
        //p.expiry_ = expiry;
        p.tenor_ = iborIndex_->tenor();
        std::vector<Date> paymentDates;
        std::vector<Real> yearFractions;
        Date valueDate = iborIndex_->valueDate(expiry);
        Date endDate = iborIndex_->fixingCalendar().advance(valueDate,iborIndex_->tenor(),
            iborIndex_->businessDayConvention(),iborIndex_->endOfMonth()); // FIXME Here we should use a calculation date calendar ?
        paymentDates.push_back(endDate);
        yearFractions.push_back(iborIndex_->dayCounter().yearFraction(expiry,endDate)); // adjust the first period to start on expiry
        p.yearFractions_ = yearFractions;
        p.paymentDates_ = paymentDates;
        calibrationPoints_[expiry]=p;

    }

    void MarkovFunctional::updateSmiles() const {

        QL_MFMESSAGE(modelOutputs_,"updating smiles");
        modelOutputs_.dirty_=true;

        for(std::map<Date,CalibrationPoint>::reverse_iterator i = calibrationPoints_.rbegin(); i != calibrationPoints_.rend(); i++) {

            boost::shared_ptr<SmileSection> smileSection;
            if(i->second.isCaplet_) {
                i->second.annuity_= i->second.yearFractions_[0] * termStructure()->discount(i->second.paymentDates_[0],true);
                i->second.atm_= (termStructure()->discount(i->first,true) - termStructure()->discount(i->second.paymentDates_[0],true)) / i->second.annuity_;
                smileSection=capletVol_->smileSection(i->first,true);
            }
            else {
                Real annuity=0.0;
                for(unsigned int k=0; k<i->second.paymentDates_.size(); k++) {
                    annuity += i->second.yearFractions_[k] * termStructure()->discount(i->second.paymentDates_[k],true);
                }
                i->second.annuity_=annuity;
                i->second.atm_= (termStructure()->discount(i->first,true) - termStructure()->discount(i->second.paymentDates_.back(),true) ) / annuity; 
                smileSection=swaptionVol_->smileSection(i->first,i->second.tenor_,true);
            }

            i->second.rawSmileSection_ = boost::shared_ptr<SmileSection>(new AtmSmileSection(smileSection,i->second.atm_));

            switch(modelSettings_.smilePretreatment_) {
                    case ModelSettings::NoPretreatment:
                        i->second.smileSection_ = i->second.rawSmileSection_;
                        break;
                    case ModelSettings::KahaleExtrapolation:
                        i->second.smileSection_ = boost::shared_ptr<KahaleSmileSection>(new KahaleSmileSection(i->second.rawSmileSection_,i->second.atm_,false,modelSettings_.smileMoneynessCheckpoints_,modelSettings_.digitalGap_));
                        break;
                    default:
                        QL_FAIL("Unknown Smile Pretreatment (" << modelSettings_.smilePretreatment_ << ")");
                        break;
            }

            i->second.minRateDigital_ = i->second.smileSection_->digitalOptionPrice(modelSettings_.lowerRateBound_,Option::Call,i->second.annuity_,modelSettings_.digitalGap_);
            i->second.maxRateDigital_ = i->second.smileSection_->digitalOptionPrice(modelSettings_.upperRateBound_,Option::Call,i->second.annuity_,modelSettings_.digitalGap_);

            //TEST output smile section
            //{std::cout << "-----SMILE SECTION OUTPUT: " << i->first << " / " << i->second.tenor_ << std::endl;
            //Real strike=0.00001;
            //std::cout << "min / max = " << i->second.rawSmileSection_->minStrike() << "," << i->second.rawSmileSection_->minStrike() << std::endl;
            //std::cout << "arbitrage free = " << i->second.rawSmileSection_->arbitragefreeRegion().first << " / " << i->second.rawSmileSection_->arbitragefreeRegion().second << std::endl;
            //std::cout << "atm = " << i->second.smileSection_->atmLevel() << std::endl;
            //std::cout << "strike;volSource;volKahale;callSource;callKahale;digitalCallSource;digitalCallKahale;densitySource;densityKahale" << std::endl;
            //for(int j=0;j<1000;j++) {
            //  std::cout << strike << ";" << i->second.rawSmileSection_->volatility(strike) << ";" << i->second.smileSection_->volatility(strike) << ";" <<
            //      i->second.rawSmileSection_->optionPrice(strike,Option::Call) << ";" << i->second.smileSection_->optionPrice(strike,Option::Call) << ";" <<
            //      i->second.rawSmileSection_->digitalOptionPrice(strike,Option::Call,1.0) << ";" << i->second.smileSection_->digitalOptionPrice(strike,Option::Call,1.0) << ";" <<
            //      i->second.rawSmileSection_->density(strike,1.0) << ";" << i->second.smileSection_->density(strike,1.0) << std::endl;
            //  strike+=0.0010;
            //}}
            //END TEST

        }

    }

    void MarkovFunctional::updateNumeraireTabulation() const {

        QL_MFMESSAGE(modelOutputs_,"updating numeraire tabulation");
        modelOutputs_.dirty_=true;

        modelOutputs_.adjustmentFactors_.clear();
        modelOutputs_.digitalsAdjustmentFactors_.clear();

        int idx = times_.size()-2;

        for(std::map<Date,CalibrationPoint>::reverse_iterator i = calibrationPoints_.rbegin(); i != calibrationPoints_.rend(); i++,idx--) {

            Array discreteDeflatedAnnuities(y_.size(),0.0);
            Array deflatedFinalPayments;

            Real numeraire0=termStructure()->discount(numeraireTime_,true);
            Real normalization=termStructure()->discount(times_[idx],true) / numeraire0;

            for(unsigned int k=0;k<i->second.paymentDates_.size();k++) {
                deflatedFinalPayments = deflatedZerobond(termStructure()->timeFromReference(i->second.paymentDates_[k]),times_[idx],y_);
                discreteDeflatedAnnuities += deflatedFinalPayments*i->second.yearFractions_[k];
            }

            CubicInterpolation deflatedAnnuities(y_.begin(),y_.end(),discreteDeflatedAnnuities.begin(),CubicInterpolation::Spline,
                                                        true,CubicInterpolation::Lagrange,0.0,CubicInterpolation::Lagrange,0.0);
            deflatedAnnuities.enableExtrapolation();

            Real digitalsCorrectionFactor = 1.0;
            modelOutputs_.digitalsAdjustmentFactors_.insert(modelOutputs_.digitalsAdjustmentFactors_.begin(),digitalsCorrectionFactor);

            Real digital,swapRate,swapRate0;

            for(int c=0;c==0 || (c==1 && (modelSettings_.adjustments_ & ModelSettings::AdjustDigitals));c++) {

                if(c==1) {
                    digitalsCorrectionFactor = i->second.annuity_ / digital;
                    modelOutputs_.digitalsAdjustmentFactors_.front() = digitalsCorrectionFactor;
                }
                
                digital=0.0;
                swapRate0=modelSettings_.upperRateBound_/2.0; // initial guess
                for(int j=y_.size()-1;j>=0;j--) {

                    Real integral = 0.0;

                    if(j==(int)(y_.size()-1)) {
                        if((modelSettings_.adjustments_ & ModelSettings::NoPayoffExtrapolation) == 0) { 
                            if((modelSettings_.adjustments_ & ModelSettings::ExtrapolatePayoffFlat) != 0) {
                                integral = gaussianShiftedPolynomialIntegral(0.0,0.0,0.0,0.0,discreteDeflatedAnnuities[j-1],y_[j-1],y_[j], 100.0);
                            }
                            else {
                                Real ca = deflatedAnnuities.aCoefficients()[j-1];
                                Real cb = deflatedAnnuities.bCoefficients()[j-1];
                                Real cc = deflatedAnnuities.cCoefficients()[j-1];
                                integral = gaussianShiftedPolynomialIntegral(0.0,cc,cb,ca,discreteDeflatedAnnuities[j-1],y_[j-1],y_[j],100.0);
                            }
                        }
                    }
                    else {
                        Real ca = deflatedAnnuities.aCoefficients()[j];
                        Real cb = deflatedAnnuities.bCoefficients()[j];
                        Real cc = deflatedAnnuities.cCoefficients()[j];
                        integral = gaussianShiftedPolynomialIntegral(0.0,cc,cb,ca,discreteDeflatedAnnuities[j],y_[j],y_[j],y_[j+1]);
                    }

                    if(integral < 0) {
                        QL_MFMESSAGE(modelOutputs_,"WARNING: integral for digitalPrice is negative for j=" << j << " (" << integral << ") --- reset it to zero.");
                        integral = 0.0;
                    }

                    digital += integral * numeraire0 * digitalsCorrectionFactor;
            
                    if(digital >= i->second.minRateDigital_) swapRate = modelSettings_.lowerRateBound_;
                    else {
                        if(digital <= i->second.maxRateDigital_) swapRate = modelSettings_.upperRateBound_;
                        else {
                            swapRate = marketSwapRate(i->first,i->second,digital, swapRate0);
                            if(j < (int)y_.size()-1 && swapRate > swapRate0) {
                                QL_MFMESSAGE(modelOutputs_,"WARNING: swap rate is decreasing in y for t=" << times_[idx] << ", j=" << j << " (y, swap rate) is (" << y_[j] << "," << swapRate << ") but for j=" << j+1 << " it is (" << y_[j+1] << "," << swapRate0 << ") --- reset rate to " << swapRate0 << " in node j=" << j);
                                swapRate = swapRate0;
                            }
                        }
                    }
                    swapRate0 = swapRate; 
                    Real numeraire = 1.0 / (swapRate*discreteDeflatedAnnuities[j]+deflatedFinalPayments[j]);
                    (*discreteNumeraire_)[idx][j] = numeraire * normalization;
                }
            }

            if(modelSettings_.adjustments_ & ModelSettings::AdjustYts) {
                numeraire_[idx]->update();
                Real modelDeflatedZerobond = deflatedZerobond(times_[idx],0.0);
                Real marketDeflatedZerobond = termStructure()->discount(times_[idx],true) / termStructure()->discount(numeraireTime_,true);
                for(int j=y_.size()-1;j>=0;j--) {
                    (*discreteNumeraire_)[idx][j] *= modelDeflatedZerobond/marketDeflatedZerobond;
                }
                modelOutputs_.adjustmentFactors_.insert(modelOutputs_.adjustmentFactors_.begin(),modelDeflatedZerobond/marketDeflatedZerobond);
            }
            else {
                modelOutputs_.adjustmentFactors_.insert(modelOutputs_.adjustmentFactors_.begin(),1.0);
            }

            numeraire_[idx]->update();
        }

    }

    const MarkovFunctional::ModelOutputs& MarkovFunctional::modelOutputs() const {

        if(modelOutputs_.dirty_) {

            calculate();

            // yield term structure
            modelOutputs_.marketZerorate_.clear();
            modelOutputs_.modelZerorate_.clear();
            for(Size i=1;i<times_.size()-1;i++) {
                modelOutputs_.marketZerorate_.push_back(termStructure()->zeroRate(times_[i],QuantLib::Continuous,QuantLib::Annual));
                modelOutputs_.modelZerorate_.push_back( -std::log( this->zerobond(times_[i]) ) / times_[i]);
            }

            // volatility surface
            modelOutputs_.smileStrikes_.clear();
            modelOutputs_.marketCallPremium_.clear();
            modelOutputs_.marketPutPremium_.clear();
            modelOutputs_.modelCallPremium_.clear();
            modelOutputs_.modelPutPremium_.clear();
            modelOutputs_.marketVega_.clear();
            modelOutputs_.marketRawCallPremium_.clear();
            modelOutputs_.marketRawPutPremium_.clear();

            for(std::map<Date,CalibrationPoint>::iterator i=calibrationPoints_.begin(); i != calibrationPoints_.end(); i++) {
                modelOutputs_.atm_.push_back(i->second.atm_);
                modelOutputs_.annuity_.push_back(i->second.annuity_);
                boost::shared_ptr<SmileSection> sec = i->second.smileSection_;
                boost::shared_ptr<SmileSection> rawSec = i->second.rawSmileSection_;
                SmileSectionUtils ssutils;
                std::vector<Real> money = ssutils.makeMoneynessGrid(modelSettings_.smileMoneynessCheckpoints_);
                std::vector<Real> strikes, marketCall, marketPut, modelCall, modelPut, marketVega, marketRawCall, marketRawPut;
                for(Size j=0;j<money.size();j++) {
                    strikes.push_back(money[j]*i->second.atm_);
                    try {
                        marketRawCall.push_back(rawSec->optionPrice(strikes[j],Option::Call,i->second.annuity_));
                        marketRawPut.push_back(rawSec->optionPrice(strikes[j],Option::Put,i->second.annuity_));
                    } catch(QuantLib::Error) { // the smile section might not be able to output an option price because it has no atm level
                        marketRawCall.push_back(0.0);
                        marketRawPut.push_back(0.0);
                    }
                    marketCall.push_back(sec->optionPrice(strikes[j],Option::Call,i->second.annuity_));
                    marketPut.push_back(sec->optionPrice(strikes[j],Option::Put,i->second.annuity_));
                    modelCall.push_back(i->second.isCaplet_ ? capletPrice(Option::Call,i->first,strikes[j],Null<Date>(),0.0,true) : 
                                                              swaptionPrice(Option::Call,i->first,i->second.tenor_,strikes[j],Null<Date>(),0.0,true)); 
                    modelPut.push_back(i->second.isCaplet_ ? capletPrice(Option::Put,i->first,strikes[j],Null<Date>(),0.0,true) : 
                                                              swaptionPrice(Option::Put,i->first,i->second.tenor_,strikes[j],Null<Date>(),0.0,true)); 
                    marketVega.push_back(sec->vega(strikes[j],i->second.annuity_));
                }
                modelOutputs_.smileStrikes_.push_back(strikes);
                modelOutputs_.marketCallPremium_.push_back(marketCall);
                modelOutputs_.marketPutPremium_.push_back(marketPut);
                modelOutputs_.modelCallPremium_.push_back(modelCall);
                modelOutputs_.modelPutPremium_.push_back(modelPut);
                modelOutputs_.marketVega_.push_back(marketVega);
                modelOutputs_.marketRawCallPremium_.push_back(marketRawCall);
                modelOutputs_.marketRawPutPremium_.push_back(marketRawPut);
            }

            modelOutputs_.dirty_=false;

        }

        return modelOutputs_;

    }

    const Real MarkovFunctional::deflatedZerobond(Time T, Time t, Real y) const {

        calculate();
        Array ya(1,y);
        return deflatedZerobond(T,t,ya)[0];

    }

    const Disposable<Array> MarkovFunctional::deflatedZerobond(const Time T, const Time t, const Array& y) const {

        calculate();

        Array result(y.size(),0.0);

        // Gauss Hermite

        Real stdDev_0_t = stateProcess_->stdDeviation(0.0,0.0,t); // we use that the standard deviation is independent of $x$ here
        Real stdDev_0_T = stateProcess_->stdDeviation(0.0,0.0,T);
        Real stdDev_t_T = stateProcess_->stdDeviation(t,0.0,T-t);


        for(Size j=0;j<y.size();j++) {
            Array ya(modelSettings_.gaussHermitePoints_);
            for(Size i=0;i<modelSettings_.gaussHermitePoints_;i++) {
                ya[i] = (y[j]*stdDev_0_t + stdDev_t_T*normalIntegralX_[i]) / stdDev_0_T;
                
            }
            Array res=numeraire(T,ya);
            for(Size i=0;i<modelSettings_.gaussHermitePoints_;i++) {
                result[j]+=normalIntegralW_[i] / res[i];
            }
        }

        // Spline

        //Array z = yGrid(modelSettings_.yStdDevs_,modelSettings_.yGridPoints_);
        //Array p(z.size());

        //for(Size j=0;j<y.size();j++) {

        //  Array yg = yGrid(modelSettings_.yStdDevs_,modelSettings_.yGridPoints_,T,t,y[j]);

        //  for(Size i=0;i<yg.size();i++) {
        //      p[i] = 1.0 / numeraire(T,yg[i]);
        //  }

        //  CubicInterpolation payoff(z.begin(),z.end(),p.begin(),CubicInterpolation::Spline,true,CubicInterpolation::Lagrange,0.0,CubicInterpolation::Lagrange,0.0);

        //  Real price = 0.0;
        //  for(Size i=0;i<z.size()-1;i++) {
        //      price += gaussianShiftedPolynomialIntegral( 0.0, payoff.cCoefficients()[i], payoff.bCoefficients()[i], payoff.aCoefficients()[i], p[i], z[i], z[i], z[i+1] );
        //  }
        //  if(modelSettings_.extrapolatePayoff_) {
        //      if(modelSettings_.flatPayoffExtrapolation_) {
        //          price += gaussianShiftedPolynomialIntegral( 0.0, 0.0, 0.0, 0.0, p[z.size()-2], z[z.size()-2], z[z.size()-1], 100.0 );
        //          price += gaussianShiftedPolynomialIntegral( 0.0, 0.0, 0.0, 0.0, p[0], z[0], -100.0 , z[0] );
        //      }
        //      else {
        //          price += gaussianShiftedPolynomialIntegral( 0.0, payoff.cCoefficients()[z.size()-2], payoff.bCoefficients()[z.size()-2], payoff.aCoefficients()[z.size()-2], p[z.size()-2], z[z.size()-2], z[z.size()-1], 100.0 );
        //          price += gaussianShiftedPolynomialIntegral( 0.0, payoff.cCoefficients()[0], payoff.bCoefficients()[0], payoff.aCoefficients()[0], p[0], z[0], -100.0 , z[0] );
        //      }
        //  }

        //  result[j] = price;

        //}

        return result;

    }

    const Disposable<Array> MarkovFunctional::yGrid(const Real stdDevs, const int gridPoints, const Real T, const Real t, const Real y) const {

        Array result(2*gridPoints+1,0.0);

        Real stdDev_0_t = stateProcess_->stdDeviation(0.0,0.0,t); // we use that the standard deviation is independent of $x$ here
        Real stdDev_0_T = stateProcess_->stdDeviation(0.0,0.0,T);
        Real stdDev_t_T = stateProcess_->stdDeviation(t,0.0,T-t);

        Real h = stdDevs / ((Real)gridPoints);

        for(int j=-gridPoints;j<=gridPoints;j++) {
            result[j+gridPoints] = (y*stdDev_0_t + stdDev_t_T*((Real)j)*h) / stdDev_0_T;
        }

        return result;

    }
    
    const Real MarkovFunctional::zerobond(Time T, Time t, Real y) const {

        calculate();

        return deflatedZerobond(T,t,y)*numeraire(t,y);

    }

    const Real MarkovFunctional::zerobond(const Date& maturity, const Date& referenceDate, const Real y) const {

        calculate();

        return zerobond(termStructure()->timeFromReference(maturity),
            referenceDate == Null<Date>() ? 0.0 : termStructure()->timeFromReference(referenceDate),y);

    }

    const Real MarkovFunctional::numeraire(Time t, Real y) const {

        calculate();
        Array ay(1,y);
        return numeraire(t,ay)[0];

    }

    const Disposable<Array> MarkovFunctional::numeraire(const Time t, const Array& y) const {

        calculate();
        Array res(y.size(), termStructure()->discount(numeraireTime_,true));
        if(t<QL_EPSILON) return res;
        
        Real inverseNormalization=termStructure()->discount(numeraireTime_,true)/termStructure()->discount(t,true);

        Time tz = std::min(t,times_.back());
        Size i= std::min<Size> ( std::upper_bound(times_.begin(),times_.end()-1,t)-times_.begin(), times_.size()-1 );

        Real ta = times_[i-1];
        Real tb = times_[i];
        Real dt = tb-ta;

        for(Size j=0;j<y.size();j++) {
            Real yv = y[j];
            if(yv<y_.front()) yv=y_.front(); // FIXME flat extrapolation should be incoperated into interpolation object, see above
            if(yv>y_.back()) yv=y_.back();
            Real na= (*numeraire_[i-1])(yv);
            Real nb = (*numeraire_[i])(yv);
            res[j] = inverseNormalization / ( (tz-ta) / nb + (tb-tz) / na ) * dt;   // linear in reciprocal of normalized numeraire
        }

        return res;  

    }

    const Real MarkovFunctional::forwardRate(const Date& fixing, const Date& referenceDate, const Real y, const bool zeroFixingDays, boost::shared_ptr<IborIndex> iborIdx) const {

        calculate();

        if(!iborIdx) iborIdx = iborIndex_;

        Date valueDate = zeroFixingDays ? fixing : iborIdx->valueDate(fixing);
        Date endDate = iborIdx->fixingCalendar().advance(valueDate,iborIdx->tenor(),
                                iborIdx->businessDayConvention(),iborIdx->endOfMonth()); // FIXME Here we should use the calculation date calendar ?
        Real dcf = iborIdx->dayCounter().yearFraction(valueDate,endDate);

        return ( zerobond(zeroFixingDays? fixing : valueDate,referenceDate,y) - zerobond(endDate,referenceDate,y) ) / 
                            (dcf * zerobond(endDate,referenceDate,y));

    }

    const Real MarkovFunctional::swapRate(const Date& fixing, const Period& tenor, const Date& referenceDate, const Real y, bool zeroFixingDays, boost::shared_ptr<SwapIndex> swapIdx) const {

        calculate();

        if(!swapIdx) swapIdx = swapIndexBase_;
        QL_REQUIRE(swapIdx,"No swap index given");

        SwapIndex tmpIdx = SwapIndex(swapIdx->familyName(), tenor, swapIdx->fixingDays(),
                                 swapIdx->currency(), swapIdx->fixingCalendar(), swapIdx->fixedLegTenor(),
                                 swapIdx->fixedLegConvention(), swapIdx->dayCounter(), swapIdx->iborIndex());
        boost::shared_ptr<VanillaSwap> underlying = tmpIdx.underlyingSwap(fixing);
        Schedule sched = underlying->fixedSchedule();
        Real annuity = swapAnnuity(fixing,tenor,referenceDate,y,zeroFixingDays,swapIdx);
        Rate atm = ( zerobond(zeroFixingDays ? fixing : sched.dates().front(),referenceDate,y) - zerobond(sched.calendar().adjust(sched.dates().back(),underlying->paymentConvention()),referenceDate,y) ) / annuity;
        return atm;

    }

    const Real MarkovFunctional::swapAnnuity(const Date& fixing, const Period& tenor, const Date& referenceDate, const Real y, const bool zeroFixingDays, boost::shared_ptr<SwapIndex> swapIdx) const {

        calculate();

        if(!swapIdx) swapIdx = swapIndexBase_;
        QL_REQUIRE(swapIdx,"No swap index given");

        SwapIndex tmpIdx = SwapIndex(swapIdx->familyName(), tenor, swapIdx->fixingDays(),
                                 swapIdx->currency(), swapIdx->fixingCalendar(), swapIdx->fixedLegTenor(),
                                 swapIdx->fixedLegConvention(), swapIdx->dayCounter(), swapIdx->iborIndex());
        boost::shared_ptr<VanillaSwap> underlying = tmpIdx.underlyingSwap(fixing);
        Schedule sched = underlying->fixedSchedule();

        Real annuity=0.0;
        for(unsigned int j=1; j<sched.size(); j++) {
            annuity += zerobond(sched.calendar().adjust(sched.date(j),underlying->paymentConvention()),referenceDate,y) * 
                swapIdx->dayCounter().yearFraction( j==1 && zeroFixingDays ? fixing : sched.date(j-1) , sched.date(j) );
        }
        return annuity;

    }

    const Real MarkovFunctional::zerobondOption(const Option::Type& type, const Date& expiry, const Date& maturity, const Rate strike, const Date& referenceDate, const Real y) const {

        calculate();

        Time fixingTime = termStructure()->timeFromReference(expiry);
        Time referenceTime = referenceDate == Null<Date>() ? 0.0 : termStructure()->timeFromReference(referenceDate);

        Array yg = yGrid(modelSettings_.yStdDevs_,modelSettings_.yGridPoints_,fixingTime,referenceTime,y);
        Array z = yGrid(modelSettings_.yStdDevs_,modelSettings_.yGridPoints_);

        Array p(yg.size());

        for(Size i=0;i<yg.size();i++) {
            Real discount = zerobond(maturity,expiry,yg[i]);
            p[i] = std::max((type == Option::Call ? 1.0 : -1.0) * (discount-strike), 0.0 ) / numeraire(fixingTime,yg[i]);
        }

        CubicInterpolation payoff(z.begin(),z.end(),p.begin(),CubicInterpolation::Spline,true,CubicInterpolation::Lagrange,0.0,CubicInterpolation::Lagrange,0.0);

        Real price = 0.0;
        for(Size i=0;i<z.size()-1;i++) {
            price += gaussianShiftedPolynomialIntegral( 0.0, payoff.cCoefficients()[i], payoff.bCoefficients()[i], payoff.aCoefficients()[i], p[i], z[i], z[i], z[i+1] );
        }
        if((modelSettings_.adjustments_ & ModelSettings::NoPayoffExtrapolation) == 0) {
            if((modelSettings_.adjustments_ & ModelSettings::ExtrapolatePayoffFlat) != 0) {
                price += gaussianShiftedPolynomialIntegral( 0.0, 0.0, 0.0, 0.0, p[z.size()-2], z[z.size()-2], z[z.size()-1], 100.0 );
                price += gaussianShiftedPolynomialIntegral( 0.0, 0.0, 0.0, 0.0, p[0], z[0], -100.0 , z[0] );
            }
            else {
                if(type == Option::Call) price += gaussianShiftedPolynomialIntegral( 0.0, payoff.cCoefficients()[z.size()-2], payoff.bCoefficients()[z.size()-2], payoff.aCoefficients()[z.size()-2], p[z.size()-2], z[z.size()-2], z[z.size()-1], 100.0 );
                if(type == Option::Put) price += gaussianShiftedPolynomialIntegral( 0.0, payoff.cCoefficients()[0], payoff.bCoefficients()[0], payoff.aCoefficients()[0], p[0], z[0], -100.0 , z[0] );
            }
        }

        return numeraire(referenceTime,y) * price;

    }

    const Real MarkovFunctional::swaptionPrice(const Option::Type& type, const Date& expiry, const Period& tenor, const Rate strike, const Date& referenceDate, const Real y, const bool zeroFixingDays, boost::shared_ptr<SwapIndex> swapIdx) const {
        
        calculate();

        Time fixingTime = termStructure()->timeFromReference(expiry);
        Time referenceTime = referenceDate == Null<Date>() ? 0.0 : termStructure()->timeFromReference(referenceDate);

        Array yg = yGrid(modelSettings_.yStdDevs_,modelSettings_.yGridPoints_,fixingTime,referenceTime,y);
        Array z = yGrid(modelSettings_.yStdDevs_,modelSettings_.yGridPoints_);
        Array p(yg.size());

        for(Size i=0;i<yg.size();i++) {
            Real annuity=swapAnnuity(expiry,tenor,expiry,yg[i],zeroFixingDays,swapIdx);
            Rate atm=swapRate(expiry,tenor,expiry,yg[i],zeroFixingDays,swapIdx);
            p[i] = annuity * std::max((type == Option::Call ? 1.0 : -1.0) * (atm-strike), 0.0) / numeraire(fixingTime,yg[i]);
        }

        CubicInterpolation payoff(z.begin(),z.end(),p.begin(),CubicInterpolation::Spline,true,CubicInterpolation::Lagrange,0.0,CubicInterpolation::Lagrange,0.0);

        Real price = 0.0;
        for(Size i=0;i<z.size()-1;i++) {
            price += gaussianShiftedPolynomialIntegral( 0.0, payoff.cCoefficients()[i], payoff.bCoefficients()[i], payoff.aCoefficients()[i], p[i], z[i], z[i], z[i+1] );
        }
        if((modelSettings_.adjustments_ & ModelSettings::NoPayoffExtrapolation) == 0) {
            if((modelSettings_.adjustments_ & ModelSettings::ExtrapolatePayoffFlat) != 0) {
                price += gaussianShiftedPolynomialIntegral( 0.0, 0.0, 0.0, 0.0, p[z.size()-2], z[z.size()-2], z[z.size()-1], 100.0 );
                price += gaussianShiftedPolynomialIntegral( 0.0, 0.0, 0.0, 0.0, p[0], z[0], -100.0 , z[0] );
            }
            else {
                if(type == Option::Call) price += gaussianShiftedPolynomialIntegral( 0.0, payoff.cCoefficients()[z.size()-2], payoff.bCoefficients()[z.size()-2], payoff.aCoefficients()[z.size()-2], p[z.size()-2], z[z.size()-2], z[z.size()-1], 100.0 );
                if(type == Option::Put) price += gaussianShiftedPolynomialIntegral( 0.0, payoff.cCoefficients()[0], payoff.bCoefficients()[0], payoff.aCoefficients()[0], p[0], z[0], -100.0 , z[0] );
            }
        }


        return numeraire(referenceTime,y) * price;

    }

    const Real MarkovFunctional::capletPrice(const Option::Type& type, const Date& expiry, const Rate strike, const Date& referenceDate, const Real y, const bool zeroFixingDays, boost::shared_ptr<IborIndex> iborIdx) const {

        calculate();

        if(!iborIdx) iborIdx = iborIndex_;

        Time fixingTime = termStructure()->timeFromReference(expiry);
        Time referenceTime = referenceDate == Null<Date>() ? 0.0 : termStructure()->timeFromReference(referenceDate);

        Array yg = yGrid(modelSettings_.yStdDevs_,modelSettings_.yGridPoints_,fixingTime,referenceTime,y);
        Array z = yGrid(modelSettings_.yStdDevs_,modelSettings_.yGridPoints_);
        Array p(yg.size());

        Date valueDate = iborIdx->valueDate(expiry);
        Date endDate = iborIdx->fixingCalendar().advance(valueDate,iborIdx->tenor(),
                                iborIdx->businessDayConvention(),iborIdx->endOfMonth()); // FIXME Here we should use the calculation date calendar ?
        Real dcf = iborIdx->dayCounter().yearFraction(zeroFixingDays ? expiry : valueDate,endDate);


        for(Size i=0;i<yg.size();i++) {
            Real annuity=zerobond(endDate,expiry,yg[i]) * dcf;
            Rate atm=forwardRate(expiry,expiry,yg[i],zeroFixingDays,iborIdx);
            p[i] = annuity * std::max((type == Option::Call ? 1.0 : -1.0) * (atm-strike), 0.0 ) / numeraire(fixingTime,yg[i]);
        }

        CubicInterpolation payoff(z.begin(),z.end(),p.begin(),CubicInterpolation::Spline,true,CubicInterpolation::Lagrange,0.0,CubicInterpolation::Lagrange,0.0);

        Real price = 0.0;
        for(Size i=0;i<z.size()-1;i++) {
            price += gaussianShiftedPolynomialIntegral( 0.0, payoff.cCoefficients()[i], payoff.bCoefficients()[i], payoff.aCoefficients()[i], p[i], z[i], z[i], z[i+1] );
        }
        if((modelSettings_.adjustments_ & ModelSettings::NoPayoffExtrapolation) == 0) {
            if((modelSettings_.adjustments_ & ModelSettings::ExtrapolatePayoffFlat) != 0) {
                price += gaussianShiftedPolynomialIntegral( 0.0, 0.0, 0.0, 0.0, p[z.size()-2], z[z.size()-2], z[z.size()-1], 100.0 );
                price += gaussianShiftedPolynomialIntegral( 0.0, 0.0, 0.0, 0.0, p[0], z[0], -100.0 , z[0] );
            }
            else {
                if(type==Option::Call) price += gaussianShiftedPolynomialIntegral( 0.0, payoff.cCoefficients()[z.size()-2], payoff.bCoefficients()[z.size()-2], payoff.aCoefficients()[z.size()-2], p[z.size()-2], z[z.size()-2], z[z.size()-1], 100.0 );
                if(type==Option::Put) price += gaussianShiftedPolynomialIntegral( 0.0, payoff.cCoefficients()[0], payoff.bCoefficients()[0], payoff.aCoefficients()[0], p[0], z[0], -100.0 , z[0] );
            }
        }

        return numeraire(referenceTime,y) * price;

    }

    const Real MarkovFunctional::marketSwapRate(const Date& expiry, const CalibrationPoint& p, const Real digitalPrice, const Real guess) const {

        ZeroHelper z(this,expiry,p,digitalPrice);
        Brent b;
        Real solution=b.solve(z,modelSettings_.marketRateAccuracy_,std::max(std::min(guess,modelSettings_.upperRateBound_-0.00001),modelSettings_.lowerRateBound_+0.00001),modelSettings_.lowerRateBound_,modelSettings_.upperRateBound_);
        return solution;

    }

    const Real MarkovFunctional::marketDigitalPrice(const Date& expiry,const CalibrationPoint& p, const Option::Type& type, const Real strike) const {
        return p.smileSection_->digitalOptionPrice(strike,type,p.annuity_,modelSettings_.digitalGap_);

    }

    const Real MarkovFunctional::gaussianPolynomialIntegral(const Real a, const Real b, const Real c, const Real d, const Real e, const Real y0, const Real y1) const {
        #ifdef MF_ENABLE_NTL
            if(modelSettings_.enableNtl_) {
                const boost::math::ntl::RR aa=4.0*a, ba=2.0*M_SQRT2*b, ca=2.0*c, da=M_SQRT2*d;
                const boost::math::ntl::RR x0=y0*M_SQRT1_2, x1=y1*M_SQRT1_2;
                const boost::math::ntl::RR res = (0.125*(3.0*aa+2.0*ca+4.0*e)*boost::math::erf(x1)-1.0/(4.0*M_SQRTPI)*exp(-x1*x1)*(2.0*aa*x1*x1*x1+3.0*aa*x1+2.0*ba*(x1*x1+1.0)+2.0*ca*x1+2.0*da))-
                                                    (0.125*(3.0*aa+2.0*ca+4.0*e)*boost::math::erf(x0)-1.0/(4.0*M_SQRTPI)*exp(-x0*x0)*(2.0*aa*x0*x0*x0+3.0*aa*x0+2.0*ba*(x0*x0+1.0)+2.0*ca*x0+2.0*da));
                return NTL::to_double(res.value());
            }
        #endif
        const Real aa=4.0*a, ba=2.0*M_SQRT2*b, ca=2.0*c, da=M_SQRT2*d;
        const Real x0=y0*M_SQRT1_2, x1=y1*M_SQRT1_2;
        return (0.125*(3.0*aa+2.0*ca+4.0*e)*boost::math::erf(x1)-1.0/(4.0*M_SQRTPI)*exp(-x1*x1)*(2.0*aa*x1*x1*x1+3.0*aa*x1+2.0*ba*(x1*x1+1.0)+2.0*ca*x1+2.0*da))-
            (0.125*(3.0*aa+2.0*ca+4.0*e)*boost::math::erf(x0)-1.0/(4.0*M_SQRTPI)*exp(-x0*x0)*(2.0*aa*x0*x0*x0+3.0*aa*x0+2.0*ba*(x0*x0+1.0)+2.0*ca*x0+2.0*da));
    }

    const Real MarkovFunctional::gaussianShiftedPolynomialIntegral(const Real a, const Real b, const Real c, const Real d, const Real e, const Real h, const Real x0, const Real x1) const {
        return gaussianPolynomialIntegral(a,-4.0*a*h+b,6.0*a*h*h-3.0*b*h+c,-4*a*h*h*h+3.0*b*h*h-2.0*c*h+d,a*h*h*h*h-b*h*h*h+c*h*h-d*h+e,x0,x1);
    }

    std::ostream& operator<<(std::ostream& out, const MarkovFunctional::ModelOutputs& m) {
        out << "Markov functional model trace output " << std::endl;
        out << "Model settings" << std::endl;
        out << "Grid points y        : " << m.settings_.yGridPoints_ << std::endl;
        out << "Std devs y           : " << m.settings_.yStdDevs_ << std::endl;
        out << "Lower rate bound     : " << m.settings_.lowerRateBound_ << std::endl;
        out << "Upper rate bound     : " << m.settings_.upperRateBound_ << std::endl;
        out << "Gauss Hermite points : " << m.settings_.gaussHermitePoints_ << std::endl;
        out << "Digital gap          : " << m.settings_.digitalGap_ << std::endl;
        out << "Adjustments          : " << ((m.settings_.adjustments_ & MarkovFunctional::ModelSettings::AdjustDigitals) ? "AdjustDigitals " : "") 
                               << ((m.settings_.adjustments_ & MarkovFunctional::ModelSettings::AdjustYts) ? "AdjustYts " : "")
                               << std::endl;
        out << "Smile pretreatment   : " << ((m.settings_.smilePretreatment_ & MarkovFunctional::ModelSettings::NoPretreatment) ? "NoPretreatment" : "")
                                      << ((m.settings_.smilePretreatment_ & MarkovFunctional::ModelSettings::KahaleExtrapolation) ? "KahaleExtrapolation" : "")
                                      << std::endl;
        out << "Payoff Extrapolation : " << ((m.settings_.adjustments_ & MarkovFunctional::ModelSettings::NoPayoffExtrapolation) ? "No Extrapolation" : "Yes") << std::endl;
        out << "Flat Payoff Extrap.  : " << ((m.settings_.adjustments_ & MarkovFunctional::ModelSettings::ExtrapolatePayoffFlat) ? "Yes" : "No") << 
                                            ((m.settings_.adjustments_ & MarkovFunctional::ModelSettings::NoPayoffExtrapolation) ? " (overridden by No Extrapolation)" : "") << std::endl;
        out << "High precision / NTL : " << (m.settings_.enableNtl_ ? "On" : "Off") << std::endl;
        #ifdef MF_ENABLE_NTL
            out << "NTL Precision        : " << boost::math::ntl::RR::precision() << std::endl;
        #endif
        out << "Smile moneyness checkpoints: ";
        for(Size i=0;i<m.settings_.smileMoneynessCheckpoints_.size();i++) out << m.settings_.smileMoneynessCheckpoints_[i] << (i < m.settings_.smileMoneynessCheckpoints_.size()-1 ? ";" : "");
        out << std::endl;

        QL_REQUIRE(!m.dirty_,"model outputs are dirty");

        if(m.expiries_.size()==0) return out; // no trace information was collected so no output
        out << std::endl;
        out << "Messages:" << std::endl;
        for(std::vector<std::string>::const_iterator i=m.messages_.begin(); i != m.messages_.end() ; i++) out << (*i) << std::endl;
        out << std::endl << std::setprecision(16);
        out << "Yield termstructure fit:" << std::endl;
        out << "expiry;tenor;atm;annuity;digitalAdj;ytsAdj;marketzerorate;modelzerorate;diff(bp)" << std::endl;
        for(Size i=0;i<m.expiries_.size();i++) {
            out << m.expiries_[i] << ";" << m.tenors_[i] << ";" << m.atm_[i] << ";" << m.annuity_[i] << ";" << m.digitalsAdjustmentFactors_[i] << ";" << m.adjustmentFactors_[i] << ";" << m.marketZerorate_[i] << ";" << m.modelZerorate_[i] << ";" << (m.marketZerorate_[i]-m.modelZerorate_[i])*10000.0 << std::endl;
        }
        out << std::endl;
        out << "Volatility smile fit:" << std::endl;
        for(Size i=0;i<m.expiries_.size();i++) {
            std::ostringstream os;
            os << m.expiries_[i] << "/" << m.tenors_[i];
            std::string p = os.str();
            out << "strike("<<p<<");marketCallRaw("<<p<<";marketCall("<<p<<");modelCall("<<p<<");marketPutRaw("<<p<<");marketPut("<<p<<");modelPut("<<p<<");marketVega("<<p<<")" << (i<m.expiries_.size()-1 ? ";" : "");
        }
        out << std::endl;
        for(Size j=0;j<m.smileStrikes_[0].size();j++) {
            for(Size i=0;i<m.expiries_.size();i++) {
                out << m.smileStrikes_[i][j] << ";" << m.marketRawCallPremium_[i][j] << ";" << m.marketCallPremium_[i][j] << ";" << m.modelCallPremium_[i][j] << ";" << 
                                                       m.marketRawPutPremium_[i][j] << ";" << m.marketPutPremium_[i][j] << ";" << m.modelPutPremium_[i][j] << ";" << 
                                                       m.marketVega_[i][j] << (i<m.expiries_.size()-1 ? ";" : "");
            }
            out << std::endl;
        }
        return out;
    }


}

