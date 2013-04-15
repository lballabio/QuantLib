/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Giorgio Facchinetti

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

/*! \file swaptionvolcube1.hpp
    \brief Swaption volatility cube, fit-early-interpolate-later approach
*/

#ifndef quantlib_swaption_volcube_fit_early_interpolate_later_h
#define quantlib_swaption_volcube_fit_early_interpolate_later_h

#include <ql/termstructures/volatility/swaption/swaptionvolcube.hpp>
#include <ql/math/matrix.hpp>

namespace QuantLib {

    class Interpolation2D;
    class EndCriteria;
    class OptimizationMethod;

    class SwaptionVolCube1 : public SwaptionVolatilityCube {
        class Cube {
          public:
            Cube() {}
            Cube(const std::vector<Date>& optionDates,
                 const std::vector<Period>& swapTenors,
                 const std::vector<Time>& optionTimes,
                 const std::vector<Time>& swapLengths,
                 Size nLayers,
                 bool extrapolation = true);
            Cube& operator=(const Cube& o);
            Cube(const Cube&);
            virtual ~Cube() {}
            void setElement(Size IndexOfLayer,
                            Size IndexOfRow,
                            Size IndexOfColumn,
                            Real x);
            void setPoints(const std::vector<Matrix>& x);
            void setPoint(const Date& optionDate,
                          const Period& swapTenor,
                          const Time optionTime,
                          const Time swapLengths,
                          const std::vector<Real>& point);
            void setLayer(Size i,
                          const Matrix& x);
            void expandLayers(Size i,
                              bool expandOptionTimes,
                              Size j,
                              bool expandSwapLengths);
            const std::vector<Date>& optionDates() const {
                return optionDates_;
            }
            const std::vector<Period>& swapTenors() const {
                return swapTenors_;
            }
            const std::vector<Time>& optionTimes() const;
            const std::vector<Time>& swapLengths() const;
            const std::vector<Matrix>& points() const;
            std::vector<Real> operator()(const Time optionTime,
                                         const Time swapLengths) const;
            void updateInterpolators()const;
            Matrix browse() const;
          private:
            std::vector<Time> optionTimes_, swapLengths_;
            std::vector<Date> optionDates_;
            std::vector<Period> swapTenors_;
            Size nLayers_;
            std::vector<Matrix> points_;
            mutable std::vector<Disposable<Matrix> > transposedPoints_;
            bool extrapolation_;
            mutable std::vector< boost::shared_ptr<Interpolation2D> > interpolators_;
         };
      public:
        SwaptionVolCube1(
            const Handle<SwaptionVolatilityStructure>& atmVolStructure,
            const std::vector<Period>& optionTenors,
            const std::vector<Period>& swapTenors,
            const std::vector<Spread>& strikeSpreads,
            const std::vector<std::vector<Handle<Quote> > >& volSpreads,
            const boost::shared_ptr<SwapIndex>& swapIndexBase,
            const boost::shared_ptr<SwapIndex>& shortSwapIndexBase,
            bool vegaWeightedSmileFit,
            const std::vector<std::vector<Handle<Quote> > >& parametersGuess,
            const std::vector<bool>& isParameterFixed,
            bool isAtmCalibrated,
            const boost::shared_ptr<EndCriteria>& endCriteria
                = boost::shared_ptr<EndCriteria>(),
            Real maxErrorTolerance = Null<Real>(),
            const boost::shared_ptr<OptimizationMethod>& optMethod
                = boost::shared_ptr<OptimizationMethod>(),
            const Real errorAccept = 0.0020,
            const bool useMaxError = false,
            const Size maxGuesses = 50);
        //! \name LazyObject interface
        //@{
        void performCalculations() const;
        //@}
        //! \name SwaptionVolatilityCube interface
        //@{
        boost::shared_ptr<SmileSection> smileSectionImpl(
                                              Time optionTime,
                                              Time swapLength) const;
        //@}
        //! \name Other inspectors
        //@{
        const Matrix& marketVolCube(Size i) const {
            return marketVolCube_.points()[i];
        }
        Matrix sparseSabrParameters()const;
        Matrix denseSabrParameters() const;
        Matrix marketVolCube() const;
        Matrix volCubeAtmCalibrated() const;
        //@}
        void sabrCalibrationSection(const Cube& marketVolCube,
                                    Cube& parametersCube,
                                    const Period& swapTenor) const;
        void recalibration(Real beta,
                           const Period& swapTenor);
     protected:
        void registerWithParametersGuess();
        boost::shared_ptr<SmileSection> smileSection(
                                    Time optionTime,
                                    Time swapLength,
                                    const Cube& sabrParametersCube) const;
        Cube sabrCalibration(const Cube& marketVolCube) const;
        void fillVolatilityCube() const;
        void createSparseSmiles() const;
        std::vector<Real> spreadVolInterpolation(const Date& atmOptionDate,
                                                 const Period& atmSwapTenor) const;
      private:
        mutable Cube marketVolCube_;
        mutable Cube volCubeAtmCalibrated_;
        mutable Cube sparseParameters_;
        mutable Cube denseParameters_;
        mutable std::vector< std::vector<boost::shared_ptr<SmileSection> > >
                                                                sparseSmiles_;
        std::vector<std::vector<Handle<Quote> > > parametersGuessQuotes_;
        mutable Cube parametersGuess_;
        std::vector<bool> isParameterFixed_;
        bool isAtmCalibrated_;
        const boost::shared_ptr<EndCriteria> endCriteria_;
        Real maxErrorTolerance_;
        const boost::shared_ptr<OptimizationMethod> optMethod_;
        const Real errorAccept_;
        const bool useMaxError_;
        const Size maxGuesses_;
    };

}

#endif
