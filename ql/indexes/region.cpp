/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Chris Kenyon
 Copyright (C) 2014 StatPro Italia srl

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

#include <ql/indexes/region.hpp>

namespace QuantLib {

    CustomRegion::CustomRegion(const std::string& name,
                               const std::string& code) {
        data_ = boost::shared_ptr<Data>(new Data(name,code));
    }


    AustraliaRegion::AustraliaRegion() {
        static boost::shared_ptr<Data> AUdata(new Data("Australia","AU"));
        data_ = AUdata;
    }

    EURegion::EURegion() {
        static boost::shared_ptr<Data> EUdata(new Data("EU","EU"));
        data_ = EUdata;
    }

    FranceRegion::FranceRegion() {
        static boost::shared_ptr<Data> FRdata(new Data("France","FR"));
        data_ = FRdata;
    }

    UKRegion::UKRegion() {
        static boost::shared_ptr<Data> UKdata(new Data("UK","UK"));
        data_ = UKdata;
    }

    USRegion::USRegion() {
        static boost::shared_ptr<Data> UKdata(new Data("USA","US"));
        data_ = UKdata;
    }


}

