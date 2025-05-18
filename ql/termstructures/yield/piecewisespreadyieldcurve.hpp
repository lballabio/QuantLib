/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef quantlib_piecewise_spread_yield_curve_hpp
#define quantlib_piecewise_spread_yield_curve_hpp

#include <ql/termstructures/iterativebootstrap.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/yield/spreadbootstraptraits.hpp>
#include <utility>

namespace QuantLib {

    //! Piecewise spread yield term structure
    template <class Traits, class Interpolator,
              template <class> class Bootstrap = IterativeBootstrap>
    class PiecewiseSpreadYieldCurve
        : public PiecewiseYieldCurve<detail::SpreadTraits<Traits>, Interpolator, Bootstrap> {
      private:
        typedef detail::SpreadTraits<Traits> traits_type;
        typedef PiecewiseYieldCurve<traits_type, Interpolator, Bootstrap> base_curve;

      public:
        //! \name Constructors
        //@{
        PiecewiseSpreadYieldCurve(
            Handle<YieldTermStructure> baseCurve,
            std::vector<ext::shared_ptr<typename traits_type::helper>> instruments,
            const Interpolator& i = {},
            typename base_curve::bootstrap_type bootstrap = {})
        : base_curve(std::move(instruments), std::move(bootstrap),
                     std::move(baseCurve), i) {}
        //@}
    };

}

#endif
