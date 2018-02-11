// przykład z treści

#include <iostream>
#include <exception>
#include <cassert>

#include "priorityqueue.hh"

PriorityQueue<int, int> f(PriorityQueue<int, int> q)
{
    return q;
}

int main() {

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

/*
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

*/
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
        std::cout << "pqeex" << pqeex.what() << std::endl;
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
        std::cout << "ex" << ex.what() << std::endl;
    }
    catch (...)
    {
        assert(!"exception missing!");
    }

    PriorityQueue<int, int> T;
    T.insert(1, 1);
    T.insert(2, 4);
    std::cout << "T " <<  T.size() << std::endl;
    S.insert(3, 9);
    S.insert(4, 16);
    std::cout << "S " <<  S.size() << std::endl;
    S.merge(T);
    assert(S.size() == 4);
    assert(S.minValue() == 1);
    assert(S.maxValue() == 16);
    assert(T.empty());

    S = R;
    swap(R, T);

/*
    assert(T == S);
    assert(T != R);
*/

    R = std::move(S);
/*
    assert(T != S);
    assert(T == R);
*/

    std::cout << "ALL OK!" << std::endl;


struct comp_op_ex : std::exception {
    const char* what() const noexcept {return "comp_op_ex\n";}
};

struct assign_op_ex : std::exception {
    const char* what() const noexcept {return "assign_op_ex\n";}
};

class strange_cl {
    private:
        int value;

		int _c_lt;
		int _c_gt;

    public:
		strange_cl() : value(0), _c_lt(0), _c_gt(0) { };
		strange_cl(int x) : value(x), _c_lt(0), _c_gt(0) { };

        bool operator<(const strange_cl& x) {
			_c_lt++;
			std::cerr << "? <(" << value << ", " << x.value << "), times: " << _c_lt << std::endl;
            if (value == 7 && x.value == 4)
                throw comp_op_ex();
            return value < x.value;
        }
};

	PriorityQueue<strange_cl, int> pq1;

	pq1.insert(strange_cl(1), 2);
	pq1.insert(strange_cl(3), 4);
	pq1.insert(strange_cl(2), 7);
	pq1.insert(strange_cl(9), 1);
	pq1.insert(strange_cl(7), 1);
	pq1.insert(strange_cl(11), 22);


    return 0;
}
