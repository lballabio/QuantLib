
/*
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file barrieroption.hpp
    \brief Barrier option on a single asset
*/

#ifndef quantlib_barrier_option_h
#define quantlib_barrier_option_h

#include <ql/Instruments/vanillaoption.hpp>

namespace QuantLib {

    //! placeholder for enumerated barrier types
    struct Barrier {
        enum Type { DownIn, UpIn, DownOut, UpOut };
    };

    //! Barrier option on a single asset.
    /*! The analytic pricing engine will be used if none if passed. */
    class BarrierOption : public Option {
      public:
        class arguments;
        class results;
        BarrierOption(Barrier::Type barrierType,
                      double barrier,
                      double rebate,
                      Option::Type type,
                      const RelinkableHandle<Quote>& underlying,
                      double strike,
                      const RelinkableHandle<TermStructure>& dividendTS,
                      const RelinkableHandle<TermStructure>& riskFreeTS,
                      const Exercise& exercise,
                      const RelinkableHandle<BlackVolTermStructure>& volTS,
                      const Handle<PricingEngine>& engine =
                      Handle<PricingEngine>(),
                      const std::string& isinCode = "",
                      const std::string& description = "");
        //! \name greeks
        //@{
        double delta() const;
        double gamma() const;
        double theta() const;
        double vega() const;
        double rho() const;
        double dividendRho() const;
        double strikeSensitivity() const;
        //@}
        bool isExpired() const;
        void setupArguments(Arguments*) const;
      protected:
        void setupExpired() const;
        void performCalculations() const;
        // results
        mutable double delta_, gamma_, theta_, 
                       vega_, rho_, dividendRho_, strikeSensitivity_;
        // arguments
        Barrier::Type barrierType_;
        double barrier_;
        double rebate_;
        Option::Type type_;
        RelinkableHandle<Quote> underlying_;
        double strike_;
        Exercise exercise_;
        RelinkableHandle<TermStructure> riskFreeTS_, dividendTS_;
        RelinkableHandle<BlackVolTermStructure> volTS_;
    };

    //! %arguments for barrier option calculation
    class BarrierOption::arguments : public VanillaOption::arguments {
      public:
        Barrier::Type barrierType;
        double barrier;
        double rebate;
        void validate() const;
    };

    //! %results from barrier option calculation
    class BarrierOption::results : public virtual VanillaOption::results {};

}


#endif
