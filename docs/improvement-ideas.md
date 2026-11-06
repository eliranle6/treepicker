# Improvement ideas

Not implemented: what's left of the 2026-10-24 review's simplifications, and ideas found since,
roughly by impact. A backlog to pick from; the done ones are in git history.

## Code

1. **Thin layers?** `KeyboardReactor` is a 10-line loop and `Picker` only wires it to
   `TreeKeyboardSelector`; one class would do. But kept apart for now (2026-10-25): the picker
   may be made available online, where what drives it isn't the keyboard, and then the loop
   that reads keys is the part to replace.
