#include <iostream>
#include <sstream>
#include <string>
#include "very_long_int.h"
#include <random>
#include <climits>
using namespace std;

void tests_from_the_content() {
    {
        VeryLongInt x = 1;
        x /= 0;
        assert(!x.isValid());
    }

    {
        VeryLongInt x = 100;
        x -= 101;
        assert(!x.isValid());
    }

    {
        VeryLongInt x = 23;
        VeryLongInt y = x;
        assert(x == y);
    }

    {
        VeryLongInt x = 23;
        VeryLongInt y = 32;
        x = y;
        assert(x == y);
    }

    {
        VeryLongInt x = 23;
        VeryLongInt y = 32;
        assert(y > x);
    }

    {
        VeryLongInt x = 23;
        VeryLongInt y = 32;
        assert(y >= x);
    }

    {
        VeryLongInt x = NaN();
        assert(!x.isValid());
    }

    {
        VeryLongInt x = 10;
        if (x)
            assert(1);
        else
            assert(0);
    }

    {
        VeryLongInt x = 1;
        x <<= 123;
        x >>= 120;
        assert(8 == x);
    }

    {
        VeryLongInt x = 1;
        for (int i = 1; i <= 100; ++i)
            x *= 2;
        assert(x % 3 == 1);
    }

    {
        VeryLongInt x = Zero();
        assert(x == 0);
    }

    {
        const int N = 100;
        VeryLongInt x = 1;
        for (int i = 1; i < N; ++i)
            x *= 2;
        assert(x.numberOfBinaryDigits() == N);
    }

    {
        VeryLongInt x("1234567890123456789012345678901234567890");
        VeryLongInt z = x;
        VeryLongInt y("777777777777777777777777777777777777777");
        x = x + y;
        x -= y;
        assert(x == z);
    }

    {
        VeryLongInt x(string("12345678"));
        VeryLongInt y(12345678U);
        assert(x == y);
    }

    {
        VeryLongInt x("12345678901234567890");
        VeryLongInt y(12345678901234567890UL);
        assert(x == y);
    }

    {
        VeryLongInt x("1234567890123456789012345678901234567890");
        VeryLongInt y("1204567890123456789012345678901234567890");
        VeryLongInt z(  "30000000000000000000000000000000000000");
        assert(z == x - y);
    }

    {
        VeryLongInt x("10000000000");
        VeryLongInt y("100000000000");
        VeryLongInt z("1000000000000000000000");
        assert(z == x * y);
    }

    {
        const int N = 100;
        VeryLongInt x = 1;
        for (int i = 2; i <= N; ++i)
            x *= i;
        for (int i = 2; i <= N; ++i)
            x /= i;
        assert(x == 1);
    }

    {
        assert(Zero().numberOfBinaryDigits() == 1);
        assert(NaN().numberOfBinaryDigits() == 0);
    }
}

void smart_and_sneaky() {
    {
        VeryLongInt x("7878a76734");
        assert(!x.isValid());
    }
    
    {
        // x = 1 0 0 0 0 1
        VeryLongInt x("1461501637330902918203684832716283019655932542977");
        x -= 2;
        assert(x == VeryLongInt("1461501637330902918203684832716283019655932542975"));
    }
    
    {
        VeryLongInt x(1);
        x <<= 160;
        x += 1;
        assert(x == VeryLongInt("1461501637330902918203684832716283019655932542977"));
    }
    
    {
        VeryLongInt x = NaN();
        x += 1;
        assert(!x.isValid());
    }
    
    {
        VeryLongInt x(1);
        VeryLongInt y(1);
        x <<= 434;
        y <<= 435;
        assert(x != y);
        assert(!(x == y));
        assert(y != x);
        assert(x <= y);
        assert(y >= y);
        assert(x < y);
        assert(y > x);
        
        x >>= 434;
        y >>= 435;
        x <<= 11;
        y <<= 11;
        assert(x == y);
        assert(y == x);
        assert(x >= y);
        assert(x <= y);
        assert(y >= x);
        assert(y <= x);
    }
    
    {
        VeryLongInt x(0);
        VeryLongInt y = Zero();
        assert(x == y);
        assert(y == x);
        assert(x >= y);
        assert(x <= y);
        assert(y >= x);
        assert(y <= x);
    }
    
    {
        VeryLongInt x = 1;
        VeryLongInt y = 1;
        VeryLongInt z = 1;
        x <<= 128;
        x = x + 2;
        y <<= 32;
        y = y - 1;
        z <<= 128;
        z += 1 + (VeryLongInt(1) << 32);
        assert(x + y == z);
    }
    
    {
        VeryLongInt x(1);
        VeryLongInt y(1);
        VeryLongInt z(1);
        x <<= 96;
        //x:  1 0 0 0
        //y : (2^32 - 1) 0 0 0
        y = (VeryLongInt(1) << 96) * ((VeryLongInt(1) << 32) - 1);
        z <<= 128;
        assert(x + y == z);
    }
    
    {
        VeryLongInt x = 1;
        VeryLongInt nan = NaN();
        VeryLongInt zero = Zero();
        x += zero;
        assert(zero == Zero());
        assert(x == 1);
        zero += x;
        assert(zero == 1);
        x += nan;
        nan = nan + NaN();
        assert(!x.isValid());
        assert(!(x == nan));
        assert(!(x != nan));
        assert(!(nan != NaN()));
        assert(!(nan > NaN()));
        assert(!(nan < NaN()));
        assert(!(nan >= NaN()));
        assert(!(nan <= NaN()));
        assert(!(nan == NaN()));
        assert(!(nan != NaN()));
    }
    
    {
        VeryLongInt x(1);
        VeryLongInt y(1);
        x <<= 64;
        y <<= 32;
        y *= ((VeryLongInt(1) << 32) - 1);
        assert(x - y == (VeryLongInt(1) << 32));
    }
    
    {
        VeryLongInt x(0);
        VeryLongInt zero = Zero();
        assert(x - zero == 0);
        x += 1;
        assert(!(zero - x).isValid());
        assert(x - zero == 1);
    }
    
     {
        VeryLongInt x(1);
        VeryLongInt y(1);
        x <<= 160;
        x += 1;
        y <<= 192;
        assert(x < y);
        assert(y > x);
        assert(x <= y);
        assert(y >= x);
        assert(!(x == y));
        assert(x != y);
        assert(y != x);
        x -= y;
        assert(!x.isValid());
    }
    
    {
        VeryLongInt x = 1;
        VeryLongInt y = 1;
        x <<= 32;
        assert(x - y == ((VeryLongInt(1) << 32) - 1));
    }
    
    {
        VeryLongInt x("79228162532711081671548469248");
        VeryLongInt y("79228162532711081671548469249");
        x -= y;
        assert(!x.isValid());
    }
    
    {
        VeryLongInt x("9485763849576493457693475693475694735");
        VeryLongInt y("9485763849576493457693375693475694735");
        x -= y;
        assert(x == 100000000000000ULL);
    }
    
    {
        //x: 1 1 1 0
        VeryLongInt x("79228162532711081671548469248");
        VeryLongInt z = x;
        //y: 1 0 0 0 0 0 0
        VeryLongInt y(1);
        y <<= 192;
        x -= y;
        assert(!x.isValid());
        x = z;
        assert(y - x == VeryLongInt("6277101735386680763835789423128438253569644362792486043648"));
        y += VeryLongInt("18446744073709551616"); // 2 ^ 64
        y = (y - x) % VeryLongInt("10000000000000000000");
        assert(y == VeryLongInt("8091106866195595264")); 
    }
    
    {
        VeryLongInt x("79228162532711081671548469248");
        VeryLongInt y("79228162532711081671548469249");
        y -= x;
        assert(y == 1);
    }
    
    {
        VeryLongInt x(1);
        assert(x * Zero() == 0);
        assert(x * x == 1);
        assert(!(NaN() * Zero()).isValid());
        assert(!(Zero() * NaN()).isValid());
        assert(Zero() * Zero() == Zero());
    }
    
    {
        VeryLongInt x("373463587658732645893245");
        VeryLongInt y("3745687325623423423442");
        assert(x * y == VeryLongInt("1398877826875167245635179667309398519962449290"));
    }
    
    {
        VeryLongInt x("73658769234573269458723465923");
        VeryLongInt y("1");
        assert(x * y == x);
        assert(x * Zero() == Zero());
    }
    
    {
        VeryLongInt x("293840280938673405867340586704");
        VeryLongInt y("93472036032673409560847564");
        assert(x * y == VeryLongInt("27465849327750558230058456421514641147961029147269189056"));
    }
    
    {
        VeryLongInt x = 1;
        VeryLongInt y = 1;
        assert(x / y == 1);
        assert(!(x / Zero()).isValid());
        y <<= 32;
        assert(x / y == Zero());
        assert(y / x == y);
        assert(!(NaN() / NaN()).isValid());
        assert(!(Zero() / NaN().isValid()));
        assert(!(NaN() / Zero()).isValid());
        assert(!(x / NaN()).isValid());
        assert(!(NaN() / y).isValid());
    }
    
    {
        VeryLongInt x(1);
        VeryLongInt y(1);
        x <<= 4533;
        y <<= 4340;
        assert(x / y == VeryLongInt("12554203470773361527671578846415332832204710888928069025792"));
    }
    
    {
        VeryLongInt x(1);
        VeryLongInt y(1);
        x <<= 2345;
        y <<= 9345;
        assert(x / y == 0);
    }
    
    {
        VeryLongInt x(1);
        VeryLongInt y(1);
        x <<= 128;
        y <<= 64;
        assert(x / y == VeryLongInt("18446744073709551616"));
    }
    
    {
        VeryLongInt x(1);
        VeryLongInt y(1);
        x <<= 45675;
        y <<= 35675;
        assert(x / y == (VeryLongInt(1) << 10000));
    }
    
    {
        VeryLongInt x(1);
        VeryLongInt y(1);
        x <<= 777;
        y <<= 776;
        assert(x / y == 2);
    }
    
    {
        VeryLongInt x(1);
        VeryLongInt y(1);
        x <<= 9843569;
        y <<= 4564;
        assert(x % y == Zero());
    }
    
    {
        VeryLongInt x(1);
        VeryLongInt y(1);
        x <<= 2345;
        y <<= 9345;
        assert(x % y == x);
    }
    
    {
        VeryLongInt x(1);
        VeryLongInt y(1);
        x <<= 2345;
        y <<= 2345;
        assert(x % y == 0);
    }
    
    {
        VeryLongInt x(1);
        VeryLongInt y(1);
        x <<= 2;
        y <<= 9345;
        assert(y % x == 0);
    }
    
    {
        VeryLongInt x("9325235");
        VeryLongInt y("34534");
        x <<= 234;
        y <<= 953;
        assert(x % y == VeryLongInt("257441626376854012401276582333443589780351668704389258670924006320484112138240"));
    }
    
    {
        VeryLongInt x(1);
        VeryLongInt y(1);
        x <<= 1000;
        y <<= 10;
        assert(x % y == 0);
    }
    
    {
        VeryLongInt x(1);
        x <<= 0;
        assert(x == x);
        x <<= 1;
        assert(x == 2);
        x <<= 2;
        assert(x == 8);  
    }
    
    {
        VeryLongInt x(128);
        x >>= 0;
        assert(x == x);
        x >>= 1;
        assert(x == 64);
        x >>= 2;
        assert(x == 16);  
    }
    
    {
        VeryLongInt x(1);
        VeryLongInt y(1);
        x <<= 34598;
        x >>= 0;
        assert(x == x);
        x >>= 8475;
        y <<= (34598 - 8475);
        assert(x == y); 
    }
    
    {
        VeryLongInt x(1);
        x <<= 34598;
        x >>= 99999;
        assert(x == 0);
    }
    
    {
        VeryLongInt x(1);
        x >>= 1;
        assert(x == 0);
    }
    
    {
        assert(Zero() >> 1 == 0);
        assert(Zero() << 1 == 0);
        assert(!(NaN() >> 1).isValid());
        assert(!(NaN() << 1).isValid());
    }
    
    {
        VeryLongInt x(1);
        x <<= 999999;
        x >>= 999998;
        assert(x == 2);
    }
    
}

int main() {

    smart_and_sneaky();
  {
      VeryLongInt x = 1000000;
      x -= 100000000000;
      assert(!(x != NaN()));
  }
  
  {
    VeryLongInt x("1152921504606847210"), y(1152921504606847210);
    assert(x == y);
  }

  {
    VeryLongInt x(1), y(1);
    x <<= 40;
    for (int i = 0; i < 40; ++i)
        y *= 2;
    assert(y == 1099511627776);
    assert(x == 1099511627776);
  }

  {
    auto x = VeryLongInt(1);
    assert(x.numberOfBinaryDigits() == 1);
    x <<= 30;
    assert(x.numberOfBinaryDigits() == 31);
    x <<= 5;
    assert(x.numberOfBinaryDigits() == 36);
    assert(x == VeryLongInt(1) << 35);
  }

  //VeryLongInt x(ULONG_MAX);
 // VeryLongInt y(2);
  VeryLongInt a(294967296);
  VeryLongInt b(589934593);
  VeryLongInt c(589934593);
  VeryLongInt d(100000000);
  const VeryLongInt nan = NaN();
  //assert(nan.isValid());
  assert(!(nan.isValid()));
  assert(a.isValid());
  assert(a < b);
  assert(a <= b);
  assert(b == c);
  assert(a > d);
  assert(a >= d);
  assert(a != d);
  assert(b != d);
  assert(d != b);
  
  b -= a;
  //b /= a; 

  {
    auto x = Zero();
    x <<= 32;
    assert(x.numberOfBinaryDigits() == 1);
  }

  {
    VeryLongInt x = 2, y = 3;
    assert(x * y == 6);
  }

  {
    VeryLongInt x = 5;
    assert(x << 1 == 10);

    assert(x >> 1 == 2);
  }

  {
    VeryLongInt x = 71234;
    x <<= 100;
    assert(x >> 100 == 71234);
  }

  {
    VeryLongInt x = 123456789, shifted = x;
    unsigned int shift = 6;
    for (unsigned int i = 0; i < shift; ++i) {
        shifted *= 2;
    }
    auto recovered = shifted >> shift;
    assert(recovered == x);
  }

  {
    VeryLongInt x("10"), y(10);
    assert(x == y);
  }

  {
    VeryLongInt x(1);
    x <<= 34;
    x >>= 5;
    assert(x == (VeryLongInt(1) << 29));
  }

  {
    VeryLongInt x(10);
    assert(x / 5 == 2);
  }

  {
    VeryLongInt x("190");
    assert(x / 3 == 63);
  }

  {
    VeryLongInt x(24100654080);
    x /= 10;
    assert(x.isValid());
    assert(x == 2410065408);
  }

  {
    VeryLongInt x("123456789123456789");
    VeryLongInt y("976543219876543219");
    assert(x < y);
    assert(y > x);
    assert(x <= y);
    assert(y >= x);
  }

  tests_from_the_content();

  {
    VeryLongInt x("123456789123456789123456789");
    VeryLongInt y("987321987654321987654321");
    VeryLongInt sum = x + y;
    VeryLongInt expected_sum("124444111111111111111111110");

    assert(sum == expected_sum);
    assert(sum - x == y);
  }



  {
    VeryLongInt x("1000000000000");
    assert(x / 1000000 == 1000000);
  }

  {
    VeryLongInt x = 2, y = 3;
    for (int i = 0; i < 14; ++i) {
        x *= x;
        y *= y;
    }

    VeryLongInt z = x * y;
    assert(z / y == x);
    assert(z / x == y);
  }

  {
    VeryLongInt x("1237940039285380274899124220");
    x += 4;
    assert(x == VeryLongInt(1) << 90);
  }

  {
    VeryLongInt x("1237940039285380274899124224");
    auto shifted = x >> 90;
    assert(shifted == 1);
  }

  {
    VeryLongInt x = 123456789, y = 100;
    assert(x % y == 89);
  }

  {
    auto nan = VeryLongInt(0) / 0;
    assert(!nan.isValid());
    stringstream s;
    s << nan;
    assert(s.str() == "NaN");
  }

  {
    VeryLongInt ten = 10;
    stringstream s;
    s << ten;
    assert(s.str() == "10");
  }

  {
    assert(!(NaN() == NaN()));
    assert(!(NaN() != NaN()));
    assert(!(NaN() != 7));
    assert(!(NaN() >= 7));
    assert(!(NaN() > 7));
    assert(!(NaN() < 7));
    assert(!(NaN() <= 7));
  }

  {
    auto x = 2 + VeryLongInt("100000000000000000000000000000000000000");
    assert(x == VeryLongInt("100000000000000000000000000000000000002"));
  }

  {
    VeryLongInt x("");
    assert(x == 0);
  }

  {
    VeryLongInt(1);
    VeryLongInt(1U);
    VeryLongInt(10000000000L);
    VeryLongInt(10000000000UL);
    VeryLongInt(10000000000LL);
    VeryLongInt(10000000000ULL);
  }

  {
    VeryLongInt x(1);
    VeryLongInt billion(1000000000);
    auto a = (x << 2000) + 1, b = billion * billion * billion;
    auto res = a % b;
    assert(res == VeryLongInt("817965453762184851149029377"));
  }
     
  return 0;
}
