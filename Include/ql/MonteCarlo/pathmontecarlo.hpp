
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
/*! \file pathmontecarlo.hpp
    \brief Create a sample generator from a path generator and a path pricer

    $Source$
    $Name$
    $Log$
    Revision 1.3  2001/05/24 11:34:07  nando
    smoothing #include xx.hpp

    Revision 1.2  2001/05/23 19:30:27  nando
    smoothing #include xx.hpp

    Revision 1.1  2001/04/09 14:05:48  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.2  2001/04/06 18:46:20  nando
    changed Authors, Contributors, Licence and copyright header

*/

#ifndef quantlib_montecarlo_option_h
#define quantlib_montecarlo_option_h

#include "ql/handle.hpp"
#include "ql/MonteCarlo/mcoptionsample.hpp"
#include "ql/MonteCarlo/generalmontecarlo.hpp"

namespace QuantLib {

    namespace MonteCarlo {

    //! \brief General purpose Monte Carlo for path samples
    /*! Any Monte Carlo which uses a path samples has three main components,
            S, a sample accumulator,
            PG, a path generator, and
            PP, a path pricer.
        PathMonteCarlo<S, PG, PP> puts together these three elements.
        The constructor accepts two safe references, i.e. two smart
        pointers, one to a path generator and the other to a path pricer.
        The GeneralMonteCarlo is used to fill the statistic accumulator.

        The minimal interfaces for the classes S, PG, and PP are:

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

    */
        template<class S, class PG, class PP>
        class PathMonteCarlo {
        public:
            PathMonteCarlo():isInitialized_(false){}
            PathMonteCarlo(Handle<PG> pathGenerator, Handle<PP> pathPricer);
            S sampleAccumulator(long samples = 0) const;
        private:
            mutable bool isInitialized_;
            S sampleAccumulator_;
            OptionSample<PG,PP> optionSample_;
            GeneralMonteCarlo<S, OptionSample<PG,PP> > monteCarlo_;
        };

        // inline definitions
        template<class S, class PG, class PP>
        inline PathMonteCarlo<S, PG, PP>::PathMonteCarlo(
                Handle<PG> pathGenerator,
                Handle<PP> pathPricer) :
                sampleAccumulator_(),
                optionSample_(pathGenerator, pathPricer),
                monteCarlo_(sampleAccumulator_, optionSample_),
                isInitialized_(true){}

        template<class S, class PG, class PP>
        inline S PathMonteCarlo<S, PG, PP>::
                    sampleAccumulator(long samples) const{
            QL_REQUIRE(isInitialized_ == true,
                       "PathMonteCarlo must be initialized");
            return monteCarlo_.sampleAccumulator(samples);
        }

    }

}
#endif
