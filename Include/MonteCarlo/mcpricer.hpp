
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

/*! \file mcpricer.hpp

    $Source$
    $Name$
    $Log$
    Revision 1.3  2001/04/06 18:46:20  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.2  2001/04/04 12:13:23  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.1  2001/04/04 11:07:22  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.7  2001/02/02 10:40:56  marmar
    Remark updated

    Revision 1.6  2001/01/30 15:56:06  marmar
    Now using OneFactorMonteCarloOption

    Revision 1.5  2001/01/30 09:03:38  marmar
    GeneralMonteCarlo contains the basic ideas of any Monte Carlo
    simulation: sample from a "sample generator" and accumulate
    in a "sample accumulator".

    Revision 1.4  2001/01/17 14:37:56  nando
    tabs removed

    Revision 1.3  2001/01/17 11:54:02  marmar
    Some documentation added and 80 columns format enforced.

    Revision 1.2  2001/01/05 11:42:37  lballabio
    Renamed SinglePathEuropeanPricer to EuropeanPathPricer

    Revision 1.1  2001/01/04 17:31:22  marmar
    Alpha version of the Monte Carlo tools.

*/

#ifndef quantlib_montecarlo_pricer_h
#define quantlib_montecarlo_pricer_h

#include "qldefines.hpp"
#include "Math/statistics.hpp"
#include "pathpricer.hpp"
#include "standardpathgenerator.hpp"
#include "onefactormontecarlooption.hpp"

namespace QuantLib {

    namespace Pricers {
        //! Base class for one-factor Monte Carlo pricers
        /*! McPricer is the base class for the Monte Carlo pricers depending
            from one factor.
            Eventually it might be linked to the general tree of pricers,
            in order to have available tools like impliedVolaitlity.
            Also, it will, eventually, implement the calculation of greeks
            in montecarlo methods.
            Deriving a class from McPricer gives an easy way to write
            a one-factor Monte Carlo Pricer.
            See McEuropeanPricer as an example
        */

        class McPricer {
        public:
            McPricer() : isInitialized_(false){}
            McPricer(long samples, long seed=0);
            ~McPricer(){}
            virtual double value() const;
            virtual double errorEstimate() const;
        protected:
            bool isInitialized_;
            long seed_;
            mutable long samples_;
            mutable MonteCarlo::OneFactorMonteCarloOption montecarloPricer_;
        };

        inline McPricer::McPricer(long samples, long seed):
                    samples_(samples), seed_(seed), isInitialized_(true){}

        inline double McPricer::value() const{
            QL_REQUIRE(isInitialized_,
                "McPricer::value has not been initialized");
            return montecarloPricer_.sampleAccumulator(samples_).mean();
        }

        inline double McPricer::errorEstimate() const {
            QL_REQUIRE(isInitialized_,
                "McPricer::errorEstimate has not been initialized");
            return montecarloPricer_.sampleAccumulator().errorEstimate();
        }

    }

}

#endif
