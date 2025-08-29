/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 J. Erik Radmall

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

#include <ql/experimental/commodities/commoditycashflow.hpp>
#include <ql/patterns/visitor.hpp>
#include <iomanip>

namespace QuantLib {

    void CommodityCashFlow::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<CommodityCashFlow>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            CashFlow::accept(v);
    }

    std::ostream& operator<<(std::ostream& out,
                             const CommodityCashFlows& cashFlows) {
        if (cashFlows.empty())
            return out << "no cashflows" << std::endl;
        out << "cashflows" << std::endl;
        std::string currencyCode; //= cashFlows[0]->discountedAmount().currency().code();
        Real totalDiscounted = 0;
        Real totalUndiscounted = 0;
        for (const auto& i : cashFlows) {
            //const ext::shared_ptr<CommodityCashFlow> cashFlow = *i;
            const ext::shared_ptr<CommodityCashFlow> cashFlow = i.second;
            totalDiscounted += cashFlow->discountedAmount().value();
            totalUndiscounted += cashFlow->undiscountedAmount().value();
            //out << io::iso_date(cashFlow->date()) << " " <<
            out << io::iso_date(i.first) << " " << std::setw(16) << std::right << std::fixed
                << std::setprecision(2) << cashFlow->discountedAmount().value() << " "
                << currencyCode << std::setw(16) << std::right << std::fixed << std::setprecision(2)
                << cashFlow->undiscountedAmount().value() << " " << currencyCode << std::endl;
        }
        out << "total      "
            << std::setw(16) << std::right << std::fixed
            << std::setprecision(2) << totalDiscounted << " " << currencyCode
            << std::setw(16) << std::right << std::fixed
            << std::setprecision(2) << totalUndiscounted << " "
            << currencyCode << std::endl;
        return out;
    }

}

