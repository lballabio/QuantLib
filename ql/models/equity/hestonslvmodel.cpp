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
#include <ql/math/integrals/discreteintegrals.hpp>
#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>
#include <ql/methods/finitedifferences/meshers/predefined1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/concentrating1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/utilities/fdmmesherintegral.hpp>

#include <ql/experimental/finitedifferences/fdmhestonfwdop.hpp>
#include <ql/experimental/finitedifferences/localvolrndcalculator.hpp>
#include <ql/experimental/finitedifferences/squarerootprocessrndcalculator.hpp>

#include <boost/make_shared.hpp>
#include <boost/assign/std/vector.hpp>

using namespace boost::assign;

namespace QuantLib {

    namespace {
        boost::shared_ptr<Fdm1dMesher> varianceMesher(
            const SquareRootProcessRNDCalculator& rnd, Time t, Size vGrid) {

            const Real eps = 1e-4;
            const Real lowerBound = rnd.invcdf(eps, t);
            const Real upperBound = rnd.invcdf(1-eps, t);

            std::vector<boost::tuple<Real, Real, bool> > cPoints;

            const Real lowerBoundDensity = 0.1;
            cPoints += boost::make_tuple(
                lowerBound, lowerBoundDensity, false);

            return boost::make_shared<Concentrating1dMesher>(
                lowerBound, upperBound, vGrid, cPoints, 1e-8);
        }

        Disposable<Array> rescalePDF(
            const boost::shared_ptr<FdmMesherComposite>& mesher,
            const Array& p) {

            Array retVal = p/FdmMesherIntegral(
                mesher, DiscreteSimpsonIntegral()).integrate(p);

            return retVal;
        }
    }

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

        const Real v0    = hestonProcess->v0();
        const Real kappa = hestonProcess->kappa();
        const Real theta = hestonProcess->theta();
        const Real sigma = hestonProcess->sigma();
        const Real rho   = hestonProcess->rho();

        const Size xGrid = params_.xGrid;
        const Size vGrid = params_.vGrid;

        const DayCounter dc = rTS->dayCounter();
        const Date referenceDate = rTS->referenceDate();

        const Time T = dc.yearFraction(
            referenceDate, params_.finalCalibrationMaturity);

        QL_REQUIRE(referenceDate < params_.finalCalibrationMaturity,
            "reference date must be smaller than final calibration date");

        QL_REQUIRE(localVol_->maxTime() >= T,
            "final calibration maturity exceeds local volatility surface");

        // set-up exponential time step scheme
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

        // build 1d meshers
        const LocalVolRNDCalculator localVolRND(
            spot, rTS, qTS, localVol_.currentLink(),
            timeGrid, xGrid,
            params_.epsProbability,
            params_.undefinedlLocalVolOverwrite,
            params_.maxIntegrationIterations);

        const std::vector<Size> rescaleSteps
            = localVolRND.rescaleTimeSteps();

        const SquareRootProcessRNDCalculator squareRootRnd(
            v0, kappa, theta, sigma);

        std::vector<boost::shared_ptr<Fdm1dMesher> > xMesher, vMesher;
        xMesher.reserve(timeGrid->size());
        vMesher.reserve(timeGrid->size());

        xMesher.push_back(localVolRND.mesher(0.0));
        vMesher.push_back(boost::make_shared<Predefined1dMesher>(
            std::vector<Real>(vGrid, v0)));

        xMesher.push_back(localVolRND.mesher(timeGrid->at(1)));
        vMesher.push_back(
            varianceMesher(squareRootRnd, timeGrid->at(1), vGrid));

        for (Size i=2; i < timeGrid->size(); ++i) {
            xMesher.push_back(localVolRND.mesher(timeGrid->at(i)));

            const Size idx = std::distance(rescaleSteps.begin(),
                std::lower_bound(rescaleSteps.begin(),
                                 rescaleSteps.end(), i));

            if (i == rescaleSteps[idx-1]+1)
                vMesher.push_back(varianceMesher(squareRootRnd,
                    (idx < rescaleSteps.size())
                        ? timeGrid->at(rescaleSteps[idx])
                        : timeGrid->back(),
                    vGrid));
            else
                vMesher.push_back(vMesher.back());
        }

        // start probability distribution
        boost::shared_ptr<FdmMesherComposite> mesher
            = boost::make_shared<FdmMesherComposite>(
                xMesher.at(1), vMesher.at(1));

        const FdmSquareRootFwdOp::TransformationType trafoType
            = FdmSquareRootFwdOp::Plain;

        const Volatility lv0
            = localVol_->localVol(0.0, spot->value())/std::sqrt(v0);

        Array p = FdmHestonGreensFct(mesher, hestonProcess, trafoType, lv0)
            .get(timeGrid->at(1), params_.greensAlgorithm);

//        boost::shared_ptr<Matrix> L(new Matrix(xGrid, timeGrid->size()));
//        boost::shared_ptr<LocalVolTermStructure> leverageFct(
//            new FixedLocalVolSurface(
//                referenceDate,
//                std::vector<Time>(timeGrid->begin(), timeGrid->end()),
//                ds, L))


//        const boost::shared_ptr<FdmLinearOpComposite> hestonFwdOp(
//            new FdmHestonFwdOp(mesher, hestonProcess, trafoType, leverageFct));

//        const boost::shared_ptr<Quote>& spot,
//                const boost::shared_ptr<YieldTermStructure>& rTS,
//                const boost::shared_ptr<YieldTermStructure>& qTS,
//                const boost::shared_ptr<LocalVolTermStructure>& localVol,
//                Size xGrid = 101, Size tGrid = 51,
    }

}

