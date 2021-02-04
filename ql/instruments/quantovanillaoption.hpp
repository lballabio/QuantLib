/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file quantovanillaoption.hpp
    \brief Quanto version of a vanilla option
*/

#ifndef quantlib_quanto_vanilla_option_hpp
#define quantlib_quanto_vanilla_option_hpp

#include <ql/instruments/oneassetoption.hpp>
#include <ql/instruments/payoffs.hpp>

namespace QuantLib {

    //! %Results from quanto option calculation
    template<class ResultsType>
    class QuantoOptionResults : public ResultsType {
      public:
        QuantoOptionResults() { reset() ;}
        void reset() override {
            ResultsType::reset();
            qvega = qrho = qlambda = Null<Real>();
        }
        Real qvega;
        Real qrho;
        Real qlambda;
    };

    //! quanto version of a vanilla option
    /*! \ingroup instruments */
    class QuantoVanillaOption : public OneAssetOption {
      public:
        typedef OneAssetOption::arguments arguments;
        typedef QuantoOptionResults<OneAssetOption::results> results;
        typedef GenericEngine<arguments, results> engine;
        QuantoVanillaOption(const ext::shared_ptr<StrikedTypePayoff>&,
                            const ext::shared_ptr<Exercise>&);
        //! \name greeks
        //@{
        Real qvega() const;
        Real qrho() const;
        Real qlambda() const;
        //@}
        void fetchResults(const PricingEngine::results*) const override;

      private:
        void setupExpired() const override;
        // results
        mutable Real qvega_, qrho_, qlambda_;
    };

}


#endif
