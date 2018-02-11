#ifndef JNP1_VERY_LONG_INT_H
#define JNP1_VERY_LONG_INT_H

#include <iostream>
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#include <climits>
#include <tuple>

class VeryLongInt;

bool operator!=(VeryLongInt const &x, VeryLongInt const &y);
bool operator>(VeryLongInt const &x, VeryLongInt const &y);
bool operator<=(VeryLongInt const &x, VeryLongInt const &y);
bool operator>=(VeryLongInt const &x, VeryLongInt const &y);
const VeryLongInt operator+(VeryLongInt x, VeryLongInt const &y);
const VeryLongInt operator-(VeryLongInt x, VeryLongInt const &y);
const VeryLongInt operator*(VeryLongInt x, VeryLongInt const &y);
const VeryLongInt operator/(VeryLongInt x, VeryLongInt const &y);
const VeryLongInt operator%(VeryLongInt x, VeryLongInt const &y);
const VeryLongInt operator<<(VeryLongInt x, size_t y);
const VeryLongInt operator>>(VeryLongInt x, size_t y);

class VeryLongInt {
private:
    using Component = uint32_t;

    /* Wektor w którym trzymane są kolejne cyfry naszej liczby,
     * w systemie little endian.
     * Zero reprezentowane jako wektor rozmiaru 1, posiada tylko liczbę '0'
     * NaN reprezentowane jako pusty wektor */
    std::vector<Component> digits;

    /* Liczba bitów do zapisu cyfry */
    const size_t USED_DIGITS = 32;
    /* Baza systemu liczbowego */
    const uint64_t BASE = static_cast<uint64_t>(1) << USED_DIGITS;

    /* Usuwa zbędne zera z przodu */
    void removeLeadingZeros();
    /* Dodaje na początku wektora shift zer */
    void shiftByComponentsSize(size_t shift);
    /* Mnoży VeryLongInt przez liczbę typu Component */
    VeryLongInt multiplyByComponent(Component) const;
    /* Zwraca cyfrę jakos char */
    char getLastDecimalDigit() const;
    /* Dzieli x / y. Zwraca parę <iloraz, reszta> */
    static std::tuple<VeryLongInt, VeryLongInt> divMod(
        VeryLongInt const &x, VeryLongInt const &y);

public:
    VeryLongInt() : VeryLongInt(0) {}
    VeryLongInt(uint64_t num);
    VeryLongInt(int num) : VeryLongInt(static_cast<uint64_t>(num)) {}
    VeryLongInt(unsigned num) : VeryLongInt(static_cast<uint64_t>(num)) {}
    VeryLongInt(long num) : VeryLongInt(static_cast<uint64_t>(num)) {}
    VeryLongInt(long long num) : VeryLongInt(static_cast<uint64_t>(num)) {}
    VeryLongInt(unsigned long long num) :
         VeryLongInt(static_cast<uint64_t>(num)) {}
    VeryLongInt(VeryLongInt const &x) : digits(x.digits) {}
    VeryLongInt(VeryLongInt &&x) : digits(std::move(x.digits)) {}

    /* Pusty string jest równoważny liczbie 0,
     * string który jest niepoprawny (posiada inne znaki niż liczby)
     * jest reprezentowany jako NaN */
    explicit VeryLongInt(std::string const &x);
    explicit VeryLongInt(const char* x) : VeryLongInt(std::string(x)) {}

    explicit VeryLongInt(char c) = delete;
    explicit VeryLongInt(signed char c) = delete;
    explicit VeryLongInt(unsigned char c) = delete;
    explicit VeryLongInt(char16_t c) = delete;
    explicit VeryLongInt(char32_t c) = delete;
    explicit VeryLongInt(wchar_t c) = delete;
    explicit VeryLongInt(bool b) = delete;

    VeryLongInt& operator+=(VeryLongInt const &x);
    VeryLongInt& operator-=(VeryLongInt const &x);
    VeryLongInt& operator*=(VeryLongInt const &x);
    VeryLongInt& operator/=(VeryLongInt const &x);
    VeryLongInt& operator%=(VeryLongInt const &x);

    VeryLongInt& operator<<=(size_t x);
    VeryLongInt& operator>>=(size_t x);

    VeryLongInt& operator=(VeryLongInt const &x) {
        digits = x.digits;
        return *this;
    }

    VeryLongInt& operator=(VeryLongInt &&x) {
        digits = std::move(x.digits);
        return *this;
    }

    explicit operator bool() const {
        if (digits.size() == 0)
            return false;
        if (digits.size() == 1 && digits[0] == 0)
            return false;
        return true;
    }

    /* Zwraca liczbę cyfr w zapisie dwójkowym */
    size_t numberOfBinaryDigits() const;

    /* Zwraca false jeżeli liczba jest NaN */
    bool isValid() const {
        return !(digits.empty());
    }

    friend bool operator<(VeryLongInt const &x, VeryLongInt const &y);
    friend bool operator==(VeryLongInt const &x, VeryLongInt const &y);
    friend std::ostream &operator<<(std::ostream &out, VeryLongInt const &x);
    friend const VeryLongInt& NaN();
};

const VeryLongInt& Zero();
const VeryLongInt& NaN();

#endif
