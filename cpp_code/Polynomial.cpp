#include "Polynomial.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstddef>
#include <stdexcept>

using namespace std;
// Polynomial ::  Polynomial(const std::vector<double> &listCoeffsIn){}

// Polynomial ::  Polynomial(const Polynomial &rhs){}

// Polynomial::Polynomial(double coeff) : listCoeffsIn({coeff}) {}

double Polynomial::GetCoeff(int index) const
{
    if (index < 0 || index >= static_cast<int>(listCoeffsIn.size()))
    {
        return 0.0;
    }

    return listCoeffsIn[index];
}

void Polynomial::SetCoeffAt(int index, double value)
{
    if (index < 0)
    {
        return;
    }

    if (index >= static_cast<int>(listCoeffsIn.size()))
    {
        listCoeffsIn.resize(index + 1, 0.0);
    }

    listCoeffsIn[index] = value;
    Normalize();
}

// cast from size_t to int
int Polynomial::GetDegree() const
{
    return static_cast<int>(listCoeffsIn.size()) - 1;
}

// scale function
Polynomial Polynomial ::Scale(double factor) const 
{
    vector<double> scaledCoeffs = listCoeffsIn;
    for (auto &x : scaledCoeffs)
    {
        x *= factor;
    }
    return Polynomial(scaledCoeffs);
}


void Polynomial::Normalize(){
    while (listCoeffsIn.size() > 1 && listCoeffsIn.back() == 0.0){
        listCoeffsIn.pop_back();
    }

    if (listCoeffsIn.empty()){
        listCoeffsIn.push_back(0.0);
    }

}



// add function
Polynomial Polynomial ::operator+(const Polynomial &rhs) const
{
    vector<double> OutputVec;
    std::size_t tmpSize = max(listCoeffsIn.size(), rhs.listCoeffsIn.size());

    for (std::size_t i = 0; i < tmpSize; ++i)
    {
        double cur = 0;
        if (i < listCoeffsIn.size())
        {
            cur += listCoeffsIn[i];
        }
        if (i < rhs.listCoeffsIn.size())
        {
            cur += rhs.listCoeffsIn[i];
        }
        OutputVec.push_back(cur);
    }

    return Polynomial(OutputVec);
}

// subtraction function

Polynomial Polynomial ::operator-(const Polynomial &rhs) const
{
    vector<double> tmpVec = rhs.listCoeffsIn;
    for (auto &x : tmpVec)
    {
        x *= -1;
    }
    Polynomial tmpPoly(tmpVec);

    return *this + tmpPoly;
}

// multiply function
Polynomial Polynomial ::operator*(const Polynomial &rhs) const
{
    vector<double> OutputVec(rhs.listCoeffsIn.size() + listCoeffsIn.size() - 1, 0);

    for (std::size_t i = 0; i < listCoeffsIn.size(); ++i)
    {
        for (std::size_t j = 0; j < rhs.listCoeffsIn.size(); ++j)
        {
            OutputVec[i + j] += listCoeffsIn[i] * rhs.listCoeffsIn[j];
        }
    }
    return Polynomial(OutputVec);
}
std::pair<Polynomial, Polynomial> DivMod(
    const Polynomial &dividend,
    const Polynomial &divisor)
{
    if (divisor.GetDegree() == 0 && divisor.GetCoeff(0) == 0.0)
    {
        throw std::domain_error("cannot divide by the zero polynomial");
    }

    int dividendDegree = dividend.GetDegree();
    int divisorDegree = divisor.GetDegree();

    if (dividendDegree < divisorDegree)
    {
        return {Polynomial(), dividend};
    }

    vector<double> quotient(dividendDegree - divisorDegree + 1, 0);
    vector<double> remainder = dividend.getListCoeffsIn();
    double leadingDivisor = divisor.GetCoeff(divisorDegree);

    for (int currentDegree = dividendDegree;
         currentDegree >= divisorDegree;
         --currentDegree)
    {
        int quotientDegree = currentDegree - divisorDegree;
        double factor = remainder[currentDegree] / leadingDivisor;
        quotient[quotientDegree] = factor;

        for (int divisorIndex = 0;
             divisorIndex <= divisorDegree;
             ++divisorIndex)
        {
            remainder[quotientDegree + divisorIndex] -=
                factor * divisor.GetCoeff(divisorIndex);
        }

        remainder[currentDegree] = 0.0;
    }

    return {Polynomial(quotient), Polynomial(remainder)};
}

Polynomial Polynomial::operator/(const Polynomial &rhs) const
{
    return DivMod(*this, rhs).first;
}

Polynomial Polynomial ::operator%(const Polynomial &rhs) const
{
    return DivMod(*this, rhs).second;
}


// equal function (assignment)
Polynomial& Polynomial::operator=(const Polynomial &rhs)
{
 
    listCoeffsIn = rhs.listCoeffsIn; 

    return *this; // Return the current object
}



Polynomial& Polynomial::operator+=(const Polynomial &rhs) 
{
    *this = *this + rhs;
    return *this;
}

Polynomial& Polynomial::operator-=(const Polynomial &rhs) 
{
    *this = *this - rhs;
    return *this;
}

Polynomial& Polynomial::operator*=(const Polynomial &rhs) 
{
    *this = *this * rhs;
    return *this;
}

Polynomial& Polynomial::operator/=(const Polynomial &rhs) 
{
    *this = *this / rhs;
    return *this;
}

Polynomial& Polynomial::operator%=(const Polynomial &rhs) 
{
    *this = *this % rhs;
    return *this;
}

bool Polynomial::operator==(const Polynomial &rhs) const
{
    if (listCoeffsIn.size() != rhs.listCoeffsIn.size())
    {
        return false;
    }

    if (listCoeffsIn == rhs.listCoeffsIn)
    {
        return true;
    }
    return false;
}




bool Polynomial::operator!=(const Polynomial &rhs) const
{
    return !(listCoeffsIn == rhs.listCoeffsIn);
}



void Polynomial ::Dump() const
{
    bool firstTerm = true;

    for (std::size_t power = 0; power < listCoeffsIn.size(); ++power)
    {
        double coefficient = listCoeffsIn[power];

        if (coefficient == 0.0)
        {
            continue;
        }

        if (!firstTerm)
        {
            cout << (coefficient < 0.0 ? " - " : " + ");
        }
        else if (coefficient < 0.0)
        {
            cout << "-";
        }

        double magnitude = std::abs(coefficient);
        if (power == 0 || magnitude != 1.0)
        {
            cout << magnitude;
        }

        if (power > 0)
        {
            cout << "x";
            if (power > 1)
            {
                cout << "^" << power;
            }
        }

        firstTerm = false;
    }

    if (firstTerm)
    {
        cout << "0";
    }

    cout << endl;
}

    Polynomial operator+(double lhs, const Polynomial& rhs) {
    Polynomial lhsPoly(lhs);
    return lhsPoly + rhs;
    }

    Polynomial operator+(int lhs, const Polynomial& rhs) {
    Polynomial lhsPoly(lhs);
    return lhsPoly + rhs;
    }

    Polynomial operator-(double lhs, const Polynomial& rhs) {
    
    Polynomial lhsPoly(lhs); 
    return lhsPoly - rhs;    
    
    }

    Polynomial operator-(int lhs, const Polynomial& rhs) {

    Polynomial lhsPoly(lhs); 
    return lhsPoly - rhs;    
    }

    Polynomial operator*(double lhs, const Polynomial& rhs) {
    Polynomial lhsPoly(lhs);
    return lhsPoly * rhs;
    }

    Polynomial operator*(int lhs, const Polynomial& rhs) {
    Polynomial lhsPoly(lhs);
    return lhsPoly * rhs;
    }

    Polynomial operator/(double lhs, const Polynomial& rhs) {
    Polynomial lhsPoly(lhs);
    return lhsPoly / rhs;
    }

    Polynomial operator/(int lhs, const Polynomial& rhs) {
    Polynomial lhsPoly(lhs);
    return lhsPoly / rhs;
    }

