/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Ferdinando Ametrano

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

#include <ql/experimental/risk/sensitivityanalysis.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/instrument.hpp>

using std::vector;
using boost::shared_ptr;

namespace QuantLib {

    std::ostream& operator<<(std::ostream& out,
                             SensitivityAnalysis s) {
        switch (s) {
          case OneSide:
            return out << "OneSide";
          case Centered:
            return out << "Centered";
          default:
            QL_FAIL("unknown SensitivityAnalysis (" << Integer(s) << ")");
        }
    }

    Real aggregateNPV(const vector<shared_ptr<Instrument> >& instruments,
                      const vector<Real>& quant) {
        Size n = instruments.size();
        Real npv = 0.0;
        if (quant.empty() || (quant.size()==1 && quant[0]==1.0)) {
            for (Size k=0; k<n; ++k)
                npv += instruments[k]->NPV();
        } else {
            QL_REQUIRE(quant.size()==n,
                       "dimension mismatch between instruments (" << n <<
                       ") and quantities (" << quant.size() << ")");
            for (Size k=0; k<n; ++k)
                npv += quant[k] * instruments[k]->NPV();
        }
        return npv;
    }

    std::pair<Real, Real>
    parallelAnalysis(const vector<Handle<SimpleQuote> >& quotes,
                     const vector<shared_ptr<Instrument> >& instruments,
                     const std::vector<Real>& quantities,
                     Real shift,
                     SensitivityAnalysis type,
                     Real referenceNpv)
    {
        QL_REQUIRE(!quotes.empty(), "empty SimpleQuote vector");
        Size n = quotes.size();

        QL_REQUIRE(shift!=0.0, "zero shift not allowed");

        std::pair<Real, Real> result(0.0, 0.0);
        if (instruments.empty()) return result;

        if (referenceNpv==Null<Real>())
            referenceNpv = aggregateNPV(instruments, quantities);

        vector<Real> quoteValues(n, Null<Real>());
        for (Size i=0; i<n; ++i)
            if (quotes[i]->isValid())
                quoteValues[i] = quotes[i]->value();
        try {
            for (Size i=0; i<n; ++i)
                if (quotes[i]->isValid())
                    quotes[i]->setValue(quoteValues[i]+shift);
            Real npv = aggregateNPV(instruments, quantities);
            switch (type) {
              case OneSide:
                result.first = (npv-referenceNpv)/shift;
                result.second = Null<Real>();
                break;
              case Centered:
                {
                for (Size i=0; i<n; ++i)
                    if (quotes[i]->isValid())
                        quotes[i]->setValue(quoteValues[i]-shift);
                Real npv2 = aggregateNPV(instruments, quantities);
                result.first = (npv-npv2)/(2.0*shift);
                result.second = (npv-2.0*referenceNpv+npv2)/(shift*shift);
                }
                break;
              default:
                  QL_FAIL("unknown SensitivityAnalysis (" <<
                          Integer(type) << ")");
            }
            for (Size i=0; i<n; ++i)
                if (quotes[i]->isValid())
                    quotes[i]->setValue(quoteValues[i]);
        } catch (std::exception& e) {
            for (Size i=0; i<n; ++i)
                if (quoteValues[i]!=Null<Real>())
                    quotes[i]->setValue(quoteValues[i]);
            throw e;
        }

        return result;
    }

    std::pair<Real, Real>
    bucketAnalysis(Handle<SimpleQuote> quote,
                   const vector<shared_ptr<Instrument> >& instruments,
                   const std::vector<Real>& quantities,
                   Real shift,
                   SensitivityAnalysis type,
                   Real referenceNpv)
    {
        QL_REQUIRE(shift!=0.0, "zero shift not allowed");

        std::pair<Real, Real> result(0.0, 0.0);
        if (instruments.empty()) return result;

        if (referenceNpv==Null<Real>())
            referenceNpv = aggregateNPV(instruments, quantities);

        if (!quote->isValid()) return result;
        Real quoteValue = quote->value();

        try {
            quote->setValue(quoteValue+shift);
            Real npv = aggregateNPV(instruments, quantities);
            switch (type) {
              case OneSide:
                result.first = (npv-referenceNpv)/shift;
                result.second = Null<Real>();
                break;
              case Centered:
                {
                quote->setValue(quoteValue-shift);
                Real npv2 = aggregateNPV(instruments, quantities);
                result.first = (npv-npv2)/(2.0*shift);
                result.second = (npv-2.0*referenceNpv+npv2)/(shift*shift);
                }
                break;
              default:
                  QL_FAIL("unknown SensitivityAnalysis (" <<
                          Integer(type) << ")");
            }
            quote->setValue(quoteValue);
        } catch (std::exception& e) {
            quote->setValue(quoteValue);
            throw e;
        }

        return result;
    }

    std::pair<vector<Real>, vector<Real> >
    bucketAnalysis(const vector<Handle<SimpleQuote> >& quotes,
                   const vector<shared_ptr<Instrument> >& instr,
                   const std::vector<Real>& quant,
                   Real shift,
                   SensitivityAnalysis type)
    {
        QL_REQUIRE(!quotes.empty(), "empty SimpleQuote vector");
        Size n = quotes.size();
        std::pair<vector<Real>, vector<Real> > result(vector<Real>(n, 0.0),
                                                      vector<Real>(n, 0.0));

        if (instr.empty()) return result;

        Real npv = aggregateNPV(instr, quant);

        std::pair<Real, Real> tmp;
        for (Size i=0; i<n; ++i) {
            tmp = bucketAnalysis(quotes[i], instr, quant, shift, type, npv);
            result.first[i] = tmp.first;
            result.second[i] = tmp.second;
        }

        return result;
    }

    std::pair<vector<vector<Real> >, vector<vector<Real> > >
    bucketAnalysis(const vector<vector<Handle<SimpleQuote> > >& quotes,
                   const vector<shared_ptr<Instrument> >& instr,
                   const std::vector<Real>& quant,
                   Real shift,
                   SensitivityAnalysis type)
    {
        QL_REQUIRE(!quotes.empty(), "empty SimpleQuote range");
        Size n = quotes.size();
        vector<vector<Real> > first(n);
        vector<vector<Real> > second(n);
        for (Size i=0; i<n; ++i) {
            Size tmp = quotes[i].size();
            first[i] = vector<Real>(tmp, 0.0);
            second[i] = vector<Real>(tmp, 0.0);
        }

        std::pair<vector<vector<Real> >, vector<vector<Real> > >
            result(first, second);

        if (instr.empty()) return result;

        Real npv = aggregateNPV(instr, quant);

        std::pair<Real, Real> tmp;
        for (Size i=0; i<n; ++i) {
          for (Size j=0; j<quotes[i].size(); ++j) {
            tmp = bucketAnalysis(quotes[i][j], instr, quant, shift, type, npv);
            result.first[i][j] = tmp.first;
            result.second[i][j] = tmp.second;
          }
        }

        return result;
    }

}
