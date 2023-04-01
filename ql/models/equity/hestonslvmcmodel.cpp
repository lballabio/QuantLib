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

/*! \file hestonslvmcmodel.cpp
*/

#include <ql/math/functional.hpp>
#include <ql/termstructures/volatility/equityfx/fixedlocalvolsurface.hpp>
#include <ql/models/equity/hestonslvmcmodel.hpp>
#include <ql/processes/hestonslvprocess.hpp>

#pragma push_macro("BOOST_DISABLE_ASSERTS")
#ifndef BOOST_DISABLE_ASSERTS
#define BOOST_DISABLE_ASSERTS
#endif
#include <boost/multi_array.hpp>
#pragma pop_macro("BOOST_DISABLE_ASSERTS")

#include <utility>

namespace QuantLib {
    HestonSLVMCModel::HestonSLVMCModel(
        Handle<LocalVolTermStructure> localVol,
        Handle<HestonModel> hestonModel,
        ext::shared_ptr<BrownianGeneratorFactory> brownianGeneratorFactory,
        const Date& endDate,
        Size timeStepsPerYear,
        Size nBins,
        Size calibrationPaths,
        const std::vector<Date>& mandatoryDates,
        const Real mixingFactor)
    : localVol_(std::move(localVol)), hestonModel_(std::move(hestonModel)),
      brownianGeneratorFactory_(std::move(brownianGeneratorFactory)), endDate_(endDate),
      nBins_(nBins), calibrationPaths_(calibrationPaths), mixingFactor_(mixingFactor) {

        registerWith(localVol_);
        registerWith(hestonModel_);

        const DayCounter dc = hestonModel_->process()->riskFreeRate()->dayCounter();
        const Date refDate = hestonModel_->process()->riskFreeRate()->referenceDate();

        std::vector<Time> gridTimes;
        gridTimes.reserve(mandatoryDates.size()+1);
        for (auto mandatoryDate : mandatoryDates) {
            gridTimes.push_back(dc.yearFraction(refDate, mandatoryDate));
        }
        gridTimes.push_back(dc.yearFraction(refDate, endDate));

        timeGrid_ = ext::make_shared<TimeGrid>(gridTimes.begin(), gridTimes.end(),
                std::max(Size(2), Size(gridTimes.back()*timeStepsPerYear)));
    }

    ext::shared_ptr<HestonProcess> HestonSLVMCModel::hestonProcess() const {
        return hestonModel_->process();
    }

    ext::shared_ptr<LocalVolTermStructure> HestonSLVMCModel::localVol() const {
        return localVol_.currentLink();
    }

    ext::shared_ptr<LocalVolTermStructure>
    HestonSLVMCModel::leverageFunction() const {
        calculate();

        return leverageFunction_;
    }

    void HestonSLVMCModel::performCalculations() const {
        const ext::shared_ptr<HestonProcess> hestonProcess
            = hestonModel_->process();
        const ext::shared_ptr<Quote> spot
            = hestonProcess->s0().currentLink();

        const Real v0            = hestonProcess->v0();
        const DayCounter dc      = hestonProcess->riskFreeRate()->dayCounter();
        const Date referenceDate = hestonProcess->riskFreeRate()->referenceDate();

        const Volatility lv0
            = localVol_->localVol(0.0, spot->value())/std::sqrt(v0);

        const ext::shared_ptr<Matrix> L(new Matrix(nBins_, timeGrid_->size()));

        std::vector<ext::shared_ptr<std::vector<Real> > >
            vStrikes(timeGrid_->size());
        for (Size i=0; i < timeGrid_->size(); ++i) {
            const Integer u = nBins_/2;
            const Real dx = spot->value()*std::sqrt(QL_EPSILON);

            vStrikes[i] = ext::make_shared<std::vector<Real> >(nBins_);

            for (Integer j=0; j < Integer(nBins_); ++j)
                vStrikes[i]->at(j) = spot->value() + (j - u)*dx;
        }

        std::fill(L->column_begin(0),L->column_end(0), lv0);

        leverageFunction_ = ext::make_shared<FixedLocalVolSurface>(
            referenceDate,
            std::vector<Time>(timeGrid_->begin(), timeGrid_->end()),
            vStrikes, L, dc);

        const ext::shared_ptr<HestonSLVProcess> slvProcess
            = ext::make_shared<HestonSLVProcess>(hestonProcess, leverageFunction_, mixingFactor_);

        std::vector<std::pair<Real, Real> > pairs(
                calibrationPaths_, std::make_pair(spot->value(), v0));

        const Size k = calibrationPaths_ / nBins_;
        const Size m = calibrationPaths_ % nBins_;

        const Size timeSteps = timeGrid_->size()-1;

        typedef boost::multi_array<Real, 3> path_type;
        path_type paths(boost::extents[calibrationPaths_][timeSteps][2]);

        const ext::shared_ptr<BrownianGenerator> brownianGenerator =
            brownianGeneratorFactory_->create(2, timeSteps);

        for (Size i=0; i < calibrationPaths_; ++i) {
            brownianGenerator->nextPath();
            std::vector<Real> tmp(2);
            for (Size j=0; j < timeSteps; ++j) {
                brownianGenerator->nextStep(tmp);
                paths[i][j][0] = tmp[0];
                paths[i][j][1] = tmp[1];
            }
        }

        for (Size n=1; n < timeGrid_->size(); ++n) {
            const Time t = timeGrid_->at(n-1);
            const Time dt = timeGrid_->dt(n-1);

            Array x0(2), dw(2);

            for (Size i=0; i < calibrationPaths_; ++i) {
                x0[0] = pairs[i].first;
                x0[1] = pairs[i].second;

                dw[0] = paths[i][n-1][0];
                dw[1] = paths[i][n-1][1];

                x0 = slvProcess->evolve(t, x0, dt, dw);

                pairs[i].first = x0[0];
                pairs[i].second = x0[1];
            }

            std::sort(pairs.begin(), pairs.end());

            Size s = 0U, e = 0U;
            for (Size i=0; i < nBins_; ++i) {
                const Size inc = k + static_cast<unsigned long>(i < m);
                e = s + inc;

                Real sum=0.0;
                for (Size j=s; j < e; ++j) {
                    sum+=pairs[j].second;
                }
                sum/=inc;

                vStrikes[n]->at(i) = 0.5*(pairs[e-1].first + pairs[s].first);
                (*L)[i][n] = std::sqrt(squared(localVol_->localVol(t, vStrikes[n]->at(i), true))/sum);

                s = e;
            }

            leverageFunction_->setInterpolation<Linear>();
        }
    }
}
