/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008 Klaus Spanderen

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

/*! \file fdmdividendhandler.hpp
    \brief dividend handler for fdm method for one equity direction
*/

#ifndef quantlib_fdm_dividend_handler_hpp
#define quantlib_fdm_dividend_handler_hpp

#include <ql/methods/finitedifferences/stepcondition.hpp>
#include <ql/experimental/finitedifferences/fdmmesher.hpp>

namespace QuantLib {

    class FdmDividendHandler : public StepCondition<Array> {
      public:
        FdmDividendHandler(const std::vector<Time> & dividendTimes,
                           const std::vector<Real> & dividends,
                           const boost::shared_ptr<FdmMesher> & mesher,
                           Size equityDirection);

        void applyTo(Array& a, Time t) const;

      private:
        Array x_; // grid-equity values in physical units

        const std::vector<Time> dividendTimes_;
        const std::vector<Real> dividends_;
        const boost::shared_ptr<FdmMesher> mesher_;
        const Size equityDirection_;
    };
}
#endif
