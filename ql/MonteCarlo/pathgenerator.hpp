
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

#include "ql/MonteCarlo/path.hpp"
#include "ql/RandomNumbers/randomarraygenerator.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        //! Generates random paths from a random number generator
        template <class RNG>
        class PathGenerator {
          public:
            typedef Sample<Path> sample_type;
            // constructors
            PathGenerator(double drift,
                          double variance,
                          Time length,
                          size_t steps,
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
            Time length, size_t steps, long seed)
        : next_(Path(steps),1.0) {
            QL_REQUIRE(variance >= 0.0, "PathGenerator: negative variance");
            next_.value.drift() = Array(steps, drift*length/steps);
            generator_ = Handle<RandomNumbers::RandomArrayGenerator<RNG> >(
                new RandomNumbers::RandomArrayGenerator<RNG>(
                    Array(steps, variance*length/steps), seed));
        }

        template <class RNG>
        PathGenerator<RNG>::PathGenerator(double drift, double variance, 
            const std::vector<Time>& times, long seed)
        : next_(Path(times.size()),1.0) {
            QL_REQUIRE(variance >= 0.0, "PathGenerator: negative variance");
            QL_REQUIRE(times.size() > 0, "PathGenerator: no times given");
            QL_REQUIRE(times[0] > 0.0, "PathGenerator: negative time given");
            Array vrnc(times.size());
            Time dt = times[0];
            next_.value.drift()[0] = drift*dt;
            vrnc[0] = variance*dt;
            for (size_t i=1; i<times.size(); i++) {
                QL_REQUIRE(times[i] > times[i-1],
                    "PathGenerator: time[" +
                    IntegerFormatter::toString(i-1)+"]="+
                    DoubleFormatter::toString(times[i-1])+
                    " is greater than time["+
                    IntegerFormatter::toString(i)+"]="+
                    DoubleFormatter::toString(times[i]));
                dt = times[i] - times[i-1];
                next_.value.drift()[i] = drift*dt;
                vrnc[i] = variance*dt;
            }
            generator_ = Handle<RandomNumbers::RandomArrayGenerator<RNG> >(
                new RandomNumbers::RandomArrayGenerator<RNG>(vrnc,seed));
        }

        template <class RNG>
        inline const PathGenerator<RNG>::sample_type& 
        PathGenerator<RNG>::next() const {
            const Sample<Array>& sample = generator_->next();
            next_.value.diffusion() = sample.value;
            next_.weight = sample.weight;
            return next_;
        }

    }

}


#endif
