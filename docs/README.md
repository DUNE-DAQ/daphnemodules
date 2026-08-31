# DAPHNE V3 gateware-mode contract

The dual-gateware client behavior described here is introduced in
`daphnemodules` 3.0.4. Released 3.0.3 clients remain suitable for self-trigger,
but they do not serialize the full-stream channel selection.

`full_stream_channels` is an ordered list: entry zero selects the source for
full-stream output zero, entry one selects output one, and so on. The DAPHNE V3
hardware exposes 32 outputs selecting from board channels 0 through 39, so the
list may contain at most 32 unique values in that range.

The current appmodel and `ConfigureRequest` do not carry an explicit gateware
mode. A non-empty `full_stream_channels` list therefore unambiguously selects
full-stream and suppresses self-trigger-counter polling. An empty list is
ambiguous: it may mean self-trigger or full-stream with every output disabled.
For compatibility, the controller treats an empty list as self-trigger for
monitoring. A future schema revision should add an explicit mode before an
empty-output full-stream configuration is supported.
