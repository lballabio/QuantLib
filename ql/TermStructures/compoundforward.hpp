
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

    using Math::LinearInterpolation;
    using Math::LogLinearInterpolation;
    
    namespace TermStructures {

        class CompoundForward : public TermStructure {
          public:
            // constructor
            CompoundForward(
                const std::vector<Date>& inpDates,
                const std::vector<Rate>& inpFwdRates,
                Currency currency,
                const DayCounter& dayCounter,
                const Date& todaysDate,
                const Calendar& calendar,
                int settlementDays,
                RollingConvention roll,
                int compoundFrequency);
            CompoundForward(
                const std::vector<std::string>& inpPeriods,
                const std::vector<Rate>& inpFwdRates,
                Currency currency,
                const DayCounter& dayCounter,
                const Date& todaysDate,
                const Calendar& calendar,
                int settlementDays,
                RollingConvention roll,
                int compoundFrequency);
            Currency currency() const;
            DayCounter dayCounter() const;
            Date todaysDate() const;
            Calendar calendar() const;
            int settlementDays() const;
            Date settlementDate() const;
            RollingConvention rollingConvention() const;
            int compoundFrequency() const;
            const std::vector<Date>& dates() const;
            Date maxDate() const;
            Date minDate() const;
            const std::vector<Time>& times() const;
            Time maxTime() const;
            Time minTime() const;
          protected:
            void validateInputs() const;
            Rate zeroYieldImpl(Time, bool extrapolate = false) const;
            DiscountFactor discountImpl(Time,
                                        bool extrapolate = false) const;
            Rate forwardImpl(Time, bool extrapolate = false) const;
          private:
            // methods
            int referenceNode(Time t, bool extrapolate) const;
            void bootstrap() const;
            // data members
            Currency currency_;
            DayCounter dayCounter_;
            Date todaysDate_;
            Calendar calendar_;
            int settlementDays_;
            Date settlementDate_;
            RollingConvention roll_;
            int compoundFrequency_;
            mutable bool needsBootstrap_;
            mutable std::vector<Date> dates_;
            mutable std::vector<Rate> forwards_;
            mutable std::vector<Time> times_;
            mutable std::vector<DiscountFactor> discounts_;
            mutable std::vector<Rate> zeroYields_;
            typedef LinearInterpolation <
                std::vector<Time>::const_iterator,
                std::vector<Rate>::const_iterator> FwdInterpolation;
            typedef LogLinearInterpolation <
                std::vector <Time>::const_iterator,
                std::vector <DiscountFactor>::const_iterator> DfInterpolation;
            Handle <FwdInterpolation> fwdinterp_;
            Handle <DfInterpolation> dfinterp_;
        };
        
        // inline definitions

        inline Currency CompoundForward::currency() const {
            return currency_;
        }

        inline DayCounter CompoundForward::dayCounter() const {
            return dayCounter_;
        }

        inline Date CompoundForward::todaysDate() const {
            return todaysDate_;
        }

        inline Calendar CompoundForward::calendar() const {
            return calendar_;
        }

        inline int CompoundForward::settlementDays() const {
            return settlementDays_;
        }

        inline Date CompoundForward::settlementDate() const {
            return settlementDate_;
        }

        inline RollingConvention CompoundForward::rollingConvention() const {
            return roll_;
        }

        inline int CompoundForward::compoundFrequency() const {
            return compoundFrequency_;
        }

        inline const std::vector<Date>& CompoundForward::dates() const {
            return dates_;
        }

        inline Date CompoundForward::maxDate() const {
            return dates_.back();
        }
      
        inline Date CompoundForward::minDate() const {
            return settlementDate_;
        }

        inline const std::vector<Time>& CompoundForward::times() const {
            return times_;
        }

        inline Time CompoundForward::maxTime() const {
            return times_.back();
        }

        inline Time CompoundForward::minTime() const {
            return 0.0;
        }
        
    }

}


#endif
