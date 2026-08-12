# Deprecation and Backward Compatibility

Read this before deprecating, changing, or removing any public API.

QuantLib follows a convention-driven deprecation strategy: features deprecated
in release **N** are removed in release **N + 5**, then removed in bulk. There
is no single formal compatibility-policy document; the operational practice is
encoded in source annotations, release notes, and a consistent removal cadence.

## Mechanics

Defined in `ql/qldefines.hpp`:

- Prefer `[[deprecated("message")]]` on methods, typedefs, classes, and data
  members.
- Use the `QL_DEPRECATED` macro only for constructors (older MSVC
  compatibility).
- Suppress internal warnings with `QL_DEPRECATED_DISABLE_WARNING` /
  `QL_DEPRECATED_ENABLE_WARNING`.
- Always pair the annotation with a Doxygen `\deprecated` comment stating the
  replacement and "Deprecated in version X.YZ" — this anchors the N+5 removal
  timeline.

## Checklist

- [ ] **DO NOT** change public API; only deprecate, with the replacement API in
      place.
- [ ] Add `[[deprecated("message")]]` (or `QL_DEPRECATED` for constructors) to
      the declaration.
- [ ] Add a `\deprecated` Doxygen comment with the replacement API and
      "Deprecated in version X.YZ."
- [ ] Provide an overload or replacement so callers have a migration path.
- [ ] Wrap internal library usage with `QL_DEPRECATED_DISABLE_WARNING` /
      `QL_DEPRECATED_ENABLE_WARNING`.
- [ ] Update tests and examples to use the replacement API.
- [ ] Document the change in `News.md` under "Removals and deprecations."
