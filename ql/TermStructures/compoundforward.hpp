
/*
 Copyright (C) 2002 Andre Louw.

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file compoundforward.hpp
    \brief compounded forward term structure

    \fullpath
    ql/TermStructures/%compoundforward.hpp
*/

// $Id$

#ifndef quantlib_compoundforward_curve_h
#define quantlib_compoundforward_curve_h

#include <ql/termstructure.hpp>
#include <ql/calendar.hpp>
#include <ql/Math/loglinearinterpolation.hpp>
#include <ql/Math/linearinterpolation.hpp>

namespace QuantLib {

    namespace TermStructures {

        class CompoundForward : public DiscountStructure {
          public:
          // constructor
            CompoundForward(const Date& todaysDate,
                            const Date& referenceDate,
                            const Calendar& calendar,
                            const std::vector<Date>& inpDates,
                            const std::vector<Rate>& inpFwdRates,
                            const RollingConvention roll,
                            const int compoundFrequency,
                            const DayCounter& dayCounter);

            CompoundForward(const Date& todaysDate,
                            const Date& referenceDate,
                            const Calendar& calendar,
                            const std::vector<std::string>& inpPeriods,
                            const std::vector<Rate>& inpFwdRates,
                            const RollingConvention roll,
                            const int compoundFrequency,
                            const DayCounter& dayCounter);

            CompoundForward(const Date& todaysDate,
                            const Date& referenceDate,
                            const Calendar& calendar,
                            const std::vector<Period>& inpPeriods,
                            const std::vector<Rate>& inpFwdRates,
                            const RollingConvention roll,
                            const int compoundFrequency,
                            const DayCounter& dayCounter);

            DayCounter dayCounter() const;
            Calendar calendar() const;
            Date todaysDate() const {return todaysDate_; }
            Date referenceDate() const;
            RollingConvention roll() const;
            int compoundFrequency() const;
            const std::vector<Date>& dates() const;
            Date maxDate() const;
            const std::vector<Time>& times() const;
            Time maxTime() const;
            //@}
          protected:
            void validateInputs() const;
            DiscountFactor discountImpl(Time, bool extrapolate = false) const;
          private:
            // methods
            int referenceNode(Time t, bool extrapolate) const;
            void bootstrap() const;
            // data members
            Date todaysDate_, referenceDate_;
            Calendar calendar_;
            int settlementDays_;
            RollingConvention roll_;
            int compoundFrequency_;
            DayCounter dayCounter_;
            mutable bool needsBootstrap_;
            mutable std::vector<Date> inputDates_;
            mutable std::vector<Date> dates_;
            mutable std::vector<Rate> forwards_;
            mutable std::vector<Time> times_;
            mutable std::vector<DiscountFactor> discounts_;
            typedef Math::LinearInterpolation<
                std::vector<Time>::const_iterator,
                std::vector<Rate>::const_iterator> FwdInterpolation;
            typedef Math::LogLinearInterpolation<
                std::vector<Time>::const_iterator,
                std::vector<DiscountFactor>::const_iterator> DfInterpolation;
            mutable Handle<FwdInterpolation> fwdinterp_;
            mutable Handle<DfInterpolation>dfinterp_;
        };

        // inline definitions

        inline DayCounter CompoundForward::dayCounter() const {
            return dayCounter_;
        }

        inline Calendar CompoundForward::calendar() const {
            return calendar_;
        }

        inline Date CompoundForward::referenceDate() const {
            return referenceDate_;
        }

        inline RollingConvention CompoundForward::roll() const {
            return roll_;
        }

        inline int CompoundForward::compoundFrequency() const {
            return compoundFrequency_;
        }

        inline const std::vector<Date>& CompoundForward::dates() const {
            return inputDates_;
        }

        inline Date CompoundForward::maxDate() const {
            return dates_.back();
        }

        inline const std::vector<Time>& CompoundForward::times() const {
            return times_;
        }

        inline Time CompoundForward::maxTime() const {
            return times_.back();
        }

    }

}


#endif
