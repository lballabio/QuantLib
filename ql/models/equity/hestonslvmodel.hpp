/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Johannes Goettker-Schnetmann
 Copyright (C) 2015 Klaus Spanderen

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

/*! \file hestonslvmodel.hpp
    \brief Heston stochastic local volatility model
*/

#ifndef quantlib_heston_slv_model_hpp
#define quantlib_heston_slv_model_hpp

#include <ql/handle.hpp>
#include <ql/patterns/observable.hpp>

namespace QuantLib {

	class HestonModel;
	class LocalVolTermStructure;
	class SimpleQuote;

	class HestonSLVModel : public virtual Observer, public virtual Observable {
      public:
        HestonSLVModel(const Handle<HestonModel>& hestonModel,
        			   const Handle<LocalVolTermStructure>& localVol,
					   const Handle<SimpleQuote>& eta);
        void update();

        boost::shared_ptr<Quote> eta() const;
        boost::shared_ptr<HestonProcess> hestonProcess() const;
        boost::shared_ptr<LocalVolTermStructure> localVol() const;

      protected:
        const Handle<HestonModel> hestonModel_;
        const Handle<LocalVolTermStructure> localVol_;
        const Handle<SimpleQuote> eta_;
    };
}


#endif

