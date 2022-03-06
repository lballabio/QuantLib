/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Stefano Fondi

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

/*! \file arithmeticaverageois.hpp
    \brief Overnight index swap paying arithmetic average of overnight vs. fixed
*/

#ifndef quantlib_arithmetic_average_ois_hpp
#define quantlib_arithmetic_average_ois_hpp

#include <ql/instruments/swap.hpp>
#include <ql/time/daycounter.hpp>

namespace QuantLib {

    class Schedule;
    class OvernightIndex;

    //! Arithemtic Average OIS: fix vs arithmetic average of overnight rate
    class ArithmeticAverageOIS : public Swap {
      public:
        ArithmeticAverageOIS(Type type,
                             Real nominal,
                             const Schedule& fixedLegSchedule,
                             Rate fixedRate,
                             DayCounter fixedDC,
                             ext::shared_ptr<OvernightIndex> overnightIndex,
                             const Schedule& overnightLegSchedule,
                             Spread spread = 0.0,
                             Real meanReversionSpeed = 0.03,
                             Real volatility = 0.00, // NO convexity adjustment by default
                             bool byApprox = false); // TRUE to use Katsumi Takada approximation
        ArithmeticAverageOIS(Type type,
                             std::vector<Real> nominals,
                             const Schedule& fixedLegSchedule,
                             Rate fixedRate,
                             DayCounter fixedDC,
                             ext::shared_ptr<OvernightIndex> overnightIndex,
                             const Schedule& overnightLegSchedule,
                             Spread spread = 0.0,
                             Real meanReversionSpeed = 0.03,
                             Real volatility = 0.00, // NO convexity adjustment by default
                             bool byApprox = false); // TRUE to use Katsumi Takada approximation
        //! \name Inspectors
        //@{
        Type type() const { return type_; }
        Real nominal() const;
        std::vector<Real> nominals() const { return nominals_; }

        //const Schedule& schedule() { return schedule_; }
        Frequency fixedLegPaymentFrequency() { return fixedLegPaymentFrequency_; }
        Frequency overnightLegPaymentFrequency() { return overnightLegPaymentFrequency_; }

        Rate fixedRate() const { return fixedRate_; }
        const DayCounter& fixedDayCount() { return fixedDC_; }

        const ext::shared_ptr<OvernightIndex>& overnightIndex() { return overnightIndex_; }
        Spread spread() const { return spread_; }

        const Leg& fixedLeg() const { return legs_[0]; }
        const Leg& overnightLeg() const { return legs_[1]; }
        //@}

        //! \name Results
        //@{
        Real fixedLegBPS() const;
        Real fixedLegNPV() const;
        Real fairRate() const;

        Real overnightLegBPS() const;
        Real overnightLegNPV() const;
        Spread fairSpread() const;
        //@}
      private:
        void initialize(const Schedule& fixedLegSchedule,
                        const Schedule& overnightLegSchedule);
        Type type_;
        std::vector<Real> nominals_;

        Frequency fixedLegPaymentFrequency_;
        Frequency overnightLegPaymentFrequency_;
        //Schedule schedule_;

        Rate fixedRate_;
        DayCounter fixedDC_;

        ext::shared_ptr<OvernightIndex> overnightIndex_;
        Spread spread_;

        bool byApprox_;
        Real mrs_;
        Real vol_;
    };


    // inline

    inline Real ArithmeticAverageOIS::nominal() const {
        QL_REQUIRE(nominals_.size()==1, "varying nominals");
        return nominals_[0];
    }

}

#endif


#ifndef id_757bf71c34da660bd08181695da77324
#define id_757bf71c34da660bd08181695da77324
inline bool test_757bf71c34da660bd08181695da77324(int* i) { return i != 0; }
#endif
