/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 J. Erik Radmall

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

#include <ql/experimental/commodities/commoditytype.hpp>
#include <ostream>

namespace QuantLib {

    std::map<std::string, std::shared_ptr<CommodityType::Data> >
    CommodityType::commodityTypes_;

    CommodityType::CommodityType(const std::string& name,
                                 const std::string& code) {
        std::map<std::string, std::shared_ptr<CommodityType::Data> >::const_iterator i = commodityTypes_.find(code);
        if (i != commodityTypes_.end())
            data_ = i->second;
        else {
            data_ = std::make_shared<CommodityType::Data>(
                                         name, code);
            commodityTypes_[code] = data_;
        }
    }

    std::ostream& operator<<(std::ostream& out, const CommodityType& c) {
        if (!c.empty())
            return out << c.code();
        else
            return out << "null commodity type";
    }

}

