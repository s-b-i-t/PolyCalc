#include "Rational.h"
#include <cmath>
#include <cstdint>
#include <numeric>

namespace
{
bool IsFinite(const Polynomial &value)
{
    for (double coefficient : value.getListCoeffsIn())
    {
        if (!std::isfinite(coefficient))
        {
            return false;
        }
    }

    return true;
}

bool IsFinite(const ComplexPoly &value)
{
    for (double coefficient : value.getListCoeffsIn())
    {
        if (!std::isfinite(coefficient))
        {
            return false;
        }
    }

    for (double coefficient : value.getcomplexCoeffsList())
    {
        if (!std::isfinite(coefficient))
        {
            return false;
        }
    }

    return true;
}

Polynomial MakeMonic(const Polynomial &factor)
{
    return factor.Scale(
        1.0 / factor.GetCoeff(factor.GetDegree()));
}

ComplexPoly MakeMonic(const ComplexPoly &factor)
{
    const int degree = factor.GetDegree();
    const ComplexPoly leadingCoefficient(
        std::vector<double>{factor.GetCoeff(degree)},
        std::vector<double>{factor.getComplexCoeff(degree)});

    return factor / leadingCoefficient;
}

bool AccumulateIntegerContent(
    const std::vector<double> &coefficients,
    std::uint64_t &content)
{
    // Above this limit, a double cannot represent every integer exactly.
    constexpr double maximumExactInteger = 9007199254740991.0;

    for (double coefficient : coefficients)
    {
        const double magnitude = std::abs(coefficient);
        if (std::trunc(coefficient) != coefficient ||
            magnitude > maximumExactInteger)
        {
            return false;
        }

        content = std::gcd(
            content,
            static_cast<std::uint64_t>(magnitude));
    }

    return true;
}

bool AccumulateIntegerContent(
    const Polynomial &value,
    std::uint64_t &content)
{
    return AccumulateIntegerContent(value.getListCoeffsIn(), content);
}

bool AccumulateIntegerContent(
    const ComplexPoly &value,
    std::uint64_t &content)
{
    return
        AccumulateIntegerContent(value.getListCoeffsIn(), content) &&
        AccumulateIntegerContent(value.getcomplexCoeffsList(), content);
}

std::vector<double> DivideCoefficients(
    const std::vector<double> &coefficients,
    double divisor)
{
    std::vector<double> quotient = coefficients;
    for (double &coefficient : quotient)
    {
        coefficient /= divisor;
        if (coefficient == 0.0)
        {
            coefficient = 0.0;
        }
    }

    return quotient;
}

void DivideByRealScalar(Polynomial &value, double divisor)
{
    value = Polynomial(DivideCoefficients(
        value.getListCoeffsIn(), divisor));
}

void DivideByRealScalar(ComplexPoly &value, double divisor)
{
    value = ComplexPoly(
        DivideCoefficients(value.getListCoeffsIn(), divisor),
        DivideCoefficients(value.getcomplexCoeffsList(), divisor));
}

bool HasNegativeRealConstant(const Polynomial &value)
{
    return value.GetDegree() == 0 && value.GetCoeff(0) < 0.0;
}

bool HasNegativeRealConstant(const ComplexPoly &value)
{
    return
        value.GetDegree() == 0 &&
        value.getComplexCoeff(0) == 0.0 &&
        value.GetCoeff(0) < 0.0;
}

template <class T>
void ReduceScalarContent(T &numerator, T &denominator)
{
    if (!IsFinite(numerator) || !IsFinite(denominator))
    {
        return;
    }

    // Restrict scalar reduction to exact integers so 1/3 stays fractional.
    std::uint64_t content = 0;
    if (AccumulateIntegerContent(numerator, content) &&
        AccumulateIntegerContent(denominator, content) &&
        content > 1)
    {
        const double divisor = static_cast<double>(content);
        DivideByRealScalar(numerator, divisor);
        DivideByRealScalar(denominator, divisor);
    }

    if (HasNegativeRealConstant(denominator))
    {
        DivideByRealScalar(numerator, -1.0);
        DivideByRealScalar(denominator, -1.0);
    }
}

template <class T>
void ReducePolynomialRatio(T &numerator, T &denominator)
{
    if (!IsFinite(numerator) || !IsFinite(denominator))
    {
        return;
    }

    if (numerator == denominator)
    {
        numerator = T{1};
        denominator = T{1};
        return;
    }

    T left = numerator;
    T right = denominator;
    int remainingSteps =
        left.GetDegree() + right.GetDegree() + 2;

    // Find a common factor without treating tiny coefficients as zero.
    while (right != T{})
    {
        if (right.GetDegree() == 0 || remainingSteps-- == 0)
        {
            return;
        }

        const auto division = DivMod(left, right);
        if (!IsFinite(division.second))
        {
            return;
        }

        left = right;
        right = division.second;
    }

    // Normalize only the polynomial factor; scalar content is handled later.
    const T commonFactor = MakeMonic(left);
    if (!IsFinite(commonFactor))
    {
        return;
    }

    const auto numeratorDivision = DivMod(numerator, commonFactor);
    const auto denominatorDivision = DivMod(denominator, commonFactor);
    if (!IsFinite(numeratorDivision.first) ||
        !IsFinite(numeratorDivision.second) ||
        !IsFinite(denominatorDivision.first) ||
        !IsFinite(denominatorDivision.second))
    {
        return;
    }

    // DivMod clears leading remainder entries, so verify reconstruction too.
    const T reconstructedNumerator =
        commonFactor * numeratorDivision.first;
    const T reconstructedDenominator =
        commonFactor * denominatorDivision.first;

    if (numeratorDivision.second == T{} &&
        denominatorDivision.second == T{} &&
        IsFinite(reconstructedNumerator) &&
        IsFinite(reconstructedDenominator) &&
        reconstructedNumerator == numerator &&
        reconstructedDenominator == denominator)
    {
        numerator = numeratorDivision.first;
        denominator = denominatorDivision.first;
    }
}
} // namespace

void rational_detail::Reduce(
    Polynomial &numerator,
    Polynomial &denominator)
{
    ReducePolynomialRatio(numerator, denominator);
    ReduceScalarContent(numerator, denominator);
}

void rational_detail::Reduce(
    ComplexPoly &numerator,
    ComplexPoly &denominator)
{
    ReducePolynomialRatio(numerator, denominator);
    ReduceScalarContent(numerator, denominator);
}

template class Rational<Polynomial>;
template class Rational<ComplexPoly>;

