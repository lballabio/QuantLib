/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2007 Katiuscia Manzoni

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

#include <ql/termstructures/volatility/interestrate/optionlet/optionletstripperadapter.hpp>
#include <ql/termstructures/volatility/interestrate/optionlet/optionletstripper.hpp>
#include <ql/termstructures/volatility/interestrate/capfloor/capfloortermvolsurface.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/interpolations/sabrinterpolation.hpp>

namespace QuantLib {

    OptionletStripperAdapter::OptionletStripperAdapter(
                const boost::shared_ptr<OptionletStripperBase>& s)
    : OptionletVolatilityStructure(s->termVolSurface()->settlementDays(),
                                   s->termVolSurface()->calendar(),
                                   s->termVolSurface()->businessDayConvention(),
                                   s->termVolSurface()->dayCounter()),
      optionletStripper_(s),
      nInterpolations_(s->optionletTimes().size()),
      strikeInterpolations_(nInterpolations_) {
        registerWith(optionletStripper_);
    }

    Volatility OptionletStripperAdapter::volatilityImpl(Time length,
                                                        Rate strike) const {
        calculate();

        std::vector<Volatility> vol(nInterpolations_);
        for (Size i=0; i<nInterpolations_; ++i)
            vol[i] = strikeInterpolations_[i]->operator()(strike);
        
        const std::vector<Time>& optionletTimes = optionletStripper_->optionletTimes();
        boost::shared_ptr<LinearInterpolation> timeInterpolator(new
            LinearInterpolation(optionletTimes.begin(), optionletTimes.end(),
                                vol.begin()));
        return timeInterpolator->operator()(length, true);
    }
        
    void OptionletStripperAdapter::performCalculations() const {

        const std::vector<Rate>& atmForward = optionletStripper_->atmOptionletRate();
        const std::vector<Time>& optionletTimes = optionletStripper_->optionletTimes();

        for (Size i=0; i<nInterpolations_; ++i) {
            const std::vector<Rate>& optionletStrikes =
                optionletStripper_->optionletStrikes(i);
            const std::vector<Volatility>& optionletVolatilities =
                optionletStripper_->optionletVolatilities(i);
            //strikeInterpolations_[i] = boost::shared_ptr<SABRInterpolation>(new
            //            SABRInterpolation(optionletStrikes.begin(), optionletStrikes.end(),
            //                              optionletVolatilities.begin(),
            //                              optionletTimes[i], atmForward[i],
            //                              0.02,0.5,0.2,0., 
            //                              false, true, false, false
            //                              //alphaGuess_, betaGuess_,
            //                              //nuGuess_, rhoGuess_,
            //                              //isParameterFixed_[0],
            //                              //isParameterFixed_[1],
            //                              //isParameterFixed_[2],
            //                              //isParameterFixed_[3]
            //                              ////,
            //                              //vegaWeightedSmileFit_,
            //                              //endCriteria_,
            //                              //optMethod_
            //                              ));
            strikeInterpolations_[i] = boost::shared_ptr<LinearInterpolation>(new 
                    LinearInterpolation(optionletStrikes.begin(),
                                       optionletStrikes.end(),
                                       optionletVolatilities.begin()));

            //QL_ENSURE(strikeInterpolations_[i]->endCriteria()!=EndCriteria::MaxIterations,
            //          "section calibration failed: "
            //          "option time " << optionletTimes[i] <<
            //          ": " <<
            //              ", alpha " <<  strikeInterpolations_[i]->alpha()<<
            //              ", beta "  <<  strikeInterpolations_[i]->beta() <<
            //              ", nu "    <<  strikeInterpolations_[i]->nu()   <<
            //              ", rho "   <<  strikeInterpolations_[i]->rho()  <<
            //              ", error " <<  strikeInterpolations_[i]->interpolationError()
            //              );

        }
    }

    Rate OptionletStripperAdapter::minStrike() const {
        return optionletStripper_->optionletStrikes(0).front(); //FIX
    }
    
    Rate OptionletStripperAdapter::maxStrike() const {
        return optionletStripper_->optionletStrikes(0).back(); //FIX
    }
    
    Date OptionletStripperAdapter::maxDate() const {
        return optionletStripper_->optionletDates().back();
    }
    
    const Date& OptionletStripperAdapter::referenceDate() const {
        return optionletStripper_->termVolSurface()->referenceDate();
    }
}
