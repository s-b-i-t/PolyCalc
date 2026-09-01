#include "wasm_api.h"

#include "Complex.h"
#include "Polynomial.h"
#include "Rational.h"

#include <algorithm>
#include <vector>

namespace
{
ComplexPoly makeComplexPolynomial(
    const double *real,
    std::size_t realSize,
    const double *imaginary,
    std::size_t imaginarySize)
{
    return ComplexPoly(
        std::vector<double>(real, real + realSize),
        std::vector<double>(imaginary, imaginary + imaginarySize));
}

std::size_t copyComplexCoefficients(
    const ComplexPoly &value,
    double *result)
{
    const auto &real = value.getListCoeffsIn();
    const auto &imaginary = value.getcomplexCoeffsList();

    std::copy(real.begin(), real.end(), result);
    std::copy(
        imaginary.begin(),
        imaginary.end(),
        result + real.size());

    return real.size() + imaginary.size();
}

std::size_t packComplexPolynomial(
    const ComplexPoly &value,
    double *result)
{
    const auto &real = value.getListCoeffsIn();
    const auto &imaginary = value.getcomplexCoeffsList();

    result[0] = static_cast<double>(real.size());
    result[1] = static_cast<double>(imaginary.size());
    copyComplexCoefficients(value, result + 2);

    return 2 + real.size() + imaginary.size();
}

struct ComplexPolynomialInput
{
    const double *real;
    std::size_t realSize;
    const double *imaginary;
    std::size_t imaginarySize;
};

Rational<ComplexPoly> makeComplexRational(
    const ComplexPolynomialInput &numerator,
    const ComplexPolynomialInput &denominator)
{
    return Rational<ComplexPoly>(
        makeComplexPolynomial(
            numerator.real,
            numerator.realSize,
            numerator.imaginary,
            numerator.imaginarySize),
        makeComplexPolynomial(
            denominator.real,
            denominator.realSize,
            denominator.imaginary,
            denominator.imaginarySize));
}

std::size_t packComplexRational(
    const Rational<ComplexPoly> &value,
    double *result)
{
    const ComplexPoly &numerator = value.GetNumerator();
    const ComplexPoly &denominator = value.GetDenominator();

    result[0] = static_cast<double>(
        numerator.getListCoeffsIn().size());
    result[1] = static_cast<double>(
        numerator.getcomplexCoeffsList().size());
    result[2] = static_cast<double>(
        denominator.getListCoeffsIn().size());
    result[3] = static_cast<double>(
        denominator.getcomplexCoeffsList().size());

    const std::size_t numeratorSize =
        copyComplexCoefficients(numerator, result + 4);
    const std::size_t denominatorSize = copyComplexCoefficients(
        denominator,
        result + 4 + numeratorSize);

    return 4 + numeratorSize + denominatorSize;
}
}

// extern "C" prevents C++ name mangling at the WASM boundary.
extern "C" std::size_t polycalc_add(const double *lhs,
                                     std::size_t lhsSize,
                                     const double *rhs,
                                     std::size_t rhsSize,
                                     double *result)
{
    Polynomial left(std::vector<double>(lhs, lhs + lhsSize));
    Polynomial right(std::vector<double>(rhs, rhs + rhsSize));
    Polynomial sum = left + right;

    const std::vector<double> &coefficients = sum.getListCoeffsIn();
    std::copy(coefficients.begin(), coefficients.end(), result);
    return coefficients.size();
}

extern "C" std::size_t polycalc_subtract(const double *lhs,
                                         std::size_t lhsSize,
                                         const double *rhs,
                                         std::size_t rhsSize,
                                         double *result)
{
    Polynomial left(std::vector<double>(lhs, lhs + lhsSize));
    Polynomial right(std::vector<double>(rhs, rhs + rhsSize));
    Polynomial diff = left - right;

    const std::vector<double> &coefficients = diff.getListCoeffsIn();
    std::copy(coefficients.begin(), coefficients.end(), result);
    return coefficients.size();
}

extern "C" std::size_t polycalc_multiply(const double *lhs,
                                         std::size_t lhsSize,
                                         const double *rhs,
                                         std::size_t rhsSize,
                                         double *result)
{
    Polynomial left(std::vector<double>(lhs, lhs + lhsSize));
    Polynomial right(std::vector<double>(rhs, rhs + rhsSize));
    Polynomial prod = left * right;

    const std::vector<double> &coefficients = prod.getListCoeffsIn();
    std::copy(coefficients.begin(), coefficients.end(), result);
    return coefficients.size();
}

extern "C" std::size_t polycalc_divmod(
    const double *lhs,
    std::size_t lhsSize,
    const double *rhs,
    std::size_t rhsSize,
    double *result)
{
    Polynomial left(std::vector<double>(lhs, lhs + lhsSize));
    Polynomial right(std::vector<double>(rhs, rhs + rhsSize));

    auto [quotient, remainder] = DivMod(left, right);

    const auto &quotientCoefficients = quotient.getListCoeffsIn();
    const auto &remainderCoefficients = remainder.getListCoeffsIn();

    result[0] = static_cast<double>(quotientCoefficients.size());
    result[1] = static_cast<double>(remainderCoefficients.size());

    std::copy(
        quotientCoefficients.begin(),
        quotientCoefficients.end(),
        result + 2);

    std::copy(
        remainderCoefficients.begin(),
        remainderCoefficients.end(),
        result + 2 + quotientCoefficients.size());

    return 2
        + quotientCoefficients.size()
        + remainderCoefficients.size();
}

extern "C" std::size_t polycalc_complex_add(
    const double *lhsReal,
    std::size_t lhsRealSize,
    const double *lhsImaginary,
    std::size_t lhsImaginarySize,
    const double *rhsReal,
    std::size_t rhsRealSize,
    const double *rhsImaginary,
    std::size_t rhsImaginarySize,
    double *result)
{
    const ComplexPoly left = makeComplexPolynomial(
        lhsReal,
        lhsRealSize,
        lhsImaginary,
        lhsImaginarySize);
    const ComplexPoly right = makeComplexPolynomial(
        rhsReal,
        rhsRealSize,
        rhsImaginary,
        rhsImaginarySize);

    return packComplexPolynomial(left + right, result);
}

extern "C" std::size_t polycalc_complex_subtract(
    const double *lhsReal,
    std::size_t lhsRealSize,
    const double *lhsImaginary,
    std::size_t lhsImaginarySize,
    const double *rhsReal,
    std::size_t rhsRealSize,
    const double *rhsImaginary,
    std::size_t rhsImaginarySize,
    double *result)
{
    const ComplexPoly left = makeComplexPolynomial(
        lhsReal,
        lhsRealSize,
        lhsImaginary,
        lhsImaginarySize);
    const ComplexPoly right = makeComplexPolynomial(
        rhsReal,
        rhsRealSize,
        rhsImaginary,
        rhsImaginarySize);

    return packComplexPolynomial(left - right, result);
}

extern "C" std::size_t polycalc_complex_multiply(
    const double *lhsReal,
    std::size_t lhsRealSize,
    const double *lhsImaginary,
    std::size_t lhsImaginarySize,
    const double *rhsReal,
    std::size_t rhsRealSize,
    const double *rhsImaginary,
    std::size_t rhsImaginarySize,
    double *result)
{
    const ComplexPoly left = makeComplexPolynomial(
        lhsReal,
        lhsRealSize,
        lhsImaginary,
        lhsImaginarySize);
    const ComplexPoly right = makeComplexPolynomial(
        rhsReal,
        rhsRealSize,
        rhsImaginary,
        rhsImaginarySize);

    return packComplexPolynomial(left * right, result);
}

extern "C" std::size_t polycalc_complex_divmod(
    const double *lhsReal,
    std::size_t lhsRealSize,
    const double *lhsImaginary,
    std::size_t lhsImaginarySize,
    const double *rhsReal,
    std::size_t rhsRealSize,
    const double *rhsImaginary,
    std::size_t rhsImaginarySize,
    double *result)
{
    const ComplexPoly left = makeComplexPolynomial(
        lhsReal,
        lhsRealSize,
        lhsImaginary,
        lhsImaginarySize);
    const ComplexPoly right = makeComplexPolynomial(
        rhsReal,
        rhsRealSize,
        rhsImaginary,
        rhsImaginarySize);

    const auto [quotient, remainder] = DivMod(left, right);
    const auto &quotientReal = quotient.getListCoeffsIn();
    const auto &quotientImaginary = quotient.getcomplexCoeffsList();
    const auto &remainderReal = remainder.getListCoeffsIn();
    const auto &remainderImaginary = remainder.getcomplexCoeffsList();

    result[0] = static_cast<double>(quotientReal.size());
    result[1] = static_cast<double>(quotientImaginary.size());
    result[2] = static_cast<double>(remainderReal.size());
    result[3] = static_cast<double>(remainderImaginary.size());

    const std::size_t quotientSize =
        copyComplexCoefficients(quotient, result + 4);
    const std::size_t remainderSize = copyComplexCoefficients(
        remainder,
        result + 4 + quotientSize);

    return 4 + quotientSize + remainderSize;
}

extern "C" std::size_t polycalc_rational_add(
    const double *lhsNumeratorReal,
    std::size_t lhsNumeratorRealSize,
    const double *lhsNumeratorImaginary,
    std::size_t lhsNumeratorImaginarySize,
    const double *lhsDenominatorReal,
    std::size_t lhsDenominatorRealSize,
    const double *lhsDenominatorImaginary,
    std::size_t lhsDenominatorImaginarySize,
    const double *rhsNumeratorReal,
    std::size_t rhsNumeratorRealSize,
    const double *rhsNumeratorImaginary,
    std::size_t rhsNumeratorImaginarySize,
    const double *rhsDenominatorReal,
    std::size_t rhsDenominatorRealSize,
    const double *rhsDenominatorImaginary,
    std::size_t rhsDenominatorImaginarySize,
    double *result)
{
    const Rational<ComplexPoly> left = makeComplexRational(
        {lhsNumeratorReal, lhsNumeratorRealSize,
         lhsNumeratorImaginary, lhsNumeratorImaginarySize},
        {lhsDenominatorReal, lhsDenominatorRealSize,
         lhsDenominatorImaginary, lhsDenominatorImaginarySize});
    const Rational<ComplexPoly> right = makeComplexRational(
        {rhsNumeratorReal, rhsNumeratorRealSize,
         rhsNumeratorImaginary, rhsNumeratorImaginarySize},
        {rhsDenominatorReal, rhsDenominatorRealSize,
         rhsDenominatorImaginary, rhsDenominatorImaginarySize});

    return packComplexRational(left + right, result);
}

extern "C" std::size_t polycalc_rational_subtract(
    const double *lhsNumeratorReal,
    std::size_t lhsNumeratorRealSize,
    const double *lhsNumeratorImaginary,
    std::size_t lhsNumeratorImaginarySize,
    const double *lhsDenominatorReal,
    std::size_t lhsDenominatorRealSize,
    const double *lhsDenominatorImaginary,
    std::size_t lhsDenominatorImaginarySize,
    const double *rhsNumeratorReal,
    std::size_t rhsNumeratorRealSize,
    const double *rhsNumeratorImaginary,
    std::size_t rhsNumeratorImaginarySize,
    const double *rhsDenominatorReal,
    std::size_t rhsDenominatorRealSize,
    const double *rhsDenominatorImaginary,
    std::size_t rhsDenominatorImaginarySize,
    double *result)
{
    const Rational<ComplexPoly> left = makeComplexRational(
        {lhsNumeratorReal, lhsNumeratorRealSize,
         lhsNumeratorImaginary, lhsNumeratorImaginarySize},
        {lhsDenominatorReal, lhsDenominatorRealSize,
         lhsDenominatorImaginary, lhsDenominatorImaginarySize});
    const Rational<ComplexPoly> right = makeComplexRational(
        {rhsNumeratorReal, rhsNumeratorRealSize,
         rhsNumeratorImaginary, rhsNumeratorImaginarySize},
        {rhsDenominatorReal, rhsDenominatorRealSize,
         rhsDenominatorImaginary, rhsDenominatorImaginarySize});

    return packComplexRational(left - right, result);
}

extern "C" std::size_t polycalc_rational_multiply(
    const double *lhsNumeratorReal,
    std::size_t lhsNumeratorRealSize,
    const double *lhsNumeratorImaginary,
    std::size_t lhsNumeratorImaginarySize,
    const double *lhsDenominatorReal,
    std::size_t lhsDenominatorRealSize,
    const double *lhsDenominatorImaginary,
    std::size_t lhsDenominatorImaginarySize,
    const double *rhsNumeratorReal,
    std::size_t rhsNumeratorRealSize,
    const double *rhsNumeratorImaginary,
    std::size_t rhsNumeratorImaginarySize,
    const double *rhsDenominatorReal,
    std::size_t rhsDenominatorRealSize,
    const double *rhsDenominatorImaginary,
    std::size_t rhsDenominatorImaginarySize,
    double *result)
{
    const Rational<ComplexPoly> left = makeComplexRational(
        {lhsNumeratorReal, lhsNumeratorRealSize,
         lhsNumeratorImaginary, lhsNumeratorImaginarySize},
        {lhsDenominatorReal, lhsDenominatorRealSize,
         lhsDenominatorImaginary, lhsDenominatorImaginarySize});
    const Rational<ComplexPoly> right = makeComplexRational(
        {rhsNumeratorReal, rhsNumeratorRealSize,
         rhsNumeratorImaginary, rhsNumeratorImaginarySize},
        {rhsDenominatorReal, rhsDenominatorRealSize,
         rhsDenominatorImaginary, rhsDenominatorImaginarySize});

    return packComplexRational(left * right, result);
}

extern "C" std::size_t polycalc_rational_divide(
    const double *lhsNumeratorReal,
    std::size_t lhsNumeratorRealSize,
    const double *lhsNumeratorImaginary,
    std::size_t lhsNumeratorImaginarySize,
    const double *lhsDenominatorReal,
    std::size_t lhsDenominatorRealSize,
    const double *lhsDenominatorImaginary,
    std::size_t lhsDenominatorImaginarySize,
    const double *rhsNumeratorReal,
    std::size_t rhsNumeratorRealSize,
    const double *rhsNumeratorImaginary,
    std::size_t rhsNumeratorImaginarySize,
    const double *rhsDenominatorReal,
    std::size_t rhsDenominatorRealSize,
    const double *rhsDenominatorImaginary,
    std::size_t rhsDenominatorImaginarySize,
    double *result)
{
    const Rational<ComplexPoly> left = makeComplexRational(
        {lhsNumeratorReal, lhsNumeratorRealSize,
         lhsNumeratorImaginary, lhsNumeratorImaginarySize},
        {lhsDenominatorReal, lhsDenominatorRealSize,
         lhsDenominatorImaginary, lhsDenominatorImaginarySize});
    const Rational<ComplexPoly> right = makeComplexRational(
        {rhsNumeratorReal, rhsNumeratorRealSize,
         rhsNumeratorImaginary, rhsNumeratorImaginarySize},
        {rhsDenominatorReal, rhsDenominatorRealSize,
         rhsDenominatorImaginary, rhsDenominatorImaginarySize});

    return packComplexRational(left / right, result);
}
