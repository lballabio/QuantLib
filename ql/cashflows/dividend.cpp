/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Joseph Wang
 Copyright (C) 2006 StatPro Italia srl

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

#include <ql/cashflows/dividend.hpp>
#include <ql/patterns/visitor.hpp>

namespace QuantLib {

    void Dividend::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<Dividend>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            CashFlow::accept(v);
    }

    std::vector<ext::shared_ptr<Dividend> >
    DividendVector(const std::vector<Date>& dividendDates,
                   const std::vector<Real>& dividends) {

        QL_REQUIRE(dividendDates.size() == dividends.size(),
                   "size mismatch between dividend dates and amounts");

        std::vector<Date>::const_iterator dd;
        std::vector<Real>::const_iterator d;
        std::vector<ext::shared_ptr<Dividend> > items;
        items.reserve(dividendDates.size());
        for (dd = dividendDates.begin(), d = dividends.begin();
             dd != dividendDates.end(); ++dd, ++d) {
            items.push_back(ext::shared_ptr<Dividend>(new
                FixedDividend(*d, *dd)));
        }
        return items;
    }

}

