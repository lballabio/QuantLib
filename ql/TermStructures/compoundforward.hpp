
/*
 Copyright (C) 2002, 2003 Decillion Pty(Ltd).

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file compoundforward.hpp
    \brief compounded forward term structure
*/

#ifndef quantlib_compoundforward_curve_h
#define quantlib_compoundforward_curve_h
 
#include <ql/termstructure.hpp>
#include <ql/Math/loglinearinterpolation.hpp>

namespace QuantLib {

    class CompoundForward : public ForwardRateStructure,
                            public Observer {
      public:
        // constructor
        CompoundForward(const Date & todaysDate,
                        const Date & referenceDate,
                        const std::vector<Date> &dates,
                        const std::vector<Rate> &forwards,
                        const Calendar & calendar,
                        const RollingConvention roll,
                        const int compounding,
                        const DayCounter & dayCounter);
        Date todaysDate() const { return todaysDate_; }
        Date referenceDate() const { return referenceDate_; };
        Calendar calendar() const { return calendar_; };
        RollingConvention roll() const { return roll_; };
        DayCounter dayCounter() const { return dayCounter_; };
        int compounding() const { return compounding_; };
        Date maxDate() const;
        Time maxTime() const;
        const std::vector<Time>& times() const;
        const std::vector<Date>& dates() const;
        const std::vector<Rate>& forwards() const;
        Handle<TermStructure> discountCurve() const;
        //! \name Observer interface
        //@{
        void update();
        //@}
      protected:
        // methods
        void calibrateNodes() const;
        Handle<TermStructure> bootstrap() const;
        Rate zeroYieldImpl(Time, bool extrapolate = false) const;
        DiscountFactor discountImpl(Time, bool extrapolate = false) const;
        int referenceNode(Time, bool extrapolate = false) const;
        Rate forwardImpl(Time, bool extrapolate = false) const;
        Rate compoundForwardImpl(Time, int, bool extrapolate = false) const;
      private:
        // data members
        Date todaysDate_;
        Date referenceDate_;
        DayCounter dayCounter_;
        Calendar calendar_;
        RollingConvention roll_;
        int compounding_;
        mutable bool needsBootstrap_;
        mutable std::vector<Date> dates_;
        mutable std::vector<Rate> forwards_;
        mutable std::vector<Time> times_;
        mutable Interpolation fwdinterp_;
        mutable Handle<TermStructure> discountCurve_;
    };

    // inline definitions

    inline Date CompoundForward::maxDate() const {
        return dates_.back();
    }

    inline Time CompoundForward::maxTime() const {
        return times_.back();
    }

    inline const std::vector<Time>& CompoundForward::times() const {
        return times_;
    }

    inline const std::vector<Date>& CompoundForward::dates() const {
        return dates_;
    }

    inline const std::vector<Rate>& CompoundForward::forwards() const {
        return forwards_;
    }

    inline void CompoundForward::update() {
        needsBootstrap_ = true;
        notifyObservers();
    }

}


#endif
