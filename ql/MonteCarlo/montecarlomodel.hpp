
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
    \brief Create a sample generator from a path generator and a path pricer

    \fullpath
    ql/MonteCarlo/%montecarlomodel.hpp

*/

// $Id$

#ifndef quantlib_montecarlo_model_h
#define quantlib_montecarlo_model_h

#include <ql/handle.hpp>

namespace QuantLib {

    //! Monte Carlo framework
    /*! See sect. \ref mcarlo */
    namespace MonteCarlo {

        //! General purpose Monte Carlo model for path samples
        /*! Any Monte Carlo which uses path samples has three main components,
            namely,
                - S, a sample accumulator,
                - PG, a path generator,
                - PP, a path pricer.
            MonteCarloModel<S, PG, PP> puts together these three elements.
            The constructor accepts two safe references, i.e. two smart
            pointers, one to a path generator and the other to a path pricer.
            In case of control variate technique the user should provide the
            additional control option, namely the option path pricer and the
            option value.

            The minimal interfaces for the classes S, PG, and PP are:

            \code
            class S{
                void add(VALUE_TYPE sample, double weight) const;
            };

            class PG{
                PATH_TYPE next() const;
                double weight() const;
            };

            class PP :: unary_function<PATH_TYPE, VALUE_TYPE> {
                VALUE_TYPE operator()(PATH_TYPE &) const;
            };
            \endcode
        */
        template<class S, class PG, class PP>
        class MonteCarloModel {
          public:
            MonteCarloModel(const Handle<PG>& pathGenerator,
                            const Handle<PP>& pathPricer,
                            const S& sampleAccumulator,
                            const Handle<PP>& cvPathPricer = Handle<PP>(),
                            double cvOptionValue=0);
            void addSamples(size_t samples);
            const S& sampleAccumulator(void) const;
          private:
            Handle<PG> pathGenerator_;
            Handle<PP> pathPricer_;
            S sampleAccumulator_;
            Handle<PP> cvPathPricer_;
            double cvOptionValue_;
            bool isControlVariate_;
        };

        // inline definitions
        template<class S, class PG, class PP>
        inline MonteCarloModel<S, PG, PP>::MonteCarloModel(
                const Handle<PG>& pathGenerator,
                const Handle<PP>& pathPricer, const S& sampleAccumulator,
                const Handle<PP>& cvPathPricer, double cvOptionValue)
        : pathGenerator_(pathGenerator), pathPricer_(pathPricer),
          sampleAccumulator_(sampleAccumulator), cvPathPricer_(cvPathPricer),
          cvOptionValue_(cvOptionValue) {
            if (cvPathPricer_.isNull())
                isControlVariate_=false;
            else
                isControlVariate_=true;
        }

        template<class S, class PG, class PP>
        inline void MonteCarloModel<S, PG, PP>::
                    addSamples(size_t samples) {
            for(size_t j = 1; j <= samples; j++) {
                typename PG::sample_type path = pathGenerator_->next();
                typename PP::result_type price = (*pathPricer_)(path);
                if (isControlVariate_)
                    price += cvOptionValue_-(*cvPathPricer_)(path);
                sampleAccumulator_.add(price, pathGenerator_->weight());
            }
        }

        template<class S, class PG, class PP>
        inline const S& MonteCarloModel<S, PG, PP>::
                    sampleAccumulator() const{
            return sampleAccumulator_;
        }
    }
}
#endif
