/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef quantlib_utilities_variants_hpp
#define quantlib_utilities_variants_hpp

namespace QuantLib::detail {

    // Helper type for use with std::visit.
    template <class... Ts>
    struct variant_visitor : Ts... {
        using Ts::operator()...;
    };

    template <class... Ts>
    variant_visitor(Ts...) -> variant_visitor<Ts...>;

}

#endif
