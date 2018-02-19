#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "../include/ring_buffer.h"
#include "catch.hpp"
#include "container_matcher.h"

using bsp::ring_buffer;
using Catch::Equals;

TEST_CASE("ring_buffer basics", "[ring_buffer]") {
    ring_buffer<int, 8> ring;
    CHECK(ring.count() == 0);
    CHECK(ring.start() == 0);
    CHECK(ring.empty());
    CHECK(ring.max_size() == 8);    
}

TEST_CASE("ring_buffer basic operations", "[ring_buffer]"){
    SECTION("clear"){
        ring_buffer<int, 8> ring;
        ring.push_back(42);
        ring.clear();
        CHECK(ring.count() == 0);
        CHECK(ring.empty());
    }

    SECTION("can add"){
        ring_buffer<int, 8> ring;
        ring.push_back(42);
        CHECK(ring.count() == 1);
        CHECK(ring.front() == 42);
        CHECK(ring.back() == 42);

        ring.push_back(-1);
        CHECK(ring.front() == 42);
        CHECK(ring.back() == -1);
        CHECK(ring.count() == 2);
    }

    SECTION("add up to max_size()"){
        ring_buffer<int, 8> ring;
        for (int i=0; i<8; i++) ring.push_back(i);
        CHECK(ring.count() == 8);
        CHECK(ring.start() == 0);
    }

    SECTION("add beyond max_size()"){
        ring_buffer<int, 8> ring;
        for (int i=0; i<8; i++) ring.push_back(i);
        ring.push_back(42);
        CHECK(ring.start() == 1);
        CHECK(ring.count() == 8);
    }

    SECTION("pushing and popping"){
        ring_buffer<int, 8> ring;
        int size = 0;
        for (int i=0; i<16; i++){
            ring.push_back(i);
            ring.push_back(i);
            CHECK(ring.back() == i);
            ring.pop_front();
            size++;
            CHECK(ring.count() == std::min<int>(size, ring.max_size() - 1));
        }
    }
}

TEST_CASE("ring_buffer construction", "[ring_buffer]"){
    SECTION("default"){
        ring_buffer<int, 8> ring;
        CHECK(ring.count() == 0);
        CHECK(ring.empty());
    }

    SECTION("initializer_list"){
        ring_buffer<int, 8> ring { 2, 3, 5, 7, 11 };
        CHECK(ring.count() == 5);
        auto it = ring.begin();        
        CHECK(*it == 2); it++;
        CHECK(*it == 3); it++;
        CHECK(*it == 5); it++;
        CHECK(*it == 7); it++;
        CHECK(*it == 11); it++;
        CHECK(it == ring.end());
    }

    SECTION("vector"){
        std::vector<int> vec { 2, 3, 5, 7, 11 };
        ring_buffer<int, 8> ring { vec };
        CHECK(ring.count() == 5);        
        CHECK_THAT(ring, Equals(ring, vec));
    }
}

TEST_CASE("ring_buffer iterator interface", "[ring_buffer]"){
    SECTION("empty"){
        ring_buffer<int, 8> ring;
        CHECK(ring.begin() == ring.end());
        CHECK(std::distance(ring.begin(), ring.end()) == 0);
    }

    SECTION("empty const"){
        ring_buffer<int, 8> ring;
        CHECK(ring.cbegin() == ring.cend());
        CHECK(std::distance(ring.cbegin(), ring.cend()) == 0);
    }

    SECTION("half size"){
        ring_buffer<int, 8> ring;
        for (int i=0; i < 4; i++) ring.push_back(i);
        CHECK(ring.begin() != ring.end());
        CHECK(std::distance(ring.begin(), ring.end()) == 4);
    }

    SECTION("full size"){
        ring_buffer<int, 8> ring;
        for (int i=0; i < 8; i++) ring.push_back(i);
        CHECK(ring.begin() != ring.end());
        CHECK(std::distance(ring.begin(), ring.end()) == 8);
    }

    SECTION("ringed around"){
        ring_buffer<int, 8> ring;
        for (int i=0; i < 12; i++) ring.push_back(i);
        CHECK(ring.begin() != ring.end());
        CHECK(std::distance(ring.begin(), ring.end()) == 8);
    }

    SECTION("ringed around + pop_front"){
        ring_buffer<int, 8> ring;
        for (int i=0; i < 12; i++) ring.push_back(i);
        for (int i=0; i < 4; i++) ring.pop_front();
        CHECK(ring.begin() != ring.end());
        CHECK(std::distance(ring.begin(), ring.end()) == 4);
    }

    SECTION("iterator"){
        ring_buffer<int, 8> ring { 1, 2, 3, 4};
        auto it = ring.begin();
        CHECK(*it == 1); it++;
        CHECK(*it == 2); it++;
        CHECK(*it == 3); it++;
        CHECK(*it == 4); it++;
        CHECK(it == ring.end());
    }

    SECTION("const iterator"){
        const ring_buffer<int, 8> ring { 1, 2, 3, 4};
        auto it = ring.begin();
        CHECK(*it == 1); it++;
        CHECK(*it == 2); it++;
        CHECK(*it == 3); it++;
        CHECK(*it == 4); it++;
        CHECK(it == ring.end());
    }

    SECTION("reverse iterator"){
        ring_buffer<int, 8> ring { 1, 2, 3, 4};
        auto it = ring.rbegin();
        CHECK(*it == 4); it++;
        CHECK(*it == 3); it++;
        CHECK(*it == 2); it++;
        CHECK(*it == 1); it++;
        CHECK(it == ring.rend());
    }

    SECTION("const reverse iterator"){
        const ring_buffer<int, 8> ring { 1, 2, 3, 4};
        auto it = ring.rbegin();
        CHECK(*it == 4); it++;
        CHECK(*it == 3); it++;
        CHECK(*it == 2); it++;
        CHECK(*it == 1); it++;
        CHECK(it == ring.rend());
    }
}

TEST_CASE("ring_buffer internals", "[ring_buffer]"){
    SECTION("valid_index 1"){
        ring_buffer<int, 8> ring;
        for (int i=0; i<4; i++) ring.push_back(i);
        CHECK(ring.valid_index(0));
        CHECK(ring.valid_index(1));
        CHECK(ring.valid_index(2));
        CHECK(ring.valid_index(3));
        CHECK(!ring.valid_index(4));
        CHECK(!ring.valid_index(5));
        CHECK(!ring.valid_index(6));
        CHECK(!ring.valid_index(7));
    }

    SECTION("valid_index 2"){
        ring_buffer<int, 8> ring;
        for (int i=0; i<12; i++) ring.push_back(i);
        for (int i=0; i<8; i++) CHECK(ring.valid_index(i));
    }

    SECTION("valid_index 3"){
        ring_buffer<int, 8> ring;
        ring.push_back(42);
        for (int i=0; i<8; i++) {
            ring.push_back(42);
            ring.pop_front();

            CHECK(ring.valid_index((i + 1) % ring.max_size()));
            for (int j=0; j<8; j++){
                if (j != i) CHECK(!ring.valid_index((j + 1) % ring.max_size()));
            }
        }
    }
}

TEST_CASE("ring_buffer operator<<", "[ring_buffer]"){
    SECTION("printing"){
        ring_buffer<int, 8> ring { 2, 3, 5, 7, 11 };
        std::cout << "Should print {2, 3, 5, 7, 11, _, _, _}\n";
        std::cout << ring << "\n";
    }

    SECTION("printing"){
        ring_buffer<int, 8> ring;
        std::cout << "Should print {}\n";
        std::cout << ring << "\n";
    }

    SECTION("printing"){
        ring_buffer<int, 4> ring { 1, 2, 3, 4, 5, 6, 7, 8 };
        std::cout << "Should print {5, 6, 7, 8}\n";
        std::cout << ring << "\n";
    }

    SECTION("printing"){
        ring_buffer<int, 4> ring { 1, 2, 3, 4 };
        ring.pop_front();
        ring.pop_front();
        std::cout << "Should print {_, _, 3, 4}\n";
        std::cout << ring << "\n";
    }
}