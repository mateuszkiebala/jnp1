/* Moduł implemenctacji funkcji operacji na słownikach */

#include "maptel.h"
#include <cstdio>
#include <iostream>
#include <assert.h>
#include <string>
#include <cstring>
#include <cctype>
#include <sstream>
#include <unordered_map>
#include <set>

#define MAX_U_LONG_INT 4294967295

using namespace std;
using MAPTEL = unordered_map<string, string>;

namespace {
  /* Zmienna do wypisywania informacji diagnostycznych */
  #ifndef NDEBUG
    const bool debug = true;
  #else
    const bool debug = false;
  #endif
  
  /* Tworzy mape map służąca do trzymania wielu słowników
   * W wewnętrznej mapie pod kluczem - numer trzymana jest ewentualna
   * jego zmiana - przekierowanie */
  unordered_map<unsigned long, MAPTEL>& dicts() {
    static unordered_map<unsigned long, MAPTEL> map;
    return map;
  }

  /* Podąża ciągiem kolejnych zmian, aby znaleźć końcowy numer
   * jeśli znajdzie końcowy numer zwraca go
   * jeśli nie ma zmiany zwraca początkowego stringa
   * jeśli zmiany prowadzą do cyklu zwraca empty string o długości 0 
   * do szukania powtórzeń wykorzystuje set, 
   * zapisuje w nim numery, które już wystąpiły*/
  string find_dst(MAPTEL& m, string& s_src) {
    set<string> repeated;
    string s (s_src);
    MAPTEL::iterator it;

    while ((it = m.find(s)) != m.end()) {
      if (repeated.count(s) > 0) {
        return "\0";
      }
      repeated.insert(s);
      s = it->second;
    }
    return s;
  }
  
  /* Sprawdza czy wskaźnik nie jest NULL, 
   * czy wszystkie znaki do cyfry,
   * czy numer telefonu jest zakończony znakiem '\0' oraz
   * czy ma długość <= TEL_NUM_MAX_LEN */
  bool if_string_correct(char const *s) {
    if (s == NULL)
      return false;
    for (size_t i = 0; i <= TEL_NUM_MAX_LEN; i++) {
      if (*(s + i) == '\0')
        return true;
      if (!isdigit(*(s + i)))
        return false;
    }

    return false;
  }
}

unsigned long maptel_create() {
  if (debug)
    cerr << "maptel: maptel_create()\n";
  static unsigned long next_id;      // id przydzielane słownikom

  assert(next_id != MAX_U_LONG_INT); //przepełnienie
  dicts().emplace(next_id, MAPTEL{});
  if (debug)
    cerr << "maptel: maptel_create: new map id = " << next_id << "\n"; 
 
  return next_id++;
}

void maptel_delete(unsigned long id) {
  if (debug)
    cerr << "maptel: maptel_delete(" << id << ")\n";
  auto it = dicts().find(id);
  assert(it != dicts().end());      // czy istnieje słownik o danym id
  dicts().erase(it);
  if (debug)
    cerr << "maptel: maptel_delete: map " << id << " deleted\n";
}

void maptel_insert(unsigned long id, 
                   char const *tel_src, 
                   char const *tel_dst) {
  
  if (debug)
    cerr << "maptel: maptel_insert(" << id << ", "
         << tel_src << ", " << tel_dst << ")\n";     
  assert(if_string_correct(tel_src));
  assert(if_string_correct(tel_dst));
  string s_src (tel_src);
  string s_dst (tel_dst);
  assert(!s_src.empty());
  assert(!s_dst.empty());
  
  auto it_dicts = dicts().find(id);
  assert(it_dicts != dicts().end()); // czy istnieje słownik o danym id
  
  auto it_maptel = it_dicts->second.find(s_src);
  if (it_maptel != it_dicts->second.end()) { //istnieje zmiana, nadpisuje 
    it_maptel->second = s_dst; 
  } else {                                   //nie istnieje, dodaj nowe
    it_dicts->second.emplace(s_src, s_dst); 
  }

  if (debug)
    cerr << "maptel: maptel_insert: inserted\n";
}

void maptel_erase(unsigned long id, char const *tel_src) {
  if (debug)
    cerr << "maptel: maptel_erase(" << id << ", " << tel_src << ")\n";

  assert(if_string_correct(tel_src));
  string s_src (tel_src);
  assert(!s_src.empty());
  
  auto it_dicts = dicts().find(id);
  assert(it_dicts != dicts().end()); // czy istnieje słownik o danym id
  
  auto it_maptel = it_dicts->second.find(s_src);
  if (it_maptel == it_dicts->second.end()) {
    if (debug)
      cerr << "maptel: maptel_erase: nothing to erase\n";
  } else {
    it_dicts->second.erase(it_maptel);
    if (debug)
      cerr << "maptel: maptel_erase: erased\n";
  }
}

void maptel_transform(unsigned long id, char const *tel_src, 
                      char *tel_dst, size_t len) {
  if (debug)
    cerr << "maptel: maptel_transform(" << id << ", "
         << tel_src << ", " << &tel_dst << ", " << len << ")\n";
  assert(tel_dst != NULL);
  assert(if_string_correct(tel_src));
  assert(len > 0);

  auto it_dicts = dicts().find(id);
  assert(it_dicts != dicts().end()); // czy istnieje słownik o danym id
  
  string s_src (tel_src);
  assert(!s_src.empty());
  string result = find_dst(it_dicts->second, s_src);
  assert(len > result.length());
  
  if (result.empty()) {
    strcpy(tel_dst, s_src.c_str());
    if (debug)
      cerr << "maptel: maptel_transform: cycle detected\n";
  } else {
    strcpy(tel_dst, result.c_str());
  }
  
  if (debug)
    cerr << "maptel: maptel_transform: "
         << s_src << " -> " << tel_dst << ", \n";
}
