
/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
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
        template <class SG>
        class MultiPathGenerator {
          public:
            typedef Sample<MultiPath> sample_type;
            MultiPathGenerator(const Array& drifts,
                               const Math::Matrix& covariance,
                               Time length,
                               Size timeSteps,
                               SG generator);
            MultiPathGenerator(const Array& drifts,
                               const Math::Matrix& covariance,
                               const TimeGrid& timeGrid,
                               SG generator);
            const sample_type& next() const;
            const sample_type& antithetic() const;
        private:
            Size numAssets_;
            Math::Matrix sqrtCovariance_;
            SG generator_;
            mutable sample_type next_;
        };

        template <class SG>
        inline MultiPathGenerator<SG>::MultiPathGenerator(
            const Array& drifts, const Math::Matrix& covariance,
            Time length, Size timeSteps, SG generator)
        : numAssets_(covariance.rows()), sqrtCovariance_(Math::matrixSqrt(covariance)),
          generator_(generator),
          next_(MultiPath(covariance.rows(),TimeGrid(length, timeSteps)),1.0) {

            QL_REQUIRE(generator_.dimension() == numAssets_*timeSteps,
                       "generator's dimension is not equal to "
                       "the number of assets time the number of time steps");
            QL_REQUIRE(drifts.size() == numAssets_,
                       "MultiPathGenerator covariance and average "
                       "do not have the same size");
            QL_REQUIRE(sqrtCovariance_.cols() == numAssets_,
                       "MultiPathGenerator covariance is not "
                       "a square matrix");
            QL_REQUIRE(timeSteps > 0, "MultiPathGenerator: Time steps(" +
                IntegerFormatter::toString(timeSteps) +
                ") must be greater than zero");
            QL_REQUIRE(length > 0, "MultiPathGenerator: length must be > 0");

            for (Size j=0; j<numAssets_; j++) {
                for (Size i=0; i<timeSteps; i++) {
                    next_.value[j].drift()[i]=drifts[j] *
                        next_.value[j].timeGrid().dt(i);
                }
            }

        }

        template <class SG>
        inline MultiPathGenerator<SG>::MultiPathGenerator(
            const Array& drifts, const Math::Matrix& covariance,
            const TimeGrid& times, SG generator)
        : numAssets_(covariance.rows()), sqrtCovariance_(Math::matrixSqrt(covariance)),
          generator_(generator),
          next_(MultiPath(covariance.rows(), times), 1.0) {

            QL_REQUIRE(generator_.dimension() == numAssets_*(times.size()-1),
                       "generator's dimension is not equal to "
                       "the number of assets time the number of time steps");
            QL_REQUIRE(drifts.size() == numAssets_,
                       "MultiPathGenerator covariance and average "
                       "do not have the same size");
            QL_REQUIRE(sqrtCovariance_.cols() == numAssets_,
                       "MultiPathGenerator covariance is not "
                       "a square matrix");
            QL_REQUIRE(times.size() > 1,
                "MultiPathGenerator: no times given");

            for (Size j=0; j<numAssets_; j++) {
                for (Size i = 0; i< times.size()-1; i++) {
                    next_.value[j].drift()[i] = drifts[j] * 
                        next_.value[j].timeGrid().dt(i);
                }
            }
        }


        template <class SG>
        inline  const typename MultiPathGenerator<SG>::sample_type&
        MultiPathGenerator<SG>::next() const {


            typedef typename SG::sample_type sequence_type;
            Array temp(numAssets_);
            const sequence_type& randomExtraction = generator_.nextSequence();
            next_.weight = randomExtraction.weight;
            for (Size i = 0; i < next_.value[0].size(); i++) {
                Size offset = i*numAssets_;
                std::copy(randomExtraction_.begin()+offset,
                          randomExtraction_.begin()+offset+numAssets,
                          temp.begin());
                temp = sqrtCovariance_ * temp;
                double sqrtDt = QL_SQRT(next_.value[j].timeGrid().dt(i));
                for (Size j=0; j<numAssets_; j++) {
                    next_.value[j].diffusion()[i] =
                        temp[j] * sqrtDt;
                }
            }
            return next_;
        }
        

        


        
        //! Generates a multipath from a random number generator
        /*! MultiPathGenerator_old<RAG> is a class that returns a random
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
        class MultiPathGenerator_old {
          public:
            typedef Sample<MultiPath> sample_type;
            MultiPathGenerator_old(const Array& drifts,
                               const Math::Matrix& covariance,
                               Time length,
                               Size timeSteps,
                               long seed);
            MultiPathGenerator_old(const Array& drifts,
                               const Math::Matrix& covariance,
                               const TimeGrid& times,
                               long seed=0);
            const sample_type& next() const;
        private:
            Size numAssets_;
            RAG rndArrayGen_;
            mutable sample_type next_;
            std::vector<Time> timeDelays_;
        };

        template <class RAG>
        inline MultiPathGenerator_old<RAG >::MultiPathGenerator_old(
            const Array& drifts, const Math::Matrix& covariance,
            Time length, Size timeSteps, long seed)
        : numAssets_(covariance.rows()),
          rndArrayGen_(covariance, seed),
          next_(MultiPath(covariance.rows(),timeSteps),1.0) {

            QL_REQUIRE(drifts.size() == numAssets_,
                       "MultiPathGenerator_old covariance and average "
                       "do not have the same size");
            QL_REQUIRE(timeSteps > 0, "MultiPathGenerator_old: Time steps(" +
                IntegerFormatter::toString(timeSteps) +
                ") must be greater than zero");
            QL_REQUIRE(length > 0, "MultiPathGenerator_old: length must be > 0");
            Time dt = length/timeSteps;
            timeDelays_ = std::vector<Time>(timeSteps, dt);
            Array variances = covariance.diagonal();
            for (Size j=0; j<numAssets_; j++) {
                QL_REQUIRE(variances[j]>=0, "MultiPathGenerator_old: negative variance");
                for (Size i=0; i<timeSteps; i++) {
                    next_.value[j].TimeGrid()[i] = (i+1)*dt;
                    next_.value[j].drift()[i]=drifts[j]*timeDelays_[i];
                }
            }

        }

        template <class RAG>
        inline MultiPathGenerator_old<RAG >::MultiPathGenerator_old(
            const Array& drifts, const Math::Matrix& covariance,
            const TimeGrid& times, long seed)
        : numAssets_(covariance.rows()),
          rndArrayGen_(covariance, seed),
          next_(MultiPath(covariance.rows(), times),1.0),
          timeDelays_(times.size()-1) {

            QL_REQUIRE(drifts.size() == numAssets_,
                       "MultiPathGenerator_old covariance and average "
                       "do not have the same size");
            QL_REQUIRE(times.size() > 1,
                "MultiPathGenerator_old: no times given");
            QL_REQUIRE(times[0] >= 0, "MultiPathGenerator_old: first time(" +
                 DoubleFormatter::toString(times[0]) +
                 ") must be non negative");
            Array variances = covariance.diagonal();
            for(Size i = 1; i < times.size(); i++) {
                QL_REQUIRE(times[i] >= times[i-1],
                    "MultiPathGenerator_old: time(" +
                    IntegerFormatter::toString(i-1)+")=" +
                    DoubleFormatter::toString(times[i-1]) +
                    " is later than time(" +
                    IntegerFormatter::toString(i) + ")=" +
                    DoubleFormatter::toString(times[i]));
                timeDelays_[i-1] = times[i] - times[i-1];
            }


            for (Size j=0; j<numAssets_; j++) {
                QL_REQUIRE(variances[j]>=0, "MultiPathGenerator_old: negative variance");
                for (Size i = 0; i< times.size()-1; i++) {
                    next_.value[j].drift()[i] = drifts[j] * timeDelays_[i];
                }
            }


        }

        template <class RAG>
        inline  const typename MultiPathGenerator_old<RAG >::sample_type&
        MultiPathGenerator_old<RAG >::next() const {

            Array randomExtraction(numAssets_);
            next_.weight = 1.0;
            for (Size i = 0; i < next_.value[0].size(); i++) {
                const Sample<Array>& randomExtraction = rndArrayGen_.next();
                next_.weight *= randomExtraction.weight;
                for (Size j=0; j<numAssets_; j++) {
                    next_.value[j].diffusion()[i] =
                        randomExtraction.value[j] * QL_SQRT(timeDelays_[i]);
                }
            }
            return next_;
        }

    }

}

#endif
