/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Johannes Goettker-Schnetmann
 Copyright (C) 2015 Klaus Spanderen

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


#include <ql/timegrid.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/models/equity/hestonslvmodel.hpp>
#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>

#include <ql/experimental/finitedifferences/localvolrndcalculator.hpp>

#include <iostream>

namespace QuantLib {

	HestonSLVModel::HestonSLVModel(
		const Handle<LocalVolTermStructure>& localVol,
		const Handle<HestonModel>& hestonModel,
		const HestonSLVFokkerPlanckFdmParams& params,
		const std::vector<Date>& mandatoryDates)
		: localVol_(localVol),
		  hestonModel_(hestonModel),
		  params_(params),
		  mandatoryDates_(mandatoryDates) {

		registerWith(hestonModel_);
		registerWith(localVol_);
    }

	void HestonSLVModel::update() {
		notifyObservers();
	}

	boost::shared_ptr<HestonProcess> HestonSLVModel::hestonProcess() const {
		return hestonModel_->process();
	}

	boost::shared_ptr<LocalVolTermStructure> HestonSLVModel::localVol() const {
		return localVol_.currentLink();
	}

	boost::shared_ptr<LocalVolTermStructure>
	HestonSLVModel::leverageFunction() const {
		calculate();

		return leverageFunction_;
	}

    void HestonSLVModel::performCalculations() const {
    	const boost::shared_ptr<HestonProcess> hestonProcess
			= hestonModel_->process();
    	const boost::shared_ptr<Quote> spot
			= hestonProcess->s0().currentLink();
    	const boost::shared_ptr<YieldTermStructure> rTS
    		= hestonProcess->riskFreeRate().currentLink();
    	const boost::shared_ptr<YieldTermStructure> qTS
    		= hestonProcess->dividendYield().currentLink();

    	const DayCounter dc = rTS->dayCounter();
    	const Date referenceDate = rTS->referenceDate();

    	const Time T = dc.yearFraction(
    		referenceDate, params_.finalCalibrationMaturity);

    	QL_REQUIRE(referenceDate < params_.finalCalibrationMaturity,
    		"reference date must be smaller than final calibration date");

    	QL_REQUIRE(localVol_->maxTime() >= T,
    		"final calibration maturity exceeds local volatility surface");

		const Time maxDt = 1.0/params_.tMaxStepsPerYear;
		const Time minDt = 1.0/params_.tMinStepsPerYear;

		Time t=0.0;
		std::vector<Time> times(1, t);
		times.reserve(T*params_.tMinStepsPerYear);
		while (t < T) {
			const Real decayFactor = std::exp(-params_.tStepNumberDecay*t);
			const Time dt = maxDt*decayFactor + minDt*(1.0-decayFactor);

			times.push_back(std::min(T, t+=dt));
		}
		for (Size i=0; i < mandatoryDates_.size(); ++i) {
			times.push_back(
				dc.yearFraction(referenceDate, mandatoryDates_[i]));
		}

		const boost::shared_ptr<TimeGrid> timeGrid(
			new TimeGrid(times.begin(), times.end()));

		LocalVolRNDCalculator localVolRND(
			spot, rTS, qTS, localVol_.currentLink(),
			timeGrid, params_.xGrid,
			params_.epsProbability, params_.undefinedlLocalVolOverwrite,
			params_.maxIntegrationIterations, params_.firstAnalyticalStepTime);

		const std::vector<Size> xRescaleTimeSteps
			= localVolRND.rescaleTimeSteps();

//		const boost::shared_ptr<Quote>& spot,
//				const boost::shared_ptr<YieldTermStructure>& rTS,
//				const boost::shared_ptr<YieldTermStructure>& qTS,
//				const boost::shared_ptr<LocalVolTermStructure>& localVol,
//				Size xGrid = 101, Size tGrid = 51,
    }

}

