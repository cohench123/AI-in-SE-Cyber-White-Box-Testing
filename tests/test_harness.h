#ifndef TEST_HARNESS_H
#define TEST_HARNESS_H

// Minimal dependency-free unit test harness.
// Usage: TEST(name) { CHECK(cond); CHECK_EQ(a, b); }

#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct TestCase {
    const char* name;
    std::function<void()> fn;
    const char* xfailReason;  // nullptr for normal tests
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

inline int& failureCounter() {
    static int failures = 0;
    return failures;
}

struct Registrar {
    Registrar(const char* name, std::function<void()> fn, const char* xfailReason = nullptr) {
        registry().push_back({name, fn, xfailReason});
    }
};

// Redirects std::cout into a buffer for the lifetime of the object.
class CoutCapture {
public:
    CoutCapture() : old_(std::cout.rdbuf(buffer_.rdbuf())) {}
    ~CoutCapture() { std::cout.rdbuf(old_); }
    std::string str() const { return buffer_.str(); }
private:
    std::stringstream buffer_;
    std::streambuf* old_;
};

#define TEST(name)                          \
    static void name();                     \
    static Registrar reg_##name(#name, name); \
    static void name()

// A test that documents a known defect. It must fail while the defect exists.
// If it passes, the runner reports XPASS and counts it as a failure, so the
// marker gets removed once the defect is fixed.
#define XFAIL_TEST(name, reason)                              \
    static void name();                                       \
    static Registrar reg_##name(#name, name, reason);         \
    static void name()

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            std::cerr << __FILE__ << ":" << __LINE__                         \
                      << ": CHECK failed: " #cond "\n";                      \
            failureCounter()++;                                              \
        }                                                                    \
    } while (0)

#define CHECK_EQ(actual, expected)                                           \
    do {                                                                     \
        auto _actual = (actual);                                             \
        auto _expected = (expected);                                         \
        if (!(_actual == _expected)) {                                       \
            std::cerr << __FILE__ << ":" << __LINE__                         \
                      << ": CHECK_EQ failed: " #actual " != " #expected "\n"; \
            failureCounter()++;                                              \
        }                                                                    \
    } while (0)

inline int runAllTests() {
    int failedTests = 0, expectedFailures = 0;
    for (const auto& t : registry()) {
        int before = failureCounter();
        t.fn();
        bool failed = failureCounter() != before;
        if (t.xfailReason) {
            if (failed) {
                expectedFailures++;
                std::cout << "[XFAIL] " << t.name << " -- " << t.xfailReason << "\n";
            } else {
                failedTests++;
                std::cerr << "[XPASS] " << t.name << " -- expected to fail (" << t.xfailReason
                          << "); the defect may be fixed, so remove the XFAIL marker\n";
            }
        } else if (failed) {
            failedTests++;
            std::cerr << "[FAIL] " << t.name << "\n";
        } else {
            std::cout << "[ OK ] " << t.name << "\n";
        }
    }
    std::cout << registry().size() << " tests, " << failedTests << " failed, "
              << expectedFailures << " expected failures (known defects)\n";
    return failedTests == 0 ? 0 : 1;
}

#endif
