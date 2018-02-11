#include <cstdlib>
#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>

#include "priorityqueue.hh"


#if TEST_NUM != 100
bool gChecking = false;
int gInstancesCounter;

int ThisCanThrow (const int& i = 0)
{
    if (gChecking)
    {
        throw 0;
    }
    return i;
}

void* operator new(size_t size)
{
    try
    {
        ThisCanThrow();
        void* p = malloc(size);
        if (!p)
            throw "operator new[]() error";
        return p;
    }
    catch (...)
    {
        throw ::std::bad_alloc();
    }
}

void* operator new[](size_t size)
{
    try
    {
        ThisCanThrow();
        void* p = malloc(size);
        if (!p)
            throw "operator new[]() error";
        return p;
    }
    catch (...)
    {
        throw ::std::bad_alloc();
    }
}

void operator delete (void* p) noexcept
{
    free(p);
}

void operator delete[](void *p) noexcept
{
    free(p);
}

struct Tester
{
    int* p;
    explicit Tester (int v = 0) : p(new int(ThisCanThrow(v)))
    {
        ++gInstancesCounter;
    }

    Tester (const Tester& rhs) : p(new int(ThisCanThrow(*rhs.p)))
    {
        ++gInstancesCounter;
    }

    Tester(Tester&& t) = default;

    Tester& operator=(const Tester& rhs)
    {
        ThisCanThrow();
        *p = *rhs.p;
        return *this;
    }

    ~Tester() noexcept
    {
        /* This can NOT throw! */
        gInstancesCounter--;
        delete p;
    }

    bool const operator<(const Tester& rhs) const
    {
        ThisCanThrow();
        return *p < *rhs.p;
    }

    bool const operator>(const Tester& rhs) const
    {
        ThisCanThrow();
        return *p > *rhs.p;
    }

    bool const operator<=(const Tester& rhs) const
    {
        ThisCanThrow();
        return *p <= *rhs.p;
    }

    bool const operator>=(const Tester& rhs) const
    {
        ThisCanThrow();
        return *p >= *rhs.p;
    }

    bool const operator==(const Tester& rhs) const
    {
        ThisCanThrow();
        return *p == *rhs.p;
    }

    bool const operator!=(const Tester& rhs) const
    {
        ThisCanThrow();
        return *p != *rhs.p;
    }
};

::std::ostream & operator << (::std::ostream& os, Tester t) {
    return os << "<tester(" << t.p << ")>";
}


template <class Value, class Operation, class Result>
Result NoThrowCheck(Value& v, const Operation& op)
{
    Result result;

    try
    {
        gChecking = true;
        result = op(v); /* Try the operation. */
        gChecking = false;
    }
    catch(...) /* Catch all exceptions. */
    {
        gChecking = false;
        ::std::clog << "Operacja '" << op.what() << "' okazala sie nie byc NO-THROW\n" << ::std::endl;
        assert(false);
    }

    return result;
}

template <class Value, class Operation>
void NoThrowCheckVoid(Value& v, const Operation& op)
{
    try
    {
        gChecking = true;
        op(v); /* Try the operation. */
        gChecking = false;
    }
    catch(...) /* Catch all exceptions. */
    {
        gChecking = false;
        ::std::clog << "Operacja '" << op.what() << "' okazala sie nie byc NO-THROW\n";
        assert(false);
    }
}

template <class Value, class Operation, class Result>
Result StrongCheck(Value& v, const Operation& op)
{
    Result result;
    bool succeeded = false;

    Value duplicate = v;
    try
    {
        gChecking = true;
        result = op(duplicate); /* Try the operation. */
        gChecking = false;
        succeeded = true;
    }
    catch(...) /* Catch all exceptions. */
    {
        gChecking = false;
        bool unchanged = duplicate == v; /* Test strong guarantee. */
        if (!unchanged)
        {
            ::std::clog << "Operacja '" << op.what() << "' okazala sie nie byc STRONG." << ::std::endl;
            assert(unchanged);
        }
    }

    if (succeeded)
    {
        v = duplicate;
    }

    return result;
}

template <class Value, class Operation>
void StrongCheckVoid(Value& v, const Operation& op)
{
    bool succeeded = false;
    Value duplicate = v;

    try
    {
        gChecking = true;
        op(duplicate); /* Try the operation. */
        gChecking = false;
        succeeded = true;
    }
    catch(...) /* Catch all exceptions. */
    {
        gChecking = false;
        bool unchanged = duplicate == v; /* Test strong guarantee. */
        if (!unchanged)
        {
            ::std::clog << "Operacja '" << op.what() << "' okazala sie nie byc STRONG." << ::std::endl;
            assert(unchanged);
        }
    }

    if (succeeded)
    {
        v = duplicate;
    }
}

typedef PriorityQueue<Tester, Tester> TesterPQ;

struct Assignment
{
    const TesterPQ& q;
    Assignment(const TesterPQ& _q) : q(_q)
    {
    }

    void operator()(TesterPQ& q1) const
    {
        q1 = q;
    }

    ::std::string what() const
    {
        return ::std::string("operator=");
    }
};

struct Empty
{
    bool const operator()(TesterPQ& q) const
    {
        return q.empty();
    }

    ::std::string what() const
    {
        return ::std::string("empty");
    }
};

struct Size
{
    TesterPQ::size_type operator()(TesterPQ & q) const
    {
        return q.size();
    }

    ::std::string what() const
    {
        return ::std::string("size");
    }
};

struct Insert
{
    Tester const t1, t2;

    Insert(const Tester& _t1, const Tester& _t2) : t1(_t1), t2(_t2)
    {
    }

    void operator()(TesterPQ& q) const
    {
        q.insert(t1, t2);
    }

    ::std::string what() const
    {
        return ::std::string("insert");
    }
};

struct Find
{
    const Tester& (TesterPQ::*m)() const;
    Find(const Tester& (TesterPQ::*_m)() const) : m(_m)
    {
    }

    const Tester& operator()(TesterPQ& q) const
    {
        return (q.*m)();
    }
    ::std::string what() const
    {
        return ::std::string("[min|max]Value lub [min|max]Key");
    }
};

struct Delete
{
    void (TesterPQ::*m)();

    Delete(void (TesterPQ::*_m)()) : m(_m)
    {
    }

    void operator()(TesterPQ& q) const
    {
        (q.*m)();
    }

    ::std::string what() const
    {
        return ::std::string("delete");
    }
};

struct ChangeValue
{
    Tester const t1, t2;

    ChangeValue(const Tester& _t1, const Tester& _t2) : t1(_t1), t2(_t2)
    {
    }

    void operator()(TesterPQ& q) const
    {
        q.changeValue(t1, t2);
    }

    ::std::string what() const
    {
        return ::std::string("changeValue");
    }
};

struct Merge
{
    TesterPQ& q;

    Merge(TesterPQ& _q) : q(_q)
    {
    }

    void operator()(TesterPQ& q1) const
    {
        q1.merge(q);
    }

    ::std::string what() const
    {
        return ::std::string("merge");
    }
};

struct SwapMethod
{
    TesterPQ& q;

    SwapMethod(TesterPQ & _q) : q(_q)
    {
    }

    void operator()(TesterPQ& q1) const
    {
        q1.swap(q);
    }
    ::std::string what() const
    {
        return ::std::string("swap");
    }
};

struct Swap
{
    TesterPQ& q;

    Swap(TesterPQ& _q) : q(_q)
    {
    }

    void operator()(TesterPQ& q1) const
    {
        swap(q1, q);
    }

    ::std::string what() const
    {
        return ::std::string("swap");
    }
};

struct LT
{
    TesterPQ const q;

    LT(const TesterPQ& _q) : q(_q)
    {
    }

    bool const operator()(TesterPQ& q1) const
    {
        return q1 < q;
    }

    ::std::string what() const
    {
        return ::std::string("<");
    }
};

struct LE
{
    TesterPQ const q;

    LE(const TesterPQ& _q) : q(_q)
    {
    }

    bool const operator()(TesterPQ & q1) const
    {
        return q1 <= q;
    }

    ::std::string what() const
    {
        return ::std::string("<=");
    }
};

struct GT
{
    TesterPQ const q;

    GT(const TesterPQ& _q) : q(_q)
    {
    }

    bool const operator()(TesterPQ& q1) const
    {
        return q1 > q;
    }

    ::std::string what() const
    {
        return ::std::string(">");
    }
};

struct GE
{
    TesterPQ const q;

    GE(const TesterPQ& _q) : q(_q)
    {
    }

    bool const operator()(TesterPQ & q1) const
    {
        return q1 >= q;
    }

    ::std::string what() const
    {
        return ::std::string(">=");
    }
};

struct EQ
{
    TesterPQ const q;

    EQ(const TesterPQ& _q) : q(_q)
    {
    }

    bool const operator()(TesterPQ& q1) const
    {
        return q1 == q;
    }

    ::std::string what() const
    {
        return ::std::string("==");
    }
};

struct NE
{
    TesterPQ const q;

    NE(const TesterPQ& _q) : q(_q)
    {
    }

    bool const operator()(TesterPQ& q1) const
    {
        return q1 != q;
    }

    ::std::string what() const
    {
        return ::std::string("!=");
    }
};

void PriorityQueueEmptyExceptionCheck(const TesterPQ& q, const Tester& (TesterPQ::*m)() const)
{
    bool exception_occured = false;
    try
    {
        (q.*m)();
    }
    catch (PriorityQueueEmptyException& e)
    {
        exception_occured = true;
        ::std::string s = e.what();
    }
    catch (...)
    {
    }

    assert(exception_occured);
}

#endif


PriorityQueue<int, int> f(PriorityQueue<int, int> q)
{
    return q;
}

int main() {

// Test z treści zadania
#if TEST_NUM == 100
    PriorityQueue<int, int> P = f(PriorityQueue<int, int>());
    assert(P.empty());

    P.insert(1, 42);
    P.insert(2, 13);

    assert(P.size() == 2);
    assert(P.maxKey() == 1);
    assert(P.maxValue() == 42);
    assert(P.minKey() == 2);
    assert(P.minValue() == 13);

    PriorityQueue<int, int> Q(f(P));

    Q.deleteMax();
    Q.deleteMin();
    Q.deleteMin();

    assert(Q.empty());

    PriorityQueue<int, int> R(Q);

    R.insert(1, 100);
    R.insert(2, 100);
    R.insert(3, 300);

    PriorityQueue<int, int> S;
    S = R;

    bool exception_occured = false;
    try
    {
          S.changeValue(4, 400);
    }
    catch (const PriorityQueueNotFoundException& pqnfex)
    {
        exception_occured = true;
    }
    catch (...)
    {
        assert(false);
    }
    assert(exception_occured);

    S.changeValue(2, 200);
    exception_occured = false;
    try
    {
        while (true)
        {
            assert(S.minValue());
            S.deleteMin();
        }
    }
    catch (const PriorityQueueEmptyException& pqeex)
    {
        exception_occured = true;
    }
    catch (...)
    {
        assert(false);
    }
    assert(exception_occured);

    exception_occured = false;
    try
    {
        assert(S.minKey());
    }
    catch (const ::std::exception& ex)
    {
        exception_occured = true;
    }
    catch (...)
    {
        assert(false);
    }
    exception_occured = true;

    PriorityQueue<int, int> T;
    T.insert(1, 1);
    T.insert(2, 4);
    S.insert(3, 9);
    S.insert(4, 16);
    S.merge(T);
    assert(S.size() == 4);
    assert(S.minValue() == 1);
    assert(S.maxValue() == 16);
    assert(T.empty());

    S = R;
    swap(R, T);
    assert(T == S);
    assert(T != R);

    R = ::std::move(S);
    assert(T != S);
    assert(T == R);
#endif

// Testy poprawnościowe: sprawdza czy wszystkie funcje dają wyniki/efekty zgodne z treścią zadania
// Ten test nie sprawdza kwestii związanych z wyjątkami

// Konstruktor bezparametrowy, konstruktor kopiujący, konstruktor przenoszący, operator przypisania
#if TEST_NUM == 200
    PriorityQueue<int,int> q;

    PriorityQueue<int, int> r = PriorityQueue<int, int>(q);
    assert(r == q);

    PriorityQueue<int, int> s(f(q));
    assert(s == q);

    PriorityQueue<int, int> t;
    t = q;
    assert(t == q);

    r.insert(3,33);
    r.insert(4,44);
    r.insert(5,44);

    PriorityQueue<int, int> u = PriorityQueue<int, int>(r);
    assert(u == r);

    PriorityQueue<int, int> v = PriorityQueue<int, int>(f(r));
    assert(v == r);

    t = r;
    assert(t == r);

    t = f(r);
    assert(t == r);
#endif

// empty, size, insert, deleteMin, deleteMax
#if TEST_NUM == 201
    PriorityQueue<int, int> q;
    assert(q.empty());
    assert(q.size() == 0);

    q.deleteMin();
    assert(q.empty());
    assert(q.size() == 0);

    q.deleteMax();
    assert(q.empty());
    assert(q.size() == 0);

    q.deleteMin();
    assert(q.empty());
    assert(q.size() == 0);

    for (int i = 0; i < 10000; i++)
        q.insert(i, i);

    assert(q.size() == 10000);
    assert(!q.empty());

    q.deleteMin();
    assert(!q.empty());
    assert(q.size() == 9999);

    q.deleteMax();
    assert(!q.empty());
    assert(q.size() == 9998);

    for (int i = 9998; i > 0; i-= 2)
    {
        q.deleteMin();
        q.deleteMax();
    }

    assert(q.size() == 0);
    assert(q.empty());

    for (int i = 0; i < 10000; i++)
        q.insert(0, 0);

    assert(q.size() == 10000);
    assert(!q.empty());

    q.deleteMin();
    assert(!q.empty());
    assert(q.size() == 9999);

    q.deleteMax();
    assert(!q.empty());
    assert(q.size() == 9998);

    for (int i = 9998; i >= 0; i-= 2)
    {
        q.deleteMin();
        q.deleteMax();
    }
    assert(q.empty());
    assert(q.size() == 0);
#endif

// minValue, maxValue, minKey, maxKey, insert, deleteMin, deleteMax
#if TEST_NUM == 202
    PriorityQueue<int, int> q;

    for (int i = 0; i < 10000; i++)
    {
        q.insert(i,i);
        assert(q.minValue() == 0);
        assert(q.minKey() == 0);
        assert(q.maxValue() == i);
        assert(q.maxKey() == i);
    }

    for (int i = 0; i < 5000; i++)
    {
        q.deleteMin();
        assert(q.minValue() == i + 1);
        assert(q.minKey() == i + 1);
        assert(q.maxValue() == 9999);
        assert(q.maxKey() == 9999);
    }

    for (int i = 9999; i > 5000; i--)
    {
        q.deleteMax();
        assert(q.minValue() == 5000);
        assert(q.minKey() == 5000);
        assert(q.maxValue() == i - 1);
        assert(q.maxKey() == i - 1);
    }
#endif

// changeValue
#if TEST_NUM == 203
    PriorityQueue<int, int> q;

    q.insert(1,100);
    q.changeValue(1,101);
    assert(q.minValue() == 101 && q.minKey() == 1);

    q.insert(2,200);
    q.changeValue(2,201);
    assert(q.maxValue() == 201 && q.maxKey() == 2);

    q.changeValue(1,102);
    assert(q.minValue() == 102 && q.minKey() == 1 && q.maxValue() == 201 && q.maxKey() == 2);

    PriorityQueue<int, int> r;
    for (int i = 0; i < 10000; i++)
    {
        r.insert(1,100);
    }
    r.changeValue(1,101);
    assert(r.maxValue() == 101 && r.maxKey() == 1);
    r.changeValue(1,99);
    assert(r.minValue() == 99 && r.minKey() == 1);
    assert((r.maxValue() == 100 || r.maxValue() == 101) && r.maxKey() == 1);

    TesterPQ s;

    for (int i = 0; i < 1000; i++)
    {
        s.insert(Tester(i), Tester(i));
    }

    for (int i = 0; i < 1000; i++)
    {
        s.changeValue(Tester(i), Tester(-1));
    }
    assert(s.minValue() == Tester(-1) && s.maxValue() == Tester(-1));


#endif

// merge
#if TEST_NUM == 204
    PriorityQueue<int, int> q;
    PriorityQueue<int, int> r;

    q.merge(r);
    assert(q == r);

    r.merge(r);
    assert(q == r);

    PriorityQueue<int, int> s;
    for (int i = 0; i < 10000; i++)
    {
        s.insert(i,i);
    }
    PriorityQueue<int, int> ss = PriorityQueue<int, int>(s);
    q.merge(s);
    assert(q == ss && s.empty());

    PriorityQueue<int, int> t = PriorityQueue<int, int>(ss);
    t.merge(r);
    assert(t == ss);

    PriorityQueue<int, int> tt = PriorityQueue<int, int>(t);
    ss.merge(t);
    assert(t.empty() && ss.size() == 2 * tt.size());

    PriorityQueue<int, int> ttt = PriorityQueue<int, int>(tt);
    tt.merge(tt);
    assert(tt == ttt);

#endif

// swap (lokalny i globalny)
#if TEST_NUM == 205
    PriorityQueue<int, int> q;
    for (int i = 0; i < 10000; i++)
    {
        q.insert(i,i);
    }
    PriorityQueue<int, int> qq = PriorityQueue<int, int>(q);

    PriorityQueue<int, int> r;
    for (int i = 0; i > -10000; i--)
    {
        r.insert(-i,i);
    }
    PriorityQueue<int, int> rr = PriorityQueue<int, int>(r);

    q.swap(r);
    assert(q == rr && r == qq);

    r.swap(q);
    assert(q == qq && r == rr);

    swap(r,q);
    assert(q == rr && r == qq);

    swap(q,r);
    assert(q == qq && r == rr);

    swap(q,q);
    assert(q == qq);

    PriorityQueue<int, int> s,t;
    s.swap(t);
    assert(s.empty() && s == t);
#endif

// operatory porównania
#if TEST_NUM == 206
      PriorityQueue<int, int> o;

      PriorityQueue<int, int> p;
      for (int i = 0; i < 1000; i++)
      {
          p.insert(i,i);
      }

      PriorityQueue<int, int> q;
      for (int i = 0; i < 1000; i++)
      {
          q.insert(i,i);
      }

      PriorityQueue<int, int> r;
      for (int i = 0; i < 10000; i++)
      {
          r.insert(i,i);
      }

      PriorityQueue<int, int> s;
      s.insert(0,1);

      PriorityQueue<int, int> t;
      t.insert(1,0);

      assert(o == o);
      assert(q == q);
      assert(p == q);

      assert(o <= o);
      assert(q <= q);
      assert(p <= q);

      assert(o >= o);
      assert(q >= q);
      assert(p >= q);

      assert(o != q);
      assert(q != o);

      assert(o < q);
      assert(q < r);
      assert(r < s);
      assert(s < t);
      assert(r < t);

      assert(q > o);
      assert(r > q);
      assert(s > r);
      assert(t > s);
      assert(t > r);
#endif

// Testy gwarancji no-throw: empty, size, swap, operator=

// empty
#if TEST_NUM == 300
    TesterPQ q;

    bool empty = NoThrowCheck<TesterPQ, Empty, bool>(q, Empty());
    assert(empty);
#endif

// size
#if TEST_NUM == 301
    TesterPQ q;

    bool size = NoThrowCheck<TesterPQ, Size, bool>(q, Size());
    assert(size == 0);
#endif

// swap
#if TEST_NUM == 302
    TesterPQ q1, q2;

    for (int i = 1; i < 10000; i++)
    {
        q1.insert(Tester(i), Tester(i));
        q2.insert(Tester(-i), Tester(-i));
    }

    TesterPQ q1Copy(q1);
    TesterPQ q2Copy(q2);

    NoThrowCheckVoid(q1, SwapMethod(q2));
    assert(q1Copy == q2);
    assert(q2Copy == q1);

    NoThrowCheckVoid(q1, Swap(q2));
    assert(q1Copy == q1);
    assert(q2Copy == q2);
#endif

// operator=
#if TEST_NUM == 303
    TesterPQ q1, q2;

    NoThrowCheckVoid(q1, Assignment(q2));
#endif

// Testy gwarancji silnych

// insert
#if TEST_NUM == 400
    TesterPQ q;

    StrongCheckVoid(q, Insert(Tester(0), Tester(100)));
#endif

// minValue i maxValue, minKey i maxKey
#if TEST_NUM == 401
    TesterPQ q;

    q.insert(Tester(1), Tester(42));
    q.insert(Tester(2), Tester(13));

    StrongCheck<TesterPQ, Find, TesterPQ::value_type>(q, Find(&TesterPQ::minValue));
    StrongCheck<TesterPQ, Find, TesterPQ::value_type>(q, Find(&TesterPQ::maxValue));
    StrongCheck<TesterPQ, Find, TesterPQ::key_type>(q, Find(&TesterPQ::minKey));
    StrongCheck<TesterPQ, Find, TesterPQ::key_type>(q, Find(&TesterPQ::maxKey));
#endif

// deleteMin i deleteMax
#if TEST_NUM == 402
    TesterPQ q;

    q.insert(Tester(1), Tester(42));
    q.insert(Tester(2), Tester(13));

    StrongCheckVoid(q, Delete(&TesterPQ::deleteMin));
    StrongCheckVoid(q, Delete(&TesterPQ::deleteMax));
    StrongCheckVoid(q, Delete(&TesterPQ::deleteMin));
#endif

// changeValue
#if TEST_NUM == 403
    TesterPQ q;

    q.insert(Tester(1), Tester(100));
    q.insert(Tester(2), Tester(100));
    q.insert(Tester(3), Tester(300));

    StrongCheckVoid(q, ChangeValue(Tester(2), Tester(200)));
#endif

// merge
#if TEST_NUM == 404
    TesterPQ p, q;

    p.insert(Tester(1), Tester(1));
    p.insert(Tester(2), Tester(4));
    q.insert(Tester(3), Tester(9));
    q.insert(Tester(4), Tester(16));

    StrongCheckVoid(p, Merge(q));
#endif

// porównania
#if TEST_NUM == 405
    TesterPQ p;
    for (int i = 0; i < 1000; i++)
    {
        p.insert(Tester(i),Tester(i));
    }

    TesterPQ q;
    for (int i = 0; i < 1000; i++)
    {
        q.insert(Tester(i),Tester(i));
    }

    TesterPQ r;
    for (int i = 0; i < 10000; i++)
    {
        r.insert(Tester(i),Tester(i));
    }

    StrongCheckVoid<TesterPQ, LT>(p, LT(r));
    StrongCheckVoid<TesterPQ, LE>(p, LE(q));
    StrongCheckVoid<TesterPQ, LE>(q, LE(p));
    StrongCheckVoid<TesterPQ, GT>(r, GT(p));
    StrongCheckVoid<TesterPQ, GE>(p, GE(q));
    StrongCheckVoid<TesterPQ, GE>(q, GE(p));
    StrongCheckVoid<TesterPQ, EQ>(p, EQ(q));
    StrongCheckVoid<TesterPQ, NE>(q, NE(r));
#endif

// Czy rzucane są wyjątki zgodnie ze specyfikacją, czy dziedziczą po ::std::exception

// PriorityQueueEmptyException
#if TEST_NUM == 500
    TesterPQ q;

    PriorityQueueEmptyExceptionCheck(q, &TesterPQ::minValue);
    PriorityQueueEmptyExceptionCheck(q, &TesterPQ::maxValue);
    PriorityQueueEmptyExceptionCheck(q, &TesterPQ::minKey);
    PriorityQueueEmptyExceptionCheck(q, &TesterPQ::maxKey);
#endif

#if TEST_NUM == 501
    try
    {
        throw PriorityQueueEmptyException();
    }
    catch (::std::exception&)
    {
    }
    catch (...)
    {
        assert(false);
    }
#endif

// PriorityQueueNotFoundException
#if TEST_NUM == 502
    TesterPQ q;

    bool exception_occured = false;
    try
    {
        q.changeValue(Tester(0), Tester(0));
    }
    catch (PriorityQueueNotFoundException& e)
    {
        exception_occured = true;
        ::std::string s = e.what();
    }
    catch (...)
    {
    }

    assert(exception_occured);
#endif

#if TEST_NUM == 503
    try
    {
        throw PriorityQueueNotFoundException();
    }
    catch (::std::exception&)
    {
    }
    catch (...)
    {
        assert(false);
    }
#endif

// Czy zmiany kolejki wypływają na jej kopie

// changeValue
//#if TEST_NUM == 600
    PriorityQueue<int, int> p, q;

    for (int i = 1; i < 10000; i++)
    {
        q.insert(i, i);
        p.insert(i, i);
    }

    PriorityQueue<int, int> qCopy1(q);
    PriorityQueue<int, int> qCopy2;
    qCopy2 = q;

    q.changeValue(5,0);

    assert(qCopy1 == p);
    assert(qCopy1 != q);
    assert(qCopy2 == p);
    assert(qCopy2 != q);

//#endif

// deleteMin
#if TEST_NUM == 601
    PriorityQueue<int, int> p, q;

    for (int i = 1; i < 10000; i++)
    {
        q.insert(i, i);
        p.insert(i, i);
    }

    PriorityQueue<int, int> qCopy1(q);
    PriorityQueue<int, int> qCopy2;
    qCopy2 = q;

    q.deleteMin();

    assert(qCopy1 == p);
    assert(qCopy1 != q);
    assert(qCopy2 == p);
    assert(qCopy2 != q);
#endif

// deleteMax
#if TEST_NUM == 602
    PriorityQueue<int, int> p, q;

    for (int i = 1; i < 10000; i++)
    {
        q.insert(i, i);
        p.insert(i, i);
    }

    PriorityQueue<int, int> qCopy1(q);
    PriorityQueue<int, int> qCopy2;
    qCopy2 = q;

    q.deleteMax();

    assert(qCopy1 == p);
    assert(qCopy1 != q);
    assert(qCopy2 == p);
    assert(qCopy2 != q);
#endif

// Test pomocniczy: czy przechowywana jest co najwyżej jedna kopia każdego elementu?
#if TEST_NUM == 700
    gInstancesCounter = 0;

    TesterPQ q;

    for (int i = 0; i < 1000; i++)
    {
        q.insert(Tester(i), Tester(i));
    }

    for (int i = 0; i < 1000; i++)
    {
        q.changeValue(Tester(i), Tester(-1));
    }

    q.deleteMin();
    q.deleteMax();

    assert(gInstancesCounter == 1996);
#endif

}
