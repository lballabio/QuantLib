/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 StatPro Italia srl

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

/*! \file integralhestonvarianceoptionengine.hpp
    \brief integral Heston-model variance-option engine
*/

#ifndef quantlib_integral_heston_variance_option_engine_hpp
#define quantlib_integral_heston_variance_option_engine_hpp

#include <ql/experimental/varianceoption/varianceoption.hpp>
#include <ql/processes/hestonprocess.hpp>

namespace QuantLib {

    //! integral Heston-model variance-option engine
    /*! This engine implements the approach described in
        <http://www.econ.univpm.it/recchioni/finance/w4/>.

        \ingroup forwardengines
    */
    class IntegralHestonVarianceOptionEngine : public VarianceOption::engine  {
      public:
        IntegralHestonVarianceOptionEngine(
                    const boost::shared_ptr<HestonProcess>&);
        void calculate() const;
      private:
        boost::shared_ptr<HestonProcess> process_;
    };

}


#endif
