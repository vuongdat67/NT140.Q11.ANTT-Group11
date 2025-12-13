# AES‑CFB (Cipher Feedback Mode)

## 0/ Filevault context  
Tài liệu này mô tả AES‑CFB để đưa vào Filevault: mục tiêu giải thích khái niệm, toán học, triển khai, rủi ro và biện pháp giảm thiểu cho mã hóa luồng/khối bán luồng trong hệ thống lưu trữ/tuyến truyền.

## 1/ Khái niệm — Thuật toán, vấn đề giải quyết, bảo vệ gì  
- AES‑CFB biến block cipher thành stream cipher tự đồng bộ (self‑synchronizing).  
- Giải quyết: mã hóa dữ liệu theo từng phần (stream/segment) trên kênh có thể mất byte/gói.  
- Bảo vệ: tính bí mật (confidentiality) của dữ liệu; không cung cấp tính toàn vẹn/không thể chối bỏ.

## 2/ Toán học, công thức  
- Notation: $E_K(\cdot)$ là AES với key $K$.  
- Mã hóa (CFB‑s với block size $n$ bit, thường $n=128$):  
    $C_i = P_i \oplus E_K(C_{i-1})$ với $C_0 = IV$.  
- Giải mã:  
    $P_i = C_i \oplus E_K(C_{i-1})$.  
- Khi sử dụng phần segment nhỏ hơn block (CFB‑8, CFB‑1), chỉ lấy $s$ bit từ $E_K(\cdot)$ mỗi lần.

## 3/ Cách hoạt động (ý chính)  
- Khởi tạo: chọn IV dài bằng kích thước block; IV nên ngẫu nhiên/không lặp.  
- Mỗi bước: lấy khối trước (ban đầu IV), mã hóa bằng AES, XOR kết quả với plaintext segment → ciphertext segment.  
- Kết quả ciphertext dùng làm input cho bước tiếp theo (feedback).

## 4/ Cấu trúc dữ liệu  
- Kích thước block AES = 128 bit; key size = 128/192/256 bit.  
- IV: 128 bit (khuyến nghị).  
- Segment size $s$: 128 (CFB‑128), 8 (CFB‑8), 1 (CFB‑1) bits.  
- Internal state: register chứa khối feedback hiện tại.

## 5/ So sánh với các chế độ khác  
- CBC: xử lý block, không tự đồng bộ; cần padding; không cho xử lý byte‑stream trực tiếp.  
- CTR: có thể song song, cho truy cập ngẫu nhiên, nhưng cần counter/nonce; CFB không song song và tự đồng bộ.  
- OFB: tạo keystream độc lập, không lây lan lỗi; CFB lây lan lỗi cho 1‑2 block tiếp theo khi có lỗi ciphertext.

## 6/ Luồng hoạt động (mermaid)  
```mermaid
flowchart LR
    IV[IV = C0] --> E{E_K(C_{i-1})}
    P[Plaintext Pi] --> XOR[XOR]
    E --> XOR
    XOR --> C[Ciphertext Ci]
    C --> Feedback[feed Ci as C_{i-1}]
    Feedback --> E
```

## 7/ Các sai lầm triển khai phổ biến  
- Tái sử dụng IV với cùng key.  
- Dùng IV không đủ ngẫu nhiên hoặc dự đoán được.  
- Không xác thực dữ liệu (không có MAC/AEAD).  
- Nhầm segment_size giữa các hệ thành (CFB‑8 vs CFB‑128).  
- Sử dụng chế độ CFB cho trường hợp cần truy cập ngẫu nhiên/parallel.

## 8/ Threat Model  
- Kẻ tấn công có thể quan sát/giả mạo ciphertext trên kênh; không có quyền biết key.  
- Mục tiêu: ngăn đọc plaintext, nhưng integrity/anti‑tamper không đảm bảo bởi CFB đơn thuần.

## 9/ Biện pháp giảm thiểu  
- Luôn dùng IV duy nhất và không lặp; IV tốt nhất là ngẫu nhiên.  
- Kết hợp xác thực: HMAC hoặc chuyển sang AEAD (AES‑GCM, AES‑CCM).  
- Kiểm soát segment_size và tương thích giữa các bên.  
- Bảo vệ key và quy trình sinh IV.

## 10/ Test Vectors / Kiểm tra  
- Đề nghị dùng bộ vector NIST SP 800‑38A (CFB samples) cho kiểm thử chính xác.  
- Cách tạo nhanh với OpenSSL (CFB128):  
    openssl enc -aes-128-cfb -in plain.bin -out cipher.bin -K <hexkey> -iv <hexiv> -nopad -nosalt  
- Kiểm tra các trường hợp: tất cả‑zero plaintext, khác độ dài block, các segment_size khác nhau.

## 11/ Code (ví dụ ngắn, Python + PyCryptodome)  
```python
from Crypto.Cipher import AES
key = bytes.fromhex("00112233445566778899aabbccddeeff")
iv  = bytes.fromhex("000102030405060708090a0b0c0d0e0f")
cipher = AES.new(key, AES.MODE_CFB, iv=iv, segment_size=128)
ct = cipher.encrypt(b"Secret message here")
# decrypt: new cipher with same key+iv
dec = AES.new(key, AES.MODE_CFB, iv=iv, segment_size=128)
pt = dec.decrypt(ct)
```

## 12/ Checklist bảo mật (tóm tắt)  
- [ ] IV ngẫu nhiên/không lặp.  
- [ ] Key phân phối/bảo quản an toàn.  
- [ ] Dùng MAC hoặc AEAD để đảm bảo integrity.  
- [ ] Đoạn mã dùng segment_size đúng.  
- [ ] Kiểm thử bằng vector chuẩn.

## 13/ Hạn chế (nếu có)  
- Không cung cấp integrity; dễ bị bit‑flipping nếu không xác thực.  
- Không phù hợp cho truy cập ngẫu nhiên và không song song hóa tốt.  
- Lây lan lỗi: lỗi một bit trong ciphertext phá vỡ block hiện tại và sau đó.

## 14/ Ứng dụng  
- Mã hóa luồng dữ liệu/terminal sessions, kênh có thể mất byte/gói, hệ thống legacy cần self‑synchronization.

## 15/ Nguồn tham khảo  
- NIST SP 800‑38A (mô tả chế độ vận hành AES, gồm CFB).  
- PyCryptodome / OpenSSL docs (hướng dẫn sử dụng chế độ CFB).  
- Tài liệu tổng quan về chế độ vận hành block cipher (bài viết, sách giáo khoa).

Ghi chú: ưu tiên thêm xác thực (MAC/AEAD) trước khi dùng AES‑CFB trong môi trường sản xuất.
