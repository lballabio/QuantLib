
/*
 Copyright (C) 2002 Ferdinando Ametrano

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
/*! \file forwardengines.hpp
    \brief Forward (strike-resetting) option engines

    \fullpath
    ql/PricingEngines/%forwardengines.hpp
*/

// $Id$

#ifndef quantlib_forward_engines_h
#define quantlib_forward_engines_h

#include <ql/PricingEngines/genericengine.hpp>

namespace QuantLib {

    namespace PricingEngines {

        //! parameters for forward (strike-resetting) option calculation
        template<class ArgumentsType>
        class ForwardOptionParameters : public ArgumentsType {
          public:
            ForwardOptionParameters() : moneyness(Null<double>()),
                                        resetTime(Null<double>()) {}
            void validate() const;
            double moneyness;
            Time resetTime;
        };

        template<class ArgumentsType>
        void ForwardOptionParameters<ArgumentsType>::validate() const {
            ArgumentsType::validate();
            QL_REQUIRE(moneyness != Null<double>(),
                       "null moneyness given");
            QL_REQUIRE(moneyness > 0.0,
                       "negative or zero moneyness given");
            QL_REQUIRE(resetTime != Null<double>(),
                       "null reset time given");
            QL_REQUIRE(resetTime >= 0.0,
                       "negative reset time given");
            QL_REQUIRE(residualTime >= resetTime,
                       "reset time greater than residual time");
        }

        //! Forward engine base class
        template<class ArgumentsType, class ResultsType>
        class ForwardEngine : public
            GenericEngine<ForwardOptionParameters<ArgumentsType>,
                          ResultsType> {
        public:
            ForwardEngine(const Handle<GenericEngine<ArgumentsType,
                ResultsType> >&);
            void calculate() const;
        private:
            Handle<GenericEngine<ArgumentsType, ResultsType> > originalEngine_;
            ArgumentsType* args_;
            const ResultsType* results_;
        };

        template<class ArgumentsType, class ResultsType>
        ForwardEngine<ArgumentsType, ResultsType>::ForwardEngine(
            const Handle<GenericEngine<ArgumentsType, ResultsType> >& originalEngine)
        : originalEngine_(originalEngine) {
            QL_REQUIRE(!originalEngine_.isNull(), "null engine or wrong engine type");
            results_ = dynamic_cast<const ResultsType*>(originalEngine_->results());
            args_ = dynamic_cast<ArgumentsType*>(originalEngine_->arguments());
        }

        template<class ArgumentsType, class ResultsType>
        void ForwardEngine<ArgumentsType, ResultsType>::calculate() const {
        }

    }

}

#endif
