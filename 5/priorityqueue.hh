#ifndef PRIORITY_QUEUE_HH
#define PRIORITY_QUEUE_HH

/*
 * Plik nagłówkowy klasy PriorityQueue
 * Autorzy: Mateusz Kiebała, Łukasz Kowalewski
 * 
 * Aktualizacja 7 I 2016
 * Kolejka jako dwa symetryczne zbiory (sety): 
 *   zbiór par (values): wartość (niepowtarzalne) i przypisany jej zbior 
 *                       (multiset - powtarzalny) kluczy
 *   zbiór par (keys): klucz (niepowtarzalne) i przypisany jej zbior 
 *                       (multiset - powtarzalny) wartości
 * Wszystko jako shared_ptr.
 * Operacje erase na obiektach set i multiset uważamy za dające gwarancję
 * nothrow w przypadku podawania jako argument iteratora na usuwany element.
 * Silną gwarancję metod insert i changeValue zapewniamy poprzez usuwanie
 * elementów uprzednio wstawionych do zbiorów podając iterator (wracanie z
 * kolejką do stanu sprzed wywołania metody w przypadku wystąpienia wyjątku).
 *
 */

#include <set>
#include <vector>
#include <utility>
#include <iostream>
#include <memory>

struct PriorityQueueEmptyException: public std::exception {
  const char* what() const noexcept {
    return "PriorityQueueEmptyException\n";
  }
};

struct PriorityQueueNotFoundException: public std::exception {
  const char* what() const noexcept {
    return "PriorityQueueNotFoundException\n";
  }
};

template <typename K, typename V>
class PriorityQueue {
private:
  /* Wskaźnik na element */
  template<typename T>
  using el_ptr = std::shared_ptr<T>;

  /* Wskaźnik na funkcję porównującą dwa wskaźniki */
  template<typename T>
  using cmp_ele_ptr = bool(*)(const el_ptr<T>& p1, const el_ptr<T>& p2);

  /* Element zbioru - para <wskaźnik na K/V, zbiór wskaźnikow na V/K> */
  template<typename T1, typename T2>
  using elem = std::pair<el_ptr<T1>,
                    std::shared_ptr<std::multiset<el_ptr<T2>, cmp_ele_ptr<T2>>>>;

  /* Zbiór elementów (kluczy lub wartości) wraz z funkcją porównującą */
  template<typename T1, typename T2>
  using queue_set = std::set<elem<T1, T2>, bool(*)(const elem<T1, T2>& el1,
                                                   const elem<T1, T2>& el2)>;

  /* Funkcja porównująca elementy na postawie wartości pierwszego elementu pary */
  template<typename T1, typename T2>
  static bool cmp_pairs(const elem<T1, T2>& el1, const elem<T1, T2>& el2) {
    return *(el1.first) < *(el2.first);
  }

  /* Funkcja porównująca pojedyncze elementy zbioru wskaźników na
   * podstawie wartości przechowywanej przez wskaźnik */
  template<typename T>
  static bool cmp_ele(const el_ptr<T>& p1, const el_ptr<T>& p2) {
    return *(p1) < *(p2);
  }

  /* Ilość elementów w kolejce */
  size_t queue_size;

  /* Zbiór kluczy, pod jednym kluczem może być wiele wartości */
  queue_set<K, V> keys;

  /* Zbiór wartości, każda wartość może mieć wiele kluczy */
  queue_set<V, K> values;

  /* Pomocnicza metoda - tworzy nowy pusty multiset i zwraca shared_ptr */
  template<typename T>
  std::shared_ptr<std::multiset<el_ptr<T>, cmp_ele_ptr<T>>> empty_mset() {
    return std::make_shared<std::multiset<el_ptr<T>,
                                          cmp_ele_ptr<T>>>(cmp_ele<T>);
  }

  /* Pomocnicza metoda, tworzy nowy element (elem<T1, T2>),
   * a następnie wrzuca ten element do seta (kluczy lub wartości).
   * Jeżeli taki klucz/wartość już tam jest, to zwraca wskaźnik na niego,
   * a jeżeli nie ma, to wrzuca go do seta i zwraca wskaźnik */
  template<typename T1, typename T2>
  typename queue_set<T1, T2>::iterator insert_vk(const T1& v_or_k,
                                                 queue_set<T1, T2>& queue) {
    elem<T1, T2> new_element;
    /* Tworzy nowy obiekt T1 na podstawie kopii v_or_k i podaje shred_ptr 
     * do niego. T1 musi mieć konstruktor kopiujący. */
    new_element.first = std::make_shared<T1>(v_or_k);
    new_element.second = empty_mset<T2>();
    /* Silna gwarancja na insert z STL'a */
    return queue.insert(new_element).first;
  }

  /* Pomocnicza metoda - usuwa informacje o wartości value w zbiorze wartości
   * klucza key (w zbiorze keys). Zakładamy że w kluczach istnieje klucz key */
  void delete_value(const el_ptr<V>& value, const el_ptr<K>& key) {
    elem<K, V> element;
    element.first = key;
    element.second = empty_mset<V>();
    try {
      /* Poszukiwanie w kluczach klucza key, silna gwarancja z STL'a */
      auto key_iter = keys.find(element);
      /* Poszukiwanie dokladnie tego wskaźnika w multisecie */
      auto val_iter = key_iter->second->find(value);
      key_iter->second->erase(val_iter);
      /* Usuwanie całego rekordu z keys jeśli pusty */
      if (key_iter->second->empty())
        keys.erase(key_iter);
      --queue_size;
    } catch (...) {
      throw;
    }
  }

  /* Pomocnicza metoda - usuwa z kolejek pierwszy klucz w zbiorze kluczy o
   * wadze, na ktorą wskazuje iterator deleted_val (w values) */
  void delete_in_values(typename queue_set<V, K>::iterator& deleted_val) {
    auto deleted_key = deleted_val->second->begin();
    delete_value(deleted_val->first, *deleted_key);
    /* Usuwanie z values - usuwanie przez iterator nie powinno rzucać
     * wyjątku. */
    deleted_val->second->erase(deleted_key);
    /* Usuwanie całego rekordu z values jeśli pusty */
    if (deleted_val->second->empty())
      values.erase(deleted_val);
  }

  /* Szablon metody kopiującej elem (kopiuje zawartość wskazywaną przez
   * drugi element pary elem */
  template<typename T1, typename T2>
  void copy_elem(elem<T1, T2>& result, const elem<T1, T2>& from) {
    /* Kopiowanie zawartości wskazywanej przez first */
    result.first = from.first;
    /* Tworzenie nowego pustego multisetu */
    result.second = empty_mset<T2>();
    /* Kopiowanie zawartości multisetu */
    *(result.second) = *(from.second);
  }

public:
  using size_type = size_t;                     /* Typ rozmiaru kolejki */
  using key_type = K;                           /* Typ klucza */
  using value_type = V;                         /* Typ wartosci */

  /* Pusty konstruktor */
  PriorityQueue() : queue_size(0), keys(cmp_pairs<K, V>), values(cmp_pairs<V, K>) {};

  /* Konstruktor kopiujący [O(queue.size())]
   * Silną gwarancję zapewnia kopiowanie najpierw do obiektu
   * tymczasowego, a następnie swap(tmp, *this) */
  PriorityQueue(const PriorityQueue<K, V>& queue) :
                queue_size(queue.queue_size),
                keys(cmp_pairs<K, V>),
                values(cmp_pairs<V, K>) {
    auto val_it = queue.values.begin();
    auto key_it = queue.keys.begin();
    auto val_end = queue.values.end();
    auto key_end = queue.keys.end();
    auto tmp = PriorityQueue<K, V>();
    tmp.queue_size = queue.queue_size;
    /* Wstawianie po kolei na koniec zbioru jest w czasie
     * zamortyzowanym stałym (cplusplus.com/reference) */
    for (; val_it != val_end; ++val_it) {
      elem<V, K> new_elem;
      copy_elem<V, K>(new_elem, *val_it);
      tmp.values.insert(tmp.values.end(), new_elem);
    }

    for (; key_it != key_end; ++key_it) {
      elem<K, V> new_elem;
      copy_elem<K, V>(new_elem, *key_it);
      tmp.keys.insert(tmp.keys.end(), new_elem);
    }
    swap(tmp);
  }

  /* Konstruktor przenoszący [O(1)] */
  PriorityQueue(PriorityQueue<K, V>&& queue) : PriorityQueue<K, V>() { 
    swap(queue); 
  }

  /* Zwykly operator przypisania przyjmujący wartość
   * [O(1) jeśli queue jest przenoszone, O(size()) jesli kopiowane] */
  PriorityQueue<K, V>& operator=(PriorityQueue<K, V> queue) {
    swap(queue);
    return *this;
  }

  /* Metoda zwracająca true, gdy kolejka jest pusta [O(1)] */
  bool empty() const {
    return queue_size == 0;
  }

  /* Metoda zwracająca liczbę par (klucz, wartość) 
   * przechowywanych w kolejce [O(1)] */
  size_type size() const {
    return queue_size;
  }

  /* Metoda wstawiająca do kolejki parę o kluczu key i wartości value
   * [O(log size())] (dopuszczamy możliwość występowania w kolejce wielu
   * par o tym samym kluczu). */
  void insert(const K& key, const V& value) {
    try {
      ++queue_size;
      /* Nowe wskaźniki które bedą wstawiane do kolejki */
      std::shared_ptr<K> new_key_ptr;
      std::shared_ptr<V> new_val_ptr;
      /* Probuje wstawić do keys nowy klucz i nową wartość do values. */
      auto key_result = insert_vk<K, V>(key, keys);
      try {
        auto val_result = insert_vk<V, K>(value, values);
        new_key_ptr = key_result->first;
        new_val_ptr = val_result->first;
        /* Dodawanie do zbioru kluczy o wadze value wskaźnika na klucz key
         * i odwrotnie. */
        try {
          auto new_key_in_val = val_result->second->insert(new_key_ptr);
          try {
            key_result->second->insert(new_val_ptr);
          } catch (...) {
            /* Wstawianie do multizbioru w values się powiodlo */
            val_result->second->erase(new_key_in_val);
            throw;
          }
        } catch (...) {
          /* Wstawianie do values się powiodło */
          if (val_result->second->empty())
            /* Do values wstawiono nowy pusty element */
            values.erase(val_result);
          throw;
        }
      } catch (...) {
        /* Wstawienie do keys się powiodło */
        if (key_result->second->empty())
          /* Do keys wstawiono nowy pusty element */
          keys.erase(key_result);
        throw;
      }
    } catch (...) {
      --queue_size;
      throw;
    }
  }

  /* Zwraca najmniejszą wartość przechowywaną w kolejce [O(1)] */
  const V& minValue() const {
    if (!empty())
      return *(values.begin()->first);
    else
      throw PriorityQueueEmptyException();
  }

  /* Zwraca największą wartość przechowywaną w kolejce [O(1)] */
  const V& maxValue() const {
    if (!empty())
      return *(values.rbegin()->first);
    else
      throw PriorityQueueEmptyException();
  }

  /* Metoda zwracająca klucz o przypisanej najmniejszej wartości [O(1)] */
  const K& minKey() const {
    if (!empty())
      return **(values.begin()->second->begin());
    else
      throw PriorityQueueEmptyException();
  }

  /* Metoda zwracająca klucz o przypisanej największej wartości [O(1)] */
  const K& maxKey() const {
    if (!empty())
      return **(values.rbegin()->second->begin());
    else
      throw PriorityQueueEmptyException();
  }

  /* Metoda usuwająca z kolejki jedną parę o najmniejszej 
   * wartości [O(log size())] */
  void deleteMin() {
    if (!empty()) {
      auto deleted_val = values.begin();
      delete_in_values(deleted_val);
    }
  }

  /* Metoda usuwająca z kolejki jedną parę o największej 
   * wartości [O(log size())] */
  void deleteMax() {
    if (!empty()) {
      auto deleted_val = values.end();
      --deleted_val;
      delete_in_values(deleted_val);
    }
  }

  /* Metoda zmieniająca dotychczasową wartość przypisaną kluczowi key na nową
   * wartość value [O(log size())]; w przypadku gdy w kolejce nie ma pary
   * o kluczu key, powinien zostać zgłoszony wyjątek
   * PriorityQueueNotFoundException(); w przypadku kiedy w kolejce jest kilka par
   * o kluczu key, zmienia wartość w dowolnie wybranej parze o podanym kluczu */
  void changeValue(const K& key, const V& value) {
    elem<K, V> element_key;
    element_key.first = std::make_shared<K>(key);
    element_key.second = empty_mset<V>();
    /* Wyszukujemy wskaźnik na klucz w keys */
    auto keys_it = keys.find(element_key);
    if (keys_it == keys.end()) {
      throw PriorityQueueNotFoundException();
    } else {
      /* Wskaźnik na pierwszą wartość spod klucza */
      auto value_ptr = *(keys_it->second->begin());

      /* Istniejący element w values, z którego zostanie usunięty klucz */
      elem<V, K> element_val;
      element_val.first = value_ptr;
      element_val.second = empty_mset<K>();
      /* (nowy) element w values, do którego zostanie wstawiony klucz */
      elem<V, K> new_element_val;
      new_element_val.first = std::make_shared<V>(value);
      new_element_val.second = empty_mset<K>();

      /* Wyszukiwanie (lub wstawianie) nowego elementu w values. W razie 
       * rzucenia wyjątku nic się nie zmieni. */
      auto new_values_it = values.insert(new_element_val);
      try {
        /* Wyszukujemy iterator na znalezioną wcześniej wartość w values */
        auto values_it = values.find(element_val);
        /* Wyszukujemy klucz w zbiorze kluczy pod daną wartością */
        auto old_key_it = values_it->second->find(element_key.first);

        /* Wstawiamy klucz do zbioru kluczy nowej wartości */
        auto new_key_in_val = new_values_it.first->second->insert(keys_it->first);
        try {
          /* Wstawianie nowej wartości do zbioru klucza - może być rzucony wyjątek */
          auto inserted = keys_it->second->insert(new_values_it.first->first);
          try {
            /* Usuwamy wartość pod danym kluczem - może być rzucony wyjątek */
            keys_it->second->erase(keys_it->second->find(value_ptr));

            /* Jeżeli wszystko było ok, to usuwamy klucz pod daną wartością */
            /* Usuwanie przez iterator nie powinno rzucać wyjątku */
            values_it->second->erase(old_key_it);
            if (values_it->second->empty())
              values.erase(values_it);
          } catch (...) {
            /* Usuwanie wcześniej wstawionego elementu */
            keys_it->second->erase(inserted);
            throw;
          }
        } catch (...) {
          /* Usuwamy wcześniej wstawiony klucz do nowego zbioru w values */
          new_values_it.first->second->erase(new_key_in_val);
          throw;
        }
      } catch (...) {
        /* Jeżeli wcześniej wstawiliśmy nową wartość do values to usuwamy */
        if (new_values_it.second)
          values.erase(new_values_it.first);
        throw;
      }
    }
  }

  /* Metoda scalająca zawartość kolejki z podaną kolejką queue; ta operacja 
   * usuwa wszystkie elementy z kolejki queue i wstawia je do kolejki *this
   * [O(size() + queue.size() * log (queue.size() + size()))] */
  void merge(PriorityQueue<K, V>& queue) {
    /* Nie można mergować z samym sobą */
    if (this != &queue) {
      auto tmp = *this;
      auto values_it = queue.values.begin();
      auto values_end = queue.values.end();
      for (; values_it != values_end; ++values_it) {
        auto value_ptr = values_it->first;
        auto keys_it = values_it->second->begin();
        auto keys_end = values_it->second->end();
        for (; keys_it != keys_end; ++keys_it)
          tmp.insert(**keys_it, *value_ptr);
      }
      /* Gwarancja nothrow z STL'a */
      queue.keys.clear();
      queue.values.clear();
      queue.queue_size = 0;
      swap(tmp);
    }
  }

  /* Metoda zamieniającą zawartość kolejki z podaną kolejką queue [O(1)] */
  void swap(PriorityQueue<K, V>& queue) noexcept {
    std::swap(keys, queue.keys);
    std::swap(values, queue.values);
    std::swap(queue_size, queue.queue_size);
  }

  template<typename K2, typename V2>
  friend bool operator==(const PriorityQueue<K2, V2>& q1, 
                         const PriorityQueue<K2, V2>& q2);

  template<typename K2, typename V2>
  friend bool operator<(const PriorityQueue<K2, V2>& q1, 
                        const PriorityQueue<K2, V2>& q2);
};

/* Metoda zamieniająca zawartości podanych kolejek [O(1)] */
template<typename K, typename V>
void swap(PriorityQueue<K, V>& q1, PriorityQueue<K, V>& q2) {
  q1.swap(q2);
}

/* [O(size())] */
template<typename K, typename V>
bool operator==(const PriorityQueue<K, V>& q1, const PriorityQueue<K, V>& q2) {
  if (q1.size() != q2.size())
    return false;
  auto q1_keys_it = q1.keys.begin();
  auto q2_keys_it = q2.keys.begin();
  while (q1_keys_it != q1.keys.end() && q2_keys_it != q2.keys.end()) {
    /* Sprawdzamy czy klucze są takie same i czy ich zbiory wartości
     * są identyczne */
    if (!(*(q1_keys_it->first) == *(q2_keys_it->first))) {
      return false;
    } else {
      if (q1_keys_it->second->size() == q2_keys_it->second->size()) {
        auto f_val_it = q1_keys_it->second->begin();
        auto s_val_it = q2_keys_it->second->begin();
        auto f_val_end = q1_keys_it->second->end();
        auto s_val_end = q2_keys_it->second->end();
        while (f_val_it != f_val_end && s_val_it != s_val_end) {
          if (!(**f_val_it == **s_val_it))
            return false;
          ++f_val_it;
          ++s_val_it;
        }
        return true;
      } else {
        return false;
      }
    }
    ++q1_keys_it;
    ++q2_keys_it;
  }
  return true;
}

/* [O(size())] */
template<typename K, typename V>
bool operator!=(const PriorityQueue<K, V>& q1, const PriorityQueue<K, V>& q2) {
  return !(q1 == q2);
}

/* Porównywanie najpierw klucz, potem jego wartości [O(size())] */
template<typename K, typename V>
bool operator<(const PriorityQueue<K, V>& q1, const PriorityQueue<K, V>& q2) {
  auto q1_keys_it = q1.keys.begin();
  auto q2_keys_it = q2.keys.begin();
  auto q1_keys_end = q1.keys.end();
  auto q2_keys_end = q2.keys.end();
  /* Przechodzimy po zbiorze keys w q1 i q2 */
  while (q1_keys_it != q1_keys_end && q2_keys_it != q2_keys_end) {
    if (*(q1_keys_it->first) < *(q2_keys_it->first))
      return true;
    else if (*(q2_keys_it->first) < *(q1_keys_it->first))
      return false;
    else {
      /* Jeżeli napotkamy takie same wartości kluczy, to porównujemy 
       * ich multisety wartości */
      auto q1_val_it = q1_keys_it->second;
      auto q2_val_it = q2_keys_it->second;
      auto first1 = q1_val_it->begin();
      auto last1 = q1_val_it->end();
      auto first2 = q2_val_it->begin();
      auto last2 = q2_val_it->end();
      /* Porównywanie leksykograficzne dwóch zbiorów wartości */
      while (first1 != last1) {
        if (first2 == last2) {
          if (++q2_keys_it == q2_keys_end)
            return false;
          else
            return true;
        }
        if (PriorityQueue<K, V>::cmp_ele(*first2, *first1))
          return false;
        else if (PriorityQueue<K, V>::cmp_ele(*first1, *first2))
          return true;
        ++first1; 
        ++first2;
      }
      if (q1_val_it->size() != q2_val_it->size())
        return true;
      ++q1_keys_it;
      ++q2_keys_it;
    }
  }

  /* Jeżeli q1 jest podzbiorem q2, to q1 < q2 */
  if (q1_keys_it == q1_keys_end && q2_keys_it != q2_keys_end)
    return true;
  return false;
}

/* [O(size())] */
template<typename K, typename V>
bool operator>(const PriorityQueue<K, V>& q1, const PriorityQueue<K, V>& q2) {
  return q2 < q1;
}

/* [O(size())] */
template<typename K, typename V>
bool operator<=(const PriorityQueue<K, V>& q1, const PriorityQueue<K, V>& q2) {
  return !(q1 > q2);
}

/* [O(size())] */
template<typename K, typename V>
bool operator>=(const PriorityQueue<K, V>& q1, const PriorityQueue<K, V>& q2) {
  return !(q1 < q2);
}

#endif /* PRIORITY_QUEUE_HH */