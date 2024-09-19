#include <gtest/gtest.h>
#include <list>

#ifdef TEST_CHUNK_VECTOR
#include "chunk_vector.hpp"
template <typename T>
using vector = CustomVector::chunk_vector<T>;
#endif

#ifdef TRIVIALLY_COPYABLE
class TriviallyCopyableInt {
public:
    TriviallyCopyableInt(int value = 0) : m_value(value) {
    }

    int m_value;
};

using test_int = TriviallyCopyableInt;
#elif NON_TRIVIALLY_COPYABLE
class NonTriviallyCopyableInt {
public:
    NonTriviallyCopyableInt(int value = 0) : m_value(value) {
    }

    NonTriviallyCopyableInt(const NonTriviallyCopyableInt &other)
        : m_value(other.m_value) {
    }

    NonTriviallyCopyableInt &operator=(const NonTriviallyCopyableInt &other) {
        if (this != &other) {
            m_value = other.m_value;
        }
        return *this;
    }

    int m_value;
};

using test_int = NonTriviallyCopyableInt;
#endif

// Iterator testing
namespace {
class ConstIteratorTest : public testing::Test {
protected:
    const vector<test_int> cv;

    ConstIteratorTest()
        : cv([]() {
              vector<test_int> tmp;
              tmp.push_back(test_int(1));
              tmp.push_back(test_int(2));
              tmp.push_back(test_int(3));
              tmp.push_back(test_int(4));
              tmp.push_back(test_int(5));
              return tmp;
          }()) {
    }
};

TEST_F(ConstIteratorTest, LegacyInputIterator_swapable) {
    auto bg = cv.begin();
    auto ed = cv.end();
    std::swap(bg, ed);
    EXPECT_EQ(ed->m_value, 1);
}

TEST_F(ConstIteratorTest, LegacyInputIterator_copyable) {
    auto bg = cv.begin();
    auto copy = bg;
    EXPECT_EQ(copy->m_value, 1);
    auto copy_constructed(bg);
    EXPECT_EQ(copy_constructed->m_value, 1);
}

TEST_F(ConstIteratorTest, LegacyInputIterator_dereferenceable) {
    auto bg = cv.begin();
    EXPECT_EQ((*bg).m_value, 1);
    EXPECT_EQ(bg->m_value, 1);
}

TEST_F(ConstIteratorTest, LegacyInputIterator_incrementable) {
    auto bg = cv.begin();
    EXPECT_EQ((++bg)->m_value, 2);
    EXPECT_EQ((bg++)->m_value, 2);
    EXPECT_EQ(bg->m_value, 3);
}

TEST_F(ConstIteratorTest, LegacyInputIterator_comparebale) {
    EXPECT_TRUE(cv.begin() == cv.begin());
    EXPECT_FALSE(cv.begin() != cv.begin());
    EXPECT_TRUE(cv.begin() != cv.end());
    EXPECT_FALSE(cv.begin() == cv.end());
}

TEST_F(ConstIteratorTest, LegacyBidirectionalIterator_decrementable) {
    auto bg = cv.end();
    EXPECT_EQ((--bg)->m_value, 5);
    EXPECT_EQ((bg--)->m_value, 5);
    EXPECT_EQ(bg->m_value, 4);
}

TEST_F(ConstIteratorTest, LegacyRandomAccessIterator_arithmetic) {
    auto bg = cv.begin();
    EXPECT_EQ((bg + 2)->m_value, 3);
    EXPECT_EQ((2 + bg)->m_value, 3);
    bg += 2;
    ASSERT_EQ(bg->m_value, 3);
    EXPECT_EQ((bg - 2)->m_value, 1);
    EXPECT_EQ((bg - cv.begin()), 2);
    bg -= 2;
    ASSERT_EQ(bg->m_value, 1);
    EXPECT_EQ(cv.end() - cv.begin(), 5);
    EXPECT_EQ(cv[3].m_value, 4);
}

TEST_F(ConstIteratorTest, LegacyRandomAccessIterator_wide_comparebale) {
    EXPECT_TRUE(cv.begin() < cv.end());
    EXPECT_TRUE(cv.begin() <= cv.end());
    EXPECT_FALSE(cv.begin() > cv.end());
    EXPECT_FALSE(cv.begin() >= cv.end());
    EXPECT_TRUE(cv.begin() + 5 <= cv.end());
    EXPECT_FALSE(cv.begin() + 5 < cv.end());
    EXPECT_TRUE(cv.end() - 5 >= cv.begin());
    EXPECT_FALSE(cv.end() - 5 > cv.begin());
}

class MutableIteratorTest : public testing::Test {
protected:
    vector<test_int> v;
    vector<test_int> v_shuffled;

    MutableIteratorTest() {
        v.push_back(test_int(1));
        v.push_back(test_int(2));
        v.push_back(test_int(3));
        v.push_back(test_int(4));
        v.push_back(test_int(5));
        v_shuffled.push_back(test_int(5));
        v_shuffled.push_back(test_int(1));
        v_shuffled.push_back(test_int(3));
        v_shuffled.push_back(test_int(2));
        v_shuffled.push_back(test_int(4));
    }
};

TEST_F(MutableIteratorTest, LegacyInputIterator_swapable) {
    auto bg = v.begin();
    auto ed = v.end();
    std::swap(bg, ed);
    EXPECT_EQ(ed->m_value, 1);
}

TEST_F(MutableIteratorTest, LegacyInputIterator_copyable) {
    auto bg = v.begin();
    auto copy = bg;
    EXPECT_EQ(copy->m_value, 1);
    auto copy_constructed(bg);
    EXPECT_EQ(copy_constructed->m_value, 1);
}

TEST_F(MutableIteratorTest, LegacyInputIterator_dereferenceable) {
    auto bg = v.begin();
    EXPECT_EQ((*bg).m_value, 1);
    EXPECT_EQ(bg->m_value, 1);
}

TEST_F(MutableIteratorTest, LegacyInputIterator_incrementable) {
    auto bg = v.begin();
    EXPECT_EQ((++bg)->m_value, 2);
    EXPECT_EQ((bg++)->m_value, 2);
    EXPECT_EQ(bg->m_value, 3);
}

TEST_F(MutableIteratorTest, LegacyInputIterator_comparebale) {
    EXPECT_TRUE(v.begin() == v.begin());
    EXPECT_FALSE(v.begin() != v.begin());
    EXPECT_TRUE(v.begin() != v.end());
    EXPECT_FALSE(v.begin() == v.end());
}

TEST_F(MutableIteratorTest, LegacyBidirectionalIterator_decrementable) {
    auto bg = v.end();
    EXPECT_EQ((--bg)->m_value, 5);
    EXPECT_EQ((bg--)->m_value, 5);
    EXPECT_EQ(bg->m_value, 4);
}

TEST_F(MutableIteratorTest, LegacyRandomAccessIterator_arithmetic) {
    auto bg = v.begin();
    EXPECT_EQ((bg + 2)->m_value, 3);
    EXPECT_EQ((2 + bg)->m_value, 3);
    bg += 2;
    ASSERT_EQ(bg->m_value, 3);
    EXPECT_EQ((bg - 2)->m_value, 1);
    EXPECT_EQ((bg - v.begin()), 2);
    bg -= 2;
    ASSERT_EQ(bg->m_value, 1);
    EXPECT_EQ(v.end() - v.begin(), 5);
    EXPECT_EQ(v[3].m_value, 4);
}

TEST_F(MutableIteratorTest, LegacyRandomAccessIterator_wide_comparebale) {
    EXPECT_TRUE(v.begin() < v.end());
    EXPECT_TRUE(v.begin() <= v.end());
    EXPECT_FALSE(v.begin() > v.end());
    EXPECT_FALSE(v.begin() >= v.end());
    EXPECT_TRUE(v.begin() + 5 <= v.end());
    EXPECT_FALSE(v.begin() + 5 < v.end());
    EXPECT_TRUE(v.end() - 5 >= v.begin());
    EXPECT_FALSE(v.end() - 5 > v.begin());
}

TEST_F(MutableIteratorTest, change_element) {
    auto bg = v.begin();
    bg[1] = test_int(100);
    EXPECT_EQ(v[1].m_value, 100);
    *bg = 52;
    EXPECT_EQ(v[0].m_value, 52);
}

TEST_F(MutableIteratorTest, use_std_sort) {
    std::sort(
        v_shuffled.begin(), v_shuffled.end(),
        [](const test_int &a, const test_int &b) {
            return a.m_value < b.m_value;
        }
    );
    for (size_t i = 0; i < v.size(); ++i) {
        EXPECT_EQ(v_shuffled[i].m_value, v[i].m_value);
    }
    std::sort(
        v_shuffled.rbegin(), v_shuffled.rend(),
        [](const test_int &a, const test_int &b) {
            return a.m_value < b.m_value;
        }
    );
    for (size_t i = 0; i < v.size(); ++i) {
        EXPECT_EQ(v_shuffled[i].m_value, v[v.size() - i - 1].m_value);
    }
}
}  // namespace

// Vector testing
namespace {
class ConstructorsTest : public testing::Test {
protected:
    vector<test_int> v;
    vector<test_int> v_short;

    ConstructorsTest() {
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        v.push_back(4);
        v.push_back(5);
        v_short.push_back(1);
        v_short.push_back(2);
    }
};

TEST_F(ConstructorsTest, assign_operator) {
    v_short = v;
    EXPECT_EQ(v_short.size(), 5);
    for (size_t i = 0; i < 5; ++i) {
        EXPECT_EQ(v_short[i].m_value, v[i].m_value);
    }
    v_short = v_short;
    EXPECT_EQ(v_short.size(), 5);
    for (size_t i = 0; i < 5; ++i) {
        EXPECT_EQ(v_short[i].m_value, v[i].m_value);
    }
}

TEST_F(ConstructorsTest, move_assign_operator) {
    v_short = std::move(v);
    EXPECT_EQ(v_short.size(), 5);
    for (size_t i = 0; i < 5; ++i) {
        EXPECT_EQ(v_short[i].m_value, i + 1);
    }
    v_short = std::move(v_short);
    EXPECT_EQ(v_short.size(), 5);
    for (size_t i = 0; i < 5; ++i) {
        EXPECT_EQ(v_short[i].m_value, i + 1);
    }
}

TEST_F(ConstructorsTest, assign) {
    v.assign(2, 123);
    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v[0].m_value, 123);
    EXPECT_EQ(v[1].m_value, 123);
    v.assign(5, 52);
    EXPECT_EQ(v.size(), 5);
    EXPECT_EQ(v[0].m_value, 52);
    EXPECT_EQ(v[1].m_value, 52);
    EXPECT_EQ(v[2].m_value, 52);
    EXPECT_EQ(v[3].m_value, 52);
    EXPECT_EQ(v[4].m_value, 52);
    v.assign(v_short.begin(), v_short.end());
    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v[0].m_value, 1);
    EXPECT_EQ(v[1].m_value, 2);
    v.assign({1, 2, 3, 4, 5});
    EXPECT_EQ(v.size(), 5);
    EXPECT_EQ(v[0].m_value, 1);
    EXPECT_EQ(v[1].m_value, 2);
    EXPECT_EQ(v[2].m_value, 3);
    EXPECT_EQ(v[3].m_value, 4);
    EXPECT_EQ(v[4].m_value, 5);
}

TEST_F(ConstructorsTest, get_allocator) {
    auto alloc = v.get_allocator();
    EXPECT_TRUE(alloc == std::allocator<test_int>());
}

class AccessTest : public testing::Test {
protected:
    vector<test_int> v;

    AccessTest() {
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        v.push_back(4);
        v.push_back(5);
    }
};

TEST_F(AccessTest, at) {
    EXPECT_EQ(v.at(0).m_value, 1);
    EXPECT_EQ(v.at(1).m_value, 2);
    EXPECT_EQ(v.at(2).m_value, 3);
    EXPECT_EQ(v.at(3).m_value, 4);
    EXPECT_EQ(v.at(4).m_value, 5);
    EXPECT_THROW(v.at(5), std::out_of_range);
    v.at(3) = 123;
    EXPECT_EQ(v.at(3).m_value, 123);
}

TEST_F(AccessTest, operator_brackets) {
    EXPECT_EQ(v[0].m_value, 1);
    EXPECT_EQ(v[1].m_value, 2);
    EXPECT_EQ(v[2].m_value, 3);
    EXPECT_EQ(v[3].m_value, 4);
    EXPECT_EQ(v[4].m_value, 5);
    v[3] = 123;
    EXPECT_EQ(v[3].m_value, 123);
}

TEST_F(AccessTest, front) {
    EXPECT_EQ(v.front().m_value, 1);
    v.front() = 123;
    EXPECT_EQ(v.front().m_value, 123);
}

TEST_F(AccessTest, back) {
    EXPECT_EQ(v.back().m_value, 5);
    v.back() = 123;
    EXPECT_EQ(v.back().m_value, 123);
}

TEST_F(AccessTest, dump_data) {
    auto data = v.copy_data();
    for (std::size_t i = 0; i < 5; ++i) {
        EXPECT_EQ(data[i].m_value, v[i].m_value);
    }
}

class IteratorsTest : public testing::Test {
protected:
    vector<test_int> v;
    const vector<test_int> cv;

    IteratorsTest()
        : cv([]() {
              vector<test_int> tmp;
              tmp.push_back(test_int(1));
              tmp.push_back(test_int(2));
              tmp.push_back(test_int(3));
              tmp.push_back(test_int(4));
              tmp.push_back(test_int(5));
              return tmp;
          }()) {
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        v.push_back(4);
        v.push_back(5);
    }
};

TEST_F(IteratorsTest, begin) {
    EXPECT_EQ(v.begin()->m_value, 1);
    v.begin()->m_value = 123;
    EXPECT_EQ(v.begin()->m_value, 123);
    EXPECT_EQ(cv.cbegin()->m_value, 1);
}

TEST_F(IteratorsTest, end) {
    EXPECT_EQ((v.end() - 1)->m_value, 5);
    (v.end() - 1)->m_value = 123;
    EXPECT_EQ((v.end() - 1)->m_value, 123);
    EXPECT_EQ((cv.cend() - 1)->m_value, 5);
    v.clear();
    EXPECT_EQ(v.begin(), v.end());
    EXPECT_EQ(v.end() - v.begin(), 0);
}

TEST_F(IteratorsTest, rbegin) {
    EXPECT_EQ(v.rbegin()->m_value, 5);
    v.rbegin()->m_value = 123;
    EXPECT_EQ(v.rbegin()->m_value, 123);
    EXPECT_EQ(cv.crbegin()->m_value, 5);
}

TEST_F(IteratorsTest, rend) {
    EXPECT_EQ((v.rend() - 1)->m_value, 1);
    (v.rend() - 1)->m_value = 123;
    EXPECT_EQ((v.rend() - 1)->m_value, 123);
    EXPECT_EQ((cv.crend() - 1)->m_value, 1);
}

class CapacityTest : public testing::Test {
protected:
    vector<test_int> v;
    vector<test_int> empty_v;

    CapacityTest() {
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        v.push_back(4);
        v.push_back(5);
    }
};

TEST_F(CapacityTest, empty) {
    EXPECT_TRUE(empty_v.empty());
    EXPECT_FALSE(v.empty());
}

TEST_F(CapacityTest, size) {
    EXPECT_EQ(empty_v.size(), 0);
    EXPECT_EQ(v.size(), 5);
    v.pop_back();
    v.pop_back();
    EXPECT_EQ(v.size(), 3);
    v.push_back(1);
    EXPECT_EQ(v.size(), 4);
}

TEST_F(CapacityTest, max_size) {
    EXPECT_GE(v.max_size(), 5);
}

TEST_F(CapacityTest, reserve_capacity) {
    v.reserve(10000);
    EXPECT_GE(v.capacity(), 10000);
    v.reserve(10);
    EXPECT_GE(v.capacity(), 10000);
}

TEST_F(CapacityTest, shrink_to_fit) {
    v.reserve(100000);
    v.shrink_to_fit();
    EXPECT_LT(v.capacity(), 100000);
}

class ModifiersTest : public testing::Test {
protected:
    vector<test_int> v;
    vector<test_int> empty_v;

    ModifiersTest() {
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        v.push_back(4);
        v.push_back(5);
    }
};

TEST_F(ModifiersTest, clear) {
    v.clear();
    EXPECT_TRUE(v.empty());
}

TEST_F(ModifiersTest, insert_one_element) {
    EXPECT_EQ(v.insert(v.begin() + 2, 123)->m_value, 123);
    EXPECT_EQ(v[0].m_value, 1);
    EXPECT_EQ(v[1].m_value, 2);
    EXPECT_EQ(v[2].m_value, 123);
    EXPECT_EQ(v[3].m_value, 3);
    EXPECT_EQ(v[4].m_value, 4);
    EXPECT_EQ(v[5].m_value, 5);
    EXPECT_EQ(v.size(), 6);
}

TEST_F(ModifiersTest, insert_multiple_elements) {
    EXPECT_EQ(v.insert(v.begin() + 2, 3, 123)->m_value, 123);
    EXPECT_EQ(v[0].m_value, 1);
    EXPECT_EQ(v[1].m_value, 2);
    EXPECT_EQ(v[2].m_value, 123);
    EXPECT_EQ(v[3].m_value, 123);
    EXPECT_EQ(v[4].m_value, 123);
    EXPECT_EQ(v[5].m_value, 3);
    EXPECT_EQ(v[6].m_value, 4);
    EXPECT_EQ(v[7].m_value, 5);
    EXPECT_EQ(v.size(), 8);
}

TEST_F(ModifiersTest, insert_range) {
    std::list<test_int> l;
    l.emplace_back(111);
    l.emplace_back(222);
    l.emplace_back(333);
    EXPECT_EQ(v.insert(v.begin() + 2, l.begin(), l.end())->m_value, 111);
    EXPECT_EQ(v[0].m_value, 1);
    EXPECT_EQ(v[1].m_value, 2);
    EXPECT_EQ(v[2].m_value, 111);
    EXPECT_EQ(v[3].m_value, 222);
    EXPECT_EQ(v[4].m_value, 333);
    EXPECT_EQ(v[5].m_value, 3);
    EXPECT_EQ(v[6].m_value, 4);
    EXPECT_EQ(v[7].m_value, 5);
    EXPECT_EQ(v.size(), 8);
}

TEST_F(ModifiersTest, insert_initializer_list) {
    EXPECT_EQ(v.insert(v.begin() + 2, {111, 222, 333})->m_value, 111);
    EXPECT_EQ(v[0].m_value, 1);
    EXPECT_EQ(v[1].m_value, 2);
    EXPECT_EQ(v[2].m_value, 111);
    EXPECT_EQ(v[3].m_value, 222);
    EXPECT_EQ(v[4].m_value, 333);
    EXPECT_EQ(v[5].m_value, 3);
    EXPECT_EQ(v[6].m_value, 4);
    EXPECT_EQ(v[7].m_value, 5);
    EXPECT_EQ(v.size(), 8);
}

TEST_F(ModifiersTest, insert_in_empty_vector) {
    v.clear();
    EXPECT_EQ(v.insert(v.end(), 123)->m_value, 123);
    EXPECT_EQ(v[0].m_value, 123);
    EXPECT_EQ(v.size(), 1);
    v.clear();
    EXPECT_EQ(v.insert(v.end(), 3, 123)->m_value, 123);
    EXPECT_EQ(v[0].m_value, 123);
    EXPECT_EQ(v[1].m_value, 123);
    EXPECT_EQ(v[2].m_value, 123);
    EXPECT_EQ(v.size(), 3);
}

TEST_F(ModifiersTest, emplace) {
    EXPECT_EQ(v.emplace(v.begin() + 2, 123)->m_value, 123);
    EXPECT_EQ(v[0].m_value, 1);
    EXPECT_EQ(v[1].m_value, 2);
    EXPECT_EQ(v[2].m_value, 123);
    EXPECT_EQ(v[3].m_value, 3);
    EXPECT_EQ(v[4].m_value, 4);
    EXPECT_EQ(v[5].m_value, 5);
    EXPECT_EQ(v.size(), 6);
    EXPECT_EQ(v.emplace(v.end(), 999)->m_value, 999);
    EXPECT_EQ(v[6].m_value, 999);
    EXPECT_EQ(v.size(), 7);
}

TEST_F(ModifiersTest, erase_one_element) {
    EXPECT_EQ(v.erase(v.begin() + 2)->m_value, 4);
    EXPECT_EQ(v[0].m_value, 1);
    EXPECT_EQ(v[1].m_value, 2);
    EXPECT_EQ(v[2].m_value, 4);
    EXPECT_EQ(v[3].m_value, 5);
    EXPECT_EQ(v.size(), 4);
}

TEST_F(ModifiersTest, erase_range) {
    EXPECT_EQ(v.erase(v.begin() + 1, v.begin() + 4)->m_value, 5);
    EXPECT_EQ(v[0].m_value, 1);
    EXPECT_EQ(v[1].m_value, 5);
    EXPECT_EQ(v.size(), 2);
}

TEST_F(ModifiersTest, erase_all) {
    v.erase(v.begin(), v.end());
    EXPECT_TRUE(v.empty());
}

TEST_F(ModifiersTest, erase_empty_range) {
    v.erase(v.begin() + 2, v.begin() + 2);
    EXPECT_EQ(v.size(), 5);
}

TEST_F(ModifiersTest, push_back) {
    v.push_back(123);
    EXPECT_EQ(v[5].m_value, 123);
    EXPECT_EQ(v.size(), 6);
    test_int tmp = 999;
    v.push_back(tmp);
    EXPECT_EQ(v[6].m_value, 999);
    EXPECT_EQ(v.size(), 7);
}

TEST_F(ModifiersTest, emplace_back) {
    v.emplace_back(123);
    EXPECT_EQ(v[5].m_value, 123);
    EXPECT_EQ(v.size(), 6);
    v.emplace_back(999);
    EXPECT_EQ(v[6].m_value, 999);
    EXPECT_EQ(v.size(), 7);
}

TEST_F(ModifiersTest, pop_back) {
    v.pop_back();
    EXPECT_EQ(v.size(), 4);
    v.pop_back();
    EXPECT_EQ(v.size(), 3);
}

TEST_F(ModifiersTest, resize) {
    v.resize(10);
    EXPECT_EQ(v.size(), 10);
    EXPECT_EQ(v[5].m_value, 0);
    EXPECT_EQ(v[6].m_value, 0);
    EXPECT_EQ(v[7].m_value, 0);
    EXPECT_EQ(v[8].m_value, 0);
    EXPECT_EQ(v[9].m_value, 0);
    v.resize(3);
    EXPECT_EQ(v.size(), 3);
    v.resize(5, 123);
    EXPECT_EQ(v[0].m_value, 1);
    EXPECT_EQ(v[1].m_value, 2);
    EXPECT_EQ(v[2].m_value, 3);
    EXPECT_EQ(v[3].m_value, 123);
    EXPECT_EQ(v[4].m_value, 123);
}

TEST_F(ModifiersTest, swap) {
    v.swap(empty_v);
    EXPECT_TRUE(v.empty());
    EXPECT_EQ(empty_v.size(), 5);
    std::swap(v, empty_v);
    EXPECT_EQ(v.size(), 5);
    EXPECT_TRUE(empty_v.empty());
    EXPECT_EQ(v[0].m_value, 1);
    EXPECT_EQ(v[1].m_value, 2);
    EXPECT_EQ(v[2].m_value, 3);
    EXPECT_EQ(v[3].m_value, 4);
    EXPECT_EQ(v[4].m_value, 5);
}

class NonMemberTest : public testing::Test {
protected:
    vector<int> v_1;
    vector<int> v_eq;
    vector<int> v_little;
    vector<int> v_less;
    vector<int> v_greater;
    vector<test_int> v_repeatable;

    NonMemberTest() {
        v_1.push_back(1);
        v_1.push_back(2);
        v_1.push_back(3);
        v_1.push_back(4);
        v_1.push_back(5);
        v_eq.push_back(1);
        v_eq.push_back(2);
        v_eq.push_back(3);
        v_eq.push_back(4);
        v_eq.push_back(5);
        v_little.push_back(1);
        v_little.push_back(2);
        v_less.push_back(1);
        v_less.push_back(2);
        v_less.push_back(0);
        v_greater.push_back(1);
        v_greater.push_back(2);
        v_greater.push_back(4);
        v_repeatable.push_back(1);
        v_repeatable.push_back(1);
        v_repeatable.push_back(2);
        v_repeatable.push_back(2);
        v_repeatable.push_back(3);
        v_repeatable.push_back(3);
    }
};

TEST_F(NonMemberTest, equal) {
    EXPECT_TRUE(v_1 == v_eq);
    EXPECT_FALSE(v_1 == v_little);
}

TEST_F(NonMemberTest, not_equal) {
    EXPECT_FALSE(v_1 != v_eq);
    EXPECT_TRUE(v_1 != v_little);
}

TEST_F(NonMemberTest, less) {
    EXPECT_FALSE(v_1 < v_eq);
    EXPECT_FALSE(v_1 < v_little);
    EXPECT_TRUE(v_1 < v_greater);
    EXPECT_FALSE(v_1 < v_less);
    EXPECT_TRUE(v_little < v_1);
}

TEST_F(NonMemberTest, less_or_equal) {
    EXPECT_TRUE(v_1 <= v_eq);
    EXPECT_FALSE(v_1 <= v_little);
    EXPECT_TRUE(v_1 <= v_greater);
    EXPECT_FALSE(v_1 <= v_less);
    EXPECT_TRUE(v_little <= v_1);
}

TEST_F(NonMemberTest, greater) {
    EXPECT_FALSE(v_1 > v_eq);
    EXPECT_TRUE(v_1 > v_little);
    EXPECT_FALSE(v_1 > v_greater);
    EXPECT_TRUE(v_1 > v_less);
    EXPECT_FALSE(v_little > v_1);
}

TEST_F(NonMemberTest, greater_or_equal) {
    EXPECT_TRUE(v_1 >= v_eq);
    EXPECT_TRUE(v_1 >= v_little);
    EXPECT_FALSE(v_1 >= v_greater);
    EXPECT_TRUE(v_1 >= v_less);
    EXPECT_FALSE(v_little >= v_1);
}

TEST_F(NonMemberTest, swap) {
    std::swap(v_1, v_eq);
    EXPECT_TRUE(v_1 == v_eq);
    std::swap(v_1, v_little);
    EXPECT_EQ(v_1[0], 1);
    EXPECT_EQ(v_1[1], 2);
    EXPECT_EQ(v_little[0], 1);
    EXPECT_EQ(v_little[1], 2);
    EXPECT_EQ(v_little[2], 3);
    EXPECT_EQ(v_little[3], 4);
    EXPECT_EQ(v_little[4], 5);
}
}  // namespace

// TODO tests for incomplete types

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}