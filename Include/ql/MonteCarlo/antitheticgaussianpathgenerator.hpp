
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
/*! \file antitheticgaussianpathgenerator.hpp
    \fullpath Include/ql/MonteCarlo/%antitheticgaussianpathgenerator.hpp
    \brief Generates path from random points

*/

// $Id$
// $Log$
// Revision 1.1  2001/08/22 11:18:06  nando
// removed unused default constructor
// added a few typedef for argument type and value type
//


#ifndef quantlib_montecarlo_antithetic_gaussian_path_generator_h
#define quantlib_montecarlo_antithetic_gaussian_path_generator_h

#include "ql/MonteCarlo/gaussianrandomgenerator.hpp"
#include "ql/MonteCarlo/randomarraygenerator.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        /*! 
            The following a path generator
            with anthitetic variance reduction technique.
            To be used with a Gaussian (or at least symmetric) RandomArrayGenerator
        */
        template <class RAG>
        class AntitheticGaussianPathGenerator {
            public:
                AntitheticGaussianPathGenerator<RAG>(const RAG& rag)
                    : rag_(rag), refreshArray_(true) {}
                Array next() const;
                double weight() const {return weight_;}
                int size() const {return rag_.size();}
            private:
                RAG rag_;
                mutable double weight_;
                mutable Array lastArray_;
                mutable bool refreshArray_;
        };

        template <class RAG>
        inline Array AntitheticGaussianPathGenerator<RAG>::next() const {
            if (refreshArray_) {
                lastArray_ = rag_.next();
                weight_ = rag_.weight();
                refreshArray_ = false;
                return lastArray_;
            } else {
                refreshArray_ = true;
                return -lastArray_;
            }
        }

    }
}


#endif
