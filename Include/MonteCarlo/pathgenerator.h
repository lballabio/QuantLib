/*
 * Copyright (C) 2000
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
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/
/*! \file pathgenerator.h
    \brief Generates path from random points

    $Source$
    $Name$
    $Log$
    Revision 1.6  2001/02/05 14:49:12  enri
    added some files to Makefile.am files

    Revision 1.5  2001/02/02 10:21:27  marmar
    PathGenerator replaced by RandomArrayGenerator

    Revision 1.4  2001/01/25 10:03:55  marmar
    JacobiDecomposition renamed SymmetricSchurDecomposition

    Revision 1.3  2001/01/17 14:37:56  nando
    tabs removed

    Revision 1.2  2001/01/17 11:54:02  marmar
    Some documentation added and 80 columns format enforced.

    Revision 1.1  2001/01/04 17:31:22  marmar
    Alpha version of the Monte Carlo tools.

*/

#ifndef quantlib_montecarlo_path_generator_h
#define quantlib_montecarlo_path_generator_h

#include "qldefines.h"
#include "path.h"

namespace QuantLib {

    namespace MonteCarlo {
    /*!
    PathGenerator<RP> returns a path starting from a random point,
    where RP is a sample generator which returns a random point
    */

        template <class RP>
        class PathGenerator {
        public:
            // typedef Path SampleType;
            // this typedef would make PathGenerator into a sample generator
            PathGenerator() : theDimension(0), weight_(0) {}
            PathGenerator(int dimension, double average = 0.0,
                          double stddev = 1.0, long seed=0);
            const Path & next() const;
            const double weight() const{return weight_;}
            const double average() const{return average_;}
            const double stddev() const{return stddev_;}
        private:
          int theDimension;
          double average_, stddev_;
          RP rndPoint;
          mutable Path path_;
          mutable double weight_;
        };

        template <class RP>
        inline PathGenerator<RP >::PathGenerator(int dimension, double average,
                double stddev, long seed): theDimension(dimension), 
                path_(dimension), average_(average), stddev_(stddev), 
                rndPoint(seed){}

        template <class RP>
        inline const Path & PathGenerator<RP >::next() const{

            QL_REQUIRE(theDimension > 0,
                        "template PathGenerator: dimension must be positive");
            unsigned int n = path_.size();
            weight_ = 1.0;
            for(unsigned int i=0;i<n;i++){
                path_[i] = rndPoint.next()*stddev_+average_;
                weight_ *= rndPoint.weight();
            }
            return path_;
        }

    }

}

#endif

