// ARMOR-NODE-HMI - SHA-256 (FIPS 180-4), small and without any library, so that what is checked on the board is checked the same way in the tests on a computer.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace armor::sha256 {

namespace detail {
constexpr std::array<std::uint32_t, 64> kK = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
constexpr std::uint32_t rotr(std::uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }
}  // namespace detail

class Hasher {
 public:
  void update(const std::uint8_t* data, std::size_t size) {
    total_ += size;
    while (size > 0) {
      const std::size_t take = std::min(size, buffer_.size() - used_);
      for (std::size_t i = 0; i < take; ++i) buffer_[used_ + i] = data[i];
      used_ += take; data += take; size -= take;
      if (used_ == buffer_.size()) { block(buffer_.data()); used_ = 0; }
    }
  }
  std::array<std::uint8_t, 32> finish() {
    const std::uint64_t bits = total_ * 8;
    const std::uint8_t one = 0x80;
    update(&one, 1);
    const std::uint8_t zero = 0;
    while (used_ != 56) update(&zero, 1);
    std::uint8_t length[8];
    for (int i = 0; i < 8; ++i) length[i] = static_cast<std::uint8_t>(bits >> (56 - 8 * i));
    update(length, 8);
    std::array<std::uint8_t, 32> out{};
    for (int i = 0; i < 8; ++i) for (int j = 0; j < 4; ++j) out[static_cast<std::size_t>(i * 4 + j)] = static_cast<std::uint8_t>(h_[static_cast<std::size_t>(i)] >> (24 - 8 * j));
    return out;
  }

 private:
  void block(const std::uint8_t* p) {
    std::uint32_t w[64];
    for (int i = 0; i < 16; ++i) w[i] = (std::uint32_t{p[i * 4]} << 24) | (std::uint32_t{p[i * 4 + 1]} << 16) | (std::uint32_t{p[i * 4 + 2]} << 8) | std::uint32_t{p[i * 4 + 3]};
    for (int i = 16; i < 64; ++i) {
      const std::uint32_t s0 = detail::rotr(w[i - 15], 7) ^ detail::rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
      const std::uint32_t s1 = detail::rotr(w[i - 2], 17) ^ detail::rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
      w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    std::uint32_t a = h_[0], b = h_[1], c = h_[2], d = h_[3], e = h_[4], f = h_[5], g = h_[6], h = h_[7];
    for (int i = 0; i < 64; ++i) {
      const std::uint32_t s1 = detail::rotr(e, 6) ^ detail::rotr(e, 11) ^ detail::rotr(e, 25);
      const std::uint32_t ch = (e & f) ^ (~e & g);
      const std::uint32_t t1 = h + s1 + ch + detail::kK[static_cast<std::size_t>(i)] + w[i];
      const std::uint32_t s0 = detail::rotr(a, 2) ^ detail::rotr(a, 13) ^ detail::rotr(a, 22);
      const std::uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
      const std::uint32_t t2 = s0 + maj;
      h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }
    h_[0] += a; h_[1] += b; h_[2] += c; h_[3] += d; h_[4] += e; h_[5] += f; h_[6] += g; h_[7] += h;
  }

  std::array<std::uint32_t, 8> h_ = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
  std::array<std::uint8_t, 64> buffer_{};
  std::size_t used_ = 0;
  std::uint64_t total_ = 0;
};

// The SHA-256 of the bytes, as 64 lowercase hexadecimal digits.
inline std::string hex_of(std::string_view data) {
  Hasher hasher;
  hasher.update(reinterpret_cast<const std::uint8_t*>(data.data()), data.size());
  const auto digest = hasher.finish();
  static const char* kDigits = "0123456789abcdef";
  std::string out;
  for (std::uint8_t byte : digest) { out += kDigits[byte >> 4]; out += kDigits[byte & 15]; }
  return out;
}

}  // namespace armor::sha256
