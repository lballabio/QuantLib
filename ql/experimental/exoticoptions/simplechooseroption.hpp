/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file simplechooseroption.hpp
    \brief Simple chooser option on a single asset
*/

#ifndef quantlib_simple_chooser_option_hpp
#define quantlib_simple_chooser_option_hpp

#include <ql/instruments/oneassetoption.hpp>

namespace QuantLib {

    //! Simple chooser option
    /*! This option gives the holder the right to choose, at a future
        date prior to exercise, whether the option should be a call or
        a put.  The exercise date and strike are the same for both
        call and put option.
    */
    class SimpleChooserOption : public OneAssetOption {
      public:
        class arguments;
        class engine;
        SimpleChooserOption(Date choosingDate,
                            Real strike,
                            const std::shared_ptr<Exercise>& exercise);
        void setupArguments(PricingEngine::arguments*) const override;

      protected:
        Date choosingDate_;
    };

    //! Extra %arguments for single chooser option
    class SimpleChooserOption::arguments
        : public OneAssetOption::arguments {
      public:
        arguments() : choosingDate(Null<Date>()) {}
        void validate() const override;
        Date choosingDate;
    };

    //! Simple chooser option %engine base class
    class SimpleChooserOption::engine
        : public GenericEngine<SimpleChooserOption::arguments,
                               SimpleChooserOption::results> {};

}

#endif
