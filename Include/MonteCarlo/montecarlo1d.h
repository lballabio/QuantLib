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
    Revision 1.9  2001/02/05 14:49:12  enri
    added some files to Makefile.am files

    Revision 1.8  2001/01/30 15:47:36  marmar
    MonteCarlo1D obsolete, replaced by OneFactorMonteCarloOption

    Revision 1.7  2001/01/30 09:03:38  marmar
    GeneralMonteCarlo contains the basic ideas of any Monte Carlo
    simulation: sample from a "sample generator" and accumulate
    in a "sample accumulator".

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
#include "handle.h"
#include "statistics.h"
#include "pathpricer.h"
#include "standardpathgenerator.h"
#include "mcoptionsample.h"
#include "generalmontecarlo.h"

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
            Math::Statistics sampleAccumulator_;
            OptionSample<StandardPathGenerator,PathPricer> optionSample_;
            GeneralMonteCarlo<Math::Statistics, 
                  OptionSample<StandardPathGenerator,PathPricer> > monteCarlo_;
        };

        // inline definitions        
        inline MonteCarlo1D::MonteCarlo1D(
                Handle<StandardPathGenerator> pathGenerator,
                Handle<PathPricer> pathPricer) :
                sampleAccumulator_(), 
                optionSample_(pathGenerator, pathPricer),
                monteCarlo_(sampleAccumulator_, optionSample_){}

        inline double MonteCarlo1D::value(long samples) const{        
            monteCarlo_.sample(samples);
            return monteCarlo_.statisticAccumulator().mean();
        }

        inline double MonteCarlo1D::errorEstimate() const{        
            return monteCarlo_.statisticAccumulator().errorEstimate();
        }

    }

}

#endif
