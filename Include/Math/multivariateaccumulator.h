/*
 * Copyright (C) 2001
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file multivariateaccumulator.h
    \brief A simple accumulator for vector-type samples

    $Source$
    $Log$
    Revision 1.7  2001/02/22 14:25:23  lballabio
    Template methods moved to header file

    Revision 1.6  2001/02/20 13:59:00  nando
    added class VarTool.
    RiskStatistics was derived from Statistics:
    now RiskStatistics includes VarTool and Statistics.
    VarTool is not based on Statistics, but requires
    mean and standardDeviation as input.

    Revision 1.5  2001/01/26 11:08:55  marmar
    Now the covariance() method is consistent with the variance()
    method of the class Statistics

    Revision 1.4  2001/01/25 15:11:53  lballabio
    Added helper functions to make iterators

    Revision 1.3  2001/01/25 11:57:32  lballabio
    Included outer product and sqrt into matrix.h

    Revision 1.2  2001/01/24 14:56:47  aleppo
    Added iterator combining-iterator

    Revision 1.1  2001/01/24 10:50:50  marmar
    Sample accumulator for multivariate analisys

*/

#ifndef quantlib_math_multivariate_accumulator_h
#define quantlib_math_multivariate_accumulator_h

#include "qldefines.h"
#include "null.h"
#include "qlerrors.h"
#include "dataformatters.h"
#include "array.h"
#include "matrix.h"
#include <vector>

namespace QuantLib {

    namespace Math {

    //! A sample accumulator for multivariate analysis
    /*! MultivariateAccumulator can accumulate vector-type samples and
        return the average vector, both in Array form and std::vector<double>
        form, and the covariance matrix
    */

        class MultivariateAccumulator {
        public:
            MultivariateAccumulator();
            MultivariateAccumulator(int size);
            //! \name Inspectors
            //@{
            //! size of each sample
            int size() const;
            //! number of samples collected
            double samples() const;
            //! returns the mean as an Array
            Array mean() const;
            //! returns the mean as a std::vector<double>
            std::vector<double> meanVector() const;
            //! returns the covariance Matrix
            Matrix covariance() const;
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
            int size_;
            double sampleNumber_;
            double sampleWeight_;
            Array sum_;
            Matrix quadraticSum_;
        };


        // inline definitions
        inline int MultivariateAccumulator::size() const {
            return size_;
        }

        inline double MultivariateAccumulator::samples() const {
          return sampleNumber_;
        }

        inline Array MultivariateAccumulator::mean() const {
          QL_REQUIRE(sampleWeight_ > 0.0,
                "Stat::mean() : sampleWeight_=0, unsufficient");
          return sum_/sampleWeight_;
        }

    }

}

#endif
