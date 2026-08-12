# Deprecation and Backward Compatibility

Read this before deprecating, changing, or removing any public API.

QuantLib follows a convention-driven deprecation strategy: features deprecated
in release **N** are removed in release **N + 5**, then removed in bulk. There
is no single formal compatibility-policy document; the operational practice is
encoded in source annotations, release notes, and a consistent removal cadence.

## Mechanics

Defined in `ql/qldefines.hpp`:

- Use `[[deprecated("message")]]` everywhere — methods, constructors, typedefs,
  classes, and data members.
- Do **not** use the legacy `QL_DEPRECATED` macro in new code. It only existed to
  deprecate constructors under VC++2015, which is no longer supported since the
  move to C++17; the macro is itself slated for deprecation.
- First try to avoid warnings by using the new interfaces,
  and only if that's not possible suppress internal warnings with `QL_DEPRECATED_DISABLE_WARNING` /
  `QL_DEPRECATED_ENABLE_WARNING`.
- Always pair the annotation with a Doxygen `\deprecated` comment stating the
  replacement and "Deprecated in version X.YZ" — this anchors the N+5 removal
  timeline.

## Pitfalls

When a data member is deprecated, some compilers emit a deprecation warning for
the class destructor, even when that destructor is implicitly generated. Give the
class an explicit destructor with the warning suppressed:

```cpp
QL_DEPRECATED_DISABLE_WARNING
~SomeClass() = default;   // add `override` if the destructor is virtual
QL_DEPRECATED_ENABLE_WARNING
```

The destructor can be dropped again once the deprecated data member is removed.

## Checklist

- [ ] **DO NOT** change public API; only deprecate, with the replacement API in
      place.
- [ ] Add `[[deprecated("message")]]` to the declaration.
- [ ] Add a `\deprecated` Doxygen comment with the replacement API and
      "Deprecated in version X.YZ."
- [ ] Provide an overload or replacement so callers have a migration path.
- [ ] Move internal library usage to the replacement API; only where that is not
      possible, wrap it with `QL_DEPRECATED_DISABLE_WARNING` /
      `QL_DEPRECATED_ENABLE_WARNING`.
- [ ] When deprecating a data member, add the explicit destructor described under
      [Pitfalls](#pitfalls).
- [ ] Update tests and examples to use the replacement API.
- [ ] Document the change in `News.md` under "Removals and deprecations."
