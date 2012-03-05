/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Klaus Spanderen
 Copyright (C) 2007 StatPro Italia srl

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

#include <ql/pricingengines/vanilla/analytichestonhullwhiteengine.hpp>

namespace QuantLib {

    AnalyticHestonHullWhiteEngine::AnalyticHestonHullWhiteEngine(
                        const boost::shared_ptr<HestonModel>& hestonModel,
                        const boost::shared_ptr<HullWhite>& hullWhiteModel,
                        Size integrationOrder)
    : AnalyticHestonEngine(hestonModel, integrationOrder),
      hullWhiteModel_(hullWhiteModel) {

        update();
        registerWith(hullWhiteModel_);
    }

    AnalyticHestonHullWhiteEngine::AnalyticHestonHullWhiteEngine(
                    const boost::shared_ptr<HestonModel>& hestonModel,
                    const boost::shared_ptr<HullWhite>& hullWhiteModel,
                    Real relTolerance, Size maxEvaluations)
    : AnalyticHestonEngine(hestonModel, relTolerance, maxEvaluations),
      hullWhiteModel_(hullWhiteModel) {

        update();
        registerWith(hullWhiteModel_);
    }
        
    void AnalyticHestonHullWhiteEngine::update() {
        a_ = hullWhiteModel_->params()[0];
        sigma_ = hullWhiteModel_->params()[1];

        AnalyticHestonEngine::update();
    }

    void AnalyticHestonHullWhiteEngine::calculate() const {

        const Real t = model_->process()->time(arguments_.exercise->lastDate());
        if (a_*t > std::pow(QL_EPSILON, 0.25)) {
            m_ = sigma_*sigma_/(2*a_*a_)
                *(t+2/a_*std::exp(-a_*t)-1/(2*a_)*std::exp(-2*a_*t)-3/(2*a_));
        }
        else {
            // low-a algebraic limit
            m_ = 0.5*sigma_*sigma_*t*t*t*(1/3.0-0.25*a_*t+7/60.0*a_*a_*t*t);
        }

        AnalyticHestonEngine::calculate();
    }

}
