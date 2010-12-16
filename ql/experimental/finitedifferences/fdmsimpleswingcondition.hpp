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

/*! \file fdmsimpleswingcondition.hpp
    \brief simple swing step condition
*/

#ifndef quantlib_fdm_simple_swing_condition_hpp
#define quantlib_fdm_simple_swing_condition_hpp

#include <ql/methods/finitedifferences/stepcondition.hpp>
#include <ql/experimental/finitedifferences/fdmmesher.hpp>

namespace QuantLib {

    class FdmSimpleSwingCondition : public StepCondition<Array> {
      public:
          FdmSimpleSwingCondition(const std::vector<Time> & exerciseTimes,
                                  const boost::shared_ptr<FdmMesher> & mesher,
                                  const boost::shared_ptr<Payoff> & payoff,
                                  Size equityDirection,
                                  Size swingDirection);
    
        void applyTo(Array& a, Time t) const;
    
      private:
        Array x_; // grid-equity values in physical units
    
        const std::vector<Time> exerciseTimes_;
        const boost::shared_ptr<FdmMesher> mesher_;
        const boost::shared_ptr<Payoff> payoff_;
        const Size equityDirection_, swingDirection_;
    };
}
#endif
