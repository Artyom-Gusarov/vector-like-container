#include <gtest/gtest.h>

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

namespace {
class VectorTest : public testing::Test {
protected:
    vector<test_int> v;
    const test_int num{12345};
    const test_int a{1};
    const test_int b{2};
    const test_int c{3};
};

TEST_F(VectorTest, push_back_pass_by_const_ref) {
    v.push_back(num);
    EXPECT_EQ(v[0].m_value, num.m_value);
}

TEST_F(VectorTest, push_back_pass_by_rvalue_ref) {
    test_int copy = num;
    v.push_back(std::move(copy));
    EXPECT_EQ(v[0].m_value, num.m_value);
}

TEST_F(VectorTest, push_back_multiple_elements) {
    v.push_back(a);
    v.push_back(b);
    v.push_back(c);
    EXPECT_EQ(v[0].m_value, a.m_value);
    EXPECT_EQ(v[1].m_value, b.m_value);
    EXPECT_EQ(v[2].m_value, c.m_value);
    EXPECT_EQ(v.size(), 3);
}

TEST_F(VectorTest, pop_back) {
    v.push_back(a);
    v.push_back(b);
    v.push_back(c);
    v.pop_back();
    EXPECT_EQ(v[0].m_value, a.m_value);
    EXPECT_EQ(v[1].m_value, b.m_value);
    EXPECT_EQ(v.size(), 2);
}

TEST_F(VectorTest, change_element) {
    v.push_back(a);
    v.push_back(b);
    v.push_back(c);
    v[1] = test_int(4);
    EXPECT_EQ(v[0].m_value, a.m_value);
    EXPECT_EQ(v[1].m_value, 4);
    EXPECT_EQ(v[2].m_value, c.m_value);
}

TEST_F(VectorTest, many_push_back) {
    const int n = 1000;
    for (int i = 0; i < n; ++i) {
        v.push_back(test_int(i));
    }
    for (int i = 0; i < n; ++i) {
        EXPECT_EQ(v[i].m_value, i);
    }
    EXPECT_EQ(v.size(), 1000);
    const int m = 10000;
    for (int i = n; i < m; ++i) {
        v.push_back(test_int(i));
    }
    for (int i = 0; i < m; ++i) {
        EXPECT_EQ(v[i].m_value, i);
    }
    EXPECT_EQ(v.size(), 10000);
}
}  // namespace

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}