#ifndef ComplexPoly_h
#define ComplexPoly_h

#include<vector>
#include <utility>
#include "Polynomial.h"
class ComplexPoly
{
    public:
    
    ComplexPoly() 
    : listCoeffsIn({0}), complexCoeffs({0}) {}

    

    ComplexPoly(const ComplexPoly &cp) 
    : listCoeffsIn(cp.listCoeffsIn), complexCoeffs(cp.complexCoeffs) {}

    ComplexPoly& operator=(const ComplexPoly &rhs) = default;

    explicit ComplexPoly(const Polynomial &poly)
    : listCoeffsIn(poly.getListCoeffsIn()), complexCoeffs({0}) {}


    ComplexPoly(const std::vector<double>& Polypart, const std::vector<double>& ComplexPart) 
    : listCoeffsIn(Polypart), complexCoeffs(ComplexPart) 
    {
        Normalize();
    }
    ComplexPoly(const std::initializer_list<double>& Polypart) 
    : listCoeffsIn(Polypart), complexCoeffs({0})
    {
        Normalize();
    }



    ComplexPoly operator+(const Polynomial &rhs) const;
    ComplexPoly operator-(const Polynomial &rhs) const;
    ComplexPoly operator*(const Polynomial &rhs) const;
    ComplexPoly operator/(const Polynomial &rhs) const;


    ComplexPoly operator+(const ComplexPoly &rhs) const;
    ComplexPoly operator-(const ComplexPoly &rhs) const;
    ComplexPoly operator*(const ComplexPoly &rhs) const;
    ComplexPoly operator/(const ComplexPoly &rhs) const;
    ComplexPoly operator%(const ComplexPoly &rhs) const;

    bool operator==(const ComplexPoly &rhs) const;
    bool operator!=(const ComplexPoly &rhs) const;


    const std::vector<double>& getListCoeffsIn() const {return listCoeffsIn;}
    const std::vector<double>& getcomplexCoeffsList() const {return complexCoeffs;}
    void Dump() const;
    double GetCoeff(int index) const;
    double getComplexCoeff(int index) const;
    int GetDegree() const;

    private:
    void Normalize();
    std::vector<double> listCoeffsIn;
    std::vector<double> complexCoeffs;
};

std::pair<ComplexPoly, ComplexPoly> DivMod(
    const ComplexPoly &dividend,
    const ComplexPoly &divisor);

inline ComplexPoly operator+(const Polynomial &lhs, const ComplexPoly &rhs) { return rhs + lhs; }
inline ComplexPoly operator-(const Polynomial &lhs, const ComplexPoly &rhs) { return ComplexPoly(lhs) - rhs; }
inline ComplexPoly operator*(const Polynomial &lhs, const ComplexPoly &rhs) { return rhs * lhs; }
inline ComplexPoly operator/(const Polynomial &lhs, const ComplexPoly &rhs) { 
    ComplexPoly lhs_cp(lhs);
    return lhs_cp / rhs;
}



#endif
