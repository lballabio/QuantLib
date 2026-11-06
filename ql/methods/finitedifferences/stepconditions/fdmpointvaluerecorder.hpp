/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Jogi Sidhu

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fdmpointvaluerecorder.hpp
    \brief step condition recording the value at a point over time
*/

#ifndef quantlib_fdm_point_value_recorder_hpp
#define quantlib_fdm_point_value_recorder_hpp

#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/stepcondition.hpp>
#include <map>
#include <vector>

namespace QuantLib {

    //! records the value at a given point of a one-dimensional mesher
    /*! The value is read off the solution with the same interpolation
        as Fdm1DimSolver, at each time the condition is applied.  Placed
        nth in a step-condition composite, it records the value after the
        first n-1 conditions are applied.

        The recorded values can drive a time-dependent boundary condition
        of a second solver, e.g. the value of the vanilla option on the
        barrier of a knock-in option.

        The whole solution can also be kept at given times.  For example,
        a discrete dividend can take a cum-dividend spot on the inactive
        surface of a knock-in option through the barrier; that node then
        takes its value from the active surface at the ex-dividend time.
    */
    class FdmPointValueRecorder : public StepCondition<Array> {
      public:
        /*! \c x is given in the coordinates of the mesher, e.g. log spot
            for a log-spot mesher.  The whole solution is also kept at the
            given \c solutionTimes.
        */
        FdmPointValueRecorder(const ext::shared_ptr<FdmMesher>& mesher,
                              Real x,
                              std::vector<Time> solutionTimes = {});

        void applyTo(Array& a, Time t) const override;

        //! recorded value at time t
        /*! Interpolated linearly if t lies between recorded times, and
            flat beyond them.
        */
        Real valueAt(Time t) const;

        //! solution recorded at one of the given solution times
        const Array& solutionAt(Time t) const;

      private:
        std::vector<Real> locations_;
        Real x_;
        std::vector<Time> solutionTimes_;
        mutable std::map<Time, Real> values_;
        mutable std::map<Time, Array> solutions_;
    };

}

#endif
