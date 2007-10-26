/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 François du Vignaud

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

/*! \file CapletVolatilitiesStructures.hpp
    \brief Caplet Volatilities Structures used during bootstrapping procedure
*/

#include <ql/termstructures/volatility/optionlet/capletvolatilitiesstructures.hpp>
#include <ql/instruments/capfloor.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/interpolations/bilinearinterpolation.hpp>



namespace {
    using namespace QuantLib;
    inline Real linearInterpolation(Real x, Real x1, Real x2,
                                            Real y1, Real y2){
        if (x == x1)
            return y1;
        return y1 + (x-x1)*(y2-y1)/(x2-x1);
    }

    Size upperIndex(const std::vector<Time>& times, Time time){
        if (time <= times.front())
            return 0;
        if (time >= times.back())
            return times.size();
        Size i = 1;
        while(time > times[i])
            i++;
        return i;
    }

    void findClosestBounds(Time time, const std::vector<Time>& times,
                                  Time& lowerBound, Time& higherBound){
        if (time <= times.front()){
            lowerBound = times.front();
            higherBound = lowerBound;
            return;
        }
        if (time >= times.back()){
            lowerBound = times.back();
            higherBound = lowerBound;
            return;
        }
        Size i = 1;
        while(time > times[i])
            i++;
        lowerBound = times[i-1];
        higherBound = times[i];
    }
}

namespace QuantLib {


    SmileSectionsVolStructure::SmileSectionsVolStructure(
               const Date& referenceDate,
               const DayCounter& dayCounter,
               const SmileSectionInterfaceVector& smileSections):
        OptionletVolatilityStructure(referenceDate),
        dayCounter_(dayCounter),
        tenorTimes_(smileSections.size()),
        smileSections_(smileSections) {
        QL_REQUIRE(!smileSections.empty(), "Smile Sections vector must \
                                                        not be empty!");
        for (Size i = 0; i < smileSections_.size(); i++){
            registerWith(smileSections[i]);
            tenorTimes_[i] = smileSections[i]->exerciseTime();
            if (i>0)
                QL_REQUIRE(tenorTimes_[i] > tenorTimes_[i-1],
                "Smile sections must in increasing time order !");
        }

        TermStructure::enableExtrapolation();

        minStrike_ = QL_MIN_REAL;
        maxStrike_ = QL_MAX_REAL;
        for (Size i=0; i < smileSections.size(); i++){
            if (smileSections[i]->minStrike() >  minStrike_)
                minStrike_ = smileSections[i]->minStrike();
            if (smileSections[i]->maxStrike() <  maxStrike_)
                maxStrike_ = smileSections[i]->maxStrike();
       }

       maxDate_ = smileSections.back()->exerciseDate();

    }

     Volatility SmileSectionsVolStructure::volatilityImpl(Time length,
            Rate strike) const {
            if (length <= tenorTimes_.front())
                return smileSections_.front()->volatility(strike);
            if (length >= tenorTimes_.back())
                return smileSections_.back()->volatility(strike);

            Size i = upperIndex(tenorTimes_, length);
            Volatility lowerVolatility =
                smileSections_[i-1]->volatility(strike);
            Volatility upperVolatility =
                smileSections_[i]->volatility(strike);

            return linearInterpolation(length, tenorTimes_[i-1],
                tenorTimes_[i], lowerVolatility, upperVolatility);
        }

     void SmileSectionsVolStructure::setClosestTenors(Time time,
         Time& nextLowerTenor, Time& nextHigherTenor) const {
            findClosestBounds(time, tenorTimes_,
                nextLowerTenor, nextHigherTenor);
        }


    Date SmileSectionsVolStructure::maxDate() const {
        return smileSections_.back()->exerciseDate(); }
    DayCounter SmileSectionsVolStructure::dayCounter() const {
        return dayCounter_;}
    Real SmileSectionsVolStructure::minStrike() const {
        return minStrike_;}
    Real SmileSectionsVolStructure::maxStrike() const {
        return maxStrike_;}

    BilinInterpCapletVolStructure::
        BilinInterpCapletVolStructure(
            const Date& referenceDate,
            const DayCounter dayCounter,
            const CapMatrix& referenceCaps,
            const std::vector<Rate>& strikes):
            ParametrizedCapletVolStructure(referenceDate),
            dayCounter_(dayCounter),
            tenorTimes_(referenceCaps.size()), strikes_(strikes),
            volatilities_(referenceCaps.size(), strikes.size(), .2) {

        /* we compute the times for which the volatilities points will
           be known*/
        for (Size i = 0; i < tenorTimes_.size(); i++){
            Date tenorDate = referenceCaps[i].front()->lastFloatingRateCoupon()->fixingDate();
            tenorTimes_[i] = dayCounter_.yearFraction(
                TermStructure::referenceDate(), tenorDate);
        }
        // the interpolator will be used for shorter maturities
        firstRowInterpolator_ = LinearInterpolation(strikes_.begin(),
            strikes_.end(), volatilities_.row_begin(0));
        firstRowInterpolator_.update();
        bilinearInterpolation_ = boost::shared_ptr<BilinearInterpolation>(
            new BilinearInterpolation(strikes_.begin(), strikes_.end(),
            tenorTimes_.begin(),tenorTimes_.end(), volatilities_));
        maxStrike_ = strikes_.back();
        minStrike_ = strikes_.front();
        maxDate_ = referenceCaps.back().front()->lastFloatingRateCoupon()->fixingDate();
    }


    Volatility BilinInterpCapletVolStructure::
        volatilityImpl(Time length, Rate strike) const {
        if (length < minTime())
            return firstRowInterpolator_(strike, true);
        else
            return bilinearInterpolation_->operator()(strike, length, true);
    }

    void BilinInterpCapletVolStructure::setClosestTenors(
                                            Time time, Time& nextLowerTenor,
                                            Time& nextHigherTenor){
        findClosestBounds(time, tenorTimes_, nextLowerTenor, nextHigherTenor);
    }

    Date BilinInterpCapletVolStructure::maxDate() const{
        return maxDate_; }

    DayCounter BilinInterpCapletVolStructure::
        dayCounter() const{ return dayCounter_;}

    Real BilinInterpCapletVolStructure::minStrike()
        const {return minStrike_;}

    Real BilinInterpCapletVolStructure::maxStrike()
        const {return maxStrike_;}


    DecInterpCapletVolStructure::
        DecInterpCapletVolStructure(
            const Date& referenceDate,
            const DayCounter dayCounter,
            const CapMatrix& referenceCaps,
            const std::vector<Rate>& strikes):
            ParametrizedCapletVolStructure(referenceDate),
            dayCounter_(dayCounter),
            tenorTimes_(referenceCaps.size()), strikes_(strikes),
            volatilities_(referenceCaps.size(), strikes_.size(), .2) {

        /* we compute the times for which the volatilities points will
           be known*/
        for (Size i = 0; i < tenorTimes_.size(); i++){
            Date tenorDate = referenceCaps[i].front()->lastFloatingRateCoupon()->fixingDate();
            tenorTimes_[i] = dayCounter_.yearFraction(
                TermStructure::referenceDate(), tenorDate);
            //volatilities_[i].resize(strikes_.size(), .2);
            boost::shared_ptr<Interpolation> newInterpolation(
                new LinearInterpolation(strikes_.begin(),
                    strikes_.end(), volatilities_[i]));
            newInterpolation->update();
            strikeInterpolations_.push_back(newInterpolation);
        }
        // the interpolator will be used for shorter maturities
        maxStrike_ = strikes_.back();
        minStrike_ = strikes_.front();
        maxDate_ = referenceCaps.back().front()->lastFloatingRateCoupon()->fixingDate();
    }


    Volatility DecInterpCapletVolStructure::
        volatilityImpl(Time length, Rate strike) const {
        Size i = upperIndex(tenorTimes_, length);
        if (i <= 0){
            strikeInterpolations_.front()->update();
            return strikeInterpolations_.front()->operator()(strike);
        }
        if (i ==tenorTimes_.size()){
            strikeInterpolations_.back()->update();
            return strikeInterpolations_.back()->operator()(strike);
        }
        strikeInterpolations_[i-1]->update();
        strikeInterpolations_[i]->update();
        Volatility lowerVolatiliy =
            strikeInterpolations_[i-1]->operator()(strike);
        Volatility higherVolatiliy =
            strikeInterpolations_[i]->operator()(strike);
        return linearInterpolation(length, tenorTimes_[i-1], tenorTimes_[i],
                lowerVolatiliy, higherVolatiliy);
    }



    void DecInterpCapletVolStructure::setClosestTenors(
                                            Time time, Time& nextLowerTenor,
                                            Time& nextHigherTenor){
        findClosestBounds(time, tenorTimes_, nextLowerTenor, nextHigherTenor);
    }

    Date DecInterpCapletVolStructure::maxDate() const{
        return maxDate_; }

    DayCounter DecInterpCapletVolStructure::
        dayCounter() const{ return dayCounter_;}

    Real DecInterpCapletVolStructure::minStrike()
        const {return minStrike_;}

    Real DecInterpCapletVolStructure::maxStrike()
        const {return maxStrike_;}

    void DecInterpCapletVolStructure::update() {
        for (Size i = 0; i < strikeInterpolations_.size(); ++i)
            strikeInterpolations_[i]->update();
    }


    HybridCapletVolatilityStructure::HybridCapletVolatilityStructure(
            const Date& referenceDate,
            const DayCounter dayCounter,
            const boost::shared_ptr<ParametrizedCapletVolStructure>&
            volatilitiesFromCaps,
            const boost::shared_ptr<SmileSectionsVolStructure>&
                shortTermCapletVolatilityStructure)
                           :ParametrizedCapletVolStructure(referenceDate),
                            dayCounter_(dayCounter),
                            volatilitiesFromCaps_(volatilitiesFromCaps),
                            shortTermCapletVolatilityStructure_
                            (shortTermCapletVolatilityStructure){

        registerWith(shortTermCapletVolatilityStructure);

        Time maxShortTermMaturity = shortTermCapletVolatilityStructure->
                                        maxTime();
            Time minCapMaturity = volatilitiesFromCaps_->minTime();
            overlapStart = std::min(maxShortTermMaturity, minCapMaturity);
            overlapEnd = std::max(maxShortTermMaturity, minCapMaturity);
    }

    Volatility HybridCapletVolatilityStructure::volatilityImpl(
                              Time length,
                              Rate strike) const {
            if (length < overlapStart)
                return shortTermCapletVolatilityStructure_->volatility(length,
                strike, true);
            if (length > overlapEnd)
                return volatilitiesFromCaps_->volatility(length, strike,
                                                        true);

            Time nextLowerFutureTenor, nextHigherFutureTenor,
                nextLowerCapTenor, nextHigherCapTenor,
                nextLowerTenor, nextHigherTenor;
            Volatility volAtNextLowerTenor, volAtNextHigherTenor;

            volatilitiesFromCaps_->setClosestTenors(length,
                nextLowerCapTenor, nextHigherCapTenor);

            shortTermCapletVolatilityStructure_->setClosestTenors(length,
                nextLowerFutureTenor, nextHigherFutureTenor);

            /* we determine which volatility surface should be used for the
               lower value*/
            if (nextLowerCapTenor < nextLowerFutureTenor) {
                nextLowerTenor = nextLowerFutureTenor;
                volAtNextLowerTenor = shortTermCapletVolatilityStructure_->
                    volatility(nextLowerTenor, strike, true);
            } else {
                nextLowerTenor = nextLowerCapTenor;
                volAtNextLowerTenor = volatilitiesFromCaps_->volatility(
                    nextLowerTenor, strike, true);
            }

            /* we determine which volatility surface should be used for
               the higher value*/
            if (nextHigherCapTenor < nextHigherFutureTenor){
                nextHigherTenor = nextHigherCapTenor;
                volAtNextHigherTenor = volatilitiesFromCaps_->volatility(
                    nextHigherTenor, strike, true);
            }else{
                nextHigherTenor = nextHigherFutureTenor;
                volAtNextHigherTenor = shortTermCapletVolatilityStructure_->
                    volatility(nextHigherTenor, strike, true);
            }

            return linearInterpolation(length, nextLowerTenor,
                nextHigherTenor, volAtNextLowerTenor, volAtNextHigherTenor);
    }

}

