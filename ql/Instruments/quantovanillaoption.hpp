
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file quantovanillaoption.hpp
    \brief Quanto version of a vanilla option
*/

#ifndef quantlib_quanto_vanilla_option_h
#define quantlib_quanto_vanilla_option_h

#include <ql/Instruments/vanillaoption.hpp>

namespace QuantLib {

    //! %Arguments for quanto option calculation
    template<class ArgumentsType>
    class QuantoOptionArguments : public ArgumentsType {
      public:
        QuantoOptionArguments() : correlation(Null<double>()) {}
        void validate() const;
        double correlation;
        RelinkableHandle<TermStructure> foreignRiskFreeTS;
        RelinkableHandle<BlackVolTermStructure> exchRateVolTS;
    };

    //! %Results from quanto option calculation
    template<class ResultsType>
    class QuantoOptionResults : public ResultsType {
      public:
        QuantoOptionResults() { reset() ;}
        void reset() { 
            ResultsType::reset();
            qvega = qrho = qlambda = Null<double>();
        }
        double qvega;
        double qrho;
        double qlambda;
    };

    //! quanto version of a vanilla option
    /*! \ingroup instruments */
    class QuantoVanillaOption : public VanillaOption {
      public:
        typedef QuantoOptionArguments<VanillaOption::arguments> arguments;
        typedef QuantoOptionResults<VanillaOption::results> results;
        QuantoVanillaOption(
            const RelinkableHandle<TermStructure>& foreignRiskFreeTS,
            const RelinkableHandle<BlackVolTermStructure>& exchRateVolTS,
            const RelinkableHandle<Quote>& correlation,
            const boost::shared_ptr<BlackScholesProcess>& stochProc,
            const boost::shared_ptr<StrikedTypePayoff>& payoff,
            const boost::shared_ptr<Exercise>& exercise,
            const boost::shared_ptr<PricingEngine>& engine);
        //! \name greeks
        //@{
        double qvega() const;
        double qrho() const;
        double qlambda() const;
        //@}
        void setupArguments(Arguments*) const;
      protected:
        void setupExpired() const;
        void performCalculations() const;
        // arguments
        RelinkableHandle<TermStructure> foreignRiskFreeTS_;
        RelinkableHandle<BlackVolTermStructure> exchRateVolTS_;
        RelinkableHandle<Quote> correlation_;
        // results
        mutable double qvega_, qrho_, qlambda_;
    };


    // template definitions

    template<class ArgumentsType>
    void QuantoOptionArguments<ArgumentsType>::validate() const {
        ArgumentsType::validate();
        QL_REQUIRE(!foreignRiskFreeTS.isNull(),
                   "null foreign risk free term structure");
        QL_REQUIRE(!exchRateVolTS.isNull(),
                   "null exchange rate vol term structure");
        QL_REQUIRE(correlation != Null<double>(),
                   "null correlation given");
    }

}


#endif
