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
 *          http://quantlib.sourceforge.net/LICENSE.TXT
*/
/*! \file multipathgenerator.h
    \brief Generates a multi path from a random-point generator

    $Source$
    $Name$
    $Log$
    Revision 1.2  2001/02/02 10:38:14  marmar
    Template class for a multi-path generator

    Revision 1.1  2001/01/25 16:11:17  marmar
    MultiPath, the first step for a multi-dimensional Monte Carlo
*/

#ifndef quantlib_montecarlo_multi_path_generator_h
#define quantlib_montecarlo_multi_path_generator_h

#include "qldefines.h"
#include "qlerrors.h"
#include "array.h"
#include "multipath.h"

namespace QuantLib {

    namespace MonteCarlo {
    /*! MultiPathGenerator<RAG> is a class that returns a random multi path.
        RAG is a sample generator which returns an array, must have the
        minimal interface,
        RAG{
            RAG();
            RAG(Array &average, Matrix &covariance, long seed);
            Array next();
            double weight();
        };
        
    */

        template <class RAG>
        class MultiPathGenerator {
        public:
        // typedef MultiPath SampleType;
        // this typedef would make MultiPathGenerator into a sample generator
            MultiPathGenerator();
            MultiPathGenerator(int timeDimension, const Array &average,
                            const Math::Matrix &covariance, long seed=0);
            MultiPath next() const;
            double weight() const{return weight_;}
        private:
            int numAssets_, timeDimension_;
            mutable double weight_;
            RAG rndArray_;
        };

        template <class RAG>
        inline MultiPathGenerator<RAG >::MultiPathGenerator():
                numAssets(0), timeDimension_(0), weight_(0) {}

        template <class RAG>
        inline MultiPathGenerator<RAG >::MultiPathGenerator(
            int timeDimension, const Array &average,
            const Math::Matrix &covariance, long seed):
            timeDimension_(timeDimension),
            numAssets_(average.size()),
            rndArray_(average, covariance, seed){

            QL_REQUIRE(timeDimension_ > 0,
                "Time dimension("+
                DoubleFormatter::toString(timeDimension_)+
                ") too small");
        }

        template <class RAG>
        inline MultiPath MultiPathGenerator<RAG >::next() const{

            MultiPath multiPath(numAssets_, timeDimension_);
            Array nextArray(numAssets_);

            weight_ = 1.0;
            for(int i = 0; i < timeDimension_; i++){
                nextArray = rndArray_.next();
                weight_ *= rndArray_.weight();
                std::copy(nextArray.begin(), nextArray.end(),     
                                            multiPath.column_begin(i));
            }
            return multiPath;
        }

    }

}

#endif
