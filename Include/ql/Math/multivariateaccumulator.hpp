
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

/*! \file multivariateaccumulator.hpp

    \fullpath
    Include/ql/Math/%multivariateaccumulator.hpp
    \brief A simple accumulator for vector-type samples

*/

// $Id$
// $Log$
// Revision 1.12  2001/08/31 15:23:45  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.11  2001/08/28 13:37:35  nando
// unsigned int instead of int
//
// Revision 1.10  2001/08/09 14:59:46  sigmud
// header modification
//
// Revision 1.9  2001/08/08 11:07:48  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.8  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.7  2001/07/25 15:47:27  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.6  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.5  2001/05/28 13:09:55  nando
// R019-branch-merge3 merged into trunk
//

#ifndef quantlib_math_multivariate_accumulator_h
#define quantlib_math_multivariate_accumulator_h

#include "ql/null.hpp"
#include "ql/dataformatters.hpp"
#include "ql/Math/matrix.hpp"
#include <vector>

namespace QuantLib {

    namespace Math {

        //! A sample accumulator for multivariate analysis
        /*! MultivariateAccumulator can accumulate vector-type samples and
            return the average vector, both in Array form and 
            std::vector<double> form, and the covariance matrix
        */
        class MultivariateAccumulator {
          public:
            MultivariateAccumulator();
            MultivariateAccumulator(unsigned int size);
            //! \name Inspectors
            //@{
            //! size of each sample
            unsigned int size() const;
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
            unsigned int size_;
            double sampleNumber_;
            double sampleWeight_;
            Array sum_;
            Matrix quadraticSum_;
        };


        // inline definitions
        inline unsigned int MultivariateAccumulator::size() const {
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
