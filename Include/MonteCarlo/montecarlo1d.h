/*
 * Copyright (C) 2000, 2001
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
/*! \file montecarlo1d.h
    \brief Create a sample generator from a path generator and a path pricer

    $Source$
    $Name$
    $Log$
    Revision 1.6  2001/01/29 15:00:49  marmar
    Modified to accomodate code-sharing with
    multi-dimensional Monte Carlo

    Revision 1.5  2001/01/24 13:14:25  marmar
    Removed typedef

    Revision 1.4  2001/01/17 14:37:56  nando
    tabs removed

    Revision 1.3  2001/01/17 11:54:02  marmar
    Some documentation added and 80 columns format enforced.

    Revision 1.2  2001/01/05 11:02:37  lballabio
    Renamed SinglePathPricer to PathPricer

    Revision 1.1  2001/01/04 17:31:22  marmar
    Alpha version of the Monte Carlo tools.

*/


#ifndef quantlib_montecarlo_one_dimensional_h
#define quantlib_montecarlo_one_dimensional_h

#include "qldefines.h"
#include "rate.h"
#include "statistics.h"
#include "standardpathgenerator.h"
#include "mcoptionsample.h"
#include "handle.h"
#include "pathpricer.h"

namespace QuantLib {

    namespace MonteCarlo {

        //! General purpose 1D Monte Carlo pricer
        /*! MonteCarlo1D is initialized giving a safe reference, that is
            an handle, to a PathPricer, some stochastic parameters, the number
            of time steps, and, possibly, the seed of the path generator.
            It then can be interrogated for the value obtained from a certain
            number of samples and for the estimated error.
        */

        class MonteCarlo1D {
        public:
            MonteCarlo1D(){}
            MonteCarlo1D(Handle<StandardPathGenerator> pathGenerator,
                Handle<PathPricer> pathPricer);
            double value(long samples) const;
            double errorEstimate() const;
        private:
            mutable OptionSample<StandardPathGenerator,PathPricer>
                optionSample_;
            mutable Math::Statistics sampleAccumulator_;
        };

        // inline definitions

        inline MonteCarlo1D::MonteCarlo1D(
                Handle<StandardPathGenerator> pathGenerator,
                Handle<PathPricer> pathPricer) :
                sampleAccumulator_(), 
                optionSample_(pathGenerator, pathPricer){}

        inline double MonteCarlo1D::value(long samples) const{
            for(long i=1; i<=samples; i++)
                sampleAccumulator_.add(optionSample_.next(),
                    optionSample_.weight());
            return sampleAccumulator_.mean();
        }

        inline double MonteCarlo1D::errorEstimate() const{
            return sampleAccumulator_.errorEstimate();
        }

    }

}

#endif
