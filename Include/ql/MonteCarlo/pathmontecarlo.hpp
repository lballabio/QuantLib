
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
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/Authors.txt
*/
/*! \file pathmontecarlo.hpp
    \brief Create a sample generator from a path generator and a path pricer

    $Id$
*/

// $Source$
// $Log$
// Revision 1.12  2001/07/27 14:45:23  marmar
// Method  void ddSamples(long n) added to GeneralMonteCarlo
//
// Revision 1.11  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.10  2001/07/20 13:06:57  marmar
// Monte Carlo interfaces imporved
//
// Revision 1.9  2001/07/13 15:25:13  marmar
// MonteCarlo interface changed
//
// Revision 1.8  2001/07/13 14:29:08  sigmud
// removed a few gcc compile warnings
//
// Revision 1.7  2001/06/05 09:35:13  lballabio
// Updated docs to use Doxygen 1.2.8
//
// Revision 1.6  2001/05/25 09:29:40  nando
// smoothing #include xx.hpp and cutting old Log messages
//
// Revision 1.5  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_montecarlo_option_h
#define quantlib_montecarlo_option_h

#include "ql/MonteCarlo/mcoptionsample.hpp"
#include "ql/MonteCarlo/generalmontecarlo.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        //! General purpose Monte Carlo for path samples
        /*! Any Monte Carlo which uses a path samples has three main components,
            namely, 
                - S, a sample accumulator,
                - PG, a path generator, 
                - PP, a path pricer.
            PathMonteCarlo<S, PG, PP> puts together these three elements.
            The constructor accepts two safe references, i.e. two smart
            pointers, one to a path generator and the other to a path pricer.
            The GeneralMonteCarlo is used to fill the statistic accumulator.
    
            The minimal interfaces for the classes S, PG, and PP are:
    
            \code
            class S{
                void add(VALUE_TYPE sample, double weight) const;
            };
    
            class PG{
                PATH_TYPE next() const;
                double weight() const;
            };
    
            class PP{
                VALUE_TYPE value(PATH_TYPE &) const;
            };
            \endcode
        */
        template<class S, class PG, class PP>
        class PathMonteCarlo {
          public:
            PathMonteCarlo();
            PathMonteCarlo(Handle<PG> pathGenerator, 
                           Handle<PP> pathPricer,
                           S sampleAccumulator);
            void addSamples(long samples);
            const S& sampleAccumulator(void) const;
          private:
            S sampleAccumulator_;
            OptionSample<PG,PP> optionSample_;
            GeneralMonteCarlo<S, OptionSample<PG,PP> > monteCarlo_;
            bool isInitialized_;
        };

        // inline definitions
        template<class S, class PG, class PP>
        inline PathMonteCarlo<S, PG, PP>::PathMonteCarlo() 
            : isInitialized_(false) {}
 
        template<class S, class PG, class PP>
        inline PathMonteCarlo<S, PG, PP>::PathMonteCarlo(
                Handle<PG> pathGenerator,
                Handle<PP> pathPricer,
                S sampleAccumulator) :
                sampleAccumulator_(sampleAccumulator),
                optionSample_(pathGenerator, pathPricer),
                monteCarlo_(sampleAccumulator_, optionSample_),
                isInitialized_(true){}

        template<class S, class PG, class PP>
        inline void PathMonteCarlo<S, PG, PP>::
                    addSamples(long samples){
            QL_REQUIRE(isInitialized_ == true,
                       "PathMonteCarlo must be initialized");
            monteCarlo_.addSamples(samples);
        }

        template<class S, class PG, class PP>
        inline const S& PathMonteCarlo<S, PG, PP>::
                    sampleAccumulator() const{
            return monteCarlo_.sampleAccumulator();
        }

    }

}
#endif
