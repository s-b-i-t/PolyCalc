#include "Polynomial.h"
#include "Complex.h"
#include "Rational.h"

#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

class TestRun {
public:
    void check(bool condition, const std::string &description) {
        ++checks_;

        if (condition) {
            std::cout << "[PASS] " << description << '\n';
            return;
        }

        ++failures_;
        std::cout << "[FAIL] " << description << '\n';
    }

    int finish() const {
        std::cout << '\n'
                  << checks_ << " checks, " << failures_ << " failures\n";
        return failures_ == 0 ? 0 : 1;
    }

private:
    std::size_t checks_ = 0;
    std::size_t failures_ = 0;
};

bool has_coefficients(
    const Polynomial &polynomial,
    std::initializer_list<double> expected) {
    return polynomial.getListCoeffsIn() == std::vector<double>(expected);
}

bool has_complex_coefficients(
    const ComplexPoly &polynomial,
    std::initializer_list<double> expected_real,
    std::initializer_list<double> expected_imaginary) {
    return polynomial.getListCoeffsIn() == std::vector<double>(expected_real) &&
           polynomial.getcomplexCoeffsList() ==
               std::vector<double>(expected_imaginary);
}

template <typename T>
std::string dump_output(const T &value) {
    std::ostringstream output;
    std::streambuf *original_buffer = std::cout.rdbuf(output.rdbuf());
    value.Dump();
    std::cout.rdbuf(original_buffer);
    return output.str();
}

void test_representation(TestRun &tests) {
    const Polynomial empty(std::vector<double>{});
    tests.check(
        has_coefficients(empty, {0.0}),
        "empty input has the canonical representation {0}");

    const Polynomial all_zero(std::vector<double>{0.0, 0.0, 0.0});
    tests.check(
        has_coefficients(all_zero, {0.0}),
        "all-zero input has the canonical representation {0}");

    const Polynomial trailing_zero(std::vector<double>{3.0, -2.0, 0.0, 0.0});
    tests.check(
        has_coefficients(trailing_zero, {3.0, -2.0}),
        "trailing exact zeros are removed");

    const Polynomial tiny_nonzero(std::vector<double>{0.0, 5e-12});
    tests.check(
        has_coefficients(tiny_nonzero, {0.0, 5e-12}),
        "a tiny nonzero coefficient is preserved exactly");

    tests.check(
        tiny_nonzero.GetCoeff(1) == 5e-12,
        "GetCoeff preserves a tiny nonzero coefficient");

    tests.check(
        tiny_nonzero.GetDegree() == 1,
        "a tiny nonzero leading coefficient determines the degree");

    Polynomial shortened(std::vector<double>{3.0, -2.0});
    shortened.SetCoeffAt(1, 0.0);
    tests.check(
        has_coefficients(shortened, {3.0}),
        "setting the leading coefficient to zero restores canonical storage");

    const Polynomial source(std::vector<double>{4.0, -3.0, 2.0});
    const Polynomial arithmetic_zero = source - source;
    tests.check(
        has_coefficients(arithmetic_zero, {0.0}),
        "an arithmetic result equal to zero has canonical storage");

    const Polynomial multiplied_zero =
        Polynomial() * Polynomial(std::vector<double>{1.0, 2.0});
    tests.check(
        has_coefficients(multiplied_zero, {0.0}),
        "multiplying canonical zero produces canonical zero");

    const Polynomial scalar_zero(0);
    tests.check(
        scalar_zero == all_zero && scalar_zero == arithmetic_zero,
        "all ways of constructing mathematical zero compare equal");
}

void test_addition(TestRun &tests) {
    // Coefficients are constant-first.
    // (1 + 2x + 3x^2) + (4 - 2x + 5x^3)
    // = 5 + 0x + 3x^2 + 5x^3.
    const Polynomial lhs(std::vector<double>{1.0, 2.0, 3.0});
    const Polynomial rhs(std::vector<double>{4.0, -2.0, 0.0, 5.0});
    const Polynomial sum = lhs + rhs;

    tests.check(
        has_coefficients(sum, {5.0, 0.0, 3.0, 5.0}),
        "addition combines coefficients of different-degree polynomials");
    tests.check(
        has_coefficients(lhs, {1.0, 2.0, 3.0}),
        "addition does not modify its left operand");
    tests.check(
        has_coefficients(rhs, {4.0, -2.0, 0.0, 5.0}),
        "addition does not modify its right operand");

    // (-2 + 7x^2) + 5 = 3 + 7x^2.
    const Polynomial sparse(std::vector<double>{-2.0, 0.0, 7.0});
    const Polynomial constant(std::vector<double>{5.0});
    tests.check(
        has_coefficients(sparse + constant, {3.0, 0.0, 7.0}),
        "addition preserves an internal zero coefficient");

    tests.check(
        has_coefficients(sparse + Polynomial(), {-2.0, 0.0, 7.0}),
        "zero is the additive identity");

    Polynomial accumulated(std::vector<double>{1.0, 2.0, 3.0});
    accumulated += rhs;
    tests.check(
        has_coefficients(accumulated, {5.0, 0.0, 3.0, 5.0}),
        "operator+= produces the hand-calculated sum");
}

void test_subtraction(TestRun &tests) {
    // (5 - 3x + 2x^2) - (1 + 4x - 2x^2 + x^3)
    // = 4 - 7x + 4x^2 - x^3.
    const Polynomial lhs(std::vector<double>{5.0, -3.0, 2.0});
    const Polynomial rhs(std::vector<double>{1.0, 4.0, -2.0, 1.0});
    const Polynomial difference = lhs - rhs;

    tests.check(
        has_coefficients(difference, {4.0, -7.0, 4.0, -1.0}),
        "subtraction combines coefficients of different-degree polynomials");
    tests.check(
        has_coefficients(lhs, {5.0, -3.0, 2.0}),
        "subtraction does not modify its left operand");
    tests.check(
        has_coefficients(rhs, {1.0, 4.0, -2.0, 1.0}),
        "subtraction does not modify its right operand");

    // Reversing the operands negates every coefficient:
    // rhs - lhs = -4 + 7x - 4x^2 + x^3.
    tests.check(
        has_coefficients(rhs - lhs, {-4.0, 7.0, -4.0, 1.0}),
        "reversing subtraction negates the hand-calculated difference");

    // (-2 + 7x^2) - 5 = -7 + 7x^2.
    const Polynomial sparse(std::vector<double>{-2.0, 0.0, 7.0});
    const Polynomial constant(std::vector<double>{5.0});
    tests.check(
        has_coefficients(sparse - constant, {-7.0, 0.0, 7.0}),
        "subtracting a constant preserves higher-degree coefficients");

    // 0 - rhs = -1 - 4x + 2x^2 - x^3.
    tests.check(
        has_coefficients(Polynomial() - rhs, {-1.0, -4.0, 2.0, -1.0}),
        "subtracting a polynomial from zero negates every coefficient");

    tests.check(
        has_coefficients(lhs - lhs, {0.0}),
        "subtracting a polynomial from itself produces canonical zero");

    Polynomial accumulated(std::vector<double>{5.0, -3.0, 2.0});
    accumulated -= rhs;
    tests.check(
        has_coefficients(accumulated, {4.0, -7.0, 4.0, -1.0}),
        "operator-= produces the hand-calculated difference");
}

void test_multiplication(TestRun &tests) {
    // (1 + 2x + 3x^2)(4 - x + 2x^2)
    // = 4 + 7x + 12x^2 + x^3 + 6x^4.
    // Coefficient checks:
    // c0 = 1*4 = 4
    // c1 = 1*(-1) + 2*4 = 7
    // c2 = 1*2 + 2*(-1) + 3*4 = 12
    // c3 = 2*2 + 3*(-1) = 1
    // c4 = 3*2 = 6
    const Polynomial lhs(std::vector<double>{1.0, 2.0, 3.0});
    const Polynomial rhs(std::vector<double>{4.0, -1.0, 2.0});
    const Polynomial product = lhs * rhs;

    tests.check(
        has_coefficients(product, {4.0, 7.0, 12.0, 1.0, 6.0}),
        "multiplication matches a hand-calculated convolution");
    tests.check(
        has_coefficients(lhs, {1.0, 2.0, 3.0}),
        "multiplication does not modify its left operand");
    tests.check(
        has_coefficients(rhs, {4.0, -1.0, 2.0}),
        "multiplication does not modify its right operand");

    // (-2 + 3x^2)(5 - x) = -10 + 2x + 15x^2 - 3x^3.
    const Polynomial sparse(std::vector<double>{-2.0, 0.0, 3.0});
    const Polynomial linear(std::vector<double>{5.0, -1.0});
    tests.check(
        has_coefficients(sparse * linear, {-10.0, 2.0, 15.0, -3.0}),
        "multiplication preserves sparse intermediate coefficients");

    tests.check(
        has_coefficients(lhs * Polynomial(1), {1.0, 2.0, 3.0}),
        "one is the multiplicative identity");
    tests.check(
        has_coefficients(lhs * Polynomial(), {0.0}),
        "zero is absorbing under multiplication");

    Polynomial accumulated(std::vector<double>{1.0, 2.0, 3.0});
    accumulated *= rhs;
    tests.check(
        has_coefficients(accumulated, {4.0, 7.0, 12.0, 1.0, 6.0}),
        "operator*= produces the hand-calculated product");
}

void test_remaining_polynomial_api(TestRun &tests) {
    const Polynomial polynomial(std::vector<double>{2.0, -3.0, 4.0});

    // -2.5(2 - 3x + 4x^2) = -5 + 7.5x - 10x^2.
    tests.check(
        has_coefficients(polynomial.Scale(-2.5), {-5.0, 7.5, -10.0}),
        "Scale multiplies every coefficient by the supplied factor");
    tests.check(
        has_coefficients(polynomial.Scale(0.0), {0.0}),
        "scaling by zero produces canonical zero");

    const Polynomial linear(std::vector<double>{1.0, 2.0});
    tests.check(
        has_coefficients(5 + linear, {6.0, 2.0}),
        "integer plus Polynomial promotes the integer to a constant");
    tests.check(
        has_coefficients(5 - linear, {4.0, -2.0}),
        "integer minus Polynomial preserves operand order");
    tests.check(
        has_coefficients(5 * linear, {5.0, 10.0}),
        "integer times Polynomial scales every coefficient");
    tests.check(
        has_coefficients(2.5 + linear, {3.5, 2.0}),
        "double plus Polynomial promotes the double to a constant");
    tests.check(
        has_coefficients(linear + 2, {3.0, 2.0}),
        "Polynomial plus integer uses constant-polynomial promotion");
    tests.check(
        has_coefficients(linear * 2.0, {2.0, 4.0}),
        "Polynomial times double uses constant-polynomial promotion");

    // 12 / 3 = 4 and (6 - 9x + 3x^2) / 3 = 2 - 3x + x^2.
    tests.check(
        has_coefficients(12 / Polynomial(3), {4.0}),
        "integer divided by a constant Polynomial returns the quotient");
    tests.check(
        has_coefficients(
            Polynomial(std::vector<double>{6.0, -9.0, 3.0}) / 3,
            {2.0, -3.0, 1.0}),
        "Polynomial divided by an integer uses constant-polynomial promotion");

    const Polynomial copied(polynomial);
    tests.check(
        has_coefficients(copied, {2.0, -3.0, 4.0}),
        "copy construction preserves every coefficient");

    Polynomial assigned;
    assigned = polynomial;
    tests.check(
        has_coefficients(assigned, {2.0, -3.0, 4.0}),
        "copy assignment preserves every coefficient");
    assigned = assigned;
    tests.check(
        has_coefficients(assigned, {2.0, -3.0, 4.0}),
        "self-assignment leaves the polynomial unchanged");

    const Polynomial canonical(std::vector<double>{1.0, 2.0});
    const Polynomial trailing_zeros(
        std::vector<double>{1.0, 2.0, 0.0, 0.0});
    const Polynomial different(std::vector<double>{1.0, 3.0});
    tests.check(
        canonical == trailing_zeros,
        "operator== recognizes equal canonical polynomials");
    tests.check(
        canonical != different,
        "operator!= recognizes a different coefficient");
    tests.check(
        !(canonical != trailing_zeros) && !(canonical == different),
        "equality and inequality are logical complements");

    Polynomial expanded(std::vector<double>{2.0});
    expanded.SetCoeffAt(3, 5.0);
    tests.check(
        has_coefficients(expanded, {2.0, 0.0, 0.0, 5.0}),
        "SetCoeffAt expands storage with internal zeros");
    expanded.SetCoeffAt(-1, 99.0);
    tests.check(
        has_coefficients(expanded, {2.0, 0.0, 0.0, 5.0}),
        "SetCoeffAt ignores a negative index");
    tests.check(
        expanded.GetCoeff(-1) == 0.0 && expanded.GetCoeff(99) == 0.0,
        "GetCoeff returns zero outside the represented degree range");
    tests.check(
        expanded.GetDegree() == 3 && Polynomial(7).GetDegree() == 0,
        "GetDegree reports highest represented power and constant degree zero");
}

void test_command_output(TestRun &tests) {
    tests.check(
        dump_output(Polynomial()) == "0\n",
        "Polynomial Dump prints canonical zero");
    tests.check(
        dump_output(Polynomial(std::vector<double>{3.0, -2.0, 0.0, 1.0})) ==
            "3 - 2x + x^3\n",
        "Polynomial Dump formats signs and skips zero terms");
    tests.check(
        dump_output(Polynomial(std::vector<double>{0.0, 1.0, -1.0})) ==
            "x - x^2\n",
        "Polynomial Dump formats unit coefficients and a missing constant");
    tests.check(
        dump_output(Polynomial(std::vector<double>{0.0, 5e-12})) ==
            "5e-12x\n",
        "Polynomial Dump preserves a tiny nonzero coefficient");

    tests.check(
        dump_output(ComplexPoly()) == "0\n",
        "ComplexPoly Dump prints canonical zero");
    tests.check(
        dump_output(ComplexPoly(std::vector<double>{1.0, 0.0, -2.0},
                                std::vector<double>{2.0, 0.0, 1.0})) ==
            "(1+2i) + (-2+i)x^2\n",
        "ComplexPoly Dump formats mixed coefficients and skips zero terms");
    tests.check(
        dump_output(ComplexPoly(std::vector<double>{0.0},
                                std::vector<double>{0.0, -1.0, 2.0})) ==
            "(-i)x + (2i)x^2\n",
        "ComplexPoly Dump formats pure imaginary unit coefficients");
    tests.check(
        dump_output(ComplexPoly(std::vector<double>{5e-12},
                                std::vector<double>{-7e-12})) ==
            "(5e-12-7e-12i)\n",
        "ComplexPoly Dump preserves tiny nonzero coefficients");
}

template <typename Operation>
bool throws_domain_error(Operation operation) {
    try {
        operation();
    } catch (const std::domain_error &) {
        return true;
    } catch (...) {
        return false;
    }

    return false;
}

void test_division(TestRun &tests) {
    // x^3 - 2x^2 - 4 = (x - 3)(x^2 + x + 3) + 5.
    const Polynomial dividend(std::vector<double>{-4.0, 0.0, -2.0, 1.0});
    const Polynomial divisor(std::vector<double>{-3.0, 1.0});
    const Polynomial quotient = dividend / divisor;
    const Polynomial remainder = dividend % divisor;
    const auto division = DivMod(dividend, divisor);

    tests.check(
        has_coefficients(quotient, {3.0, 1.0, 1.0}),
        "division returns the hand-calculated quotient x^2 + x + 3");
    tests.check(
        has_coefficients(remainder, {5.0}),
        "division returns the hand-calculated remainder 5");
    tests.check(
        has_coefficients(division.first, {3.0, 1.0, 1.0}),
        "DivMod returns the hand-calculated quotient");
    tests.check(
        has_coefficients(division.second, {5.0}),
        "DivMod returns the hand-calculated remainder");
    tests.check(
        has_coefficients(divisor * quotient + remainder,
                         {-4.0, 0.0, -2.0, 1.0}),
        "divisor * quotient + remainder reconstructs the dividend");
    tests.check(
        has_coefficients(dividend, {-4.0, 0.0, -2.0, 1.0}),
        "division does not modify the dividend");
    tests.check(
        has_coefficients(divisor, {-3.0, 1.0}),
        "division does not modify the divisor");

    // 2x^2 + 5x + 3 = (2x + 3)(x + 1), with remainder 0.
    const Polynomial exact_dividend(std::vector<double>{3.0, 5.0, 2.0});
    const Polynomial exact_divisor(std::vector<double>{3.0, 2.0});
    tests.check(
        has_coefficients(exact_dividend / exact_divisor, {1.0, 1.0}),
        "exact division returns the hand-calculated quotient x + 1");
    tests.check(
        has_coefficients(exact_dividend % exact_divisor, {0.0}),
        "exact division has canonical zero remainder");

    // (6 - 9x + 3x^2) / 3 = 2 - 3x + x^2, remainder 0.
    const Polynomial constant_dividend(
        std::vector<double>{6.0, -9.0, 3.0});
    const Polynomial three(3);
    tests.check(
        has_coefficients(constant_dividend / three, {2.0, -3.0, 1.0}),
        "division by a nonzero constant scales every coefficient");
    tests.check(
        has_coefficients(constant_dividend % three, {0.0}),
        "division by a nonzero constant has zero remainder");

    // Let B = 2 - x + x^2, Q = -3 + 2x^2 - x^3, R = 5 - 2x.
    // Hand convolution gives BQ = -6 + 3x + x^2 - 4x^3 + 3x^4 - x^5,
    // so A = BQ + R = -1 + x + x^2 - 4x^3 + 3x^4 - x^5.
    const Polynomial higher_dividend(
        std::vector<double>{-1.0, 1.0, 1.0, -4.0, 3.0, -1.0});
    const Polynomial higher_divisor(std::vector<double>{2.0, -1.0, 1.0});
    const auto higher_division = DivMod(higher_dividend, higher_divisor);
    tests.check(
        has_coefficients(higher_division.first, {-3.0, 0.0, 2.0, -1.0}),
        "higher-degree division returns the hand-calculated quotient");
    tests.check(
        has_coefficients(higher_division.second, {5.0, -2.0}),
        "higher-degree division returns the hand-calculated remainder");

    // Degree 1 divided by degree 2 has quotient 0 and unchanged remainder.
    const Polynomial lower_degree(std::vector<double>{2.0, 1.0});
    const Polynomial higher_degree(std::vector<double>{1.0, 0.0, 1.0});
    tests.check(
        has_coefficients(lower_degree / higher_degree, {0.0}),
        "a lower-degree dividend has quotient zero");
    tests.check(
        has_coefficients(lower_degree % higher_degree, {2.0, 1.0}),
        "a lower-degree dividend is its own remainder");

    const Polynomial zero;
    tests.check(
        has_coefficients(zero / divisor, {0.0}),
        "zero divided by a nonzero polynomial has quotient zero");
    tests.check(
        has_coefficients(zero % divisor, {0.0}),
        "zero divided by a nonzero polynomial has remainder zero");

    Polynomial quotient_assigned = dividend;
    quotient_assigned /= divisor;
    tests.check(
        has_coefficients(quotient_assigned, {3.0, 1.0, 1.0}),
        "operator/= stores the hand-calculated quotient");

    Polynomial remainder_assigned = dividend;
    remainder_assigned %= divisor;
    tests.check(
        has_coefficients(remainder_assigned, {5.0}),
        "operator%= stores the hand-calculated remainder");

    // A previous remainder calculation must not affect a later one.
    Polynomial reused_dividend = dividend;
    (void)(reused_dividend % divisor);
    const Polynomial degree_four_divisor(
        std::vector<double>{1.0, 0.0, 0.0, 0.0, 1.0});
    tests.check(
        has_coefficients(reused_dividend % degree_four_divisor,
                         {-4.0, 0.0, -2.0, 1.0}),
        "remainder does not depend on a previous division call");

    tests.check(
        throws_domain_error([&] { (void)(dividend / zero); }),
        "division by the zero polynomial throws domain_error");
    tests.check(
        throws_domain_error([&] { (void)(dividend % zero); }),
        "remainder by the zero polynomial throws domain_error");
}

void test_complex_representation_and_arithmetic(TestRun &tests) {
    const ComplexPoly zero;
    tests.check(
        has_complex_coefficients(zero, {0.0}, {0.0}),
        "ComplexPoly zero has canonical real and imaginary storage");

    const ComplexPoly all_zero(
        std::vector<double>{0.0, 0.0},
        std::vector<double>{0.0, 0.0, 0.0});
    tests.check(
        has_complex_coefficients(all_zero, {0.0}, {0.0}),
        "all-zero complex input has canonical storage");

    const ComplexPoly empty_parts(
        std::vector<double>{},
        std::vector<double>{});
    tests.check(
        has_complex_coefficients(empty_parts, {0.0}, {0.0}),
        "empty real and imaginary inputs normalize to complex zero");

    tests.check(
        zero.GetCoeff(-1) == 0.0 && zero.GetCoeff(4) == 0.0 &&
            zero.getComplexCoeff(-1) == 0.0 &&
            zero.getComplexCoeff(4) == 0.0,
        "ComplexPoly accessors return zero outside the represented range");

    const ComplexPoly tiny(
        std::vector<double>{0.0, 5e-12},
        std::vector<double>{-7e-12});
    tests.check(
        has_complex_coefficients(tiny, {0.0, 5e-12}, {-7e-12}),
        "ComplexPoly preserves tiny nonzero real and imaginary coefficients");
    tests.check(
        tiny.GetCoeff(1) == 5e-12 && tiny.getComplexCoeff(0) == -7e-12,
        "ComplexPoly accessors preserve tiny nonzero coefficients");

    // A = (1 + 3i) + 2x.
    // B = (-4 + 2i) - ix + 5x^2.
    const ComplexPoly lhs(
        std::vector<double>{1.0, 2.0},
        std::vector<double>{3.0});
    const ComplexPoly rhs(
        std::vector<double>{-4.0, 0.0, 5.0},
        std::vector<double>{2.0, -1.0});

    tests.check(
        has_complex_coefficients(lhs + rhs,
                                 {-3.0, 2.0, 5.0},
                                 {5.0, -1.0}),
        "ComplexPoly addition combines real and imaginary coefficients");
    tests.check(
        has_complex_coefficients(lhs - rhs,
                                 {5.0, 2.0, -5.0},
                                 {1.0, 1.0}),
        "ComplexPoly subtraction combines real and imaginary coefficients");
    tests.check(
        has_complex_coefficients(lhs,
                                 {1.0, 2.0},
                                 {3.0}),
        "ComplexPoly arithmetic does not modify its left operand");
    tests.check(
        has_complex_coefficients(rhs,
                                 {-4.0, 0.0, 5.0},
                                 {2.0, -1.0}),
        "ComplexPoly arithmetic does not modify its right operand");

    // Adding 2 - 2x changes only A's real part:
    // ((1 + 3i) + 2x) + (2 - 2x) = 3 + 3i.
    tests.check(
        has_complex_coefficients(
            lhs + Polynomial(std::vector<double>{2.0, -2.0}),
            {3.0},
            {3.0}),
        "Polynomial promotion changes only the ComplexPoly real part");

    // For C = 1 + i and P = 2:
    // C + P = P + C = 3 + i,
    // C - P = -1 + i, P - C = 1 - i,
    // CP = PC = 2 + 2i,
    // C / P = 0.5 + 0.5i, and P / C = 1 - i.
    const ComplexPoly mixed_complex(
        std::vector<double>{1.0},
        std::vector<double>{1.0});
    const Polynomial mixed_real(2.0);

    tests.check(
        has_complex_coefficients(mixed_complex + mixed_real,
                                 {3.0},
                                 {1.0}),
        "ComplexPoly + Polynomial preserves the imaginary part");
    tests.check(
        has_complex_coefficients(mixed_real + mixed_complex,
                                 {3.0},
                                 {1.0}),
        "Polynomial + ComplexPoly uses the mixed left-hand overload");
    tests.check(
        has_complex_coefficients(mixed_complex - mixed_real,
                                 {-1.0},
                                 {1.0}),
        "ComplexPoly - Polynomial preserves operand order");
    tests.check(
        has_complex_coefficients(mixed_real - mixed_complex,
                                 {1.0},
                                 {-1.0}),
        "Polynomial - ComplexPoly preserves operand order");
    tests.check(
        has_complex_coefficients(mixed_complex * mixed_real,
                                 {2.0},
                                 {2.0}),
        "ComplexPoly * Polynomial scales both parts");
    tests.check(
        has_complex_coefficients(mixed_real * mixed_complex,
                                 {2.0},
                                 {2.0}),
        "Polynomial * ComplexPoly uses the mixed left-hand overload");
    tests.check(
        has_complex_coefficients(mixed_complex / mixed_real,
                                 {0.5},
                                 {0.5}),
        "ComplexPoly / Polynomial divides both parts by the real constant");
    tests.check(
        has_complex_coefficients(mixed_real / mixed_complex,
                                 {1.0},
                                 {-1.0}),
        "Polynomial / ComplexPoly preserves operand order");

    // (1 + 2i)(3 - 4i) = 11 + 2i.
    const ComplexPoly first_constant(
        std::vector<double>{1.0},
        std::vector<double>{2.0});
    const ComplexPoly second_constant(
        std::vector<double>{3.0},
        std::vector<double>{-4.0});
    tests.check(
        has_complex_coefficients(first_constant * second_constant,
                                 {11.0},
                                 {2.0}),
        "ComplexPoly constant multiplication follows i^2 = -1");

    // Let C = (1 + 2x) + i(3 - x), D = (-2 + x) + i(4 + 2x).
    // AC - BD = -14 - 5x + 4x^2.
    // AD + BC = -2 + 15x + 3x^2.
    const ComplexPoly first_polynomial(
        std::vector<double>{1.0, 2.0},
        std::vector<double>{3.0, -1.0});
    const ComplexPoly second_polynomial(
        std::vector<double>{-2.0, 1.0},
        std::vector<double>{4.0, 2.0});
    tests.check(
        has_complex_coefficients(first_polynomial * second_polynomial,
                                 {-14.0, -5.0, 4.0},
                                 {-2.0, 15.0, 3.0}),
        "ComplexPoly multiplication matches hand-calculated polynomial parts");

    // (ix^2)(ix^3) = i^2 x^5 = -x^5.
    const ComplexPoly imaginary_x_squared(
        std::vector<double>{0.0},
        std::vector<double>{0.0, 0.0, 1.0});
    const ComplexPoly imaginary_x_cubed(
        std::vector<double>{0.0},
        std::vector<double>{0.0, 0.0, 0.0, 1.0});
    tests.check(
        has_complex_coefficients(imaginary_x_squared * imaginary_x_cubed,
                                 {0.0, 0.0, 0.0, 0.0, 0.0, -1.0},
                                 {0.0}),
        "ComplexPoly multiplication supports imaginary degree above real degree");

    tests.check(
        zero.GetDegree() == 0 && imaginary_x_cubed.GetDegree() == 3,
        "ComplexPoly degree uses the highest real or imaginary power");

    const ComplexPoly canonical_value(
        std::vector<double>{2.0, -1.0},
        std::vector<double>{3.0});
    const ComplexPoly same_value_with_trailing_zeros(
        std::vector<double>{2.0, -1.0, 0.0},
        std::vector<double>{3.0, 0.0});
    const ComplexPoly different_value(
        std::vector<double>{2.0, -1.0},
        std::vector<double>{-3.0});
    tests.check(
        canonical_value == same_value_with_trailing_zeros,
        "ComplexPoly equality recognizes equal canonical values");
    tests.check(
        canonical_value != different_value,
        "ComplexPoly inequality detects a different imaginary coefficient");

    ComplexPoly assigned;
    assigned = canonical_value;
    tests.check(
        assigned == canonical_value,
        "ComplexPoly copy assignment preserves both coefficient parts");

    // C = (2 - x^2) + i(-3 + x + 4x^3) deliberately gives the real and
    // imaginary parts different degrees and includes interior zeros.
    const ComplexPoly edge_value(
        std::vector<double>{2.0, 0.0, -1.0},
        std::vector<double>{-3.0, 1.0, 0.0, 4.0});
    const ComplexPoly complex_zero;
    const ComplexPoly complex_one(
        std::vector<double>{1.0},
        std::vector<double>{0.0});

    tests.check(
        edge_value + complex_zero == edge_value &&
            complex_zero + edge_value == edge_value,
        "complex zero is the additive identity in both operand positions");
    tests.check(
        edge_value * complex_one == edge_value &&
            complex_one * edge_value == edge_value,
        "complex one is the multiplicative identity in both operand positions");
    tests.check(
        has_complex_coefficients(edge_value * complex_zero,
                                 {0.0},
                                 {0.0}) &&
            has_complex_coefficients(complex_zero * edge_value,
                                     {0.0},
                                     {0.0}),
        "complex zero absorbs multiplication in both operand positions");
    tests.check(
        has_complex_coefficients(edge_value - edge_value,
                                 {0.0},
                                 {0.0}),
        "ComplexPoly self-subtraction produces canonical complex zero");

    const ComplexPoly copy_constructed(edge_value);
    tests.check(
        copy_constructed == edge_value,
        "ComplexPoly copy construction preserves both coefficient parts");

    ComplexPoly self_assigned(edge_value);
    self_assigned = self_assigned;
    tests.check(
        self_assigned == edge_value,
        "ComplexPoly self-assignment leaves the value unchanged");

    // (1 + ix^50)(1 - ix^50)
    // = 1 - ix^50 + ix^50 - i^2*x^100 = 1 + x^100.
    std::vector<double> positive_x50(51, 0.0);
    positive_x50[50] = 1.0;
    std::vector<double> negative_x50(51, 0.0);
    negative_x50[50] = -1.0;

    const ComplexPoly sparse_left(
        std::vector<double>{1.0},
        positive_x50);
    const ComplexPoly sparse_right(
        std::vector<double>{1.0},
        negative_x50);
    const ComplexPoly sparse_product = sparse_left * sparse_right;

    std::vector<double> expected_sparse_real(101, 0.0);
    expected_sparse_real[0] = 1.0;
    expected_sparse_real[100] = 1.0;
    tests.check(
        sparse_product.getListCoeffsIn() == expected_sparse_real &&
            sparse_product.getcomplexCoeffsList() ==
                std::vector<double>{0.0},
        "sparse high-degree multiplication cancels cross terms exactly");
}

void test_complex_division(TestRun &tests) {
    // Let D = (1 + i) + (1 + i)x and Q = (2 - i) + (1 + 2i)x.
    // Coefficient products are:
    // d0*q0 = (1 + i)(2 - i) = 3 + i,
    // d0*q1 + d1*q0 = (-1 + 3i) + (3 + i) = 2 + 4i,
    // d1*q1 = (1 + i)(1 + 2i) = -1 + 3i.
    // Therefore DQ = (3 + i) + (2 + 4i)x + (-1 + 3i)x^2.
    const ComplexPoly divisor(
        std::vector<double>{1.0, 1.0},
        std::vector<double>{1.0, 1.0});
    const ComplexPoly expected_quotient(
        std::vector<double>{2.0, 1.0},
        std::vector<double>{-1.0, 2.0});
    const ComplexPoly exact_dividend(
        std::vector<double>{3.0, 2.0, -1.0},
        std::vector<double>{1.0, 4.0, 3.0});

    // Adding R = 3 - 2i creates a non-exact division whose remainder degree
    // is lower than degree(D): A = DQ + R.
    const ComplexPoly expected_remainder(
        std::vector<double>{3.0},
        std::vector<double>{-2.0});
    const ComplexPoly dividend(
        std::vector<double>{6.0, 2.0, -1.0},
        std::vector<double>{-1.0, 4.0, 3.0});
    const auto division = DivMod(dividend, divisor);

    tests.check(
        division.first == expected_quotient,
        "Complex DivMod returns the hand-calculated quotient");
    tests.check(
        division.second == expected_remainder,
        "Complex DivMod returns the hand-calculated remainder");
    tests.check(
        dividend / divisor == expected_quotient,
        "ComplexPoly operator/ returns the Euclidean quotient");
    tests.check(
        dividend % divisor == expected_remainder,
        "ComplexPoly operator% returns the Euclidean remainder");
    tests.check(
        divisor * division.first + division.second == dividend,
        "complex divisor * quotient + remainder reconstructs the dividend");
    tests.check(
        division.second.GetDegree() < divisor.GetDegree(),
        "complex remainder degree is below the divisor degree");

    const auto exact_division = DivMod(exact_dividend, divisor);
    tests.check(
        exact_division.first == expected_quotient,
        "exact ComplexPoly division returns the hand-calculated quotient");
    tests.check(
        has_complex_coefficients(exact_division.second, {0.0}, {0.0}),
        "exact ComplexPoly division has canonical zero remainder");

    // (1 + ix)(1 - ix) = 1 + x^2 because the x terms cancel and
    // i(-i)x^2 = x^2. The divisor's leading coefficient is purely imaginary.
    const ComplexPoly imaginary_leading_divisor(
        std::vector<double>{1.0},
        std::vector<double>{0.0, 1.0});
    const ComplexPoly imaginary_leading_dividend(
        std::vector<double>{1.0, 0.0, 1.0},
        std::vector<double>{0.0});
    const ComplexPoly imaginary_leading_quotient(
        std::vector<double>{1.0},
        std::vector<double>{0.0, -1.0});
    const auto imaginary_leading_division =
        DivMod(imaginary_leading_dividend, imaginary_leading_divisor);
    tests.check(
        imaginary_leading_division.first == imaginary_leading_quotient,
        "Complex division supports a purely imaginary leading coefficient");
    tests.check(
        has_complex_coefficients(imaginary_leading_division.second,
                                 {0.0},
                                 {0.0}),
        "pure-imaginary-leading exact division has zero remainder");
    tests.check(
        imaginary_leading_divisor * imaginary_leading_division.first ==
            imaginary_leading_dividend,
        "pure-imaginary-leading division reconstructs the dividend");

    // (11 + 2i) / (3 - 4i) = 1 + 2i because
    // (1 + 2i)(3 - 4i) = 11 + 2i.
    const ComplexPoly constant_dividend(
        std::vector<double>{11.0},
        std::vector<double>{2.0});
    const ComplexPoly constant_divisor(
        std::vector<double>{3.0},
        std::vector<double>{-4.0});
    tests.check(
        has_complex_coefficients(constant_dividend / constant_divisor,
                                 {1.0},
                                 {2.0}),
        "division by a complex constant uses scalar complex division");
    tests.check(
        has_complex_coefficients(constant_dividend % constant_divisor,
                                 {0.0},
                                 {0.0}),
        "division by a nonzero complex constant has zero remainder");

    // 1 / (1 + i) = (1 - i) / 2 = 0.5 - 0.5i.
    const ComplexPoly fractional_dividend(
        std::vector<double>{1.0},
        std::vector<double>{0.0});
    const ComplexPoly fractional_divisor(
        std::vector<double>{1.0},
        std::vector<double>{1.0});
    tests.check(
        has_complex_coefficients(fractional_dividend / fractional_divisor,
                                 {0.5},
                                 {-0.5}),
        "division by a complex constant returns an exact fractional quotient");

    // Let P = 1 + x and Q = (2 + x) + i(1 - 2x).
    // PQ = (2 + 3x + x^2) + i(1 - x - 2x^2).
    const Polynomial nonconstant_real_divisor(
        std::vector<double>{1.0, 1.0});
    const ComplexPoly expected_complex_quotient(
        std::vector<double>{2.0, 1.0},
        std::vector<double>{1.0, -2.0});
    const ComplexPoly complex_dividend(
        std::vector<double>{2.0, 3.0, 1.0},
        std::vector<double>{1.0, -1.0, -2.0});
    tests.check(
        complex_dividend / nonconstant_real_divisor ==
            expected_complex_quotient,
        "ComplexPoly divided by a nonconstant Polynomial returns the exact quotient");

    // D = (1 + i) + (1 - i)x and Q = (1 - i) + (1 + i)x.
    // Their constant and x^2 coefficients are 2, while the x terms cancel,
    // so DQ = 2 + 2x^2.
    const Polynomial nonconstant_real_dividend(
        std::vector<double>{2.0, 0.0, 2.0});
    const ComplexPoly nonconstant_complex_divisor(
        std::vector<double>{1.0, 1.0},
        std::vector<double>{1.0, -1.0});
    const ComplexPoly expected_mixed_quotient(
        std::vector<double>{1.0, 1.0},
        std::vector<double>{-1.0, 1.0});
    tests.check(
        nonconstant_real_dividend / nonconstant_complex_divisor ==
            expected_mixed_quotient,
        "Polynomial divided by a nonconstant ComplexPoly returns the exact quotient");

    const ComplexPoly lower_degree(
        std::vector<double>{2.0},
        std::vector<double>{3.0});
    const auto lower_division = DivMod(lower_degree, divisor);
    tests.check(
        has_complex_coefficients(lower_division.first, {0.0}, {0.0}),
        "lower-degree ComplexPoly dividend has quotient zero");
    tests.check(
        lower_division.second == lower_degree,
        "lower-degree ComplexPoly dividend is its own remainder");

    const ComplexPoly zero;
    const auto zero_division = DivMod(zero, constant_divisor);
    tests.check(
        has_complex_coefficients(zero_division.first, {0.0}, {0.0}) &&
            has_complex_coefficients(zero_division.second, {0.0}, {0.0}),
        "zero divided by a nonzero ComplexPoly has zero quotient and remainder");

    tests.check(
        throws_domain_error([&] { (void)(dividend / zero); }),
        "ComplexPoly division by zero throws domain_error");
    tests.check(
        throws_domain_error([&] { (void)(dividend % zero); }),
        "ComplexPoly remainder by zero throws domain_error");
}

void test_rational_arithmetic(TestRun &tests) {
    const Rational<Polynomial> default_value;
    tests.check(
        has_coefficients(default_value.GetNumerator(), {0.0}) &&
            has_coefficients(default_value.GetDenominator(), {1.0}),
        "default Rational<Polynomial> is 0/1");

    // A = (1 + x)/(1 - x), B = 2/(1 + x).
    const Rational<Polynomial> lhs(
        Polynomial(std::vector<double>{1.0, 1.0}),
        Polynomial(std::vector<double>{1.0, -1.0}));
    const Rational<Polynomial> rhs(
        Polynomial(2.0),
        Polynomial(std::vector<double>{1.0, 1.0}));

    // A + B has numerator
    // (1 + x)(1 + x) + 2(1 - x) = 3 + x^2,
    // and denominator (1 - x)(1 + x) = 1 - x^2.
    const Rational<Polynomial> sum = lhs + rhs;
    tests.check(
        has_coefficients(sum.GetNumerator(), {3.0, 0.0, 1.0}) &&
            has_coefficients(sum.GetDenominator(), {1.0, 0.0, -1.0}),
        "Rational<Polynomial> addition uses the hand-calculated cross products");

    // A - B has numerator
    // (1 + x)^2 - 2(1 - x) = -1 + 4x + x^2.
    const Rational<Polynomial> difference = lhs - rhs;
    tests.check(
        has_coefficients(difference.GetNumerator(), {-1.0, 4.0, 1.0}) &&
            has_coefficients(difference.GetDenominator(), {1.0, 0.0, -1.0}),
        "Rational<Polynomial> subtraction uses the hand-calculated cross products");

    // AB starts as (2 + 2x)/(1 - x^2), then cancels 1 + x.
    const Rational<Polynomial> product = lhs * rhs;
    tests.check(
        has_coefficients(product.GetNumerator(), {2.0}) &&
            has_coefficients(product.GetDenominator(), {1.0, -1.0}),
        "Rational<Polynomial> multiplication cancels an exact common factor");

    // A/B = (1 + x)^2 / (2 - 2x).
    const Rational<Polynomial> quotient = lhs / rhs;
    tests.check(
        has_coefficients(quotient.GetNumerator(), {1.0, 2.0, 1.0}) &&
            has_coefficients(quotient.GetDenominator(), {2.0, -2.0}),
        "Rational<Polynomial> division multiplies by the reciprocal");

    tests.check(
        has_coefficients(lhs.GetNumerator(), {1.0, 1.0}) &&
            has_coefficients(lhs.GetDenominator(), {1.0, -1.0}) &&
            has_coefficients(rhs.GetNumerator(), {2.0}) &&
            has_coefficients(rhs.GetDenominator(), {1.0, 1.0}),
        "Rational arithmetic does not modify either operand");

    tests.check(
        throws_domain_error([] {
            (void)Rational<Polynomial>(Polynomial(1), Polynomial());
        }),
        "Rational<Polynomial> rejects a zero denominator");

    const Rational<Polynomial> zero_value(Polynomial(), Polynomial(1));
    tests.check(
        throws_domain_error([&] { (void)(lhs / zero_value); }),
        "Rational division rejects a zero numerator in the divisor");

    const Rational<int> one_half(1, 2);
    const Rational<int> one_third(1, 3);
    const Rational<int> five_sixths = one_half + one_third;
    tests.check(
        five_sixths.GetNumerator() == 5 &&
            five_sixths.GetDenominator() == 6,
        "header-defined Rational<T> arithmetic works for int");

    const Rational<double> double_sum =
        Rational<double>(1.0, 2.0) + Rational<double>(1.0, 4.0);
    tests.check(
        double_sum.GetNumerator() == 6.0 &&
            double_sum.GetDenominator() == 8.0,
        "header-defined Rational<T> arithmetic works for double");

    const Rational<float> float_product =
        Rational<float>(1.0F, 2.0F) * Rational<float>(3.0F, 4.0F);
    tests.check(
        float_product.GetNumerator() == 3.0F &&
            float_product.GetDenominator() == 8.0F,
        "header-defined Rational<T> arithmetic works for float");

    const Rational<Polynomial> copied(lhs);
    Rational<Polynomial> assigned;
    assigned = rhs;
    tests.check(
        has_coefficients(copied.GetNumerator(), {1.0, 1.0}) &&
            has_coefficients(copied.GetDenominator(), {1.0, -1.0}),
        "Rational copy construction preserves numerator and denominator");
    tests.check(
        has_coefficients(assigned.GetNumerator(), {2.0}) &&
            has_coefficients(assigned.GetDenominator(), {1.0, 1.0}),
        "Rational copy assignment preserves numerator and denominator");

    const Rational<Polynomial> reducible(
        Polynomial(std::vector<double>{2.0, 3.0, 1.0}),
        Polynomial(std::vector<double>{-3.0, -2.0, 1.0}));
    tests.check(
        has_coefficients(reducible.GetNumerator(), {2.0, 1.0}) &&
            has_coefficients(reducible.GetDenominator(), {-3.0, 1.0}),
        "Rational<Polynomial> constructor cancels an exact common factor");

    const Rational<Polynomial> canonical_zero(
        Polynomial(), Polynomial(std::vector<double>{2.0, 1.0}));
    tests.check(
        has_coefficients(canonical_zero.GetNumerator(), {0.0}) &&
            has_coefficients(canonical_zero.GetDenominator(), {1.0}),
        "zero Rational<Polynomial> canonicalizes to 0/1");

    const Rational<Polynomial> scalar_fraction(
        Polynomial(1), Polynomial(3));
    tests.check(
        has_coefficients(scalar_fraction.GetNumerator(), {1.0}) &&
            has_coefficients(scalar_fraction.GetDenominator(), {3.0}),
        "Rational<Polynomial> preserves a scalar fraction");

    const Rational<Polynomial> reducible_scalar(
        Polynomial(6), Polynomial(3));
    tests.check(
        has_coefficients(reducible_scalar.GetNumerator(), {2.0}) &&
            has_coefficients(reducible_scalar.GetDenominator(), {1.0}),
        "Rational<Polynomial> reduces exact integer scalar content");

    const Rational<Polynomial> polynomial_scalar_content(
        Polynomial(std::vector<double>{-3.0, 6.0}),
        Polynomial(3));
    tests.check(
        has_coefficients(
            polynomial_scalar_content.GetNumerator(), {-1.0, 2.0}) &&
            has_coefficients(
                polynomial_scalar_content.GetDenominator(), {1.0}),
        "Rational<Polynomial> reduces integer content across coefficients");

    const Rational<Polynomial> negative_scalar_denominator(
        Polynomial(1), Polynomial(-2));
    tests.check(
        has_coefficients(
            negative_scalar_denominator.GetNumerator(), {-1.0}) &&
            has_coefficients(
                negative_scalar_denominator.GetDenominator(), {2.0}),
        "Rational<Polynomial> keeps a scalar denominator positive");

    const Rational<Polynomial> decimal_scalar_fraction(
        Polynomial(0.5), Polynomial(1.5));
    tests.check(
        has_coefficients(
            decimal_scalar_fraction.GetNumerator(), {0.5}) &&
            has_coefficients(
                decimal_scalar_fraction.GetDenominator(), {1.5}),
        "Rational<Polynomial> does not guess decimal scalar content");

    const Rational<Polynomial> scaled_common_factor(
        Polynomial(std::vector<double>{2.0, 2.0}),
        Polynomial(std::vector<double>{4.0, 4.0}));
    tests.check(
        has_coefficients(scaled_common_factor.GetNumerator(), {1.0}) &&
            has_coefficients(scaled_common_factor.GetDenominator(), {2.0}),
        "factor cancellation also reduces remaining integer content");

    const Rational<Polynomial> identical_parts(
        Polynomial(std::vector<double>{2.0, 2.0}),
        Polynomial(std::vector<double>{2.0, 2.0}));
    tests.check(
        has_coefficients(identical_parts.GetNumerator(), {1.0}) &&
            has_coefficients(identical_parts.GetDenominator(), {1.0}),
        "identical rational parts reduce to 1/1");

    const Polynomial shared_denominator(
        std::vector<double>{1.0, -1.0});
    const Rational<Polynomial> same_denominator_sum =
        Rational<Polynomial>(
            Polynomial(std::vector<double>{1.0, 1.0}),
            shared_denominator) +
        Rational<Polynomial>(
            Polynomial(std::vector<double>{2.0, 1.0}),
            shared_denominator);
    tests.check(
        has_coefficients(same_denominator_sum.GetNumerator(), {3.0, 2.0}) &&
            has_coefficients(same_denominator_sum.GetDenominator(),
                             {1.0, -1.0}),
        "equal rational denominators are reused instead of multiplied");

    const Rational<Polynomial> near_common_factor(
        Polynomial(std::vector<double>{2.0, 3.0, 1.0}),
        Polynomial(std::vector<double>{-3.0, -2.0, 1.0 + 1e-12}));
    tests.check(
        has_coefficients(near_common_factor.GetNumerator(),
                         {2.0, 3.0, 1.0}) &&
            has_coefficients(near_common_factor.GetDenominator(),
                             {-3.0, -2.0, 1.0 + 1e-12}),
        "near-common floating-point factors are not guessed away");

    const double infinity = std::numeric_limits<double>::infinity();
    const Rational<Polynomial> nonfinite_ratio(
        Polynomial(std::vector<double>{infinity, 1.0}),
        Polynomial(std::vector<double>{infinity, 1.0}));
    tests.check(
        has_coefficients(nonfinite_ratio.GetNumerator(), {infinity, 1.0}) &&
            has_coefficients(nonfinite_ratio.GetDenominator(),
                             {infinity, 1.0}),
        "non-finite coefficients disable rational reduction");
}

void test_rational_complex_and_promotion(TestRun &tests) {
    const Rational<Polynomial> real_rational(
        Polynomial(std::vector<double>{1.0, 2.0}),
        Polynomial(std::vector<double>{3.0, -1.0}));
    const Rational<ComplexPoly> promoted = PromoteToComplex(real_rational);

    tests.check(
        has_complex_coefficients(promoted.GetNumerator(),
                                 {1.0, 2.0},
                                 {0.0}) &&
            has_complex_coefficients(promoted.GetDenominator(),
                                     {3.0, -1.0},
                                     {0.0}),
        "Rational<Polynomial> promotes losslessly to Rational<ComplexPoly>");

    // (1 + i)/(2 - i) + (3 - 2i)/(1 + i)
    // has numerator (1 + i)^2 + (3 - 2i)(2 - i)
    // = 2i + (4 - 7i) = 4 - 5i,
    // and denominator (2 - i)(1 + i) = 3 + i.
    const Rational<ComplexPoly> lhs(
        ComplexPoly(std::vector<double>{1.0}, std::vector<double>{1.0}),
        ComplexPoly(std::vector<double>{2.0}, std::vector<double>{-1.0}));
    const Rational<ComplexPoly> rhs(
        ComplexPoly(std::vector<double>{3.0}, std::vector<double>{-2.0}),
        ComplexPoly(std::vector<double>{1.0}, std::vector<double>{1.0}));
    const Rational<ComplexPoly> sum = lhs + rhs;

    tests.check(
        has_complex_coefficients(sum.GetNumerator(), {4.0}, {-5.0}) &&
            has_complex_coefficients(sum.GetDenominator(), {3.0}, {1.0}),
        "Rational<ComplexPoly> addition uses verified complex arithmetic");

    // A - B = (-4 + 9i)/(3 + i).
    const Rational<ComplexPoly> difference = lhs - rhs;
    tests.check(
        has_complex_coefficients(difference.GetNumerator(), {-4.0}, {9.0}) &&
            has_complex_coefficients(difference.GetDenominator(), {3.0}, {1.0}),
        "Rational<ComplexPoly> subtraction uses verified complex arithmetic");

    // AB = (5 + i)/(3 + i).
    const Rational<ComplexPoly> product = lhs * rhs;
    tests.check(
        has_complex_coefficients(product.GetNumerator(), {5.0}, {1.0}) &&
            has_complex_coefficients(product.GetDenominator(), {3.0}, {1.0}),
        "Rational<ComplexPoly> multiplication uses verified complex arithmetic");

    // A/B = 2i/(4 - 7i).
    const Rational<ComplexPoly> quotient = lhs / rhs;
    tests.check(
        has_complex_coefficients(quotient.GetNumerator(), {0.0}, {2.0}) &&
            has_complex_coefficients(quotient.GetDenominator(), {4.0}, {-7.0}),
        "Rational<ComplexPoly> division uses verified complex arithmetic");

    tests.check(
        has_complex_coefficients(lhs.GetNumerator(), {1.0}, {1.0}) &&
            has_complex_coefficients(lhs.GetDenominator(), {2.0}, {-1.0}) &&
            has_complex_coefficients(rhs.GetNumerator(), {3.0}, {-2.0}) &&
            has_complex_coefficients(rhs.GetDenominator(), {1.0}, {1.0}),
        "Rational<ComplexPoly> arithmetic does not modify either operand");

    const Rational<ComplexPoly> complex_zero(
        ComplexPoly(), ComplexPoly(Polynomial(1)));
    tests.check(
        throws_domain_error([&] { (void)(lhs / complex_zero); }),
        "Rational<ComplexPoly> division rejects a zero rational");

    // R = (1 + x)/(2 - x), promoted to complex coefficients, and
    // C = (3 - 2i)/(1 + i). Neither contains a common polynomial factor.
    const Rational<Polynomial> mixed_real(
        Polynomial(std::vector<double>{1.0, 1.0}),
        Polynomial(std::vector<double>{2.0, -1.0}));
    const Rational<ComplexPoly> mixed_promoted =
        PromoteToComplex(mixed_real);

    const Rational<ComplexPoly> real_minus_complex = mixed_promoted - rhs;
    const Rational<ComplexPoly> complex_minus_real = rhs - mixed_promoted;
    tests.check(
        has_complex_coefficients(real_minus_complex.GetNumerator(),
                                 {-5.0, 4.0},
                                 {5.0, -1.0}) &&
            has_complex_coefficients(real_minus_complex.GetDenominator(),
                                     {2.0, -1.0},
                                     {2.0, -1.0}) &&
            has_complex_coefficients(complex_minus_real.GetNumerator(),
                                     {5.0, -4.0},
                                     {-5.0, 1.0}) &&
            has_complex_coefficients(complex_minus_real.GetDenominator(),
                                     {2.0, -1.0},
                                     {2.0, -1.0}),
        "promoted real and complex rationals subtract correctly in both orders");

    const Rational<ComplexPoly> real_over_complex = mixed_promoted / rhs;
    const Rational<ComplexPoly> complex_over_real = rhs / mixed_promoted;
    tests.check(
        has_complex_coefficients(real_over_complex.GetNumerator(),
                                 {1.0, 1.0},
                                 {1.0, 1.0}) &&
            has_complex_coefficients(real_over_complex.GetDenominator(),
                                     {6.0, -3.0},
                                     {-4.0, 2.0}) &&
            has_complex_coefficients(complex_over_real.GetNumerator(),
                                     {6.0, -3.0},
                                     {-4.0, 2.0}) &&
            has_complex_coefficients(complex_over_real.GetDenominator(),
                                     {1.0, 1.0},
                                     {1.0, 1.0}),
        "promoted real and complex rationals divide correctly in both orders");

    tests.check(
        throws_domain_error([] {
            (void)Rational<ComplexPoly>(ComplexPoly(Polynomial(1)),
                                        ComplexPoly());
        }),
        "Rational<ComplexPoly> rejects a zero denominator");

    // Both parts contain x + i.
    const Rational<ComplexPoly> reducible_complex(
        ComplexPoly(std::vector<double>{0.0, 1.0, 1.0},
                    std::vector<double>{1.0, 1.0}),
        ComplexPoly(std::vector<double>{2.0, 0.0, 1.0},
                    std::vector<double>{0.0, -1.0}));
    tests.check(
        has_complex_coefficients(reducible_complex.GetNumerator(),
                                 {1.0, 1.0},
                                 {0.0}) &&
            has_complex_coefficients(reducible_complex.GetDenominator(),
                                     {0.0, 1.0},
                                     {-2.0}),
        "Rational<ComplexPoly> cancels an exact common complex factor");

    const Rational<ComplexPoly> canonical_complex_zero(
        ComplexPoly(),
        ComplexPoly(std::vector<double>{0.0, 1.0},
                    std::vector<double>{1.0}));
    tests.check(
        has_complex_coefficients(canonical_complex_zero.GetNumerator(),
                                 {0.0},
                                 {0.0}) &&
            has_complex_coefficients(canonical_complex_zero.GetDenominator(),
                                     {1.0},
                                     {0.0}),
        "zero Rational<ComplexPoly> canonicalizes to 0/1");

    const Rational<ComplexPoly> constant_complex_fraction(
        ComplexPoly(std::vector<double>{1.0}, std::vector<double>{1.0}),
        ComplexPoly(std::vector<double>{2.0}, std::vector<double>{-1.0}));
    tests.check(
        has_complex_coefficients(constant_complex_fraction.GetNumerator(),
                                 {1.0},
                                 {1.0}) &&
            has_complex_coefficients(constant_complex_fraction.GetDenominator(),
                                     {2.0},
                                     {-1.0}),
        "Rational<ComplexPoly> preserves a constant complex fraction");

    const Rational<ComplexPoly> reducible_real_scalar(
        ComplexPoly({6.0}), ComplexPoly({3.0}));
    tests.check(
        has_complex_coefficients(reducible_real_scalar.GetNumerator(),
                                 {2.0},
                                 {0.0}) &&
            has_complex_coefficients(reducible_real_scalar.GetDenominator(),
                                     {1.0},
                                     {0.0}),
        "Rational<ComplexPoly> reduces exact real integer content");

    const Rational<ComplexPoly> reducible_complex_scalar_content(
        ComplexPoly(std::vector<double>{2.0},
                    std::vector<double>{2.0}),
        ComplexPoly(std::vector<double>{4.0},
                    std::vector<double>{-2.0}));
    tests.check(
        has_complex_coefficients(
            reducible_complex_scalar_content.GetNumerator(),
            {1.0},
            {1.0}) &&
            has_complex_coefficients(
                reducible_complex_scalar_content.GetDenominator(),
                {2.0},
                {-1.0}),
        "Rational<ComplexPoly> reduces shared integer complex content");

    const Rational<ComplexPoly> negative_real_scalar_denominator(
        ComplexPoly({1.0}), ComplexPoly({-2.0}));
    tests.check(
        has_complex_coefficients(
            negative_real_scalar_denominator.GetNumerator(),
            {-1.0},
            {0.0}) &&
            has_complex_coefficients(
                negative_real_scalar_denominator.GetDenominator(),
                {2.0},
                {0.0}),
        "Rational<ComplexPoly> keeps a real scalar denominator positive");
}

} // namespace

int main() {
    TestRun tests;

    test_representation(tests);
    test_addition(tests);
    test_subtraction(tests);
    test_multiplication(tests);
    test_remaining_polynomial_api(tests);
    test_command_output(tests);
    test_division(tests);
    test_complex_representation_and_arithmetic(tests);
    test_complex_division(tests);
    test_rational_arithmetic(tests);
    test_rational_complex_and_promotion(tests);

    return tests.finish();
}
