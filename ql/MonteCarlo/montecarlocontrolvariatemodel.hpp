
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
    \brief Control Variate version of a Monte Carlo model

    \fullpath
    ql/MonteCarlo/%montecarlocontrolvariatemodel.hpp

*/

// $Id$

#ifndef quantlib_montecarlo_control_variate_model_h
#define quantlib_montecarlo_control_variate_model_h

namespace QuantLib {

    namespace MonteCarlo {

        //! Control Variate version of a Monte Carlo model
        /*! See the documentation of MonteCarloModel.
            MonteCarloControlVariateModel is a straight-forward
            extension, adding a control variate option with its
            path pricer and analytic value estimation.
        */
        template<class S, class PG, class PP>
        class MonteCarloControlVariateModel {
          public:
            MonteCarloControlVariateModel(const Handle<PG>& pathGenerator,
                                          const Handle<PP>& pathPricer,
                                          const Handle<PP>& cvPathPricer,
                                          double cvOptionValue,
                                          const S& sampleAccumulator);
            void addSamples(unsigned int samples);
            const S& sampleAccumulator(void) const;
          private:
            Handle<PG> pathGenerator_;
            Handle<PP> pathPricer_, cvPathPricer_;
            double cvOptionValue_;
            S sampleAccumulator_;
        };

        // inline definitions
        template<class S, class PG, class PP>
        inline MonteCarloControlVariateModel<S, PG, PP>::MonteCarloControlVariateModel(
                const Handle<PG>& pathGenerator,
                const Handle<PP>& pathPricer,
                const Handle<PP>& cvPathPricer,
                double cvOptionValue,
                const S& sampleAccumulator)
        : pathGenerator_(pathGenerator), pathPricer_(pathPricer),
          cvPathPricer_(cvPathPricer), cvOptionValue_(cvOptionValue),
          sampleAccumulator_(sampleAccumulator) {}

        template<class S, class PG, class PP>
        inline void MonteCarloControlVariateModel<S, PG, PP>::
                    addSamples(unsigned int samples) {
            for(unsigned int j = 1; j <= samples; j++) {
                typename PG::sample_type path = pathGenerator_->next();
                double weight = pathGenerator_->weight();
                typename PP::result_type price = (*pathPricer_)(path);
                price += cvOptionValue_-(*cvPathPricer_)(path);
                sampleAccumulator_.add(price, weight);
            }
        }

        template<class S, class PG, class PP>
        inline const S& MonteCarloControlVariateModel<S, PG, PP>::
                    sampleAccumulator() const{
            return sampleAccumulator_;
        }
    }
}
#endif
