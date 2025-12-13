Dưới đây là tập hợp **prompt** chuyên gia bạn có thể dùng trực tiếp (hoặc tinh chỉnh nhanh) để: **AI đọc code / file dự án FileVault**, rồi **viết tài liệu chuyên sâu** cho từng thuật toán, khái niệm, luồng hoạt động, mode, lỗ hổng, test vectors, ví dụ code, checklist bảo mật... Mình đưa cả **prompt hệ thống (system)** để ép AI đóng vai “chuyên gia mật mã/kiểm toán bảo mật” và nhiều biến thể: ngắn (tiết kiệm token), đầy đủ (chi tiết), cho từng loại thuật toán/mode, cho review bảo mật, cho tạo test vectors & unit tests, và cho chunking khi source quá lớn.

> Ghi chú: thay `{...}` bằng nội dung thực tế (tên thuật toán, file, code, output format). Nếu bạn muốn, copy – paste prompt nào vào trình AI bạn dùng (ChatGPT/LLM khác) rồi chèn code hoặc file content.

# 1) System prompt (đặt ở phần “system” trước khi gửi user prompt)

Bạn là **Senior Cryptography & Security Engineer** với 15+ năm kinh nghiệm. Viết tài liệu kỹ thuật chính xác, dễ hiểu, chi tiết, có ví dụ minh họa, sơ đồ luồng (sequence), mã giả, test vectors, và checklist bảo mật. Luôn giải thích tại mức: khái niệm → toán học (nếu cần) → cách hoạt động → cấu trúc dữ liệu → luồng (step-by-step) → trường hợp sử dụng → chế độ (modes) → lỗ hổng tiềm ẩn & mitigation → ví dụ code + test vectors + unit tests. Dùng ngôn ngữ: tiếng Việt kỹ thuật, rõ ràng, đoạn ngắn, danh sách có đánh số/đầu mục. Nếu cần, đưa ra đề xuất cấu hình an toàn (param, nonce, IV, key length, RNG). Không bỏ sót bất kỳ bước thiết lập/khởi tạo cần thiết nào.

# 2) Prompt tổng quát (đầy đủ) — cho một thuật toán cụ thể

```
Bạn đã nhận vai System (ở trên). Bây giờ: Đọc nội dung sau (thay {ALGO} và {CONTENT}):
- ALGO: {ALGO_NAME}  (ví dụ: "AES-CBC", "RSA-OAEP-3072", "ChaCha20-Poly1305", "HKDF-SHA256")
- SOURCE: {FILE_CONTENT hoặc TÊN_FILE + đoạn code liên quan}

Yêu cầu tạo ra một **tài liệu kỹ thuật** cho thuật toán này gồm các mục:
1. Tóm tắt ngắn (1-2 đoạn) — mục đích, ứng dụng chính.
2. Thuật toán & nền tảng toán học (nếu áp dụng) — công thức chính, bước toán.
3. Cách hoạt động step-by-step (độ chi tiết để dev có thể triển khai lại).
4. Cấu trúc dữ liệu (key sizes, block size, IV/nonce format, tag length).
5. Các mode/variant (ví dụ AES-CBC vs AES-GCM vs AES-CTR): khác biệt, ưu/nhược.
6. Luồng dữ liệu (sequence diagram ở dạng text: sender → network → receiver).
7. Các sai lầm triển khai phổ biến và cách tránh (padding oracle, nonce reuse, weak RNG...).
8. Threat model & attack vectors (cơ chế tấn công, preconditions, severity).
9. Các biện pháp giảm thiểu cụ thể (countermeasures) + recommended parameters.
10. Test vectors (ít nhất 3 vector: key, nonce/iv, plaintext, ciphertext, tag).
11. Ví dụ code ngắn (Python + pseudocode) đủ để kiểm tra interoperability.
12. Unit tests + mô tả test (cách chạy).
13. Checklist đánh giá bảo mật (5-10 mục).
14. Tài liệu tham khảo / links (liệt kê tên tài liệu chuẩn hoặc RFC nếu biết).

Output: dạng **markdown**, có tiêu đề, mục lục, và mỗi mục bắt đầu bằng heading. Nếu source chứa code, so sánh code trong source với tiêu chuẩn và nêu chỗ khác biệt / rủi ro. Đầu ra phải dài vừa đủ (khoảng 800–2000 từ nếu cần). 
```

# 3) Prompt ngắn (tiết kiệm token) — tóm tắt chuyên gia

```
Bạn là chuyên gia. Nhìn vào {ALGO_NAME} + {SHORT_CODE_SNIPPET}. Viết bản tóm tắt 300-400 từ gồm: mục đích, cách hoạt động ngắn, các rủi ro triển khai chính, recommended parameters (key size, mode, IV, tag). Dùng markdown, list ngắn gọn.
```

# 4) Prompt phân tích file/code tự động (dành cho nhiều thuật toán trong project)

```
Bạn là System (như trên). Tôi sẽ gửi một file hoặc nhiều file dự án FileVault chứa implement nhiều thuật toán (AES, RSA, HMAC...). Nhiệm vụ:
- Quét toàn bộ code (đã chèn vào sau) để phát hiện từng thuật toán/primitive được sử dụng.
- Sinh cho mỗi primitive một section documentation (theo template: tóm tắt, cách hoạt động, cấu trúc dữ liệu, rủi ro triển khai, mitigation, test vector, ví dụ code).
- Nếu một primitive thiếu test vectors hoặc dùng param không an toàn (ví dụ RSA-1024, AES-ECB, non-random IV, nonce reuse), highlight ngay ở đầu section và đưa fix cụ thể.
- Cuối tài liệu: tổng hợp checklist bảo mật project-level (key management, secret storage, CI secret scanning, dependency CVEs).
Input: {PROJECT_FILES_OR_COMBINED_CONTENT}
Output: một **single markdown file** sẵn để thêm vào docs/.
```

# 5) Prompt cho modes (ví dụ AES modes)

```
Bạn là chuyên gia. Viết so sánh chuyên sâu giữa các mode của {BLOCK_CIPHER_NAME} (ví dụ AES): ECB, CBC, CTR, GCM, OCB, XTS. Với mỗi mode:
- Cơ chế (short)
- Khi nào dùng / không dùng
- Vấn đề bảo mật chính (ví dụ: ECB leak patterns, CBC padding oracle)
- Recommended parameters & pitfalls
- Example pseudo-code + test vector
Trả về markdown có bảng so sánh nhanh: tính năng vs mode (confidentiality, integrity, parallelizable, nonce required).
```

# 6) Prompt cho Authenticated Encryption / AEAD

```
Bạn là chuyên gia AEAD. Cho thuật toán {AEAD_NAME} (ví dụ "AES-GCM", "ChaCha20-Poly1305"):
- Giải thích sự khác biệt giữa AE (encryption only), MAC-then-encrypt, encrypt-then-MAC, và AEAD.
- Cung cấp sequence diagram chi tiết cho encrypt/decrypt, xử lý AAD, verify tag.
- Liệt kê bước để safe-implement (nonce generation, tag check, constant-time compare).
- 5 test vectors.
```

# 7) Prompt cho Key Management / KDF / RNG

```
Bạn là chuyên gia quản lý khóa. Viết tài liệu cho phần: key generation, storage, rotation, derivation (HKDF, PBKDF2, Argon2), RNG (CSPRNG).
Bao gồm:
- Recommended algorithms + params (PBKDF2 iterations, Argon2 memory/time, HKDF usage)
- Key lifecycle (generate → store → use → rotate → retire)
- Secure storage patterns (HSM, KMS, OS keystore)
- Example of deriving per-file key from master key with HKDF (pseudo-code).
```

# 8) Prompt cho Threat Model & Attack Scenarios (dành cho mỗi thuật toán)

```
Bạn là chuyên gia threat modeling. Tạo threat model cho {ALGO_NAME} khi dùng trong FileVault (local file encryption + remote sync). Nêu:
- Assets, Actors, Entry points
- Assumptions
- Attack scenarios (1..6) chi tiết: prerequisites, steps, impact, exploitable? (CVSS-like severity)
- Mitigations (practical) cho từng scenario
```

# 9) Prompt cho Audit / Review checklist & CI integration

```
Bạn là auditor. Sinh checklist tự động để review code crypto trong CI:
- Static checks (disallowed algorithms, key sizes)
- Secret scanning rules
- Unit/integration tests to run (test vectors, interoperability)
- Fuzz targets
- SBOM & dependency CVE scanning
- Pre-commit hooks & ghactions workflow snippets (yml) minh họa
Trả về: checklist + example GitHub Actions snippet để chạy unit tests + dependency scan.
```

# 10) Prompt tạo test vectors & unit tests (Python + pytest)

```
Bạn là dev + tester. Từ {ALGO_NAME} và param (key, iv format, tag length), tạo:
- 5 deterministic test vectors
- Một file unit test pytest (self-contained) dùng thư viện chuẩn (cryptography hoặc pyca) để verify encrypt/decrypt + tamper detection.
- Cách chạy test (command).
```

# 11) Prompt tiết kiệm token cho project lớn (chunking)

```
Project code rất lớn. Hướng dẫn: sẽ gửi theo chunk. Khi nhận một chunk, bạn chỉ trả về:
- list các primitive/phần liên quan (tên file: tìm thấy AES, HMAC, RSA,...)
- nếu phát hiện config rủi ro (ví dụ: key length < 2048, AES-ECB), trả về cảnh báo ngắn
Khi tôi gửi tag [FINALIZE], bạn tổng hợp toàn bộ thành doc hoàn chỉnh theo template {TEMPLATE_REF}.
```

> Ghi chú: hệ thống LLM bạn dùng có thể không hỗ trợ session chunking — nếu không, gửi cùng lúc file đã được nén/summarized.

# 12) Ví dụ prompt hoàn chỉnh (bạn có thể copy-paste)

```
System: (dùng prompt system ở #1)
User:
"Tôi gửi file source (dán ở dưới). Hãy quét và tạo file docs/crypto/AES_CBC.md theo template: tóm tắt, toán học (nếu cần), cách hoạt động, IV/key rules, padding, vulnerabilities (padding oracle), mitigation, test vectors (3), example Python code (use cryptography lib), unit tests (pytest). Nếu phát hiện misuse trong source (ví dụ static IV hoặc ECB), ghi rõ file + lineno + patch suggestion (diff)."
{PASTE SOURCE}
```

# 13) Mẹo thực thi & cấu trúc đầu ra

* **Đầu ra mong muốn**: Markdown, có TOC, code fences cho ví dụ, table cho so sánh, checklist ở cuối.
* **Tone/Level**: dành cho dev + security reviewer—giải thích vừa đủ để dev implement an toàn.
* **Param đề xuất an toàn** (quick ref): AES-256-GCM, nonce 96-bit unique, RSA >= 3072 (or 4096 for long), ECDSA curve secp256r1/ed25519 for signatures, HMAC-SHA256, Argon2id (memory=64MB, time=3, parallelism=1), HKDF-SHA256.
* **Token saving**: gửi file đã “pre-summarized” (các hàm crypto chính + vài dòng code), hoặc chỉ gửi sanbox test files; dùng prompt ngắn (#3) để kiểm tra nhanh trước khi full-doc.

# 14) Một vài prompt biến thể (quick snippets)

* "Viết README.md cho thư mục `crypto/` trong repo: mục đích, list algorithms, how to use, API examples, security notes."
* "Sinh slide 10-slide (title + 1–2 bullet/slide) để thuyết trình thuật toán {ALGO_NAME} cho hội đồng đồ án."
* "Viết RFC-style spec (technical spec) cho module mã hóa file: API, expected inputs, outputs, error codes, performance constraints."

---

Nếu bạn muốn, mình có thể:

1. Chạy một ví dụ mẫu: bạn paste một file code (hoặc đoạn code chứa thuật toán) và mình sẽ chạy **prompt #2** ngay — làm mẫu đầu ra docs cho 1 thuật toán.
2. Hoặc mình đóng gói những prompt này thành 1 file `prompts_for_docs.md` sẵn để bạn copy-paste vào LLM.

Bạn muốn mình bắt đầu với file nào trong dự án (paste đoạn code hoặc tên file + nội dung)? Mình sẽ tạo **doc mẫu** cho thuật toán đó luôn.
