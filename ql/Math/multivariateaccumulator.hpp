
/*
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

/*! \file multivariateaccumulator.hpp
    \brief A simple accumulator for vector-type samples
*/

#ifndef quantlib_math_multivariate_accumulator_h
#define quantlib_math_multivariate_accumulator_h

#include <ql/null.hpp>
#include <ql/dataformatters.hpp>
#include <ql/Math/matrix.hpp>
#include <vector>

namespace QuantLib {

    namespace Math {

        //! A sample accumulator for multivariate analysis
        /*! MultivariateAccumulator can accumulate vector-type samples and
            return the average vector, both in Array form and
            std::vector<double> form, and the covariance matrix

            \deprecated use SequenceStatistics instead
        */
        class MultivariateAccumulator {
          public:
            MultivariateAccumulator();
            MultivariateAccumulator(Size size);
            //! \name Inspectors
            //@{
            //! size of each sample
            Size size() const;
            //! number of samples collected
            Size samples() const;
            //! sum of data weights
            double weightSum() const;
            //! returns the mean as an Array
            Disposable<Array> mean() const;
            //! returns the mean as a std::vector<double>
            std::vector<double> meanVector() const;
            //! returns the covariance Matrix
            Matrix covariance() const;
            //! returns the correlation Matrix
            Matrix correlation() const;

            //@}

            //! \name Modifiers
            //@{
            //! adds an Array to the collection, possibly with a weight
            void add(const Array &arr, double weight = 1.0);
            //! adds a vector<double> to the collection, possibly with a weight
            void add(const std::vector<double> &vec, double weight = 1.0);
            //! adds a sequence of data to the collection
            template <class DataIterator>
            void addSequence(DataIterator begin, DataIterator end) {
                for (;begin!=end;++begin)
                    add(*begin);
            }
            //! adds a sequence of data to the collection, each with its weight
            template <class DataIterator, class WeightIterator>
            void addSequence(DataIterator begin, DataIterator end,
                WeightIterator wbegin) {
                    for(;begin!=end;++begin,++wbegin)
                        add(*begin, *wbegin);
            }
            //! resets the data to a null set
            void reset();
            //@}
          private:
            Size size_;
            Size sampleNumber_;
            double sampleWeight_;
            Array sum_;
            Matrix quadraticSum_;
        };


        // inline definitions
        inline Size MultivariateAccumulator::size() const {
            return size_;
        }

        inline Size MultivariateAccumulator::samples() const {
            return sampleNumber_;
        }

        inline double MultivariateAccumulator::weightSum() const {
            return sampleWeight_;
        }

        inline Disposable<Array> MultivariateAccumulator::mean() const {
            QL_REQUIRE(sampleWeight_ > 0.0,
                "Stat::mean() : sampleWeight_=0, unsufficient");
            return sum_/sampleWeight_;
        }

    }

}

#endif
