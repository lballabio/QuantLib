
/*
 * Copyright (C) 2000-2001 QuantLib Group
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/
/*! \file mcoptionsample.hpp
    \brief Create a sample generator from a path generator and a path pricer

    $Source$
    $Name$
    $Log$
    Revision 1.1  2001/04/09 14:05:48  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.2  2001/04/06 18:46:20  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.1  2001/04/04 11:07:22  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.5  2001/01/30 15:48:51  marmar
    Documentation revised

    Revision 1.4  2001/01/29 15:00:49  marmar
    Modified to accomodate code-sharing with
    multi-dimensional Monte Carlo

    Revision 1.3  2001/01/17 14:37:56  nando
    tabs removed

    Revision 1.2  2001/01/17 11:54:02  marmar
    Some documentation added and 80 columns format enforced.

    Revision 1.1  2001/01/04 17:31:22  marmar
    Alpha version of the Monte Carlo tools.

*/

#ifndef quantlib_montecarlo_option_sample_h
#define quantlib_montecarlo_option_sample_h

#include "ql/qldefines.hpp"
#include "ql/handle.hpp"

namespace QuantLib {

    namespace MonteCarlo {
    /*!
    Given a path generator class PG, together with an instance "samplePath",
    and a path pricer PP, again with an instance "pathpricer", a sample
    generator OptionSample<PG, PP> returns, at each next(), a value for the
    option price.

    Minimal interfaces for PG and PP:

    class PG{
        PATH_TYPE next() const;
        double weight() const;
    };

    class PP{
            // The value() method will eventually evolve into
            // PP::ValueType value() const;
        double value(PATH_TYPE &) const;
    };
    */

        template<class PG, class PP>
        class OptionSample {
        public:
            OptionSample(){}
            OptionSample(Handle<PG> samplePath, Handle<PP> pathPricer);
            double next() const; // this will eventually evolve into
                                 // PP::ValueType next() const;
            double weight() const;
        private:
            mutable double weight_;
            mutable Handle<PG> samplePath_;
            Handle<PP> pathPricer_;
        };

        template<class PG, class PP>
        inline OptionSample<PG, PP>::OptionSample(Handle<PG> samplePath,
               Handle<PP> pathPricer): samplePath_(samplePath),
               pathPricer_(pathPricer), weight_(0){}

        template<class PG, class PP>
        inline double OptionSample<PG, PP>::next() const{
            double price = pathPricer_ -> value(samplePath_ -> next());
            weight_ = samplePath_ -> weight();
            return price;
        }

        template<class PG, class PP>
        inline double OptionSample<PG, PP>::weight() const{
            return weight_;
        }

    }

}
#endif
