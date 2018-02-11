#include <algorithm>
#include <iostream>
#include <cstdio>
#include <map>
#include <vector>
#include <tuple>
#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>
#include <float.h>
#include <boost/regex.hpp>
#include <stdint.h>
using namespace std;
using namespace boost;

enum class LineType {
    CURRENCY,
    DONATION,
    QUERY,
    WRONG
};

const regex currency_pattern("\\s*(\\u{3})\\s+(\\d{1,16}+)"
                             "(?:,(\\d{1,3}))?\\s*");
const regex donation_pattern("^\\s*(.*\\S)\\s+"
                             "(\\d{1,16}+)(?:,(\\d{1,3}))?"
                             "\\s+(\\u{3})\\s*");
const regex query_pattern("\\s*(\\d{1,16})(?:,(\\d{1,3}))?\\s+"
                          "(\\d{1,16})(?:,(\\d{1,3}))?\\s*");

using Amount = uint64_t;
using Query = tuple<Amount, Amount>;
using ConvertedDonation = tuple<Amount, string, Amount, string>;


/*
  Zgaduje rodzaj linii i zwraca sparsowane dane.
 */
tuple<LineType, cmatch> parseLine(const string& line) {
    cmatch data;
    LineType line_type = LineType::WRONG;
    if (regex_match(line.c_str(), data, currency_pattern)) {
        line_type = LineType::CURRENCY;
    } else if (regex_match(line.c_str(), data, query_pattern)) {
        line_type = LineType::QUERY;
    } else if (regex_match(line.c_str(), data, donation_pattern)) {
        line_type = LineType::DONATION;
    }
    return make_tuple(line_type, data);
}

/*
  Wypisuje linię z błędem na wyjście błędów.
 */
void reportError(int line_num, const string& line) {
    cerr << "Error in line " << line_num << ":" << line << endl;
}

/*
   Zwraca liczbę z ciągu cyferek z ewentualnym przecinkiem.
   frac_part powinno mieć co najwyżej 3 cyfry.
  */
Amount amountFromString(const string& integer_part, const string& frac_part) {
    Amount integer = stoull(integer_part);
    Amount fraction;
    if (frac_part.empty()) {
        fraction = 0;
    } else {
        Amount pow[] = {1, 10, 100};
        fraction = stoull(frac_part) * pow[3 - frac_part.size()];
    }
    return integer * 1000 + fraction;
}

/*
  Dzieli liczbę i zaokrągla.
 */
Amount divByThousand(Amount n) {
    Amount div = n / 1000;
    Amount rest = n % 1000;
    if (rest < 500) {
        return div;
    } else if (rest == 500) {
        if (div % 2 == 1)
            return div + 1;
        else
            return div;
    } else {
        return div + 1;
    }
}

/*
  Zwraca wynik mnożenia dwóch liczb. Rzuca wyjątek, gdy wynik nie mieści
  się w typie.
  */
Amount multi(Amount a, Amount b) {
    Amount res = a * b;
    if (b != 0 && res / b != a)
        throw "Overflow during multiplication";
    return divByThousand(res);
}

/*
  Zamienia wpłatę w walucie na walutę rodzimą.
 */
Amount exchange(const map<string, Amount>& rates, Amount amount,
                const string& currency) {
    auto rate = rates.find(currency)->second;
    auto converted = multi(amount, rate);
    return converted;
}

/*
  Interpretuje linjkę z kursem waluty. False w przypadku błędu.
 */
bool parseCurrency(cmatch data, map<string, Amount>& currencies) {
    string name = data[1];
    auto value = amountFromString(data[2], data[3]);

    if (value == 0 || currencies.find(name) != currencies.end()) {
        return false;
    } else {
        currencies[name] = value;
        return true;
    }
}

/*
  Interpretuje linijkę z dotacją. Zwraca false w przypadku błędu.
 */
bool parseDonation(cmatch data, const map<string, Amount>& currencies,
                   vector<ConvertedDonation>& donations) {

    string name = data[1];
    auto amount = amountFromString(data[2], data[3]);
    string currency = data[4];
    
    if (amount == 0)
        return false;
    if (currencies.find(currency) == currencies.end()) {
        return false;
    } else {
        try {
            auto convertedAmount = exchange(currencies, amount, currency);
            ConvertedDonation donation(convertedAmount, name, amount, currency);
            donations.push_back(donation);
            return true;
        } catch (const char* msg) {
            return false;
        }
    }
}

/*
   Interpretuje linijkę z zapytaniem. False oznacza błąd.
 */
bool parseQuery(cmatch data, vector<Query>& queries) {
    auto lower_limit = amountFromString(data[1], data[2]);
    auto upper_limit = amountFromString(data[3], data[4]);
    if (lower_limit > upper_limit) {
        return false;
    } else {
        queries.push_back(make_tuple(lower_limit, upper_limit));
        return true;
    }
}

/*
   Sprawdza typ wczytanej linii. Aby przejść do kolejnego pliku, musi 
   zostać wczytana co najmniej jedna linia poprzedniego pliku (żaden plik
   nie może być pusty). Jeżeli wczytana linia ma typ WRONG (nie pasuje do
   szablonu pliku: zbyt mało argumentów, ujemne liczby) to ją omijamy, 
   natomiast jeżeli linia ma poprawny typ, to ją wczytujemy ->
   zapisujemy, że wczytujemy już kolejny plik -> sprawdzamy
   jej poprawność (czy istnieje taka waluta, czy pocz_przedz < kon_przedz).
 */

void selectExpectedLine(LineType& expected_line, const LineType& line_type, 
                        int& type_count, int& enum_iter) {
    if (expected_line == line_type) {
        type_count++;
    } else if (line_type != LineType::WRONG && type_count != 0 &&
               line_type == LineType(enum_iter + 1)) {
        type_count = 1;
        enum_iter++;
        expected_line = static_cast<LineType>(enum_iter);
    }
}

/*
   Wczytuje i częściowo waliduje dane ze standardowego wejścia.
 */
void readInput(map<string, Amount>& currencies,
               vector<ConvertedDonation>&  donations,
               vector<Query>& queries) {
    LineType expected_line = LineType::CURRENCY;
    int type_count = 0, enum_iter = 0;
    
    string line;
    for (int line_num = 1; getline(cin, line); ++line_num) {
        LineType line_type;
        cmatch data;
        tie(line_type, data) = parseLine(line);
        selectExpectedLine(expected_line, line_type, type_count, enum_iter);

        bool correct = false;
        if (expected_line == line_type) {
            if (line_type == LineType::CURRENCY) {
                correct = parseCurrency(data, currencies);
            } else if (line_type == LineType::DONATION) {
                correct = parseDonation(data, currencies, donations);
            } else if (line_type == LineType::QUERY) {
                correct = parseQuery(data, queries);
            }
        }

        if (!correct) {
            reportError(line_num, line);
        }
    }
}

/*
  Porównuje wpłaty po wpłaconej kwocie.
  */
bool donationsComp(ConvertedDonation const& a, ConvertedDonation const& b) {
    return get<0>(a) < get<0>(b);
}

/*
  Znajduje i wypisuje darczyńców, którzy wpłacili kwotę należącą 
  do przedziału [beg, end].
  */
void printDonors(Amount begV, Amount endV,
                 const vector<ConvertedDonation>& donations) {
    vector<ConvertedDonation> res;
    ConvertedDonation begT = make_tuple(begV, "", 0, "");
    ConvertedDonation endT = make_tuple(endV, "", 0, "");
    auto begIt = lower_bound(donations.begin(), donations.end(),
                             begT, donationsComp);
    auto endIt = upper_bound(donations.begin(), donations.end(),
                             endT, donationsComp);

    while (begIt != endIt) {
        auto name = get<1>(*begIt), currency = get<3>(*begIt);
        auto amount = get<2>(*begIt);
        auto integer = amount / 1000, frac = amount % 1000;
        cout << "\""<< name << "\",\"" << integer << ",";
        cout << setfill('0') << setw(3) << frac;
        cout << "\"," << currency << endl;
        begIt++;
    }
}

/*
  Odpowiada na zapytania. Modyfikuje (sortuje) dotacje.
  */
void makeQueries(const map<string, Amount>& currencies,
                 vector<ConvertedDonation>& donations, 
                 const vector<Query>& queries) {
 
    stable_sort(donations.begin(), donations.end(), donationsComp);
  
    for (size_t i = 0; i < queries.size(); ++i)
        printDonors(get<0>(queries[i]), get<1>(queries[i]), donations);
}

int main() {
    map<string, Amount> currencies;
    vector<ConvertedDonation> donations;
    vector<Query> queries;

    readInput(currencies, donations, queries);
    makeQueries(currencies, donations, queries);

    return 0;
}
