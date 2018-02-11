#include "very_long_int.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <sstream>

using namespace std;

bool operator!=(VeryLongInt const &x, VeryLongInt const &y) {
    return x.isValid() && y.isValid() && !(x == y);
}

bool operator>(VeryLongInt const &x, VeryLongInt const &y) {
    return y < x;
}

bool operator<=(VeryLongInt const &x, VeryLongInt const &y) {
    return x.isValid() && y.isValid() && !(x > y);
}

bool operator>=(VeryLongInt const &x, VeryLongInt const &y) {
    return x.isValid() && y.isValid() && !(x < y);
}

const VeryLongInt operator+(VeryLongInt x, VeryLongInt const &y) {
    return x += y;
}

const VeryLongInt operator-(VeryLongInt x, VeryLongInt const &y) {
    return x -= y;
}

const VeryLongInt operator*(VeryLongInt x, VeryLongInt const &y) {
    return x *= y;
}

const VeryLongInt operator/(VeryLongInt x, VeryLongInt const &y) {
    return x /= y;
}

const VeryLongInt operator%(VeryLongInt x, VeryLongInt const &y) {
    return x %= y;
}

const VeryLongInt operator<<(VeryLongInt x, size_t y) {
    return x <<= y;
}

const VeryLongInt operator>>(VeryLongInt x, size_t y) {
    return x >>= y;
}

VeryLongInt::VeryLongInt(uint64_t x) {
    digits.push_back(static_cast<Component>(x));
    auto rest = static_cast<Component>(x >> 32);
    if (rest > 0)
        digits.push_back(rest);
}

VeryLongInt::VeryLongInt(string const &x) : VeryLongInt(0) {
    /* Zapisujemy znak po znaku. */
    for (auto c : x) {
        if ('0' <= c && c <= '9') {
            *this *= 10;
            *this += c - '0';
        } else {
            *this = NaN();
            break;
        }
    }
}

size_t VeryLongInt::numberOfBinaryDigits() const {
    if (!isValid())
        return 0;
    if (*this == Zero())
        return 1;
    size_t size = digits.size();
    Component res = 0;
    Component x = digits[size - 1];
    while (x != 0) {
        x /= 2;
        ++res;
    }
    res += (size - 1) * USED_DIGITS;
    return res;
}

void VeryLongInt::removeLeadingZeros() {
    size_t size = digits.size();
    while (size > 1 && digits[size - 1] == 0)
        --size;
    digits.resize(size);
}

VeryLongInt& VeryLongInt::operator+=(VeryLongInt const &x) {
    if (!isValid() || !x.isValid()) {
        *this = NaN();
    } else {
        digits.resize(max(digits.size(), x.digits.size()));
        size_t len = x.digits.size();
        uint64_t carry = 0;  /* ile dodatkowo dodać w następnej pozycji */
        for (size_t i = 0; i < len; ++i) {
            Component a = digits[i];
            Component b = x.digits[i];
            auto sum = carry + static_cast<uint64_t>(a) + b;
            digits[i] = static_cast<Component>(sum);
            carry = sum >> USED_DIGITS;
        }
        for (size_t i = len; i < digits.size(); ++i) {
            uint64_t sum = carry + digits[i];
            digits[i] = static_cast<Component>(sum);
            carry = sum >> USED_DIGITS;
        }
        if (carry != 0)
            digits.push_back(static_cast<Component>(carry));
        assert(digits.size() == 1 || digits.back() != 0);
    }
    return *this;
}

VeryLongInt& VeryLongInt::operator-=(VeryLongInt const &x) {
    if (!isValid() || !x.isValid() || (digits.size() < x.digits.size())) {
        *this = NaN();
    } else {
        uint64_t carry = 0;  /* ile dodatkowo odjąć w następnej pozycji */
        for (size_t i = 0; i < x.digits.size(); ++i) {
            uint64_t a = digits[i];
            uint64_t b = carry + x.digits[i];
            if (a < b) {
                digits[i] = a + (BASE - b);
                carry = 1;
            } else {
                digits[i] -= b;
                carry = 0;
            }
        }
        for (size_t i = x.digits.size(); i < digits.size(); ++i) {
            if (digits[i] < carry) {
                digits[i] = BASE - 1;
            } else {
                digits[i] -= carry;
                carry = 0;
            }
        }
        if (carry > 0)
            *this = NaN();
        removeLeadingZeros();
    }
    return *this;
}

VeryLongInt VeryLongInt::multiplyByComponent(Component x) const {
    assert(isValid());
    VeryLongInt result = Zero();
    size_t orig_len = digits.size();
    result.digits.resize(orig_len + 1);
    uint64_t carry = 0;      /* ile dodatkowo dodać w następnej pozycji */
    for (size_t i = 0; i < orig_len; ++i) {
        auto tmp = static_cast<uint64_t>(digits[i]) * x + carry;
        result.digits[i] = static_cast<Component>(tmp);
        carry = tmp >> USED_DIGITS;
    }
    result.digits[orig_len] = carry;
    result.removeLeadingZeros();
    return result;
}

void VeryLongInt::shiftByComponentsSize(size_t shift) {
    if (*this != Zero())
        digits.insert(begin(digits), shift, static_cast<Component>(0));
}

VeryLongInt& VeryLongInt::operator*=(VeryLongInt const &x) {
    if (!isValid() || !x.isValid()) {
        *this = NaN();
    } else {
        auto result = Zero();
        for (size_t shift = 0; shift < x.digits.size(); ++shift) {
            auto partial = this->multiplyByComponent(x.digits[shift]);
            partial.shiftByComponentsSize(shift);
            result += partial;
        }
        *this = move(result);
    }
    return *this;
}

tuple<VeryLongInt, VeryLongInt> VeryLongInt::divMod(
        VeryLongInt const &x, VeryLongInt const &y) {
    if (!x.isValid() || !y.isValid() || y == Zero()) {
        return make_tuple(NaN(), NaN());
    } else {
        VeryLongInt remainder = x;
        VeryLongInt quotient = Zero();
        const VeryLongInt one = 1;
        while (remainder >= y) {
            auto shift = remainder.numberOfBinaryDigits() -
                         y.numberOfBinaryDigits();
            VeryLongInt tmp = y << shift;
            if (tmp > remainder) {
                tmp >>= 1;
                --shift;
            }
            remainder -= tmp;
            quotient += (one << shift);
            remainder = remainder;
        }
        return make_tuple(quotient, remainder);
    }
}

VeryLongInt& VeryLongInt::operator/=(VeryLongInt const &x) {
    auto res = divMod(*this, x);
    *this = get<0>(res);
    return *this;
}

VeryLongInt& VeryLongInt::operator%=(VeryLongInt const &x) {
    auto res = divMod(*this, x);
    *this = get<1>(res);
    return *this;
}

bool operator<(VeryLongInt const &x, VeryLongInt const &y) {
    if (!x.isValid() || !y.isValid())
        return false;

    size_t x_size = x.digits.size();
    size_t y_size = y.digits.size();
    if (x_size > y_size)
        return false;
    else if (x_size < y_size)
        return true;

    return lexicographical_compare(x.digits.rbegin(), x.digits.rend(),
        y.digits.rbegin(), y.digits.rend());
}

bool operator==(VeryLongInt const &x, VeryLongInt const &y) {
    if (!x.isValid() || !y.isValid())
        return false;
    return x.digits == y.digits;
}

ostream &operator<<(ostream &out, VeryLongInt const &x) {
    if (!x.isValid()) {
        out << "NaN";
    } else if (x == Zero()) {
        out << "0";
    } else {
        stringstream ss;
        VeryLongInt y = x;
        while (y) {
            ss << (y % 10).getLastDecimalDigit();
            y /= 10;
        }
        string repr = ss.str();
        reverse(begin(repr), end(repr));
        out << repr;
    }
    return out;
}

VeryLongInt& VeryLongInt::operator<<=(size_t full_shift) {
    if (isValid() && full_shift > 0) {
        auto digit_shift = full_shift % USED_DIGITS;
        size_t len = digits.size();
        if (digit_shift > 0) {
            Component carry = 0;
            for (size_t i = 0; i < len; ++i) {
                Component tmp = digits[i];
                digits[i] = (tmp << digit_shift) | carry;
                carry = tmp >> (USED_DIGITS - digit_shift);
            }
            if (carry)
                digits.push_back(carry);
        }
        shiftByComponentsSize(full_shift / USED_DIGITS);
    }
    return *this;
}

VeryLongInt& VeryLongInt::operator>>=(size_t full_shift) {
    if (isValid() && full_shift > 0) {
        auto erase_end = begin(digits) + (full_shift / USED_DIGITS);
        if (erase_end > digits.end()) {
            *this = Zero();
        } else {
            digits.erase(begin(digits), erase_end);
            auto digit_shift = full_shift % USED_DIGITS;
            if (digit_shift > 0) {
                Component carry = 0;
                for (auto it = digits.rbegin(); it != digits.rend(); ++it) {
                    Component tmp = *it;
                    *it = (tmp >> digit_shift) | carry;
                    carry = tmp << (USED_DIGITS - digit_shift);
                }
            }
            removeLeadingZeros();
        }
    }
    return *this;
}

char VeryLongInt::getLastDecimalDigit() const {
    assert(isValid());
    return ('0' + digits.front() % 10);
}

const VeryLongInt& Zero() {
    static VeryLongInt zero = VeryLongInt(0);
    return zero;
}

const VeryLongInt& NaN() {
    static VeryLongInt nan = VeryLongInt(0);
    nan.digits.clear();
    return nan;
}
