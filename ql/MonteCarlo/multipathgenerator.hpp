
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

/*! \file multipathgenerator.hpp
    \brief Generates a multi path from a random-array generator

    \fullpath
    ql/MonteCarlo/%multipathgenerator.hpp
*/

// $Id$

#ifndef quantlib_montecarlo_multi_path_generator_h
#define quantlib_montecarlo_multi_path_generator_h

#include <ql/MonteCarlo/multipath.hpp>
#include <ql/MonteCarlo/sample.hpp>

namespace QuantLib {

    namespace MonteCarlo {

        //! Generates a multipath from a random number generator
        /*! MultiPathGenerator<RAG> is a class that returns a random
            multi path.
            RAG is a sample generator which returns a random array.
            It must have the minimal interface:
            \code
            RAG{
                RAG();
                RAG(Matrix& covariance,
                    long seed);
                Sample<Array> next();
            };
            \endcode
        */
        template <class RAG>
        class MultiPathGenerator {
          public:
            typedef Sample<MultiPath> sample_type;
            MultiPathGenerator(const Array& drifts,
                               const Math::Matrix& covariance,
                               Time length,
                               size_t timeSteps,
                               long seed);
            MultiPathGenerator(const Array& drifts,
                               const Math::Matrix& covariance,
                               const std::vector<Time>& times,
                               long seed=0);
            const sample_type& next() const;
        private:
            size_t numAssets_;
            RAG rndArrayGen_;
            mutable sample_type next_;
            std::vector<Time> timeDelays_;
        };

        template <class RAG>
        inline MultiPathGenerator<RAG >::MultiPathGenerator(
            const Array& drifts, const Math::Matrix& covariance,
            Time length, size_t timeSteps, long seed)
        : numAssets_(covariance.rows()),
          rndArrayGen_(covariance, seed),
          next_(MultiPath(covariance.rows(),timeSteps),1.0) {

            QL_REQUIRE(drifts.size() == numAssets_,
                       "MultiPathGenerator covariance and average "
                       "do not have the same size");
            QL_REQUIRE(timeSteps > 0, "MultiPathGenerator: Time steps(" +
                IntegerFormatter::toString(timeSteps) +
                ") must be greater than zero");
            QL_REQUIRE(length > 0, "MultiPathGenerator: length must be > 0");
            Time dt = length/timeSteps;
            timeDelays_ = std::vector<Time>(timeSteps, dt);
            double variances = covariance.diagonal();
            for (size_t j=0; j<numAssets_; j++) {
                QL_REQUIRE(vols[j]>=0, "MultiPathGenerator: negative variance");
                for (size_t i=0; i<timeSteps; i++) {
                    next_.value[j].times()[i] = (i+1)*dt;
                    next_.value[j].drift()[i]=drifts_[j]*timeDelays_[i];
                }
            }

        }

        template <class RAG>
        inline MultiPathGenerator<RAG >::MultiPathGenerator(
            const Array& drifts, const Math::Matrix& covariance,
            const std::vector<Time>& times, long seed)
        : numAssets_(covariance.rows()),
          rndArrayGen_(covariance, seed),
          next_(MultiPath(covariance.rows(),times.size()),1.0),
          timeDelays_(times.size()) {

            QL_REQUIRE(drifts.size() == numAssets_,
                       "MultiPathGenerator covariance and average "
                       "do not have the same size");
            QL_REQUIRE(times.size() > 0,
                "MultiPathGenerator: no times given");
            QL_REQUIRE(times[0] >= 0, "MultiPathGenerator: first time(" +
                 DoubleFormatter::toString(times[0]) +
                 ") must be non negative");
            Array variances = covariance.diagonal();
            timeDelays_[0] = times[0];
            for(size_t i = 1; i < times.size(); i++) {
                QL_REQUIRE(times[i] >= times[i-1],
                    "MultiPathGenerator: time(" +
                    IntegerFormatter::toString(i-1)+")=" +
                    DoubleFormatter::toString(times[i-1]) +
                    " is later than time(" +
                    IntegerFormatter::toString(i) + ")=" +
                    DoubleFormatter::toString(times[i]));
                timeDelays_[i] = times[i] - times[i-1];
            }


            for (size_t j=0; j<numAssets_; j++) {
                next_.value[j].times() = times;
                QL_REQUIRE(variances[j]>=0, "MultiPathGenerator: negative variance");
                for (size_t i = 0; i< times.size(); i++) {
                    next_.value[j].drift()[i] = drifts[j] * timeDelays_[i];
                }
            }


        }

        template <class RAG>
        inline const MultiPathGenerator<RAG >::sample_type&
        MultiPathGenerator<RAG >::next() const {

            Array randomExtraction(numAssets_);
            next_.weight = 1.0;
            for (size_t i = 0; i < next_.value[0].times().size(); i++) {
                const Sample<Array>& randomExtraction = rndArrayGen_.next();
                next_.weight *= randomExtraction.weight;
                for (size_t j=0; j<numAssets_; j++) {
                    next_.value[j].diffusion()[i] =
                        randomExtraction.value[j] * QL_SQRT(timeDelays_[i]);
                }
            }
            return next_;
        }

    }

}

#endif
