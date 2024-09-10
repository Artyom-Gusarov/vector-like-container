#include <gtest/gtest.h>

#ifdef TEST_CHUNK_VECTOR
#include "chunk_vector.hpp"
template <typename T>
using vector = CustomVector::chunk_vector<T>;
#endif

#ifdef TRIVIALLY_COPYABLE
class TriviallyCopyableInt {
public:
    TriviallyCopyableInt(int value = 0) : m_value(value) {}
    int m_value;
};
using test_int = TriviallyCopyableInt;
#elif NON_TRIVIALLY_COPYABLE
class NonTriviallyCopyableInt {
public:
    NonTriviallyCopyableInt(int value = 0) : m_value(value) {}

    NonTriviallyCopyableInt(const NonTriviallyCopyableInt &other)
        : m_value(other.m_value) {}

    NonTriviallyCopyableInt& operator=(const NonTriviallyCopyableInt &other) {
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