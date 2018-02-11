#include <iostream>
#include <exception>
#include <cassert>

#include "priorityqueue.hh"

PriorityQueue<int, int> f(PriorityQueue<int, int> q)
{
    return q;
}

/* Wkleić do priorityqueue.hh
 * void wypisz() {
    std::cout << "SIZE: " << queue_size << std::endl;
    for (auto it = keys.begin(); it != keys.end(); ++it) {
      std::cout << "KLUCZ " << *(it->first) << std::endl;
      for (auto ptr = it->second->begin(); ptr != it->second->end(); ++ptr)
        std::cout << **ptr << std::endl;
    }
    std::cout << "--------------------" << std::endl;
  }
*/

void test1() {
    PriorityQueue<int, int> P = f(PriorityQueue<int, int>());
    PriorityQueue<int, int> Q;
    P.insert(1, 42);
    P.insert(2, 13);
    P.insert(1, 42);
    Q = P;
    assert(P == Q);
    Q.insert(2, 14);
    Q.insert(3, 16);
    P.merge(Q);
    assert(!(P == Q));
    assert(P.size() == 8); 
    P.changeValue(2, 50);
    P.changeValue(3, 49);
    assert(P.maxValue() == 50);
    assert(P.size() == 8);

    Q.insert(2, 18);
    Q.insert(9, 20);
    assert(Q != P);
    swap(P, Q);

    PriorityQueue<int, int> P1 = f(PriorityQueue<int, int>());
    PriorityQueue<int, int> Q1 = f(PriorityQueue<int, int>());
    PriorityQueue<int, int> S1 = f(PriorityQueue<int, int>());
    PriorityQueue<int, int> G1 = f(PriorityQueue<int, int>());
    PriorityQueue<int, int> W = PriorityQueue<int, int>();

    P1.insert(1, 42);
    P1.insert(2, 13);
    Q1.insert(1, 41);
    Q1.insert(3, 12);
    S1.insert(1, 42);

    assert(Q1 < S1);
    assert(S1 < P1);
    assert(Q1 < P1);

    G1 = P1;
    assert(G1 <= P1);
    assert(G1 >= P1);
    assert(G1 == P1);
    assert(G1 != S1);
    assert(Q1 != G1);
    Q.deleteMax();
    Q.deleteMax();
    Q.deleteMin();
    
    W = P1;
    W.insert(1, 42);
    assert(W < P1);
    W = PriorityQueue<int, int>();
    W.insert(2, 12);
    assert(W > P1);
    W.insert(1, 42);
    assert(W < P1);
    W.deleteMin();
    assert(W.minKey() == 1);
    assert(W.minValue() == 42);
    assert(W < P1);
    W.deleteMin();
    assert(W.empty());
    W.deleteMin();
    W.deleteMax();
    W.insert(1, 43);
    assert(W > P1);
    
    W = PriorityQueue<int, int>();
    PriorityQueue<int, int> X = PriorityQueue<int, int>();
    assert(W == X);
    W.insert(1, 42);
    W.insert(1, 42);
    assert(X < W);
    W.insert(2, 13);
    X.insert(1, 42);
    assert(X < W);
    assert(!(W < X));
    X.insert(1, 42);
    assert(X < W);
    assert(!(W < X));
    X.insert(2, 13);
    assert(X == W);
    assert(X <= W);
    assert(X >= W);
    
    X.changeValue(1, 43);
    assert(X > W);
    
    try {
      X.changeValue(4, 70);
    } catch (PriorityQueueNotFoundException e) {
      std::cout << e.what();
    }
    
    X.merge(W);
    assert(W.empty());
    assert(W < X);
    try {
      W.maxKey();
    } catch (PriorityQueueEmptyException e) {
      std::cout << e.what();
    }
    
    X = PriorityQueue<int, int>();
    X.insert(5, 20);
    X.insert(7, 18);
    X.insert(9, 19);
    W.insert(5, 20);
    W.insert(7, 18);
    assert(W < X);
    X.merge(W);
    assert(X.maxValue() == 20);
    assert(X.size() == 5);
    
    X = PriorityQueue<int, int>();
    W = PriorityQueue<int, int>();
    X.insert(5, 19);
    X.insert(5, 20);
    W.insert(5, 19);
    W.insert(5, 20);
    X.insert(6, 80);
    W.insert(6, 80);
    X.insert(1, 1);
    W.insert(2, 0);
    assert(X < W);
    assert(!(W < X));
    X.deleteMin();
    X.insert(2, 1);
    assert(W < X);
    assert(!(X < W));
    std::cout << "Test1 OK!!!\n";
}

int main() {
    test1();
   // std::cout << "Tworze P\n";
    PriorityQueue<int, int> P = f(PriorityQueue<int, int>());

    P.insert(1, 42);
    P.insert(2, 13);

    assert(P.size() == 2);
    assert(P.maxKey() == 1);
    assert(P.maxValue() == 42);
    assert(P.minKey() == 2);
    assert(P.minValue() == 13);

   // std::cout << "Tworze Q\n";
    PriorityQueue<int, int> Q(f(P));
    Q.deleteMax();
    Q.deleteMin();
    Q.deleteMin();

    assert(Q.empty());

   // std::cout << "Tworze R\n";
    PriorityQueue<int, int> R(Q);

    R.insert(1, 100);
    R.insert(2, 100);
    R.insert(3, 300);

  //  std::cout << "Tworze S\n";
    PriorityQueue<int, int> S;
    S = R;

    try
    {
        S.changeValue(4, 400);
    }
    catch (const PriorityQueueNotFoundException& pqnfex)
    {
        std::cout << pqnfex.what() << std::endl;
    }
    catch (...)
    {
        assert(!"exception missing!");
    }

    S.changeValue(2, 200);

    try
    {
        while (true)
        {
            std::cout << S.minValue() << std::endl;
            S.deleteMin();
        }
        assert(!"S.minValue() on empty S did not throw!");
    }
    catch (const PriorityQueueEmptyException& pqeex)
    {
        std::cout << pqeex.what() << std::endl;
    }
    catch (...)
    {
        assert(!"exception missing!");
    }

    try
    {
        S.minKey();
        assert(!"S.minKey() on empty S did not throw!");
    }
    catch (const std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
    }
    catch (...)
    {
        assert(!"exception missing!");
    }

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
    //S.wypisz();
    //T.wypisz();
    //R.wypisz();
    swap(R, T);
    assert(T.size() == 3);
    assert(R.size() == 0);
    assert(T == S);
    //S.wypisz();
    //T.wypisz();
    //R.wypisz();
    assert(T != R);

    R = std::move(S);
    //R.wypisz();
    //S.wypisz();
    assert(T != S);
    assert(T == R);

    std::cout << "ALL OK!" << std::endl;

    return 0;
}