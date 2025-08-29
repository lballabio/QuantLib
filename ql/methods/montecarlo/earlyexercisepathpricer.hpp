/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Klaus Spanderen

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

/*! \file earlyexercisepathpricer.hpp
    \brief base class for early exercise single-path pricers
*/

#ifndef quantlib_early_exercise_path_pricer_hpp
#define quantlib_early_exercise_path_pricer_hpp

#include <ql/math/array.hpp>
#include <ql/methods/montecarlo/path.hpp>
#include <ql/methods/montecarlo/multipath.hpp>
#include <ql/functional.hpp>

namespace QuantLib {

    template <class PathType>
    class EarlyExerciseTraits {
        // dummy definition, will not work
    };

    template <>
    class EarlyExerciseTraits<Path> {
      public:
        typedef Real StateType;
        static Size pathLength(const Path& path) {
            return path.length();
        }
    };

    template <>
    class EarlyExerciseTraits<MultiPath> {
      public:
        typedef Array StateType;
        static Size pathLength(const MultiPath& path) {
            return path.pathSize();
        }
    };

    //! base class for early exercise path pricers
    /*! Returns the value of an option on a given path and given time.

        \ingroup mcarlo
    */
    template<class PathType,
             class TimeType=Size, class ValueType=Real>
    class EarlyExercisePathPricer {
      public:
        typedef typename EarlyExerciseTraits<PathType>::StateType StateType;

        virtual ~EarlyExercisePathPricer() = default;
        virtual ValueType operator()(const PathType& path,
                                     TimeType t) const = 0;

        virtual StateType
            state(const PathType& path, TimeType t) const = 0;
        virtual std::vector<std::function<ValueType(StateType)> >
            basisSystem() const = 0;
    };
}


#endif
