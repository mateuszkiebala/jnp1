#ifndef PIZZA_HH
#define PIZZA_HH

#include <cstddef>
#include <array>
#include <type_traits>

/* struktura reprezentująca pizzerię zdefiniowaną przez rodzaje dostępnych pizz */
template<typename... Kinds>
struct Pizzeria {
private:
    /* liczba kawałków w podstawowej pizzy (tworzonej przez make_pizza) */
    static const size_t BASIC_SIZE = 8;

    /* funkcja licząca, ile razy typ ToCount pojawia się w definicji pizzerii */
    template<typename ToCount>
    static constexpr size_t count_repetitions() {
        return ((std::is_same<ToCount, Kinds>::value ? 1 : 0) + ...);
    }

    /* deklaracja struktury znajdującej najlepszą mieszankę Pizzy1 i Pizzy2
       (typ znalezionej pizzy dostępny przez publiczny atrybut type) */
    template<typename Pizza1, typename Pizza2>
    struct find_best_mix;

public:
    /* struktura reprezentująca pizzę zdefiniowaną przez pizzerię, w której się
       znajduje, oraz przez liczby, które mówią, ile jest kawałków poszczególnych
       rodzajów w tej pizzy (dla wszystkich rodzajów dostępnych w tej pizzerii
       i zgodnie z kolejnością występowania tych rodzajów w definicji pizzerii) */
    template<uint64_t... slices>
    struct Pizza {
    private:
        /* typ pizzerii, do której należy ta pizza */
        using my_pizzeria = Pizzeria<Kinds...>;

        /* typ najlepszej mieszanki Pizzy1 i Pizzy2 */
        template<typename Pizza1, typename Pizza2>
        using best_mixed_pizza = typename find_best_mix<Pizza1, Pizza2>::type;

    public:
        /* typ pizzy, w której każdy kawałek został przekrojony na pół */
        using sliced_type = Pizza<2 * slices...>;

        /* funkcja licząca, ile jest w pizzy kawałków rodzaju TypeToCount */
        template<typename TypeToCount>
        static constexpr size_t count() {
            return ((std::is_same<TypeToCount, Kinds>::value ? slices : 0) + ...);
        }

        /* funkcja zwracająca tablicę liczb mówiących, ile jest kawałków każdego
           rodzaju (zgodnie z kolejnością rodzajów podanych w definicji pizzerii) */
        static constexpr std::array<size_t, sizeof...(slices)> as_array() {
            return {{ slices... }};
        }

        /* deklaracja zaprzyjaźnionej struktury przechowującej najlepszą mieszankę
           Pizzy1 i Pizzy2 */        
        template<typename Pizza1, typename Pizza2>
        friend struct best_mix;
    };

    /* Struktura tworząca pizzę rodzaju Kind (liczba kawałków = BASIC_SIZE) 
       (typ nowej pizzy dostępny przez publiczny atrybut type).
       Stworzenie pizzy nie uda się, jeśli przy definiowaniu pizzerii podano 
       rodzaj Kind więcej niż jeden raz bądź wcale go nie podano. */
    template<typename Kind>
    struct make_pizza {
        static_assert(count_repetitions<Kind>() == 1,
                      "No such type in menu or repetiion detected");

    public:
        using type = Pizza<(std::is_same<Kind, Kinds>::value ? BASIC_SIZE : 0)...>;
    };

private:
    /* Struktura znajdująca najlepszą mieszankę Pizzy1 i Pizzy2.
       Szukanie nie powiedzie się, jeśli co najmniej jeden z rodzajów podanych
       przy definiowaniu pizzerii nie implementuje poprawnie funkcji yumminess
       (funkcja ta powinna istnieć, przyjmować dokładnie jeden argument, 
       zwracać wynik typu arytmetycznego i zwracać 0 dla argumentu 0). */
    template<typename Pizza1, typename Pizza2>
    struct find_best_mix {
    private:
        /* struktura sprawdzająca, czy typ T implementuje funkcję yumminess
           z jednym argumentem (wynik dostępny przez publiczny atrybut value) */
        template<typename T>
        struct implements_yumminess {
        private:
            template<typename A, typename B = void>
            struct test : std::false_type {};

            /* wybrana specjalizacja, jeśli A::yumminess(0) jest poprawnym wyrażeniem */
            template<typename A>
            struct test<A, std::void_t<decltype(A::yumminess(0))>> : std::true_type {};

        public:
            static const bool value = test<T>::value;
        };

        /* funkcja licząca optymalną liczbę kawałków rodzaju TypeToCount 
           nie większą niż łączna liczba kawałków tego rodzaju w Pizzy1 i Pizzy2 */
        template<typename TypeToCount>
        static constexpr size_t optimal_number_of_slices() {
            static_assert(implements_yumminess<TypeToCount>::value,
                          "Pizza type has to implement yumminess method");

            using yum_type = decltype(TypeToCount::yumminess(size_t{}));
            static_assert(std::is_arithmetic<yum_type>::value,
                          "Yumminess's return value is of non-arithmetic type");
            static_assert(TypeToCount::yumminess(0) == 0,
                          "Yumminess(0) should be 0");

            size_t result = 0, N = Pizza1::template count<TypeToCount>() +
                                   Pizza2::template count<TypeToCount>();
            yum_type best = 0, temp = 0;
            for (size_t slices = 1; slices <= N; ++slices) 
                if ((temp = TypeToCount::yumminess(slices)) > best) 
                    best = temp, result = slices;

            return result;
        }

    public:
        /* typ pizzy złożonej z optymalnej liczby kawałków każdego rodzaju */
        using type = Pizza<optimal_number_of_slices<Kinds>()...>;
    };
};

/* Struktura przechowująca najlepszą mieszankę Pizzy1 i Pizzy2.
   Błędem jest próba mieszania pizz z różnych pizzerii. */
template<typename Pizza1, typename Pizza2>
struct best_mix {
    static_assert(std::is_same<typename Pizza1::my_pizzeria, 
                               typename Pizza2::my_pizzeria>::value,
                  "Can't mix pizzas from different pizzerias");

public:
    using type = typename Pizza1::template best_mixed_pizza<Pizza1, Pizza2>;
};

#endif /* PIZZA_HH */
