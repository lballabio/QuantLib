/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008, 2010 Ferdinando Ametrano

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

/*! \file sensitivityanalysis.hpp
    \brief sensitivity analysis function
*/

#ifndef quantlib_sensitivity_analysis_hpp
#define quantlib_sensitivity_analysis_hpp

#include <ql/types.hpp>
#include <ql/utilities/null.hpp>
#include <ql/shared_ptr.hpp>
#include <vector>

namespace QuantLib {

    template <class T>
    class Handle;
    class Quote;
    class SimpleQuote;
    class Instrument;

    //! Finite differences calculation
    enum SensitivityAnalysis {
        OneSide, /*!<  */
        Centered /*!<  */
    };

    /*! \relates SensitivityAnalysis */
    std::ostream& operator<<(std::ostream&,
                             SensitivityAnalysis);

    //! utility fuction for weighted sum of NPVs
    Real aggregateNPV(const std::vector<ext::shared_ptr<Instrument> >&,
                      const std::vector<Real>& quantities);

    //! parallel shift PV01 sensitivity analysis for a SimpleQuote vector
    /*! returns a pair of first and second derivative values calculated as
        prescribed by SensitivityAnalysis. Second derivative might not be
        available depending on SensitivityAnalysis value.

        Empty quantities vector is considered as unit vector. The same if
        the vector is just one single element equal to one.

        All SimpleQuotes are tweaked together in a parallel fashion.
    */
    std::pair<Real, Real>
    parallelAnalysis(const std::vector<Handle<SimpleQuote> >&,
                     const std::vector<ext::shared_ptr<Instrument> >&,
                     const std::vector<Real>& quantities,
                     Real shift = 0.0001,
                     SensitivityAnalysis type = Centered,
                     Real referenceNpv = Null<Real>());

    //! parallel shift PV01 sensitivity analysis for a SimpleQuote matrix
    /*! returns a pair of first and second derivative values calculated as
        prescribed by SensitivityAnalysis. Second derivative might not be
        available depending on SensitivityAnalysis value.

        Empty quantities vector is considered as unit vector. The same if
        the vector is of size one.

        All SimpleQuotes are tweaked together in a parallel fashion.
    */
    std::pair<Real, Real>
    parallelAnalysis(const std::vector<std::vector<Handle<SimpleQuote> > >&,
                     const std::vector<ext::shared_ptr<Instrument> >&,
                     const std::vector<Real>& quantities,
                     Real shift = 0.0001,
                     SensitivityAnalysis type = Centered,
                     Real referenceNpv = Null<Real>());

    //! (bucket) PV01 sensitivity analysis for a (single) SimpleQuote
    /*! returns a pair of first and second derivative values calculated as
        prescribed by SensitivityAnalysis. Second derivative might not be
        available depending on SensitivityAnalysis value.

        Empty quantities vector is considered as unit vector. The same if
        the vector is of size one.
    */
    std::pair<Real, Real> bucketAnalysis(const Handle<SimpleQuote>& quote,
                                         const std::vector<ext::shared_ptr<Instrument> >&,
                                         const std::vector<Real>& quantities,
                                         Real shift = 0.0001,
                                         SensitivityAnalysis type = Centered,
                                         Real referenceNpv = Null<Real>());

    //! (bucket) parameters' sensitivity analysis for a (single) SimpleQuote
    /*! returns a vector (one element for each paramet) of pair of first and
        second derivative values calculated as prescribed by
        SensitivityAnalysis. Second derivative might not be available
        depending on SensitivityAnalysis value.

        Empty quantities vector is considered as unit vector. The same if
        the vector is of size one.
    */
    void bucketAnalysis(std::vector<Real>& deltaVector, // result
                        std::vector<Real>& gammaVector, // result
                        std::vector<Real>& referenceValues,
                        const Handle<SimpleQuote>& quote,
                        const std::vector<Handle<Quote> >& parameters,
                        Real shift = 0.0001,
                        SensitivityAnalysis type = Centered);

    //! bucket PV01 sensitivity analysis for a SimpleQuote vector
    /*! returns a pair of first and second derivative vectors calculated as
        prescribed by SensitivityAnalysis. Second derivative might not be
        available depending on SensitivityAnalysis value.

        Empty quantities vector is considered as unit vector. The same if
        the vector is of size one.

        The (bucket) SimpleQuotes are tweaked one by one separately.
    */
    std::pair<std::vector<Real>, std::vector<Real> >
    bucketAnalysis(const std::vector<Handle<SimpleQuote> >& quotes,
                   const std::vector<ext::shared_ptr<Instrument> >&,
                   const std::vector<Real>& quantities,
                   Real shift = 0.0001,
                   SensitivityAnalysis type = Centered);

    //! bucket parameters' sensitivity analysis for a SimpleQuote vector
    /*! returns a vector (one element for each paramet) of pair of first and
        second derivative vectors calculated as prescribed by
        SensitivityAnalysis. Second derivative might not be available
        depending on SensitivityAnalysis value.

        Empty quantities vector is considered as unit vector. The same if
        the vector is of size one.

        The (bucket) SimpleQuotes are tweaked one by one separately.
    */
    void
    bucketAnalysis(std::vector<std::vector<Real> >& deltaMatrix, // result
                   std::vector<std::vector<Real> >& gammaMatrix, // result
                   const std::vector<Handle<SimpleQuote> >& quotes,
                   const std::vector<Handle<Quote> >& parameters,
                   Real shift = 0.0001,
                   SensitivityAnalysis type = Centered);


    //! bucket sensitivity analysis for a SimpleQuote matrix
    /*! returns a pair of first and second derivative metrices calculated as
        prescribed by SensitivityAnalysis. Second derivative might not be
        available depending on SensitivityAnalysis value.

        Empty quantities vector is considered as unit vector. The same if
        the vector is of size one.

        The (bucket) SimpleQuotes are tweaked one by one separately.
    */
    std::pair<std::vector<std::vector<Real> >, std::vector<std::vector<Real> > >
    bucketAnalysis(const std::vector<std::vector<Handle<SimpleQuote> > >&,
                   const std::vector<ext::shared_ptr<Instrument> >&,
                   const std::vector<Real>& quantities,
                   Real shift = 0.0001,
                   SensitivityAnalysis type = Centered);

}

#endif


#ifndef id_168266883d571f5a7c5daf03e2c70899
#define id_168266883d571f5a7c5daf03e2c70899
inline bool test_168266883d571f5a7c5daf03e2c70899(int* i) { return i != 0; }
#endif
