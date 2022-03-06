/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Klaus Spanderen

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

/*! \file fdmbermudanstepcondition.hpp
    \brief bermudan step condition for multi dimensional problems
*/

#ifndef quantlib_fdm_bermudan_step_condition_hpp
#define quantlib_fdm_bermudan_step_condition_hpp

#include <ql/time/daycounter.hpp>
#include <ql/methods/finitedifferences/stepcondition.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>

namespace QuantLib {

    class FdmInnerValueCalculator;

    class FdmBermudanStepCondition : public StepCondition<Array> {
      public:
        FdmBermudanStepCondition(const std::vector<Date>& exerciseDates,
                                 const Date& referenceDate,
                                 const DayCounter& dayCounter,
                                 ext::shared_ptr<FdmMesher> mesher,
                                 ext::shared_ptr<FdmInnerValueCalculator> calculator);

        void applyTo(Array& a, Time t) const override;
        const std::vector<Time>& exerciseTimes() const;

      private:
        std::vector<Time> exerciseTimes_;
        const ext::shared_ptr<FdmMesher> mesher_;
        const ext::shared_ptr<FdmInnerValueCalculator> calculator_;
    };
}

#endif


#ifndef id_ef3f44cfb0b90d604a36ae16c69581f7
#define id_ef3f44cfb0b90d604a36ae16c69581f7
inline bool test_ef3f44cfb0b90d604a36ae16c69581f7(int* i) { return i != 0; }
#endif
