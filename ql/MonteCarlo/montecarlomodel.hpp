

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
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
                Sample<PATH_TYPE> next() const;
            };

            class PP :: unary_function<PATH_TYPE, VALUE_TYPE> {
                VALUE_TYPE operator()(PATH_TYPE &) const;
            };
            \endcode
        */
        template<class S, class PG, class PP>
        class MonteCarloModel {
          public:
            typedef typename PG::sample_type sample_type;
	    typedef typename PP::result_type result_type;
            MonteCarloModel(const Handle<PG>& pathGenerator,
                            const Handle<PP>& pathPricer,
                            const S& sampleAccumulator,
                            bool antitheticVariate,
                            const Handle<PP>& cvPathPricer = Handle<PP>(),
                            result_type cvOptionValue = result_type());
            void addSamples(Size samples);
            const S& sampleAccumulator(void) const;
          private:
            Handle<PG> pathGenerator_;
            Handle<PP> pathPricer_;
            S sampleAccumulator_;
            bool isAntitheticVariate_;
            Handle<PP> cvPathPricer_;
            result_type cvOptionValue_;
            bool isControlVariate_;
        };

        // inline definitions
        template<class S, class PG, class PP>
        inline MonteCarloModel<S, PG, PP>::MonteCarloModel(
	    const Handle<PG>& pathGenerator,
	    const Handle<PP>& pathPricer, const S& sampleAccumulator,
        bool antitheticVariate,
	    const Handle<PP>& cvPathPricer, 
	    MonteCarloModel<S, PG, PP>::result_type cvOptionValue)
        : pathGenerator_(pathGenerator), pathPricer_(pathPricer),
          sampleAccumulator_(sampleAccumulator),
          isAntitheticVariate_(antitheticVariate),
          cvPathPricer_(cvPathPricer), cvOptionValue_(cvOptionValue) {
            if (cvPathPricer_.isNull())
                isControlVariate_=false;
            else
                isControlVariate_=true;
        }

        template<class S, class PG, class PP>
        inline void MonteCarloModel<S, PG, PP>::addSamples(Size samples) {
            for(Size j = 1; j <= samples; j++) {

                sample_type path = pathGenerator_->next();
                result_type price = (*pathPricer_)(path.value);

                if (isControlVariate_)
                    price += cvOptionValue_-(*cvPathPricer_)(path.value);

                if (isAntitheticVariate_) {
                    path = pathGenerator_->antithetic();
                    result_type price2 = (*pathPricer_)(path.value);
                    if (isControlVariate_)
                        price2 += cvOptionValue_-(*cvPathPricer_)(path.value);
                    sampleAccumulator_.add((price+price2)/2.0, path.weight);
                } else {
                    sampleAccumulator_.add(price, path.weight);
                }
            }
        }

        template<class S, class PG, class PP>
        inline const S& MonteCarloModel<S, PG, PP>::sampleAccumulator() const {
            return sampleAccumulator_;
        }
    }
}


#endif
