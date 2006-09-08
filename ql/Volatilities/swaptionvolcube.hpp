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

#include <ql/Volatilities/swaptionvolmatrix.hpp>
#include <ql/Instruments/vanillaswap.hpp>
#include <ql/Math/linearinterpolation.hpp>
#include <ql/Calendars/target.hpp>
#include <ql/swaptionvolstructure.hpp>

namespace QuantLib {

    class SwaptionVolatilityCube : public SwaptionVolatilityStructure {
    
    public:

        SwaptionVolatilityCube(
            const Handle<SwaptionVolatilityStructure>& atmVolStructure,
            const std::vector<Period>& expiries,
            const std::vector<Period>& lengths,
            const std::vector<Spread>& strikeSpreads,
            const Calendar& calendar,
			Integer swapSettlementDays,
            Frequency fixedLegFrequency,
            BusinessDayConvention fixedLegConvention,
            const DayCounter& fixedLegDayCounter,
            const boost::shared_ptr<Xibor>& iborIndex,
            Time shortTenor = 2,
            const boost::shared_ptr<Xibor>& iborIndexShortTenor = boost::shared_ptr<Xibor>());
    
        ~SwaptionVolatilityCube() {
        }

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
    
    protected:

        Handle<SwaptionVolatilityStructure> atmVolStructure_;
        std::vector<Date> exerciseDates_;
        std::vector<Time> exerciseTimes_;
        std::vector<Real> exerciseDatesAsReal_;
        LinearInterpolation exerciseInterpolator_;
        std::vector<Period> lengths_;
        std::vector<Time> timeLengths_;
        Size nStrikes_;
        std::vector<Spread> strikeSpreads_;
        mutable std::vector<Rate> localStrikes_;
        mutable std::vector<Volatility> localSmile_;
        Calendar calendar_;
        Integer swapSettlementDays_;
        Frequency fixedLegFrequency_;
        BusinessDayConvention fixedLegConvention_;
        DayCounter fixedLegDayCounter_;
        boost::shared_ptr<Xibor> iborIndex_;
        Time shortTenor_;
        boost::shared_ptr<Xibor> iborIndexShortTenor_;
    
    private:
    
    };
  
    class SwaptionVolatilityCubeByLinear : public SwaptionVolatilityCube {
      public:
        SwaptionVolatilityCubeByLinear(
            const Handle<SwaptionVolatilityStructure>& atmVolStructure,
            const std::vector<Period>& expiries,
            const std::vector<Period>& lengths,
            const std::vector<Spread>& strikeSpreads,
            const Matrix& volSpreads,
            const Calendar& calendar,
			Integer swapSettlementDays,
            Frequency fixedLegFrequency,
            BusinessDayConvention fixedLegConvention,
            const DayCounter& fixedLegDayCounter,
            const boost::shared_ptr<Xibor>& iborIndex,
            Time shortTenor = 2,
            const boost::shared_ptr<Xibor>& iborIndexShortTenor = boost::shared_ptr<Xibor>());


        //! \name Other inspectors
        //@{
        const Matrix& volSpreads(Size i) const { return volSpreads_[i]; }
        
        Rate atmStrike(const Date& start, const Period& length) const {
            std::pair<Time,Time> times = convertDates(start, length);
            return atmStrike(times.first, times.second);
        }
        //@}
      protected: 

        //! \name SwaptionVolatilityStructure interface
        //@{
        virtual std::pair<Time,Time> 
            convertDates(const Date& exerciseDate, const Period& length) const {
            return atmVolStructure_->convertDates(exerciseDate, length);
        }
        //@}

        boost::shared_ptr<Interpolation> smile(Time start,
                                               Time length) const;

       virtual boost::shared_ptr<VarianceSmileSection> smileSection(Time start, Time length) const;
       

        Rate atmStrike(Time start, Time length) const;

        Volatility volatilityImpl(Time start,
                                  Time length,
                                  Rate strike) const;
      private:

        std::vector<Matrix> volSpreads_;
        std::vector<Interpolation2D> volSpreadsInterpolator_;
    };

}

#endif
