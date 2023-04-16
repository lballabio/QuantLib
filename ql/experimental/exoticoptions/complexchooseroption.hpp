/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

/*! \file complexchooseroption.hpp
    \brief Complex chooser option
*/

#ifndef quantlib_complex_chooser_option_hpp
#define quantlib_complex_chooser_option_hpp

#include <ql/instruments/oneassetoption.hpp>

namespace QuantLib{

    class GeneralizedBlackScholesProcess;

    class ComplexChooserOption : public OneAssetOption {
      public:
        class arguments;
        class engine;
        ComplexChooserOption(Date choosingDate,
                             Real strikeCall,
                             Real strikePut,
                             const std::shared_ptr<Exercise>& exerciseCall,
                             const std::shared_ptr<Exercise>& exercisePut);
        void setupArguments(PricingEngine::arguments*) const override;

      protected:
        Date choosingDate_;
        Real strikeCall_;
        Real strikePut_;
        const std::shared_ptr<Exercise>& exerciseCall_;
        const std::shared_ptr<Exercise>& exercisePut_;
    };

    class ComplexChooserOption::arguments : public OneAssetOption::arguments {
      public:
        void validate() const override;
        Date choosingDate;
        Real strikeCall;
        Real strikePut;
        std::shared_ptr<Exercise> exerciseCall;
        std::shared_ptr<Exercise> exercisePut;
    };

    class ComplexChooserOption::engine
        : public GenericEngine<ComplexChooserOption::arguments,
                               ComplexChooserOption::results> {};

}

#endif
