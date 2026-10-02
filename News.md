Changes for QuantLib 1.44
=========================

As announced, it is no longer possible to choose the Boost implementation of `any` and `optional`. Only the `std` implementation can be used.

Features deprecated in release 1.39 were removed in this release; see <https://github.com/lballabio/QuantLib/pull/2659> for a full list.

A number of features were deprecated in this release and will be removed in a future release (probably release 1.49):

- The `ext::any` and `ext::any_cast` aliases; use `std::any` and `std::any_cast`.
- The `ext::optional` and `ext::nullopt` aliases; use `std::optional` and `std::nullopt`.
- The constructor of `MakeCapFloor` taking a strike and/or a forward start; use the other constructor plus the `withStrike` and/or `withForwardStart` methods.
- The constructors of `MakeCms` taking a spread and/or a forward start; use the other constructor plus the `withIborSpread` and/or `withForwardStart` methods.
- The constructors of `MakeOIS` taking a fixed rate and/or a forward start; use the other constructor plus the `withFixedRate` and/or `withForwardStart` methods.
- The constructors of `MakeVanillaSwap` taking a fixed rate and/or a forward start; use the other constructor plus the `withFixedRate` and/or `withForwardStart` methods.
- The constructors of `InverseCumulativeStudent` taking an accuracy and/or a number of iterations, now unused; use the other constructor.
- The unused `nominalTermStructure_` data member of `ZeroCouponInflationSwapHelper`.
- The `Italy::Exchange` element of the `Italy::Market` enumeration; the exchange follows the TARGET calendar.
- The `QL_DEPRECATED` macro; use `[[deprecated("message")]]` and suggest an alternative in the message.

Also, in a couple of release we will be changing the default of the` QL_NULL_AS_FUNCTIONS` macro from undefined to defined.


Full list of pull requests
--------------------------

All the pull requests merged in this release are listed on its release page at <https://github.com/lballabio/QuantLib/releases/tag/v1.44>.

The list of commits since the previous release is available in `ChangeLog.txt`.

