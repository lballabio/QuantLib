/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file swaptionvolcube.hpp
    \brief Swaption volatility cube
*/

#ifndef quantlib_swaption_volatility_cube_h
#define quantlib_swaption_volatility_cube_h

#include <ql/swaptionvolstructure.hpp>
#include <ql/Indexes/swapindex.hpp>
#include <ql/Math/matrix.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    class SwaptionVolatilityCube : public SwaptionVolatilityStructure {
      public:
        SwaptionVolatilityCube(
            const Handle<SwaptionVolatilityStructure>& atmVolStructure,
            const std::vector<Period>& expiries,
            const std::vector<Period>& lengths,
            const std::vector<Spread>& strikeSpreads,
            const std::vector<std::vector<Handle<Quote> > >& volSpreads,
            const boost::shared_ptr<SwapIndex>& swapIndexBase,
            bool vegaWeightedSmileFit);
        //! \name TermStructure interface
        //@{
        DayCounter dayCounter() const { return atmVol_->dayCounter(); }
        Date maxDate() const { return atmVol_->maxDate(); }
        Time maxTime() const { return atmVol_->maxTime(); }
        const Date& referenceDate() const { return atmVol_->referenceDate();}
        Calendar calendar() const { return atmVol_->calendar(); }
        //! \name SwaptionVolatilityStructure interface
        //@{
        Date maxStartDate() const { return atmVol_->maxStartDate(); }
        Time maxStartTime() const { return atmVol_->maxStartTime(); }
        Period maxLength() const { return atmVol_->maxLength(); }
        Time maxTimeLength() const { return atmVol_->maxTimeLength(); }
        Rate minStrike() const { return 0.0; }
        Rate maxStrike() const { return 1.0; }
        //@}
        //! \name Other inspectors
        //@{
        const Matrix& volSpreads(Size i) const { return volSpreadsMatrix_[i]; }
        virtual boost::shared_ptr<SmileSectionInterface> smileSection(
                                            const Date& optionDate,
                                            const Period& swapTenor) const = 0;
        virtual boost::shared_ptr<SmileSectionInterface> smileSection(
                                                  Time optionTime,
                                                  Time swapLength) const = 0;
        Rate atmStrike(const Period& optionTenor,
                       const Period& swapTenor) const {
            Date optionDate = calendar().advance(referenceDate(),
                                                 optionTenor,
                                                 Following); //FIXME
            return atmStrike(optionDate, swapTenor);
        }
        Rate atmStrike(const Date& optionDate,
                       const Period& swapTenor) const;
        //@}
      protected:
        //! \name SwaptionVolatilityStructure interface
        //@{
        std::pair<Time,Time> convertDates(const Date& optionDate,
                                          const Period& swapTenor) const {
            return atmVol_->convertDates(optionDate, swapTenor);
        }
        Volatility volatilityImpl(Time optionTime,
                                  Time swapLength,
                                  Rate strike) const;
        Volatility volatilityImpl(const Date& optionDate,
                                  const Period& swapTenor,
                                  Rate strike) const;
        //@}
        Handle<SwaptionVolatilityStructure> atmVol_;
        std::vector<Date> exerciseDates_;
        std::vector<Time> exerciseTimes_;
        std::vector<Real> exerciseDatesAsReal_;
        LinearInterpolation exerciseInterpolator_;
        std::vector<Period> lengths_;
        std::vector<Time> timeLengths_;
        Size nExercise_;
        Size nlengths_;
        Size nStrikes_;
        std::vector<Spread> strikeSpreads_;
        mutable std::vector<Rate> localStrikes_;
        mutable std::vector<Volatility> localSmile_;
        std::vector<std::vector<Handle<Quote> > > volSpreads_;
        std::vector<Matrix> volSpreadsMatrix_;
        boost::shared_ptr<SwapIndex> swapIndexBase_;
        bool vegaWeightedSmileFit_;
    };

    // inline

    inline Volatility SwaptionVolatilityCube::volatilityImpl(
                                                        Time optionTime,
                                                        Time swapLength,
                                                        Rate strike) const {
            return smileSection(optionTime, swapLength)->volatility(strike);
    }

    inline Volatility SwaptionVolatilityCube::volatilityImpl(
                                                    const Date& optionDate,
                                                    const Period& swapTenor,
                                                    Rate strike) const {
            return smileSection(optionDate, swapTenor)->volatility(strike);
    }

}

#endif
