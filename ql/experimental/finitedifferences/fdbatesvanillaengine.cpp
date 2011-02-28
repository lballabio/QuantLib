/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Klaus Spanderen

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

/*! \file fdbatesvanillaengine.cpp
    \brief Partial Integro Finite-Differences Bates vanilla option engine
*/


#include <ql/experimental/finitedifferences/fdbatesvanillaengine.hpp>
#include <ql/experimental/finitedifferences/fdhestonvanillaengine.hpp>

namespace QuantLib {

    FdBatesVanillaEngine::FdBatesVanillaEngine(
            const boost::shared_ptr<BatesModel>& model,
            Size tGrid, Size xGrid, 
            Size vGrid, Size dampingSteps,
            const FdmSchemeDesc& schemeDesc)
    : GenericModelEngine<BatesModel,
                         DividendVanillaOption::arguments,
                         DividendVanillaOption::results>(model),
      hestonEngine_(new FdHestonVanillaEngine(model, tGrid, xGrid, vGrid, 
                                              dampingSteps, schemeDesc)) {
    }

    void FdBatesVanillaEngine::calculate() const {
        hestonEngine_->calculate();
        
        results_ = *dynamic_cast<const DividendVanillaOption::results*>(
                                                hestonEngine_->getResults());
    }
    
    PricingEngine::arguments* FdBatesVanillaEngine::getArguments() const { 
        return hestonEngine_->getArguments(); 
    }
    
    const PricingEngine::results* FdBatesVanillaEngine::getResults() const {
        return hestonEngine_->getResults();
    }
    
    void FdBatesVanillaEngine::reset() {
        hestonEngine_->reset();
    }


}
