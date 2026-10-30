/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef quantlib_global_bootstrap_vars_hpp
#define quantlib_global_bootstrap_vars_hpp

#include <ql/quotes/simplequote.hpp>
#include <ql/shared_ptr.hpp>
#include <ql/termstructures/globalbootstrap.hpp>

namespace QuantLib {

class SimpleQuoteVariables : public AdditionalBootstrapVariables {
  public:
    explicit SimpleQuoteVariables(
        std::vector<ext::shared_ptr<SimpleQuote>> quotes,
        std::vector<Real> initialGuesses = {},
        std::vector<Real> lowerBounds = {});

    Array initialize(bool validData) override;
    void update(const Array& x) override;

  private:
    Real transformDirect(Real x, Size i) const;
    Real transformInverse(Real x, Size i) const;

    std::vector<ext::shared_ptr<SimpleQuote>> quotes_;
    std::vector<Real> initialGuesses_, lowerBounds_;
};

} // namespace QuantLib

#endif
