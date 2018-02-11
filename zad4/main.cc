#include "pizza.hh"

#include <limits>
#include <iostream>
#include <algorithm>
#include <cassert>
using namespace std;

struct supreme {
    static constexpr int yumminess(size_t slices) {
        return -125LL*(slices - 6)*slices*slices/243;
    }
};

struct napoli {
    static constexpr int yumminess(size_t slices) {
        return slices;
    }
};

struct capriciosa {
    static constexpr int yumminess(size_t slices) {
        return slices * 2;
    }
};

int main() {
    
    using Dominion = Pizzeria<napoli, supreme, capriciosa>;
    using Supreme = Dominion::make_pizza<supreme>::type;
    static_assert(Supreme::count<supreme>() == 8, "LOL");
    static_assert(Supreme::count<napoli>() == 0, "Co tu robi napoli??");
    static_assert(Supreme::count<capriciosa>() == 0, "Co tu robi capriciosa??");
    
    using SuperSupreme = Supreme::sliced_type;
    static_assert(SuperSupreme::count<supreme>() == 16,
            "SuperSupreme does not look supreme!");
    
    assert((Supreme::as_array() == std::array<size_t, 3>{{0, 8, 0}}));
    
    using Capriciosa = Dominion::make_pizza<capriciosa>::type;
    static_assert(Capriciosa::count<capriciosa>() == 8, "");
    
    using SuperCapriciosa = Capriciosa::sliced_type;
    static_assert(SuperCapriciosa::count<capriciosa>() == 16,
            "SuperCapriciosa does not look capriciosa!");
    
    assert((SuperCapriciosa::as_array() == std::array<size_t, 3>{{0, 0, 16}}));
    
    static_assert(SuperSupreme::count<napoli>() == 0,
            "SuperSupreme does not look napoli!");
    
    using SuperExtraSupreme = SuperSupreme::sliced_type;
    static_assert(SuperExtraSupreme::count<supreme>() == 32,
            "SuperExtraSupreme is not supreme enough!");

    assert((SuperExtraSupreme::as_array() == std::array<size_t, 3>{{0, 32, 0}}));
   
    using Napoli = Dominion::make_pizza<napoli>::type;
    static_assert(Napoli::count<napoli>() == 8,
            "Where's my napoli!?");
    static_assert(Napoli::count<supreme>() == 0,
            "Supreme!?");
    
    assert((Napoli::as_array() == std::array<size_t, 3>{{8, 0, 0}}));
    using AnotherSupreme = Dominion::make_pizza<supreme>::type;
    static_assert(std::is_same<Supreme, AnotherSupreme>::value,
            "Pizzas don't have names, so how come can you say they're different!?");
    static_assert(!std::is_same<Supreme, SuperSupreme>::value,
            "Sure, no one can see the difference between Supreme and SuperSupreme!");
    
return 0;
}
