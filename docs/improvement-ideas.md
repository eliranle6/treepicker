# Improvement ideas

Not implemented: what's left of the 2026-10-24 review's simplifications, and ideas found since,
roughly by impact. A backlog to pick from; the done ones are in git history.

## Concepts

1. **`j` into a collapsed dir.** `j`/`k` move through what's printed (see CLAUDE.md), and scroll
   past the screen's edges, but in a wide view (`music >`, its artists shown as `Bach/ (...)`),
   `j` goes from artist to artist and never into one: `l` does that. It could, at the last one,
   go on into the first artist's albums, as a full listing would; or stay as it is, since `l`
   is right there.

## Code

2. **Thin layers?** `KeyboardReactor` is a 10-line loop and `Picker` only wires it to
   `TreeKeyboardSelector`; one class would do. But kept apart for now (2026-10-25): the picker
   may be made available online, where what drives it isn't the keyboard, and then the loop
   that reads keys is the part to replace.
