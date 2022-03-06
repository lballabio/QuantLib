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

/*! \file swaptionvoldiscrete.hpp
    \brief Discretized swaption volatility
*/

#ifndef quantlib_swaption_volatility_discrete_h
#define quantlib_swaption_volatility_discrete_h

#include <ql/termstructures/volatility/swaption/swaptionvolstructure.hpp>
#include <ql/math/interpolation.hpp>
#include <ql/patterns/lazyobject.hpp>

namespace QuantLib {

    class SwaptionVolatilityDiscrete : public LazyObject,
                                       public SwaptionVolatilityStructure {
      public:
        SwaptionVolatilityDiscrete(const std::vector<Period>& optionTenors,
                                   const std::vector<Period>& swapTenors,
                                   Natural settlementDays,
                                   const Calendar& cal,
                                   BusinessDayConvention bdc,
                                   const DayCounter& dc);
        SwaptionVolatilityDiscrete(const std::vector<Period>& optionTenors,
                                   const std::vector<Period>& swapTenors,
                                   const Date& referenceDate,
                                   const Calendar& cal,
                                   BusinessDayConvention bdc,
                                   const DayCounter& dc);
        SwaptionVolatilityDiscrete(const std::vector<Date>& optionDates,
                                   const std::vector<Period>& swapTenors,
                                   const Date& referenceDate,
                                   const Calendar& cal,
                                   BusinessDayConvention bdc,
                                   const DayCounter& dc);
        const std::vector<Period>& optionTenors() const;
        const std::vector<Date>& optionDates() const;
        const std::vector<Time>& optionTimes() const;
        const std::vector<Period>& swapTenors() const;
        const std::vector<Time>& swapLengths() const;
        //@}
        //! \name Observer interface
        //@{
        void update() override;
        //@}
        //! \name LazyObject interface
        //@{
        void performCalculations() const override;
        //@}
        //! additional inspectors
        Date optionDateFromTime(Time optionTime) const;

      protected:
        Size nOptionTenors_;
        std::vector<Period> optionTenors_;
        mutable std::vector<Date> optionDates_;
        mutable std::vector<Time> optionTimes_;
        mutable std::vector<Real> optionDatesAsReal_;
        mutable Interpolation optionInterpolator_;

        Size nSwapTenors_;
        std::vector<Period> swapTenors_;
        mutable std::vector<Time> swapLengths_;
        mutable Date evaluationDate_;
      private:
        void checkOptionTenors() const;
        void checkOptionDates(const Date& reference) const;
        void checkSwapTenors() const;
        void initializeOptionDatesAndTimes() const;
        void initializeOptionTimes() const;
        void initializeSwapLengths() const;
    };

    // inline

    inline const std::vector<Period>&
    SwaptionVolatilityDiscrete::optionTenors() const {
         return optionTenors_;
    }

    inline const std::vector<Date>&
    SwaptionVolatilityDiscrete::optionDates() const {
        return optionDates_;
    }

    inline const std::vector<Time>&
    SwaptionVolatilityDiscrete::optionTimes() const {
        return optionTimes_;
    }

    inline const std::vector<Period>&
    SwaptionVolatilityDiscrete::swapTenors() const {
     return swapTenors_;
    }

    inline const std::vector<Time>&
    SwaptionVolatilityDiscrete::swapLengths() const {
        return swapLengths_;
    }

    inline Date SwaptionVolatilityDiscrete::optionDateFromTime(Time optionTime) const {
        return Date(static_cast<Date::serial_type>(optionInterpolator_(optionTime)));
    }
}

#endif


#ifndef id_d73f615302d0cda3a0ab0f33f0c09d6d
#define id_d73f615302d0cda3a0ab0f33f0c09d6d
inline bool test_d73f615302d0cda3a0ab0f33f0c09d6d(const int* i) {
    return i != nullptr;
}
#endif
