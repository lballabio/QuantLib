
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
#include "ql/MonteCarlo/randomarraygenerator.hpp"
#include "ql/dataformatters.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        //! Generates random paths from a random number generator
        template <class RNG>
        class PathGenerator {
          public:
            typedef Path sample_type;
            // constructors
            PathGenerator(double drift,
                          double variance,
                          Time length,
                          unsigned int steps,
                          long seed = 0);
            /*! \warning the initial time is assumed to be zero 
                and must <b>not</b> be included in the passed vector */
            PathGenerator(double drift,
                          double variance,
                          const std::vector<Time>& times,
                          long seed = 0);
            //! \name inspectors
            //@{
            Path next() const;
            double weight() const { return weight_; }
            unsigned int size() const { return next_.size(); }
            //@}
          private:
            mutable Path next_;
            mutable double weight_;
            Handle<RandomArrayGenerator<RNG> > generator_;
        };

        template <class RNG>
        PathGenerator<RNG>::PathGenerator(double drift, double variance, 
            Time length, unsigned int steps, long seed)
        : next_(steps) {
            QL_REQUIRE(variance >= 0.0, "PathGenerator: negative variance");
            next_.drift() = Array(steps, drift*length/steps);
            generator_ = Handle<RandomArrayGenerator<RNG> >(
                new RandomArrayGenerator<RNG>(
                Array(steps, variance*length/steps), seed));
        }

        template <class RNG>
        PathGenerator<RNG>::PathGenerator(double drift, double variance, 
            const std::vector<Time>& times, long seed)
        : next_(times.size()) {
            QL_REQUIRE(variance >= 0.0, "PathGenerator: negative variance");
            QL_REQUIRE(times.size() > 0, "PathGenerator: no times given");
            QL_REQUIRE(times[0] > 0.0, "PathGenerator: negative time given");
            Array vrnc(times.size());
            Time dt = times[0];
            next_.drift()[0] = drift*dt;
            vrnc[0] = variance*dt;
            for (unsigned int i=1; i<times.size(); i++) {
                QL_REQUIRE(times[i] > times[i-1],
                    "PathGenerator: time[" +
                    IntegerFormatter::toString(i-1)+"]="+
                    DoubleFormatter::toString(times[i-1])+
                    " is greater than time["+
                    IntegerFormatter::toString(i)+"]="+
                    DoubleFormatter::toString(times[i]));
                dt = times[i] - times[i-1];
                next_.drift()[i] = drift*dt;
                vrnc[i] = variance*dt;
            }
            generator_ = Handle<RandomArrayGenerator<RNG> >(
                new RandomArrayGenerator<RNG>(vrnc,seed));
        }

        template <class RNG>
        inline Path PathGenerator<RNG>::next() const {
            next_.diffusion() = generator_->next();
            // always call weight() _after_ next()
            weight_ = generator_->weight();
            return next_;
        }

    }

}


#endif
