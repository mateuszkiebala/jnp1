#include <iostream>
#include <exception>
#include <cassert>

#include "priorityqueue.hh"

using namespace std;

struct A {
    static int& getCounter() {
        static int counter = 0;
        return counter;
    }
    A() {
        cout << "Constructing" << endl;
        getCounter()++;
    }
    A(const A& a) {
        cout << "Copying" << endl;
        getCounter()++;
    }
    ~A() {
        cout << "Destructing" << endl;
        getCounter()--;
    }
};

bool operator<(const struct A& a, const struct A& b) {
    return false;
}

int main() {
    PriorityQueue<A, int> P;

    {
        auto a = A();
        assert(a.getCounter() == 1);
        P.insert(a, 5);
        P.insert(a, 5);
        P.insert(a, 5);
        P.insert(a, 5);
        P.insert(a, 5);
        std::cout << a.getCounter() << std::endl;
        assert(a.getCounter() == 2);
    }

    assert(A::getCounter() == 1);

    return 0;
}
