/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file writerextensibleoption.hpp
    \brief Writer-extensible option
*/

#ifndef quantlib_writer_extensible_option_hpp
#define quantlib_writer_extensible_option_hpp

#include <ql/instruments/oneassetoption.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    //! Writer-extensible option
    /*! If out of the money at the original exercise date, this option
        is extended until a later exercise date with an amended strike.
    */
    class WriterExtensibleOption : public OneAssetOption {
      public:
        class arguments;
        class engine;
        WriterExtensibleOption(const ext::shared_ptr<PlainVanillaPayoff>& payoff1,
                               const ext::shared_ptr<Exercise>& exercise1,
                               const ext::shared_ptr<PlainVanillaPayoff>& payoff2,
                               ext::shared_ptr<Exercise> exercise2);
        // inspectors
        ext::shared_ptr<Payoff> payoff2() { return payoff2_; }
        ext::shared_ptr<Exercise> exercise2() { return exercise2_; };
        // Instrument interface
        bool isExpired() const override;
        void setupArguments(PricingEngine::arguments*) const override;

      private:
        ext::shared_ptr<StrikedTypePayoff> payoff2_;
        ext::shared_ptr<Exercise> exercise2_;
    };

    //! Additional arguments for writer-extensible options
    class WriterExtensibleOption::arguments
        : public OneAssetOption::arguments {
      public:
        void validate() const override;
        ext::shared_ptr<Payoff> payoff2;
        ext::shared_ptr<Exercise> exercise2;
    };

    //! Base engine for writer-extensible options
    class WriterExtensibleOption::engine :
        public GenericEngine<WriterExtensibleOption::arguments,
                             WriterExtensibleOption::results> {};

}

#endif
