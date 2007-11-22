/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Neil Firth
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

/*! \file stulzengine.hpp
    \brief 2D European Basket formulae, due to Stulz (1982)
*/

#ifndef quantlib_stulz_engine_hpp
#define quantlib_stulz_engine_hpp

#include <ql/instruments/basketoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Pricing engine for 2D European Baskets
    /*! This class implements formulae from
        "Options on the Minimum or the Maximum of Two Risky Assets",
            Rene Stulz,
            Journal of Financial Ecomomics (1982) 10, 161-185.

        \ingroup basketengines

        \test the correctness of the returned value is tested by
              reproducing results available in literature.
    */
    class StulzEngine : public BasketOption::engine {
      public:
        StulzEngine(
            const boost::shared_ptr<GeneralizedBlackScholesProcess>& process1,
            const boost::shared_ptr<GeneralizedBlackScholesProcess>& process2,
            Real correlation);
        void calculate() const;
      private:
        boost::shared_ptr<GeneralizedBlackScholesProcess> process1_;
        boost::shared_ptr<GeneralizedBlackScholesProcess> process2_;
        Real rho_;
    };

}


#endif
