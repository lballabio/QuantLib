
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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

/*! \file swaption.hpp
    \brief Swaption class
*/

#ifndef quantlib_instruments_swaption_h
#define quantlib_instruments_swaption_h

#include <ql/numericalmethod.hpp>
#include <ql/option.hpp>
#include <ql/Instruments/simpleswap.hpp>

namespace QuantLib {

    //! %Swaption class
    /*! \ingroup instruments */
    class Swaption : public Option {
      public:
        class arguments;
        class results;
        Swaption(const boost::shared_ptr<SimpleSwap>& swap,
                 const boost::shared_ptr<Exercise>& exercise,
                 const RelinkableHandle<TermStructure>& termStructure,
                 const boost::shared_ptr<PricingEngine>& engine);
        bool isExpired() const;
        void setupArguments(Arguments*) const;
      private:
        // arguments
        boost::shared_ptr<SimpleSwap> swap_;
        const RelinkableHandle<TermStructure>& termStructure_;
    };

    //! %Arguments for swaption calculation
    class Swaption::arguments : public SimpleSwap::arguments,
                                public Option::arguments {
      public:
        arguments() : fairRate(Null<Real>()),
                      fixedRate(Null<Real>()),
                      fixedBPS(Null<Real>())
//                      , exerciseType(Exercise::Type(-1))
        {}
        Rate fairRate;
        Rate fixedRate;
        double fixedBPS;
//        Exercise::Type exerciseType;
//        std::vector<Time> exerciseTimes;
        void validate() const;
    };

    //! %Results from swaption calculation
    class Swaption::results : public Value {};

}


#endif
