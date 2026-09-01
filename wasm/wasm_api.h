#ifndef POLYCALC_WASM_API_H
#define POLYCALC_WASM_API_H

#include <cstddef>

// Return the number of doubles written to result.
extern "C" std::size_t polycalc_add(
    const double *lhs,
    std::size_t lhsSize,
    const double *rhs,
    std::size_t rhsSize,
    double *result);

extern "C" std::size_t polycalc_subtract(
    const double *lhs,
    std::size_t lhsSize,
    const double *rhs,
    std::size_t rhsSize,
    double *result);

extern "C" std::size_t polycalc_multiply(
    const double *lhs,
    std::size_t lhsSize,
    const double *rhs,
    std::size_t rhsSize,
    double *result);

extern "C" std::size_t polycalc_divmod(
    const double *lhs,
    std::size_t lhsSize,
    const double *rhs,
    std::size_t rhsSize,
    double *result);

extern "C" std::size_t polycalc_complex_add(
    const double *lhsReal,
    std::size_t lhsRealSize,
    const double *lhsImaginary,
    std::size_t lhsImaginarySize,
    const double *rhsReal,
    std::size_t rhsRealSize,
    const double *rhsImaginary,
    std::size_t rhsImaginarySize,
    double *result);

extern "C" std::size_t polycalc_complex_subtract(
    const double *lhsReal,
    std::size_t lhsRealSize,
    const double *lhsImaginary,
    std::size_t lhsImaginarySize,
    const double *rhsReal,
    std::size_t rhsRealSize,
    const double *rhsImaginary,
    std::size_t rhsImaginarySize,
    double *result);

extern "C" std::size_t polycalc_complex_multiply(
    const double *lhsReal,
    std::size_t lhsRealSize,
    const double *lhsImaginary,
    std::size_t lhsImaginarySize,
    const double *rhsReal,
    std::size_t rhsRealSize,
    const double *rhsImaginary,
    std::size_t rhsImaginarySize,
    double *result);

extern "C" std::size_t polycalc_complex_divmod(
    const double *lhsReal,
    std::size_t lhsRealSize,
    const double *lhsImaginary,
    std::size_t lhsImaginarySize,
    const double *rhsReal,
    std::size_t rhsRealSize,
    const double *rhsImaginary,
    std::size_t rhsImaginarySize,
    double *result);

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
    double *result);

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
    double *result);

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
    double *result);

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
    double *result);

#endif
