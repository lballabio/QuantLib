/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters

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

#include <ql/experimental/credit/onefactorstudentcopula.hpp>

namespace QuantLib {

    //-------------------------------------------------------------------------
    OneFactorStudentCopula::OneFactorStudentCopula (
                                             const Handle<Quote>& correlation,
                                             int nz, int nm,
                                             Real maximum,
                                             Size integrationSteps)
    : OneFactorCopula (correlation, maximum, integrationSteps),
      density_ (nm), cumulative_ (nz), nz_(nz), nm_(nm) {
    //-------------------------------------------------------------------------

        QL_REQUIRE (nz > 2 && nm > 2, "degrees of freedom must be > 2");

        scaleM_ = std::sqrt (Real (nm_ - 2) / nm_);
        scaleZ_ = std::sqrt (Real (nz_ - 2) / nz_);

        calculate ();
    }

    //-------------------------------------------------------------------------
    void OneFactorStudentCopula::performCalculations () const {
    //-------------------------------------------------------------------------
        y_.clear();
        cumulativeY_.clear();

        // FIXME:
        // compute F(ymin) and F(ymax) for the fattest case nm = nz = 2
        // set a desired confidence and work out ymin, ymax
        Real ymin = -10;
        Real ymax = +10;
        Size steps = 200;
        for (Size i = 0; i <= steps; i++) {
            Real y = ymin + (ymax - ymin) * i / steps;
            Real c = cumulativeYintegral (y);
            y_.push_back (y);
            cumulativeY_.push_back (c);
        }
    }

    //-------------------------------------------------------------------------
    Real OneFactorStudentCopula::cumulativeYintegral (Real y) const {
    //-------------------------------------------------------------------------
        Real c = correlation_->value();

        if (c == 0)
            return CumulativeStudentDistribution(nz_)(y / scaleZ_);

        if (c == 1)
            return CumulativeStudentDistribution(nm_)(y / scaleM_);

        StudentDistribution dz (nz_);
        StudentDistribution dm (nm_);

        // FIXME:
        // Find a sensitive way of setting these parameters,
        // e.g. depending on nm and nz, and the desired table range
        Real minimum = -10; // -15
        Real maximum = +10; // +15
        int steps = 400;

        Real delta = (maximum - minimum) / steps;
        Real cumulated = 0;

        if (c < 0.5) {
            // outer integral -> 1 for c -> 0
            // inner integral -> cumulativeStudent(nz)(y) for c-> 0
            for (Real m = minimum + delta/2; m < maximum; m += delta)
                for (Real z = minimum + delta/2;
                     z < (y - std::sqrt(c) * m) / std::sqrt (1. - c); z += delta)
                    cumulated += dm (m / scaleM_) / scaleM_
                        * dz (z / scaleZ_) / scaleZ_;
        }
        else {
            // outer integral -> 1 for c -> 1
            // inner integral -> cumulativeStudent(nm)(y) for c-> 1
            for (Real z = minimum + delta/2; z < maximum; z += delta)
                for (Real m = minimum + delta/2;
                     m < (y - std::sqrt(1.0 - c) * z) / std::sqrt(c); m += delta)
                    cumulated += dm (m / scaleM_) / scaleM_
                        * dz (z / scaleZ_) / scaleZ_;
        }

        return cumulated * delta * delta;
    }

    //-------------------------------------------------------------------------
    OneFactorGaussianStudentCopula::OneFactorGaussianStudentCopula (
                                             const Handle<Quote>& correlation,
                                             int nz, Real maximum,
                                             Size integrationSteps)
    : OneFactorCopula (correlation, maximum, integrationSteps),
      cumulative_(nz), nz_(nz) {
    //-------------------------------------------------------------------------

        QL_REQUIRE (nz > 2, "degrees of freedom must be > 2");

        scaleZ_ = std::sqrt (Real (nz_ - 2) / nz_);

        calculate ();
    }

    //-------------------------------------------------------------------------
    void OneFactorGaussianStudentCopula::performCalculations () const {
    //-------------------------------------------------------------------------
        y_.clear();
        cumulativeY_.clear();

        // FIXME:
        // compute F(ymin) and F(ymax) for the fattest case nm = nz = 2
        // set a desired confidence and work out ymin, ymax
        Real ymin = -10;
        Real ymax = +10;
        Size steps = 200;
        for (Size i = 0; i <= steps; i++) {
            Real y = ymin + (ymax - ymin) * i / steps;
            Real c = cumulativeYintegral (y);
            y_.push_back (y);
            cumulativeY_.push_back (c);
        }
    }

    //-------------------------------------------------------------------------
    Real OneFactorGaussianStudentCopula::cumulativeYintegral (Real y) const {
    //-------------------------------------------------------------------------
        Real c = correlation_->value();

        if (c == 0)
            return CumulativeStudentDistribution(nz_)(y / scaleZ_);

        if (c == 1)
            return CumulativeNormalDistribution()(y);

        StudentDistribution dz (nz_);
        NormalDistribution dm;

        // FIXME:
        // Find a sensitive way of setting these parameters,
        // e.g. depending on nm and nz, and the desired table range
        Real minimum = -10;
        Real maximum = +10;
        int steps = 400;

        Real delta = (maximum - minimum) / steps;
        Real cumulated = 0;

        if (c < 0.5) {
            // outer integral -> 1 for c -> 0
            // inner integral -> cumulativeStudent(nz)(y) for c-> 0
            for (Real m = minimum + delta/2; m < maximum; m += delta)
                for (Real z = minimum + delta/2;
                     z < (y - std::sqrt(c) * m) / std::sqrt (1. - c);
                     z += delta)
                    cumulated += dm (m) * dz (z / scaleZ_) / scaleZ_;
        }
        else {
            // outer integral -> 1 for c -> 1
            // inner integral -> cumulativeNormal(y) for c-> 1
            for (Real z = minimum + delta/2; z < maximum; z += delta)
                for (Real m = minimum + delta/2;
                     m < (y - std::sqrt(1.0 - c) * z) / std::sqrt(c);
                     m += delta)
                    cumulated += dm (m) * dz (z / scaleZ_) / scaleZ_;
        }

        return cumulated * delta * delta;
    }

    //-------------------------------------------------------------------------
    OneFactorStudentGaussianCopula::OneFactorStudentGaussianCopula (
                                             const Handle<Quote>& correlation,
                                             int nm, Real maximum,
                                             Size integrationSteps)
    : OneFactorCopula (correlation, maximum, integrationSteps),
      density_ (nm), nm_(nm) {
    //-------------------------------------------------------------------------

        QL_REQUIRE (nm > 2, "degrees of freedom must be > 2");

        scaleM_ = std::sqrt (Real (nm_ - 2) / nm_);

        calculate ();
    }

    //-------------------------------------------------------------------------
    void OneFactorStudentGaussianCopula::performCalculations () const {
    //-------------------------------------------------------------------------
        y_.clear();
        cumulativeY_.clear();

        // FIXME:
        // compute F(ymin) and F(ymax) for the fattest case nm = nz = 2
        // set a desired confidence and work out ymin, ymax
        Real ymin = -10;
        Real ymax = +10;
        Size steps = 200;
        for (Size i = 0; i <= steps; i++) {
            Real y = ymin + (ymax - ymin) * i / steps;
            Real c = cumulativeYintegral (y);
            y_.push_back (y);
            cumulativeY_.push_back (c);
        }
    }

    //-------------------------------------------------------------------------
    Real OneFactorStudentGaussianCopula::cumulativeYintegral (Real y) const {
    //-------------------------------------------------------------------------
        Real c = correlation_->value();

        if (c == 0)
            return CumulativeNormalDistribution()(y);

        if (c == 1)
            return CumulativeStudentDistribution(nm_)(y / scaleM_);


        StudentDistribution dm (nm_);
        NormalDistribution dz;

        // FIXME:
        // Find a sensitive way of setting these parameters,
        // e.g. depending on nm and nz, and the desired table range
        Real minimum = -10;
        Real maximum = +10;
        int steps = 400;

        Real delta = (maximum - minimum) / steps;
        Real cumulated = 0;

        if (c < 0.5) {
            // outer integral -> 1 for c -> 0
            // inner integral -> cumulativeNormal(y) for c-> 0
            for (Real m = minimum + delta/2; m < maximum; m += delta)
                for (Real z = minimum + delta/2;
                     z < (y - std::sqrt(c) * m) / std::sqrt (1. - c);
                     z += delta)
                    cumulated += dm (m / scaleM_) / scaleM_ * dz (z);
        }
        else {
            // outer integral -> 1 for c -> 1
            // inner integral -> cumulativeStudent(nm)(y) for c-> 1
            for (Real z = minimum + delta/2; z < maximum; z += delta)
                for (Real m = minimum + delta/2;
                     m < (y - std::sqrt(1.0 - c) * z) / std::sqrt(c);
                     m += delta)
                    cumulated += dm (m / scaleM_) / scaleM_ * dz (z);
        }

        return cumulated * delta * delta;
    }

}
