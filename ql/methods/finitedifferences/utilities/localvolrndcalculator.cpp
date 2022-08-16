/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Johannes Göttker-Schnetmann
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

/*! \file localvolendcalculator.cpp
    \brief local volatility risk neutral terminal density calculation
*/

#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/integrals/discreteintegrals.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/methods/finitedifferences/meshers/concentrating1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/meshers/predefined1dmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlocalvolfwdop.hpp>
#include <ql/methods/finitedifferences/schemes/douglasscheme.hpp>
#include <ql/methods/finitedifferences/utilities/localvolrndcalculator.hpp>
#include <ql/quote.hpp>
#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/timegrid.hpp>
#include <utility>


namespace QuantLib {
    LocalVolRNDCalculator::LocalVolRNDCalculator(
        ext::shared_ptr<Quote> spot,
        ext::shared_ptr<YieldTermStructure> rTS,
        ext::shared_ptr<YieldTermStructure> qTS,
        const ext::shared_ptr<LocalVolTermStructure>& localVol,
        Size xGrid,
        Size tGrid,
        Real x0Density,
        Real eps,
        Size maxIter,
        Time gaussianStepSize)
    : xGrid_(xGrid), tGrid_(tGrid), x0Density_(x0Density), localVolProbEps_(eps), maxIter_(maxIter),
      gaussianStepSize_(gaussianStepSize), spot_(std::move(spot)), localVol_(localVol),
      rTS_(std::move(rTS)), qTS_(std::move(qTS)),
      timeGrid_(new TimeGrid(localVol->maxTime(), tGrid)), xm_(tGrid),
      pm_(new Matrix(tGrid, xGrid)) {
        registerWith(spot_);
        registerWith(rTS_);
        registerWith(qTS_);
        registerWith(localVol_);
    }

    LocalVolRNDCalculator::LocalVolRNDCalculator(ext::shared_ptr<Quote> spot,
                                                 ext::shared_ptr<YieldTermStructure> rTS,
                                                 ext::shared_ptr<YieldTermStructure> qTS,
                                                 ext::shared_ptr<LocalVolTermStructure> localVol,
                                                 const ext::shared_ptr<TimeGrid>& timeGrid,
                                                 Size xGrid,
                                                 Real x0Density,
                                                 Real eps,
                                                 Size maxIter,
                                                 Time gaussianStepSize)
    : xGrid_(xGrid), tGrid_(timeGrid->size() - 1), x0Density_(x0Density), localVolProbEps_(eps),
      maxIter_(maxIter), gaussianStepSize_(gaussianStepSize), spot_(std::move(spot)),
      localVol_(std::move(localVol)), rTS_(std::move(rTS)), qTS_(std::move(qTS)),
      timeGrid_(timeGrid), xm_(tGrid_), pm_(new Matrix(tGrid_, xGrid_)) {
        registerWith(spot_);
        registerWith(rTS_);
        registerWith(qTS_);
        registerWith(localVol_);
    }

    Real LocalVolRNDCalculator::pdf(Real x, Time t) const {
        calculate();

        QL_REQUIRE(t > 0, "positive time expected");
        QL_REQUIRE(t <= timeGrid_->back(),
                "given time exceeds local vol time grid");

        const Time tMin = std::min(timeGrid_->at(1), 1.0/365);

        if (t <= tMin) {
            const Volatility vol = localVol_->localVol(0.0, spot_->value());
            const Volatility stdDev = vol * std::sqrt(t);
            const Real xm = - 0.5 * stdDev * stdDev +
                std::log(spot_->value() * qTS_->discount(t)/rTS_->discount(t));

            return GaussianDistribution(xm, stdDev)(x);
        }
        else if (t <= timeGrid_->at(1)) {
            const Volatility vol = localVol_->localVol(0.0, spot_->value());
            const Volatility stdDev = vol * std::sqrt(tMin);
            const Real xm = - 0.5 * stdDev * stdDev +
                std::log(spot_->value() * qTS_->discount(tMin)/rTS_->discount(tMin));

            const GaussianDistribution gaussianPDF(xm, stdDev);

            const Time deltaT = timeGrid_->at(1) - tMin;
            return gaussianPDF(x)*(timeGrid_->at(1) - t)/deltaT
                    + probabilityInterpolation(0, x)*(t - tMin)/deltaT;
        }
        else {
            const TimeGrid::const_iterator lb
                = std::lower_bound(timeGrid_->begin(), timeGrid_->end(), t);
            const TimeGrid::const_iterator llb = lb-1;

            const Size idx = std::distance(timeGrid_->begin(), lb)-1;

            const Time deltaT = *lb - *llb;
            return probabilityInterpolation(idx-1, x)*(*lb - t)/deltaT
                 + probabilityInterpolation(idx, x)*(t - *llb)/deltaT;
        }
    }

    Real LocalVolRNDCalculator::cdf(Real x, Time t) const {
        calculate();

        // get the left side of the integral
        const Time tc = timeGrid_->closestTime(t);
        const Size idx = (tc > t) ? timeGrid_->index(tc)-1
            : std::min(xm_.size()-1, timeGrid_->index(tc));

        Real xl = xm_[idx]->locations().front();
        Real xr = xm_[idx]->locations().back();

        if (x < xl)
            return 0.0;
        else if (x > xr)
            return 1.0;

        Real addition = 0.1*(xr-xl);

        // left or right hand integral
        if (x > 0.5*(xr+xl)) {
            while (pdf(xr, t) > 0.01*localVolProbEps_) 
            {
                 addition*=1.1;
                 xr+=addition;
            }

            return 1.0-GaussLobattoIntegral(maxIter_, 0.1*localVolProbEps_)(
                [&](Real _x){ return pdf(_x, t); }, x, xr);
        }
        else {
            while (pdf(xl, t) > 0.01*localVolProbEps_)
            {
                  addition=*1.1;
                  xl-=addition;
            }

            return GaussLobattoIntegral(maxIter_, 0.1*localVolProbEps_)(
                [&](Real _x){ return pdf(_x, t); }, xl, x);
        }
    }

    Real LocalVolRNDCalculator::invcdf(Real p, Time t) const {
        calculate();

        const Time closeGridTime(timeGrid_->closestTime(t));

        if (closeGridTime == 0.0) {
            const Real stepSize = 0.02*(
                    xm_[0]->locations().back() - xm_[0]->locations().front());
            return RiskNeutralDensityCalculator::InvCDFHelper(
                this, std::log(spot_->value()),
                0.1*localVolProbEps_, maxIter_, stepSize).inverseCDF(p, t);
        }
        else {
            Array xp(xGrid_);
            const Size idx = timeGrid_->index(closeGridTime)-1;

            const Array x(xm_[idx]->locations().begin(),
                          xm_[idx]->locations().end());
            const Real stepSize = 0.005*(x.back() - x.front());

            std::transform(x.begin(), x.end(), pm_->row_begin(idx), xp.begin(),
                           std::multiplies<Real>());

            const Real xm = DiscreteSimpsonIntegral()(x, xp);
            return RiskNeutralDensityCalculator::InvCDFHelper(
                this, xm, 0.1*localVolProbEps_, maxIter_, stepSize).inverseCDF(p, t);
        }
    }

    ext::shared_ptr<Fdm1dMesher>
    LocalVolRNDCalculator::mesher(Time t) const {
        calculate();

        const Size idx = timeGrid_->index(t);
        QL_REQUIRE(idx <= xm_.size(), "inconsistent time " << t << " given");

        if (idx > 0) {
            return xm_[idx-1];
        }
        else {
            return ext::make_shared<Predefined1dMesher>(
                std::vector<Real>(xGrid_, std::log(spot_->value())));
        }
    }

    ext::shared_ptr<TimeGrid> LocalVolRNDCalculator::timeGrid() const {
        return timeGrid_;
    }

    void LocalVolRNDCalculator::performCalculations() const {
        rescaleTimeSteps_.clear();

        const Time sT = timeGrid_->at(1);
        Time t = std::min(sT, (gaussianStepSize_ > 0.0) ? gaussianStepSize_
                                                        : 0.5*sT);
        const Volatility vol = localVol_->localVol(0.0, spot_->value());

        const Volatility stdDev = vol * std::sqrt(t);
        Real xm = - 0.5 * stdDev * stdDev +
            std::log(spot_->value() * qTS_->discount(t)/rTS_->discount(t));

        const Volatility stdDevOfFirstStep = vol * std::sqrt(sT);
        const Real normInvEps = InverseCumulativeNormal()(1 - localVolProbEps_);

        Real sLowerBound = xm - normInvEps * stdDevOfFirstStep;
        Real sUpperBound = xm + normInvEps * stdDevOfFirstStep;

        ext::shared_ptr<Fdm1dMesher> mesher(
            new Concentrating1dMesher(sLowerBound, sUpperBound, xGrid_,
                std::make_pair(xm, x0Density_), true));

        Array p(mesher->size());
        Array x(mesher->locations().begin(), mesher->locations().end());

        const GaussianDistribution gaussianPDF(xm, vol * std::sqrt(t));

        for (Size idx=0; idx < p.size(); ++idx) {
            p[idx] = gaussianPDF(x[idx]);
        }
        p = rescalePDF(x, p);

        QL_REQUIRE(x.size() > 10, "x grid is too small. "
                                  "Minimum size is greater than 10");

        const Size b = std::max(Size(1), Size(x.size()*0.04));

        ext::shared_ptr<DouglasScheme> evolver(
            new DouglasScheme(0.5,
                ext::make_shared<FdmLocalVolFwdOp>(
                    ext::make_shared<FdmMesherComposite>(mesher),
                    spot_, rTS_, qTS_, localVol_)));

        pFct_.resize(tGrid_);

        for (Size i=1; i <= tGrid_; ++i) {
            const Time dt = timeGrid_->at(i) - t;

            // leaking probability mass?
            const Real maxLeftValue =
                std::max(std::fabs(*std::min_element(p.begin(), p.begin()+b)),
                         std::fabs(*std::max_element(p.begin(), p.begin()+b)));
            const Real maxRightValue =
                std::max(std::fabs(*std::min_element(p.end()-b, p.end())),
                         std::fabs(*std::max_element(p.end()-b, p.end())));

            if (std::max(maxLeftValue, maxRightValue) > localVolProbEps_) {
                rescaleTimeSteps_.push_back(i);

                const Real oldLowerBound = sLowerBound;
                const Real oldUpperBound = sUpperBound;

                xm = DiscreteSimpsonIntegral()(x, x*p);
                Array vols(x.size());
                for (Size j=0; j < vols.size(); ++j) {
                    vols[j] = localVol_->localVol(t + dt, std::exp(x[j]));
                }

                const Real vm = DiscreteSimpsonIntegral()(x, vols)
                    /(x.back() - x.front());

                const Real scalingFactor = vm*std::sqrt(0.5*timeGrid_->back());

                if (maxLeftValue > localVolProbEps_)
                    sLowerBound -= scalingFactor*(oldUpperBound-oldLowerBound);
                if (maxRightValue > localVolProbEps_)
                    sUpperBound += scalingFactor*(oldUpperBound-oldLowerBound);

                mesher = ext::shared_ptr<Fdm1dMesher>(
                    new Concentrating1dMesher(sLowerBound, sUpperBound, xGrid_,
                        std::make_pair(xm, 0.1), false));

                const CubicNaturalSpline pSpline(x.begin(), x.end(), p.begin());
                const Array xn(mesher->locations().begin(),
                               mesher->locations().end());
                Array pn(xn.size(), 0.0);

                for (Size j=0; j < xn.size(); ++j) {
                    if (xn[j] >= oldLowerBound && xn[j] <= oldUpperBound)
                        pn[j] = pSpline(xn[j]);
                }

                x = xn;
                p = rescalePDF(xn, pn);

                evolver = ext::make_shared<DouglasScheme>(0.5,
                    ext::make_shared<FdmLocalVolFwdOp>(
                        ext::make_shared<FdmMesherComposite>(mesher),
                        spot_, rTS_, qTS_, localVol_));
            }
            evolver->setStep(dt);
            t+=dt;

            if (dt > QL_EPSILON) {
                evolver->step(p, t);
                p = rescalePDF(x, p);
            }

            xm_[i-1] = mesher;
            std::copy(p.begin(), p.end(), pm_->row_begin(i-1));
            pFct_[i-1] = ext::make_shared<CubicNaturalSpline>(
                xm_[i-1]->locations().begin(),
                xm_[i-1]->locations().end(),
                pm_->row_begin(i-1));
        }
    }


    std::vector<Size> LocalVolRNDCalculator::rescaleTimeSteps() const {
        calculate();

        return rescaleTimeSteps_;
    }

    Real LocalVolRNDCalculator::probabilityInterpolation(
        Size idx, Real x) const {
        calculate();

        if (   x < xm_[idx]->locations().front()
            || x > xm_[idx]->locations().back())
            return 0.0;
        else
            return (*pFct_[idx])(x);
    }

    Array LocalVolRNDCalculator::rescalePDF(const Array& x, const Array& p) const {
        return p/DiscreteSimpsonIntegral()(x, p);
    }

}

