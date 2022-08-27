Changes for QuantLib 1.27.1:
============================

QuantLib 1.27.1 is a bug-fix release.

It restores the old implementation of `Null<T>` which was replaced
in version 1.27 with a new one; the latter was reported to cause
an internal compiler error under Visual C++ 2022 for some client code.
The new version (which avoids some problems when replacing `Real`
with some AAD-enabled types) is still available; depending on how
you compile QuantLib, it can be enabled through the
`--enable-null-as-functions` configure flag, the cmake variable
`QL_NULL_AS_FUNCTIONS`, or the define with the same name in the
`ql/userconfig.hpp` header.

