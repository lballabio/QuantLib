
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
/*! \file montecarlomodel.hpp

    \fullpath
    Include/ql/MonteCarlo/%montecarlomodel.hpp
    \brief Create a sample generator from a path generator and a path pricer

*/

// $Id$
// $Log$
// Revision 1.1  2001/09/03 13:56:11  nando
// source (*.hpp and *.cpp) moved under topdir/ql
//
// Revision 1.4  2001/08/31 15:23:46  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.3  2001/08/09 14:59:46  sigmud
// header modification
//
// Revision 1.2  2001/08/08 11:07:49  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.1  2001/08/07 17:33:03  nando
// 1) StandardPathGenerator now is GaussianPathGenerator;
// 2) StandardMultiPathGenerator now is GaussianMultiPathGenerator;
// 3) PathMonteCarlo now is MonteCarloModel;
// 4) added ICGaussian, a Gaussian distribution that use
//    QuantLib::Math::InvCumulativeNormalDistribution to convert uniform
//    distribution extractions into gaussian distribution extractions;
// 5) added a few trailing underscore to private members
// 6) style enforced here and there ....
//
// Revision 1.13  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.12  2001/07/27 14:45:23  marmar
// Method  void ddSamples(long n) added to GeneralMonteCarlo

#ifndef quantlib_montecarlo_model_h
#define quantlib_montecarlo_model_h

#include "ql/MonteCarlo/mcoptionsample.hpp"
#include "ql/MonteCarlo/generalmontecarlo.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        //! General purpose Monte Carlo model for path samples
        /*! Any Monte Carlo which uses a path samples has three main components,
            namely,
                - S, a sample accumulator,
                - PG, a path generator,
                - PP, a path pricer.
            MonteCarloModel<S, PG, PP> puts together these three elements.
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
        class MonteCarloModel {
          public:
            MonteCarloModel();
            MonteCarloModel(Handle<PG> pathGenerator,
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
        inline MonteCarloModel<S, PG, PP>::MonteCarloModel()
            : isInitialized_(false) {}

        template<class S, class PG, class PP>
        inline MonteCarloModel<S, PG, PP>::MonteCarloModel(
                Handle<PG> pathGenerator,
                Handle<PP> pathPricer,
                S sampleAccumulator) :
                sampleAccumulator_(sampleAccumulator),
                optionSample_(pathGenerator, pathPricer),
                monteCarlo_(sampleAccumulator_, optionSample_),
                isInitialized_(true){}

        template<class S, class PG, class PP>
        inline void MonteCarloModel<S, PG, PP>::
                    addSamples(long samples){
            QL_REQUIRE(isInitialized_ == true,
                       "MonteCarloModel must be initialized");
            monteCarlo_.addSamples(samples);
        }

        template<class S, class PG, class PP>
        inline const S& MonteCarloModel<S, PG, PP>::
                    sampleAccumulator() const{
            return monteCarlo_.sampleAccumulator();
        }

    }

}
#endif
