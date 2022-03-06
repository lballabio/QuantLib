/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ralph Schreyer

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

/*! \file fdmarithmeticaveragecondition.hpp
    \brief step condition to handle arithmetic average
*/

#ifndef quantlib_fdm_arithmetic_average_condition_hpp
#define quantlib_fdm_arithmetic_average_condition_hpp

#include <ql/methods/finitedifferences/stepcondition.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>

namespace QuantLib {

    class FdmArithmeticAverageCondition : public StepCondition<Array> {
      public:
        FdmArithmeticAverageCondition(std::vector<Time> averageTimes,
                                      Real,
                                      Size pastFixings,
                                      const ext::shared_ptr<FdmMesher>& mesher,
                                      Size equityDirection);

        void applyTo(Array& a, Time t) const override;

      private:
        Array x_; // grid-equity values in physical units
        Array a_; // average values in physical units

        const std::vector<Time> averageTimes_;
        const Size pastFixings_;
        const ext::shared_ptr<FdmMesher> mesher_;
        const Size equityDirection_;
    };
}
#endif


#ifndef id_b12f226537b3145f287916f4b1b75af8
#define id_b12f226537b3145f287916f4b1b75af8
inline bool test_b12f226537b3145f287916f4b1b75af8(int* i) { return i != 0; }
#endif
