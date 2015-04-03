/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano

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

/*! \file swaptionvolcube.hpp
    \brief Swaption volatility cube
*/

#ifndef quantlib_swaption_volatility_cube_h
#define quantlib_swaption_volatility_cube_h

#include <ql/termstructures/volatility/swaption/swaptionvoldiscrete.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>

namespace QuantLib {

    class SwapIndex;
    class Quote;

    //! swaption-volatility cube
    /*! \warning this class is not finalized and its interface might
                 change in subsequent releases.
    */
    class SwaptionVolatilityCube : public SwaptionVolatilityDiscrete {
      public:
        SwaptionVolatilityCube(
            const Handle<SwaptionVolatilityStructure>& atmVolStructure,
            const std::vector<Period>& optionTenors,
            const std::vector<Period>& swapTenors,
            const std::vector<Spread>& strikeSpreads,
            const std::vector<std::vector<Handle<Quote> > >& volSpreads,
            const boost::shared_ptr<SwapIndex>& swapIndexBase,
            const boost::shared_ptr<SwapIndex>& shortSwapIndexBase,
            bool vegaWeightedSmileFit);
        //! \name TermStructure interface
        //@{
        DayCounter dayCounter() const { return atmVol_->dayCounter(); }
        Date maxDate() const { return atmVol_->maxDate(); }
        Time maxTime() const { return atmVol_->maxTime(); }
        const Date& referenceDate() const { return atmVol_->referenceDate();}
        Calendar calendar() const { return atmVol_->calendar(); }
        Natural settlementDays() const { return atmVol_->settlementDays(); }
        //! \name VolatilityTermStructure interface
        //@{
        Rate minStrike() const { return 0.0; }
        Rate maxStrike() const { return 1.0; }
        //@}
        //! \name SwaptionVolatilityStructure interface
        //@{
        const Period& maxSwapTenor() const { return atmVol_->maxSwapTenor(); }
        //@}
        //! \name Other inspectors
        //@{
        Rate atmStrike(const Date& optionDate,
                       const Period& swapTenor) const;
        Rate atmStrike(const Period& optionTenor,
                       const Period& swapTenor) const {
            Date optionDate = optionDateFromTenor(optionTenor);
            return atmStrike(optionDate, swapTenor);
        }
		Handle<SwaptionVolatilityStructure> atmVol() const { return atmVol_; }
        const std::vector<Spread>& strikeSpreads() const { return strikeSpreads_; }
        const std::vector<std::vector<Handle<Quote> > >& volSpreads() const { return volSpreads_; }
        const boost::shared_ptr<SwapIndex> swapIndexBase() const { return swapIndexBase_; }
        const boost::shared_ptr<SwapIndex> shortSwapIndexBase() const { return shortSwapIndexBase_; }
        const bool vegaWeightedSmileFit() const { return vegaWeightedSmileFit_; }
        //@}
        //! \name LazyObject interface
        //@{
        void performCalculations() const {
            QL_REQUIRE(nStrikes_ >= requiredNumberOfStrikes(),
                       "too few strikes (" << nStrikes_
                                           << ") required are at least "
                                           << requiredNumberOfStrikes());
            SwaptionVolatilityDiscrete::performCalculations();
        }
        //@}
        Real shift(Time optionTime, Time swapLength) const {
            return atmVol_->shift(optionTime,swapLength);
        }
      protected:
        void registerWithVolatilitySpread();
        virtual Size requiredNumberOfStrikes() const { return 2; }
        Volatility volatilityImpl(Time optionTime,
                                  Time swapLength,
                                  Rate strike) const;
        Volatility volatilityImpl(const Date& optionDate,
                                  const Period& swapTenor,
                                  Rate strike) const;
        Handle<SwaptionVolatilityStructure> atmVol_;
        Size nStrikes_;
        std::vector<Spread> strikeSpreads_;
        mutable std::vector<Rate> localStrikes_;
        mutable std::vector<Volatility> localSmile_;
        std::vector<std::vector<Handle<Quote> > > volSpreads_;
        boost::shared_ptr<SwapIndex> swapIndexBase_, shortSwapIndexBase_;
        bool vegaWeightedSmileFit_;
    };

    // inline

    inline Volatility SwaptionVolatilityCube::volatilityImpl(
                                                        Time optionTime,
                                                        Time swapLength,
                                                        Rate strike) const {
        return smileSectionImpl(optionTime, swapLength)->volatility(strike);
    }

    inline Volatility SwaptionVolatilityCube::volatilityImpl(
                                                    const Date& optionDate,
                                                    const Period& swapTenor,
                                                    Rate strike) const {
        return smileSectionImpl(optionDate, swapTenor)->volatility(strike);
    }

}

#endif
