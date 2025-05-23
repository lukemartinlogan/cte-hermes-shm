/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Distributed under BSD 3-Clause license.                                   *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Illinois Institute of Technology.                        *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Hermes. The full Hermes copyright notice, including  *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the top directory. If you do not  *
 * have access to the file, you may request a copy from help@hdfgroup.org.   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef HSHM_BASICS_H
#define HSHM_BASICS_H

#include <cstddef>
#include <cstdint>

#include "hermes_shm/constants/macros.h"
#include "hermes_shm/types/numbers.h"

#ifdef __cplusplus

namespace hshm {

/**
 * decimal + (numerator/65536)
 * */
struct RealNumber {
  uint64_t decimal_;
  uint32_t numerator_;
  CLS_CONST uint32_t precision = 65536;

  HSHM_INLINE_CROSS_FUN
  RealNumber() = default;

  /**
   * Converts numerator / denomintor ->
   * decimal + (numerator / 65536);
   *
   * For example,
   * 4/5 = (4 * 65536 / 5) / 65536
   * */
  HSHM_INLINE_CROSS_FUN
  explicit RealNumber(uint64_t numerator, uint64_t denominator) {
    decimal_ = numerator / denominator;
    uint64_t rem = numerator % denominator;
    numerator_ = (u32)(rem * precision / denominator);
  }

  HSHM_INLINE_CROSS_FUN
  RealNumber(const RealNumber &other)
      : decimal_(other.decimal_), numerator_(other.numerator_) {}

  HSHM_INLINE_CROSS_FUN
  RealNumber &operator=(const RealNumber &other) {
    decimal_ = other.decimal_;
    numerator_ = other.numerator_;
    return *this;
  }

  HSHM_INLINE_CROSS_FUN
  RealNumber(RealNumber &&other) noexcept
      : decimal_(other.decimal_), numerator_(other.numerator_) {}

  HSHM_INLINE_CROSS_FUN
  RealNumber &operator=(RealNumber &&other) noexcept {
    decimal_ = other.decimal_;
    numerator_ = other.numerator_;
    return *this;
  }

  /**
   * (d1 + n1/p) * d2 =
   * d1 * d2 + d2 * n1 / p
   * */
  HSHM_INLINE_CROSS_FUN
  RealNumber operator*(size_t other) const {
    RealNumber res;
    res.decimal_ = other * decimal_;
    uint64_t frac = other * numerator_;
    res.decimal_ += frac / precision;
    res.numerator_ = frac % precision;
    return res;
  }

  /**
   * (d1 + n1/p) * (d2 + n2/p) =
   * (d1 * d2) + (d1 * n2)/p + (d2 * n1) / p + (n1 * n2 / p) / p =
   * (d1 * d2) + [(d1 * n2) + (d2 * n1) + (n1 * n2)/p] / p
   * */
  HSHM_INLINE_CROSS_FUN
  RealNumber operator*(const RealNumber &other) const {
    RealNumber res;
    // d1 * d2
    res.decimal_ = other.decimal_ * decimal_;
    uint64_t frac = (decimal_ * other.numerator_) +               // d1 * n2
                    (other.decimal_ * numerator_) +               // d2 * n1
                    (numerator_ * other.numerator_) / precision;  // n1 * n2 / p
    res.decimal_ += frac / precision;
    res.numerator_ = frac % precision;
    return res;
  }

  HSHM_INLINE_CROSS_FUN
  RealNumber operator*=(size_t other) {
    (*this) = (*this) * other;
    return *this;
  }

  HSHM_INLINE_CROSS_FUN
  RealNumber operator*=(const RealNumber &other) {
    (*this) = (*this) * other;
    return *this;
  }

  HSHM_INLINE_CROSS_FUN
  size_t as_int() const { return (size_t)(decimal_ + numerator_ / precision); }
};

}  // namespace hshm

#endif

#endif  // HSHM_BASICS_H
