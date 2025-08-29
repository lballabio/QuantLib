/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Ferdinando Ametrano

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

#include <ql/experimental/risk/sensitivityanalysis.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/instrument.hpp>

using std::vector;
using std::pair;

namespace QuantLib {

    QL_DEPRECATED_DISABLE_WARNING

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

    Real aggregateNPV(const vector<ext::shared_ptr<Instrument> >& instruments,
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

    pair<Real, Real>
    parallelAnalysis(const vector<Handle<SimpleQuote> >& quotes,
                     const vector<ext::shared_ptr<Instrument> >& instruments,
                     const vector<Real>& quantities,
                     Real shift,
                     SensitivityAnalysis type,
                     Real referenceNpv)
    {
        QL_REQUIRE(!quotes.empty(), "empty SimpleQuote vector");
        Size n = quotes.size();

        QL_REQUIRE(shift!=0.0, "zero shift not allowed");

        pair<Real, Real> result(0.0, 0.0);
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
        } catch (...) {
            for (Size i=0; i<n; ++i)
                if (quoteValues[i]!=Null<Real>())
                    quotes[i]->setValue(quoteValues[i]);
            throw;
        }

        return result;
    }

    pair<Real, Real> bucketAnalysis(const Handle<SimpleQuote>& quote,
                                    const vector<ext::shared_ptr<Instrument> >& instruments,
                                    const vector<Real>& quantities,
                                    Real shift,
                                    SensitivityAnalysis type,
                                    Real referenceNpv) {
        QL_REQUIRE(shift!=0.0, "zero shift not allowed");

        pair<Real, Real> result(0.0, 0.0);
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
        } catch (...) {
            quote->setValue(quoteValue);
            throw;
        }

        return result;
    }


    void bucketAnalysis(vector<Real>& deltaVector, // delta result
                        vector<Real>& gammaVector, // gamma result
                        vector<Real>& refVals,
                        const Handle<SimpleQuote>& quote,
                        const vector<Handle<Quote> >& params,
                        Real shift,
                        SensitivityAnalysis type) {
        QL_REQUIRE(shift!=0.0, "zero shift not allowed");

        QL_REQUIRE(!params.empty(), "empty parameters vector");
        Size m = params.size();
        deltaVector.resize(m);
        gammaVector.resize(m);

        if (!quote->isValid()) {
            for (Size j=0; j<m; ++j) {
                deltaVector[j]=Null<Real>();
                gammaVector[j]=Null<Real>();
            }
            return;
        }
        Real quoteValue = quote->value();

        if (!refVals.empty()) {
            QL_REQUIRE(refVals.size()==m,
                       "referenceValues has size " <<
                       refVals.size() << ", instead of " << m);
        } else {
            // calculate parameters' reference values
            refVals = vector<Real>(m, Null<Real>());
            for (Size j=0; j<m; ++j) {
                if (params[j]->isValid()) // fault tolerant
                    refVals[j] = params[j]->value();
            }
        }

        try {
            switch (type) {
              case OneSide:
                {
                    quote->setValue(quoteValue+shift);
                    for (Size j=0; j<m; ++j) {
                        gammaVector[j] = Null<Real>();
                        if (refVals[j] != Null<Real>())
                            deltaVector[j] = (params[j]->value()-refVals[j])/shift;
                        else
                            deltaVector[j] = Null<Real>();
                    }
                }
                break;
              case Centered:
                {
                    quote->setValue(quoteValue+shift);
                    vector<Real> plus(m);
                    for (Size j=0; j<m; ++j) {
                        if (refVals[j] != Null<Real>())
                            plus[j] = params[j]->value();
                    }
                    quote->setValue(quoteValue-shift);
                    for (Size j=0; j<m; ++j) {
                        if (refVals[j] != Null<Real>()) {
                            Real minus = params[j]->value();
                            deltaVector[j] = (plus[j]-minus)/(2.0*shift);
                            gammaVector[j] = (plus[j]-2.0*refVals[j]+minus)/(shift*shift);
                        } else {
                            deltaVector[j] = Null<Real>();
                            gammaVector[j] = Null<Real>();
                        }
                    }
                }
                break;
              default:
                  QL_FAIL("unknown SensitivityAnalysis (" <<
                          Integer(type) << ")");
            } // end switch

            // restore the quote to its original state
            quote->setValue(quoteValue);

            return;
        } catch (...) {
            // restore the quote to its original state
            quote->setValue(quoteValue);
            throw;
        }

    }





    pair<vector<Real>, vector<Real> >
    bucketAnalysis(const vector<Handle<SimpleQuote> >& quotes,
                   const vector<ext::shared_ptr<Instrument> >& instr,
                   const vector<Real>& quant,
                   Real shift,
                   SensitivityAnalysis type)
    {
        QL_REQUIRE(!quotes.empty(), "empty SimpleQuote vector");
        Size n = quotes.size();
        pair<vector<Real>, vector<Real> > result(vector<Real>(n, 0.0),
                                                 vector<Real>(n, 0.0));

        if (instr.empty()) return result;

        Real npv = aggregateNPV(instr, quant);

        pair<Real, Real> tmp;
        for (Size i=0; i<n; ++i) {
            tmp = bucketAnalysis(quotes[i], instr, quant, shift, type, npv);
            result.first[i] = tmp.first;
            result.second[i] = tmp.second;
        }

        return result;
    }

    void
    bucketAnalysis(std::vector<std::vector<Real> >& deltaMatrix, // result
                   std::vector<std::vector<Real> >& gammaMatrix, // result
                   const vector<Handle<SimpleQuote> >& quotes,
                   const vector<Handle<Quote> >& parameters,
                   Real shift,
                   SensitivityAnalysis type)
    {
        QL_REQUIRE(!quotes.empty(), "empty SimpleQuote vector");
        QL_REQUIRE(!parameters.empty(), "empty parameters vector");

        Size n = quotes.size();
        deltaMatrix.resize(n);
        gammaMatrix.resize(n);

        Size m = parameters.size();
        vector<Real> referenceValues(m, Null<Real>());
        for (Size i=0; i<m; ++i) {
            if (parameters[i]->isValid())
                referenceValues[i] = parameters[i]->value();
        }

        for (Size i=0; i<n; ++i) {
            bucketAnalysis(deltaMatrix[i], gammaMatrix[i], referenceValues,
                           quotes[i], parameters, shift, type);
        }
    }

    pair<vector<vector<Real> >, vector<vector<Real> > >
    bucketAnalysis(const vector<vector<Handle<SimpleQuote> > >& quotes,
                   const vector<ext::shared_ptr<Instrument> >& instr,
                   const vector<Real>& quant,
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

        pair<vector<vector<Real> >, vector<vector<Real> > >
            result(first, second);

        if (instr.empty()) return result;

        Real npv = aggregateNPV(instr, quant);

        pair<Real, Real> tmp;
        for (Size i=0; i<n; ++i) {
          for (Size j=0; j<quotes[i].size(); ++j) {
            tmp = bucketAnalysis(quotes[i][j], instr, quant, shift, type, npv);
            result.first[i][j] = tmp.first;
            result.second[i][j] = tmp.second;
          }
        }

        return result;
    }

    QL_DEPRECATED_ENABLE_WARNING
}
