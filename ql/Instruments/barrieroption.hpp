
/*
 Copyright (C) 2003, 2004 Neil Firth
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2003, 2004 StatPro Italia srl

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

#ifndef quantlib_barrier_option_hpp
#define quantlib_barrier_option_hpp

#include <ql/Instruments/oneassetstrikedoption.hpp>

namespace QuantLib {

    //! Placeholder for enumerated barrier types
    struct Barrier {
        enum Type { DownIn, UpIn, DownOut, UpOut };
    };

    //! %Barrier option on a single asset.
    /*! The analytic pricing engine will be used if none if passed.

        \ingroup instruments
    */
    class BarrierOption : public OneAssetStrikedOption {
      public:
        class arguments;
        class engine;
        BarrierOption(Barrier::Type barrierType,
                      Real barrier,
                      Real rebate,
                      const boost::shared_ptr<BlackScholesProcess>&,
                      const boost::shared_ptr<StrikedTypePayoff>& payoff,
                      const boost::shared_ptr<Exercise>& exercise,
                      const boost::shared_ptr<PricingEngine>& engine =
                          boost::shared_ptr<PricingEngine>());
        void setupArguments(Arguments*) const;
      protected:
        void performCalculations() const;
        // arguments
        Barrier::Type barrierType_;
        Real barrier_;
        Real rebate_;
    };

    //! %Arguments for barrier option calculation
    class BarrierOption::arguments : public OneAssetStrikedOption::arguments {
      public:
        Barrier::Type barrierType;
        Real barrier;
        Real rebate;
        void validate() const;
    };

    //! %Barrier engine base class
    class BarrierOption::engine 
        : public GenericEngine<BarrierOption::arguments,
                               BarrierOption::results> {};

}


#endif
