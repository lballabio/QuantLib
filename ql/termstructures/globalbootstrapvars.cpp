/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <ql/termstructures/globalbootstrapvars.hpp>
#include <ql/utilities/null.hpp>
#include <ql/utilities/vectors.hpp>
#include <utility>

namespace QuantLib {

    SimpleQuoteVariables::SimpleQuoteVariables(std::vector<ext::shared_ptr<SimpleQuote>> quotes,
                                               std::vector<Real> initialGuesses,
                                               std::vector<Real> lowerBounds)
    : quotes_(std::move(quotes)), initialGuesses_(std::move(initialGuesses)),
      lowerBounds_(std::move(lowerBounds)) {
        QL_REQUIRE(initialGuesses_.size() <= quotes_.size(), "too many initialGuesses");
        QL_REQUIRE(lowerBounds_.size() <= quotes_.size(), "too many lowerBounds");
    }

    Array SimpleQuoteVariables::initialize(bool validData) {
        Array guesses(quotes_.size());
        for (Size i = 0, size = guesses.size(); i < size; ++i) {
            Real guess;
            if (validData) {
                guess = quotes_[i]->value();
            } else {
                guess = detail::get(initialGuesses_, i, 0.0);
                quotes_[i]->setValue(guess);
            }
            guesses[i] = transformInverse(guess, i);
        }
        return guesses;
    }

    void SimpleQuoteVariables::update(const Array& x) {
        for (Size i = 0, size = x.size(); i < size; ++i) {
            quotes_[i]->setValue(transformDirect(x[i], i));
        }
    }

    Real SimpleQuoteVariables::transformDirect(Real x, Size i) const {
        const Real lb = detail::get(lowerBounds_, i, Null<Real>());
        return lb == Null<Real>() ? x : std::exp(x) + lb;
    }

    Real SimpleQuoteVariables::transformInverse(Real x, Size i) const {
        const Real lb = detail::get(lowerBounds_, i, Null<Real>());
        return lb == Null<Real>() ? x : std::log(x - lb);
    }
}
