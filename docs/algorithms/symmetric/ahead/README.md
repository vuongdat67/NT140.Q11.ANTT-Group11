# AEAD (GCM) Algorithms

This folder lists authenticated encryption options in FileVault. Each page follows a 12-part template (concept, math, flow, data structures, comparison, sequence diagram, pitfalls, threat model, mitigations, test vectors, code, checklist).

## Quick matrix

| Algorithm | Profile hints | Key sizes | Nonce | Tag | Notes |
| :--- | :--- | :--- | :--- | :--- | :--- |
| [AES-GCM](aes-gcm.md) | Default (`PROFESSIONAL`, `ADVANCED`) | 256 (default), 128/192 | 96-bit | 128-bit | Fast with AES-NI; avoid nonce reuse |
| [ARIA-GCM](aria-gcm.md) | KR / RFC 5794 compatibility | 128/192/256 | 96-bit | 128-bit | Korean standard; no AES-NI acceleration |
| [Camellia-GCM](camellia-gcm.md) | JP/EU compliance, AES alternative | 128/192/256 | 96-bit | 128-bit | Strong AES alternative; good HW support in some SoCs |
| [ChaCha20-Poly1305](chacha20-poly1305.md) | Mobile/No AES-NI | 256 | 96-bit | 128-bit | Constant-time ARX; limit 256 GiB per nonce |
| [Serpent-GCM](serpent-gcm.md) | `PARANOID` / archival | 128/192/256 (256 default) | 96-bit | 128-bit | High security margin; slower |
| [SM4-GCM](sm4-gcm.md) | CN compliance (GB/T 32907) | 128 | 96-bit | 128-bit | For WAPI/TCM/TPM ecosystems |
| [Twofish-GCM](twofish-gcm.md) | AES-avoidance, open design | 128/192/256 | 96-bit | 128-bit | Key-dependent S-box; slower than AES |

## Usage reminders
- Never reuse a nonce with the same key; use CSPRNG or monotonic counters per key.
- Keep full 128-bit tags; avoid truncation below 96 bits.
- Always authenticate metadata via AAD (filenames, versions, policy) to prevent context confusion and replay.
- Use library-provided constant-time implementations; avoid custom cipher or GHASH code.

## Related docs
- Concepts: [docs/concepts/key_management.md](../../concepts/key_management.md)
- Workflows: [docs/workflows/encryption_flow.md](../../workflows/encryption_flow.md), [docs/workflows/decryption_flow.md](../../workflows/decryption_flow.md)
- CLI usage: [docs/cli/cli.md](../../cli/cli.md), [docs/cli/demo.md](../../cli/demo.md)
