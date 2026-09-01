//  Rational.h
// works for Polynomial, ComplexPoly, int, double, float
// polynomial ratios reduce exact integer content and verified common factors

#ifndef Rational_h
#define Rational_h
#include "Polynomial.h"
#include "Complex.h"
#include <stdexcept>

namespace rational_detail
{
template <class T>
void Reduce(T &, T &)
{
}

void Reduce(Polynomial &numerator, Polynomial &denominator);
void Reduce(ComplexPoly &numerator, ComplexPoly &denominator);
} // namespace rational_detail


template <class T>
class Rational
{
public:
    Rational() : numerator(T{}), denominator(T{1}) {}
    
    Rational(const T &numeratorIn, const T &denominatorIn = T{1})
    : numerator(numeratorIn), denominator(denominatorIn)
    {
        if (denominator == T{})
        {
            throw std::domain_error("rational denominator cannot be zero");
        }

        if (numerator == T{})
        {
            denominator = T{1};
        }
        else
        {
            rational_detail::Reduce(numerator, denominator);
        }
    }

    
    // copy constructor
    Rational(const Rational<T> &object) = default;
 
    // assignment operator
    Rational<T>& operator=(const Rational &rhs) = default;
    
    Rational<T> operator+(const Rational &rhs) const
    {
        if (denominator == rhs.denominator)
        {
            return Rational<T>(numerator + rhs.numerator, denominator);
        }

        return Rational<T>(
            numerator * rhs.denominator + rhs.numerator * denominator,
            denominator * rhs.denominator);
    }
    
    Rational<T> operator-(const Rational &rhs) const
    {
        if (denominator == rhs.denominator)
        {
            return Rational<T>(numerator - rhs.numerator, denominator);
        }

        return Rational<T>(
            numerator * rhs.denominator - rhs.numerator * denominator,
            denominator * rhs.denominator);
    }

    Rational<T> operator*(const Rational &rhs) const
    {
        return Rational<T>(numerator * rhs.numerator,
                           denominator * rhs.denominator);
    }

    Rational<T> operator/(const Rational &rhs) const
    {
        if (rhs.numerator == T{})
        {
            throw std::domain_error("cannot divide by a zero rational");
        }

        return Rational<T>(numerator * rhs.denominator,
                           denominator * rhs.numerator);
    }

    



    // access numerator and denominator
    const T &GetNumerator() const {return numerator;}
    const T &GetDenominator() const {return denominator;}
    
private:

    T numerator;
    T denominator;
};

inline Rational<ComplexPoly> PromoteToComplex(
    const Rational<Polynomial> &value)
{
    return Rational<ComplexPoly>(
        ComplexPoly(value.GetNumerator()),
        ComplexPoly(value.GetDenominator()));
}


#endif /* Rational_h */
