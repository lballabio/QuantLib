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

namespace QuantLib {

    class SwaptionVolatilityCube : public SwaptionVolatilityStructure {
      public:
        SwaptionVolatilityCube(
            const Handle<SwaptionVolatilityStructure>& atmVolStructure,
            const std::vector<Period>& expiries,
            const std::vector<Period>& lengths,
            const std::vector<Spread>& strikeSpreads,
            const Calendar& calendar,
            const boost::shared_ptr<SwapIndex>& swapIndexBase);
        //! \name TermStructure interface
        //@{
        const Date& referenceDate() const {
            return atmVolStructure_->referenceDate();
        }
        DayCounter dayCounter() const {
            return atmVolStructure_->dayCounter();
        }
        //@}
        //! \name SwaptionVolatilityStructure interface
        //@{
        Date maxStartDate() const { return atmVolStructure_->maxStartDate(); }
        Time maxStartTime() const { return atmVolStructure_->maxStartTime(); }
        Period maxLength() const { return atmVolStructure_->maxLength(); }
        Time maxTimeLength() const { return atmVolStructure_->maxTimeLength(); }
        Rate minStrike() const { return 0.0; }
        Rate maxStrike() const { return 1.0; }
        //@}
        Rate atmStrike(const Period& optionTenor,
                       const Period& swapTenor) const {
            Date optionDate = calendar().advance(referenceDate(),
                                                 optionTenor,
                                                 Following); //FIXME
            return atmStrike(optionDate, swapTenor);
        }
        Rate atmStrike(const Date& optionDate,
                       const Period& swapTenor) const;
    protected:
        //! \name SwaptionVolatilityStructure interface
        //@{
        std::pair<Time,Time> convertDates(const Date& exerciseDate,
                                          const Period& length) const {
            return atmVolStructure_->convertDates(exerciseDate, length);
        }
        //@}
        Handle<SwaptionVolatilityStructure> atmVolStructure_;
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
        boost::shared_ptr<SwapIndex> swapIndexBase_;
    };

}

#endif
