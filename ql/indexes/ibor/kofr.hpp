/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 Jongbong An

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

/*! \file kofr.hpp
    \brief %KOFR index
*/

#ifndef quantlib_kofr_hpp
#define quantlib_kofr_hpp

#include <ql/indexes/iborindex.hpp>

namespace QuantLib {

    //! %KOFR index.
    /*! Korea Overnight Financing Repo Rate (KOFR) published by Korea Securities Depository (KSD)
    Please refer to
    (1) https://www.bok.or.kr/eng/main/contents.do?menuNo=400399 (Overview)
    (2) https://www.kofr.kr/main.jsp (Detailed information)
    */
   class Kofr : public OvernightIndex {
      public:
        explicit Kofr(const Handle<YieldTermStructure>& h = {});
    };
    
}

#endif
