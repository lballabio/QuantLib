
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

#include "ql/MonteCarlo/multipath.hpp"
#include "ql/MonteCarlo/sample.hpp"

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
                               size_t timeSteps,
                               Time lenght,
                               long seed);
            MultiPathGenerator(const Array& drifts,
                               const Math::Matrix& covariance,
                               const std::vector<Time>& times,
                               long seed=0);
            const sample_type& next() const;
        private:
            Array drifts_;
            size_t timeSteps_;
            std::vector<Time> timeDelays_;
            size_t numAssets_;
            RAG rndArrayGen_;
            mutable sample_type next_;
        };

        template <class RAG>
        inline MultiPathGenerator<RAG >::MultiPathGenerator(
            const Array& drifts,
            const Math::Matrix& covariance,
            size_t timeSteps,
            Time lenght,
            long seed)
        : drifts_(covariance.rows(),0.0), timeSteps_(timeSteps),
          timeDelays_(timeSteps, lenght/timeSteps),
          numAssets_(covariance.rows()),
          rndArrayGen_(covariance, seed),
          next_(MultiPath(covariance.rows(),timeSteps),1.0) {

            QL_REQUIRE(timeSteps_ > 0, "Time steps(" +
                IntegerFormatter::toString(timeSteps_) + ") too small");

            if(drifts.size() != 0) {
                QL_REQUIRE(drifts.size() == covariance.rows(),
                           "MultiPathGenerator covariance and average "
                           "do not have the same size");
                std::copy(drifts.begin(),drifts.end(), drifts_.begin());
            }

        }

        template <class RAG>
        inline MultiPathGenerator<RAG >::MultiPathGenerator(
            const Array& drifts, const Math::Matrix& covariance,
            const std::vector<Time>& times, long seed)
        : drifts_(covariance.rows(), 0.0), timeSteps_(times.size()),
          timeDelays_(times.size()), numAssets_(covariance.rows()), 
          rndArrayGen_(covariance, seed),
          next_(MultiPath(covariance.rows(),times.size()),1.0) {

            QL_REQUIRE(timeSteps_ > 0, "Time steps(" +
                IntegerFormatter::toString(timeSteps_) + ") too small");

            if(drifts.size() != 0){
                QL_REQUIRE(drifts.size() == covariance.rows(),
                           "MultiPathGenerator covariance and average "
                           "do not have the same size");
                std::copy(drifts.begin(),drifts.end(), drifts_.begin());
            }

            QL_REQUIRE(times[0] >= 0, "MultiPathGenerator: first time(" +
                 DoubleFormatter::toString(times[0]) + ") must be positive");
            timeDelays_[0] = times[0];

            if(timeSteps_ > 1){
                for(size_t i = 1; i < timeSteps_; i++){
                    QL_REQUIRE(times[i] >= times[i-1],
                        "MultiPathGenerator: time(" +
                        IntegerFormatter::toString(i-1)+")=" +
                        DoubleFormatter::toString(times[i-1]) +
                        " is later than time(" +
                        IntegerFormatter::toString(i) + ")=" +
                        DoubleFormatter::toString(times[i]));
                    timeDelays_[i] = times[i] - times[i-1];
                }
            }
        }

        template <class RAG>
        inline const MultiPathGenerator<RAG >::sample_type& 
        MultiPathGenerator<RAG >::next() const {

            QL_REQUIRE(numAssets_ > 0,
                "MultiPathGenerator: object declared but not initialized");
            Array randomExtraction(numAssets_);
            next_.weight = 1.0;
            for (size_t i = 0; i < timeSteps_; i++) {
                const Sample<Array>& randomExtraction = rndArrayGen_.next();
                next_.weight *= randomExtraction.weight;
                for (size_t j=0; j<numAssets_; j++) {
                    next_.value[j].drift()[i] = drifts_[j] * timeDelays_[i];
                    next_.value[j].diffusion()[i] = 
                        randomExtraction.value[j] * QL_SQRT(timeDelays_[i]);
                }
            }
            return next_;
        }

    }

}

#endif
