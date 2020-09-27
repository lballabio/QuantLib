/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
Copyright (C) 2007, 2008 StatPro Italia srl
Copyright (C) 2017 Francois Botha

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

/*! \file compositezeroyieldstructure.hpp
\brief Composite zero term structure
*/

#ifndef quantlib_composite_zero_yield_structure
#define quantlib_composite_zero_yield_structure


#include <ql/termstructures/yield/zeroyieldstructure.hpp>

namespace QuantLib {
    template <class BinaryFunction>
    class CompositeZeroYieldStructure : public ZeroYieldStructure {
      public:
          CompositeZeroYieldStructure(const Handle<YieldTermStructure>& h1,
                                      const Handle<YieldTermStructure>& h2,
                                      const BinaryFunction& f,
                                      Compounding comp = Continuous,
                                      Frequency freq = NoFrequency);

          //! \name YieldTermStructure interface
          //@{
          DayCounter dayCounter() const;
          Calendar calendar() const;
          Natural settlementDays() const;
          const Date& referenceDate() const;
          Date maxDate() const;
          Time maxTime() const;
          //@}
          //! \name Observer interface
          //@{
          void update();
          //@}
      protected:
        //! returns the composite zero yield rate
        Rate zeroYieldImpl(Time) const;
      private:
        Handle<YieldTermStructure> curve1_;
        Handle<YieldTermStructure> curve2_;
        BinaryFunction f_;
        Compounding comp_;
        Frequency freq_;
    };

    // inline definitions

    template <class BinaryFunction>
    inline CompositeZeroYieldStructure<BinaryFunction>::CompositeZeroYieldStructure(
        const Handle<YieldTermStructure>& h1, 
        const Handle<YieldTermStructure>& h2, 
        const BinaryFunction& f,
        Compounding comp, 
        Frequency freq)
    : curve1_(h1), curve2_(h2), f_(f), comp_(comp), freq_(freq) {
        if (!curve1_.empty() && !curve2_.empty())
            enableExtrapolation(curve1_->allowsExtrapolation() && curve2_->allowsExtrapolation());

        registerWith(curve1_);
        registerWith(curve2_);
    }

    template <class BinaryFunction>
    inline DayCounter CompositeZeroYieldStructure<BinaryFunction>::dayCounter() const {
        return curve1_->dayCounter();
    }

    template <class BinaryFunction>
    inline Calendar CompositeZeroYieldStructure<BinaryFunction>::calendar() const {
        return curve1_->calendar();
    }

    template <class BinaryFunction>
    inline Natural CompositeZeroYieldStructure<BinaryFunction>::settlementDays() const {
        return curve1_->settlementDays();
    }

    template <class BinaryFunction>
    inline const Date& CompositeZeroYieldStructure<BinaryFunction>::referenceDate() const {
        return curve1_->referenceDate();
    }

    template <class BinaryFunction>
    inline Date CompositeZeroYieldStructure<BinaryFunction>::maxDate() const {
        return curve1_->maxDate();
    }

    template <class BinaryFunction>
    inline Time CompositeZeroYieldStructure<BinaryFunction>::maxTime() const {
        return curve1_->maxTime();
    }

    template <class BinaryFunction>
    inline void CompositeZeroYieldStructure<BinaryFunction>::update() {
        if (!curve1_.empty() && !curve2_.empty()) {
            YieldTermStructure::update();
            enableExtrapolation(curve1_->allowsExtrapolation() && curve2_->allowsExtrapolation());
        }
        else {
            /* The implementation inherited from YieldTermStructure
            asks for our reference date, which we don't have since
            the original curve is still not set. Therefore, we skip
            over that and just call the base-class behavior. */
            // NOLINTNEXTLINE(bugprone-parent-virtual-call)
            TermStructure::update();
        }
    }

    template <class BinaryFunction>
    inline Rate CompositeZeroYieldStructure<BinaryFunction>::zeroYieldImpl(Time t) const {
        Rate zeroRate1 =
            curve1_->zeroRate(t, comp_, freq_, true);

        InterestRate zeroRate2 =
            curve2_->zeroRate(t, comp_, freq_, true);

        InterestRate compositeRate(f_(zeroRate1, zeroRate2), dayCounter(), comp_, freq_);
        return compositeRate.equivalentRate(Continuous, NoFrequency, t);
    }
}
#endif
