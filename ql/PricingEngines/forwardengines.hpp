
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

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

#include <ql/PricingEngines/vanillaengines.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <ql/TermStructures/impliedtermstructure.hpp>

namespace QuantLib {

    namespace PricingEngines {

        //! arguments for forward (strike-resetting) option calculation
        template<class ArgumentsType>
        class ForwardOptionArguments : public ArgumentsType {
          public:
            ForwardOptionArguments() : moneyness(Null<double>()),
                                       resetDate(Null<Date>()) {}
            void validate() const;
            double moneyness;
            Date resetDate;
        };

        template<class ArgumentsType>
        void ForwardOptionArguments<ArgumentsType>::validate() const {
            ArgumentsType::validate();
            QL_REQUIRE(moneyness != Null<double>(),
                       "ForwardOptionArguments::validate() : "
                       "null moneyness given");
            QL_REQUIRE(moneyness > 0.0,
                       "ForwardOptionArguments::validate() : "
                       "negative or zero moneyness given");
            QL_REQUIRE(resetDate != Null<Date>(),
                       "ForwardOptionArguments::validate() : "
                       "null reset date given");
            QL_REQUIRE(exercise.lastDate() >= resetDate,
                       "ForwardOptionArguments::validate() : "
                       "reset date greater than exercise time");
        }

        //! Forward engine base class
        template<class ArgumentsType, class ResultsType>
        class ForwardEngine : public
            GenericEngine<ForwardOptionArguments<ArgumentsType>,
                          ResultsType> {
        public:
            ForwardEngine(const Handle<GenericEngine<ArgumentsType,
                ResultsType> >&);
            void setOriginalArguments() const;
            void calculate() const;
            void getOriginalResults() const;
        protected:
            Handle<GenericEngine<ArgumentsType, ResultsType> > originalEngine_;
            ArgumentsType* originalArguments_;
            const ResultsType* originalResults_;
        };

        template<class ArgumentsType, class ResultsType>
        ForwardEngine<ArgumentsType, ResultsType>::ForwardEngine(
            const Handle<GenericEngine<ArgumentsType, ResultsType> >&
            originalEngine)
        : originalEngine_(originalEngine) {
            QL_REQUIRE(!originalEngine_.isNull(),
                "ForwardEngine::ForwardEngine : "
                "null engine or wrong engine type");
            originalResults_ = dynamic_cast<const ResultsType*>(
                originalEngine_->results());
            originalArguments_ = dynamic_cast<ArgumentsType*>(
                originalEngine_->arguments());
        }




        template<class ArgumentsType, class ResultsType>
        void ForwardEngine<ArgumentsType, ResultsType>::setOriginalArguments() const {

            originalArguments_->type = arguments_.type;
            originalArguments_->underlying = 1.0;
            originalArguments_->strike = arguments_.moneyness;
            originalArguments_->dividendTS = RelinkableHandle<TermStructure>(
                Handle<TermStructure>(
                new TermStructures::ImpliedTermStructure(
                        arguments_.dividendTS,
                        arguments_.resetDate, 
                        arguments_.resetDate)));
            originalArguments_->riskFreeTS = RelinkableHandle<TermStructure>(
                Handle<TermStructure>(
                new TermStructures::ImpliedTermStructure(
                    arguments_.riskFreeTS,
                    arguments_.resetDate, 
                    arguments_.resetDate)));

            // The following approach is plain wrong.
            // The right solution would be stochastic volatility or
            // at least local volatility
            // As a bare minimum one could extract from the Black vol surface
            // the implied vol at moneyness% of the forward value,
            // istead of the moneyness% of the spot value
            originalArguments_->volTS = 
                RelinkableHandle<BlackVolTermStructure>(
                    Handle<BlackVolTermStructure>(
                    new VolTermStructures::BlackConstantVol(
                            arguments_.resetDate,
                            arguments_.volTS->blackForwardVol(
                                arguments_.resetDate,
                                arguments_.exercise.lastDate(),
                                arguments_.moneyness*arguments_.underlying),
                            arguments_.volTS->dayCounter())));

            originalArguments_->exercise = arguments_.exercise;


            originalArguments_->validate();
        }



        template<class ArgumentsType, class ResultsType>
        void ForwardEngine<ArgumentsType, ResultsType>::calculate() const {

            setOriginalArguments();
            originalEngine_->calculate();
            getOriginalResults();
        }

        template<class ArgumentsType, class ResultsType>
        void ForwardEngine<ArgumentsType, ResultsType>::getOriginalResults() const {

            Time resetTime = arguments_.riskFreeTS->dayCounter().yearFraction(
                arguments_.riskFreeTS->referenceDate(), arguments_.resetDate);
            double discQ = arguments_.dividendTS->discount(
                arguments_.resetDate);

            results_.value = discQ * arguments_.underlying *
                originalResults_->value;
            results_.delta = discQ * originalResults_->value;
            results_.gamma = 0.0;
            results_.theta = arguments_.dividendTS->zeroYield(
                arguments_.resetDate) 
                * results_.value;
            results_.vega = discQ * arguments_.underlying *
                originalResults_->vega;
            results_.rho = discQ * arguments_.underlying *
                originalResults_->rho;
            results_.dividendRho = - resetTime * results_.value
                + discQ * originalResults_->dividendRho;

        }



        
        
        
        
        
        
        
        //! Forward Performance engine base class
        template<class ArgumentsType, class ResultsType>
        class ForwardPerformanceEngine : public ForwardEngine<ArgumentsType, ResultsType> {
        public:
            ForwardPerformanceEngine(const Handle<GenericEngine<ArgumentsType,
                ResultsType> >&);
            void getOriginalResults() const;
        };

        template<class ArgumentsType, class ResultsType>
        ForwardPerformanceEngine<ArgumentsType, ResultsType>::ForwardPerformanceEngine(
            const Handle<GenericEngine<ArgumentsType, ResultsType> >&
            originalEngine)
        : ForwardEngine<ArgumentsType, ResultsType>(originalEngine) {}

        template<class ArgumentsType, class ResultsType>
        void ForwardPerformanceEngine<ArgumentsType, ResultsType>::getOriginalResults() const {

            Time resetTime = arguments_.riskFreeTS->dayCounter().yearFraction(
                arguments_.riskFreeTS->referenceDate(), arguments_.resetDate);
            double discR = arguments_.riskFreeTS->discount(arguments_.resetDate);

            results_.value = discR * originalResults_->value;
            results_.delta = 0.0;
            results_.gamma = 0.0;
            results_.theta = arguments_.riskFreeTS->zeroYield(
                arguments_.resetDate) * results_.value;
            results_.vega = discR * originalResults_->vega;
            results_.rho = - resetTime * results_.value +
                discR * originalResults_->rho;
            results_.dividendRho = discR * originalResults_->dividendRho;

        }

    }

}

#endif
