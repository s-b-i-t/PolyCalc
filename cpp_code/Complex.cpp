#include "Polynomial.h"
#include "Complex.h"
#include <vector>
#include <algorithm>
#include <cstddef>
#include <complex>
#include <stdexcept>
using namespace std;

void ComplexPoly::Normalize()
{
    Polynomial realPart(listCoeffsIn);
    Polynomial imaginaryPart(complexCoeffs);

    listCoeffsIn = realPart.getListCoeffsIn();
    complexCoeffs = imaginaryPart.getListCoeffsIn();
}

double ComplexPoly::GetCoeff(int index) const
{
    if (index < 0 || index >= static_cast<int>(listCoeffsIn.size()))
    {
        return 0.0;
    }

    return listCoeffsIn[index];
}


double ComplexPoly::getComplexCoeff(int index) const
{
    if (index < 0 || index >= static_cast<int>(complexCoeffs.size()))
    {
        return 0.0;
    }

    return complexCoeffs[index];
}

int ComplexPoly::GetDegree() const
{
    return static_cast<int>(
        std::max(listCoeffsIn.size(), complexCoeffs.size())) - 1;
}

bool ComplexPoly::operator==(const ComplexPoly &rhs) const
{
    return listCoeffsIn == rhs.listCoeffsIn &&
           complexCoeffs == rhs.complexCoeffs;
}

bool ComplexPoly::operator!=(const ComplexPoly &rhs) const
{
    return !(*this == rhs);
}


ComplexPoly ComplexPoly::operator+(const ComplexPoly &rhs) const{
    Polynomial tmpPoly = Polynomial(this->listCoeffsIn);
    Polynomial tmpPoly2 = Polynomial(rhs.listCoeffsIn);
    tmpPoly += tmpPoly2;

    // now to add the complex parts     
    vector<double> complexCoeffs = this->complexCoeffs;
    vector<double> complexCoeffs2 = rhs.complexCoeffs;

    // first pad smaller vector with zeros
    if (complexCoeffs.size() < complexCoeffs2.size())
    {
        complexCoeffs.resize(complexCoeffs2.size(), 0);
    }
    else if (complexCoeffs.size() > complexCoeffs2.size())
    {
        complexCoeffs2.resize(complexCoeffs.size(), 0);
    }

    // now add the vectors

    for (std::size_t i = 0; i < complexCoeffs.size(); ++i)
    {
        complexCoeffs[i] += complexCoeffs2[i];
    }

    return ComplexPoly(tmpPoly.getListCoeffsIn(), complexCoeffs);



}

ComplexPoly ComplexPoly::operator-(const ComplexPoly &rhs) const{
    // Subtract the real parts using the Polynomial class
    Polynomial tmpPoly = Polynomial(this->listCoeffsIn);
    Polynomial tmpPoly2 = Polynomial(rhs.listCoeffsIn);
    tmpPoly -= tmpPoly2;

    // Handle the complex parts (imaginary coefficients)
    std::vector<double> complexCoeffs = this->complexCoeffs;
    std::vector<double> complexCoeffs2 = rhs.complexCoeffs;

    // Pad the smaller vector with zeros
    if (complexCoeffs.size() < complexCoeffs2.size()) {
        complexCoeffs.resize(complexCoeffs2.size(), 0);
    } else if (complexCoeffs.size() > complexCoeffs2.size()) {
        complexCoeffs2.resize(complexCoeffs.size(), 0);
    }

    // Subtract the imaginary parts
    for (size_t i = 0; i < complexCoeffs.size(); ++i) {
        complexCoeffs[i] -= complexCoeffs2[i];
    }

    // Return the resulting ComplexPoly
    return ComplexPoly(tmpPoly.getListCoeffsIn(), complexCoeffs);
}



ComplexPoly ComplexPoly::operator*(const ComplexPoly &rhs) const{
    Polynomial leftReal(listCoeffsIn);
    Polynomial leftImaginary(complexCoeffs);
    Polynomial rightReal(rhs.listCoeffsIn);
    Polynomial rightImaginary(rhs.complexCoeffs);

    Polynomial resultReal =
        leftReal * rightReal - leftImaginary * rightImaginary;
    Polynomial resultImaginary =
        leftReal * rightImaginary + leftImaginary * rightReal;

    return ComplexPoly(resultReal.getListCoeffsIn(),
                       resultImaginary.getListCoeffsIn());
}

std::pair<ComplexPoly, ComplexPoly> DivMod(
    const ComplexPoly &dividend,
    const ComplexPoly &divisor)
{
    if (divisor == ComplexPoly())
    {
        throw std::domain_error("cannot divide by the zero complex polynomial");
    }

    int dividendDegree = dividend.GetDegree();
    int divisorDegree = divisor.GetDegree();

    if (dividendDegree < divisorDegree)
    {
        return {ComplexPoly(), dividend};
    }

    std::vector<double> quotientReal(
        dividendDegree - divisorDegree + 1, 0.0);
    std::vector<double> quotientImaginary(
        dividendDegree - divisorDegree + 1, 0.0);
    std::vector<double> remainderReal = dividend.getListCoeffsIn();
    std::vector<double> remainderImaginary =
        dividend.getcomplexCoeffsList();

    remainderReal.resize(dividendDegree + 1, 0.0);
    remainderImaginary.resize(dividendDegree + 1, 0.0);

    const std::complex<double> leadingDivisor(
        divisor.GetCoeff(divisorDegree),
        divisor.getComplexCoeff(divisorDegree));

    for (int currentDegree = dividendDegree;
         currentDegree >= divisorDegree;
         --currentDegree)
    {
        int quotientDegree = currentDegree - divisorDegree;
        std::complex<double> leadingRemainder(
            remainderReal[currentDegree],
            remainderImaginary[currentDegree]);
        std::complex<double> factor = leadingRemainder / leadingDivisor;

        quotientReal[quotientDegree] = factor.real();
        quotientImaginary[quotientDegree] = factor.imag();

        for (int divisorIndex = 0;
             divisorIndex <= divisorDegree;
             ++divisorIndex)
        {
            std::complex<double> divisorCoefficient(
                divisor.GetCoeff(divisorIndex),
                divisor.getComplexCoeff(divisorIndex));
            std::complex<double> amountToSubtract =
                factor * divisorCoefficient;
            int remainderIndex = quotientDegree + divisorIndex;

            remainderReal[remainderIndex] -= amountToSubtract.real();
            remainderImaginary[remainderIndex] -= amountToSubtract.imag();
        }

        remainderReal[currentDegree] = 0.0;
        remainderImaginary[currentDegree] = 0.0;
    }

    return {
        ComplexPoly(quotientReal, quotientImaginary),
        ComplexPoly(remainderReal, remainderImaginary)};
}

ComplexPoly ComplexPoly::operator/(const ComplexPoly &rhs) const{
    return DivMod(*this, rhs).first;
}

ComplexPoly ComplexPoly::operator%(const ComplexPoly &rhs) const{
    return DivMod(*this, rhs).second;
}

ComplexPoly ComplexPoly::operator+ (const Polynomial &rhs) const{
    ComplexPoly new_complex = ComplexPoly(rhs);
    new_complex = *this + new_complex;
    return new_complex;

}

ComplexPoly ComplexPoly::operator- (const Polynomial &rhs) const{
    ComplexPoly new_complex = ComplexPoly(rhs);
    new_complex = *this - new_complex;
    return new_complex;

}

ComplexPoly ComplexPoly::operator* (const Polynomial &rhs) const{
    ComplexPoly new_complex = ComplexPoly(rhs);
    new_complex =  *this * new_complex;
    return new_complex;

}
ComplexPoly ComplexPoly::operator/ (const Polynomial &rhs) const{
    ComplexPoly new_complex = ComplexPoly(rhs);
    new_complex = *this / new_complex;
    return new_complex;

}



void ComplexPoly::Dump() const
{
    std::size_t coefficientCount =
        std::max(listCoeffsIn.size(), complexCoeffs.size());
    bool firstTerm = true;

    for (std::size_t power = 0; power < coefficientCount; ++power)
    {
        double realPart =
            power < listCoeffsIn.size() ? listCoeffsIn[power] : 0.0;
        double imaginaryPart =
            power < complexCoeffs.size() ? complexCoeffs[power] : 0.0;

        if (realPart == 0.0 && imaginaryPart == 0.0)
        {
            continue;
        }

        if (!firstTerm)
        {
            std::cout << " + ";
        }

        std::cout << "(";
        bool printedRealPart = false;

        if (realPart != 0.0)
        {
            std::cout << realPart;
            printedRealPart = true;
        }

        if (imaginaryPart != 0.0)
        {
            if (printedRealPart && imaginaryPart > 0.0)
            {
                std::cout << "+";
            }

            if (imaginaryPart == 1.0)
            {
                std::cout << "i";
            }
            else if (imaginaryPart == -1.0)
            {
                std::cout << "-i";
            }
            else
            {
                std::cout << imaginaryPart << "i";
            }
        }

        std::cout << ")";

        if (power > 0)
        {
            std::cout << "x";
            if (power > 1)
            {
                std::cout << "^" << power;
            }
        }

        firstTerm = false;
    }

    if (firstTerm)
    {
        std::cout << "0";
    }

    std::cout << std::endl;
}
