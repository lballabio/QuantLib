
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/
/*! \file pathgenerator.hpp
    \brief generates paths from a random number generator

    \fullpath
    ql/MonteCarlo/%pathgenerator.hpp
*/

// $Id$

#ifndef quantlib_montecarlo_path_generator_h
#define quantlib_montecarlo_path_generator_h

#include <ql/MonteCarlo/path.hpp>
#include <ql/RandomNumbers/randomarraygenerator.hpp>

namespace QuantLib {

    namespace MonteCarlo {

        //! Generates random paths from a random number generator
        /*! \todo add more general path generator with drift(S,t) and
                  variance(S,t)
        */
        template <class RNG>
        class PathGenerator {
          public:
            typedef Sample<Path> sample_type;
            // constructors
            PathGenerator(double drift,
                          double variance,
                          Time length,
                          size_t timeSteps,
                          long seed = 0);
            /*! \warning the initial time is assumed to be zero
                and must <b>not</b> be included in the passed vector */
            PathGenerator(double drift,
                          double variance,
                          const std::vector<Time>& times,
                          long seed = 0);
            //! \name inspectors
            //@{
            const sample_type& next() const;
            size_t size() const { return next_.size(); }
            //@}
          private:
            mutable Sample<Path> next_;
            Handle<RandomNumbers::RandomArrayGenerator<RNG> > generator_;
        };

        template <class RNG>
        PathGenerator<RNG>::PathGenerator(double drift, double variance,
            Time length, size_t timeSteps, long seed)
        : next_(Path(timeSteps),1.0) {
            QL_REQUIRE(timeSteps > 0, "PathGenerator: Time steps(" +
                IntegerFormatter::toString(timeSteps) +
                ") must be greater than zero");
            QL_REQUIRE(length > 0, "PathGenerator: length must be > 0");
            Time dt = length/timeSteps;
            for (size_t i=0; i<timeSteps; i++) {
                next_.value.times()[i] = (i+1)*dt;
            }

            next_.value.drift() = Array(timeSteps, drift*dt);

            QL_REQUIRE(variance >= 0.0, "PathGenerator: negative variance");
            generator_ = Handle<RandomNumbers::RandomArrayGenerator<RNG> >(
                new RandomNumbers::RandomArrayGenerator<RNG>(
                    Array(timeSteps, variance*dt), seed));
        }

        template <class RNG>
        PathGenerator<RNG>::PathGenerator(double drift, double variance,
            const std::vector<Time>& times, long seed)
        : next_(Path(times.size()),1.0) {
            QL_REQUIRE(variance >= 0.0, "PathGenerator: negative variance");
            QL_REQUIRE(times.size() > 0, "PathGenerator: no times given");
            QL_REQUIRE(times[0] >= 0.0, "PathGenerator: first time(" +
                 DoubleFormatter::toString(times[0]) + ") must be non negative");
            Array variancePerTime(times.size());
            Time dt = times[0];
            next_.value.drift()[0] = drift*dt;
            variancePerTime[0] = variance*dt;
            for(size_t i = 1; i < times.size(); i++) {
                QL_REQUIRE(times[i] >= times[i-1],
                    "MultiPathGenerator: time(" +
                    IntegerFormatter::toString(i-1)+")=" +
                    DoubleFormatter::toString(times[i-1]) +
                    " is later than time(" +
                    IntegerFormatter::toString(i) + ")=" +
                    DoubleFormatter::toString(times[i]));
                dt = times[i] - times[i-1];
                next_.value.drift()[i] = drift*dt;
                variancePerTime[i] = variance*dt;
            }
            next_.value.times() = times;

            generator_ = Handle<RandomNumbers::RandomArrayGenerator<RNG> >(
                new RandomNumbers::RandomArrayGenerator<RNG>(variancePerTime,
                seed));
        }

        template <class RNG>
        inline const PathGenerator<RNG>::sample_type&
        PathGenerator<RNG>::next() const {
            const Sample<Array>& sample = generator_->next();
            next_.weight = sample.weight;
            next_.value.diffusion() = sample.value;
            return next_;
        }

    }

}


#endif
