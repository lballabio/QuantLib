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

/*! \file fdmsimplestoragecondition.hpp
    \brief simple storage step condition
*/

#ifndef quantlib_fdm_simple_storage_condition_hpp
#define quantlib_fdm_simple_storage_condition_hpp

#include <ql/math/matrix.hpp>
#include <ql/methods/finitedifferences/stepcondition.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>

namespace QuantLib {

    class FdmSimpleStorageCondition : public StepCondition<Array> {
      public:
          FdmSimpleStorageCondition(
                  const std::vector<Time> & exerciseTimes,
                  const boost::shared_ptr<FdmMesher>& mesher,
                  const boost::shared_ptr<FdmInnerValueCalculator>& calculator,
                  Real changeRate);

        void applyTo(Array& a, Time t) const;

      private:
        const std::vector<Time> exerciseTimes_;
        const boost::shared_ptr<FdmMesher> mesher_;
        const boost::shared_ptr<FdmInnerValueCalculator> calculator_;
        const Real changeRate_;

        std::vector<Real> x_, y_;
    };
}
#endif
