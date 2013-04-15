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

#include <ql/termstructures/volatility/optionlet/strippedoptionletadapter.hpp>
#include <ql/termstructures/volatility/optionlet/optionletstripper.hpp>
#include <ql/termstructures/volatility/capfloor/capfloortermvolsurface.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/interpolations/sabrinterpolation.hpp>
#include <ql/termstructures/volatility/interpolatedsmilesection.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>

namespace QuantLib {

    StrippedOptionletAdapter::StrippedOptionletAdapter(
                const boost::shared_ptr<StrippedOptionletBase>& s)
    : OptionletVolatilityStructure(s->settlementDays(),
                                   s->calendar(),
                                   s->businessDayConvention(),
                                   s->dayCounter()),
      optionletStripper_(s),
      nInterpolations_(s->optionletMaturities()),
      strikeInterpolations_(nInterpolations_) {
        registerWith(optionletStripper_);
    }

    boost::shared_ptr<SmileSection>
    StrippedOptionletAdapter::smileSectionImpl(Time t) const {
         std::vector<Rate> optionletStrikes = optionletStripper_->optionletStrikes(0); // strikes are the same for all times ?!
         std::vector<Real> stddevs;
         for(Size i=0;i<optionletStrikes.size();i++) {
             stddevs.push_back(volatilityImpl(t,optionletStrikes[i])*std::sqrt(t));
         }
         // Extrapolation may be a problem with splines, but since minStrike() and maxStrike() are set, we assume that no one will use stddevs for strikes outside these strikes
         CubicInterpolation::BoundaryCondition bc = optionletStrikes.size()>=4 ? CubicInterpolation::Lagrange : CubicInterpolation::SecondDerivative;
         return boost::shared_ptr<SmileSection>(new InterpolatedSmileSection<Cubic>(t,optionletStrikes,stddevs,Null<Real>(),
                                                            Cubic(CubicInterpolation::Spline,false,bc,0.0,bc,0.0)));
    }

    Volatility StrippedOptionletAdapter::volatilityImpl(Time length,
                                                        Rate strike) const {
        calculate();

        std::vector<Volatility> vol(nInterpolations_);
        for (Size i=0; i<nInterpolations_; ++i)
            vol[i] = strikeInterpolations_[i]->operator()(strike, true);

        const std::vector<Time>& optionletTimes =
                                    optionletStripper_->optionletFixingTimes();
        boost::shared_ptr<LinearInterpolation> timeInterpolator(new
            LinearInterpolation(optionletTimes.begin(), optionletTimes.end(),
                                vol.begin()));
        return timeInterpolator->operator()(length, true);
    }

    void StrippedOptionletAdapter::performCalculations() const {

        //const std::vector<Rate>& atmForward = optionletStripper_->atmOptionletRate();
        //const std::vector<Time>& optionletTimes = optionletStripper_->optionletTimes();

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

    Rate StrippedOptionletAdapter::minStrike() const {
        return optionletStripper_->optionletStrikes(0).front(); //FIX
    }

    Rate StrippedOptionletAdapter::maxStrike() const {
        return optionletStripper_->optionletStrikes(0).back(); //FIX
    }

    Date StrippedOptionletAdapter::maxDate() const {
        return optionletStripper_->optionletFixingDates().back();
    }

}
