import createPolyCalcModule from "./generated/polycalc.mjs";

// Start loading the WebAssembly module once.
const modulePromise = createPolyCalcModule();

export function normalizeCoeffs(coeffs)
{
    if (coeffs.length === 0)
    {
        throw new Error("Polynomials need at least 1 coefficient");
    }

    let size = coeffs.length;
    while (size > 1 && coeffs[size - 1] === 0)
    {
        --size;
    }

    return coeffs.slice(0, size);
}

export function normalizeComplexPolynomial(value)
{
    if (
        value === null ||
        typeof value !== "object" ||
        value.real === undefined ||
        value.imaginary === undefined)
    {
        throw new Error(
            "A complex polynomial requires real and imaginary coefficients");
    }

    return {
        real: normalizeCoeffs(value.real),
        imaginary: normalizeCoeffs(value.imaginary)
    };
}

function isZeroComplexPolynomial(value)
{
    return value.real.length === 1 &&
        value.real[0] === 0 &&
        value.imaginary.length === 1 &&
        value.imaginary[0] === 0;
}

export function normalizeRationalFunction(value)
{
    if (
        value === null ||
        typeof value !== "object" ||
        value.numerator === undefined ||
        value.denominator === undefined)
    {
        throw new Error(
            "A rational function requires a numerator and denominator");
    }

    const numerator = normalizeComplexPolynomial(value.numerator);
    const denominator = normalizeComplexPolynomial(value.denominator);

    if (isZeroComplexPolynomial(denominator))
    {
        throw new Error("A rational function denominator cannot be zero");
    }

    return {numerator, denominator};
}

function complexCoefficientCount(value)
{
    return Math.max(value.real.length, value.imaginary.length);
}

function complexProductCoefficientCapacity(left, right)
{
    return complexCoefficientCount(left)
        + complexCoefficientCount(right)
        - 1;
}

function unpackComplexPolynomial(packed)
{
    const realSize = packed[0];
    const imaginarySize = packed[1];
    const imaginaryStart = 2 + realSize;

    return {
        real: packed.slice(2, imaginaryStart),
        imaginary: packed.slice(
            imaginaryStart,
            imaginaryStart + imaginarySize)
    };
}

function unpackRationalFunction(packed)
{
    const sizes = packed.slice(0, 4);
    const sizesAreValid = sizes.every(
        size => Number.isSafeInteger(size) && size >= 1);
    const packedSize = 4 + sizes.reduce(
        (total, size) => total + size,
        0);

    if (!sizesAreValid || packedSize !== packed.length)
    {
        throw new Error("Invalid rational result from WebAssembly");
    }

    const numeratorRealStart = 4;
    const numeratorImaginaryStart =
        numeratorRealStart + sizes[0];
    const denominatorRealStart =
        numeratorImaginaryStart + sizes[1];
    const denominatorImaginaryStart =
        denominatorRealStart + sizes[2];

    return {
        numerator: {
            real: packed.slice(
                numeratorRealStart,
                numeratorImaginaryStart),
            imaginary: packed.slice(
                numeratorImaginaryStart,
                denominatorRealStart)
        },
        denominator: {
            real: packed.slice(
                denominatorRealStart,
                denominatorImaginaryStart),
            imaginary: packed.slice(
                denominatorImaginaryStart,
                denominatorImaginaryStart + sizes[3])
        }
    };
}

function allocationByteLength(coefficientCount)
{
    const bytesPerCoefficient = Float64Array.BYTES_PER_ELEMENT;
    const byteLength = coefficientCount * bytesPerCoefficient;

    if (
        !Number.isSafeInteger(coefficientCount) ||
        coefficientCount < 1 ||
        byteLength > 0xffffffff)
    {
        throw new Error("WebAssembly allocation size is invalid");
    }

    return byteLength;
}

async function runWasmOperation(
    operationName,
    inputs,
    resultCapacity)
{
    const module = await modulePromise;
    const bytesPerCoefficient = Float64Array.BYTES_PER_ELEMENT;
    const operation = module[operationName];

    if (typeof operation !== "function")
    {
        throw new Error(`Unknown WASM operation: ${operationName}`);
    }

    const inputPointers = [];
    let resultPointer = 0;

    try
    {
        for (const input of inputs)
        {
            const pointer = module._malloc(
                allocationByteLength(input.length));

            if (pointer === 0)
            {
                throw new Error("WebAssembly memory allocation failed");
            }

            inputPointers.push(pointer);
        }

        resultPointer = module._malloc(
            allocationByteLength(resultCapacity));

        if (resultPointer === 0)
        {
            throw new Error("WebAssembly memory allocation failed");
        }

        for (let index = 0; index < inputs.length; ++index)
        {
            module.HEAPF64.set(
                inputs[index],
                inputPointers[index] / bytesPerCoefficient);
        }

        const operationArguments = [];
        for (let index = 0; index < inputs.length; ++index)
        {
            operationArguments.push(
                inputPointers[index],
                inputs[index].length);
        }

        const resultSize = operation(
            ...operationArguments,
            resultPointer);

        if (
            !Number.isSafeInteger(resultSize) ||
            resultSize < 1 ||
            resultSize > resultCapacity)
        {
            throw new Error("Invalid result size from WebAssembly");
        }

        const resultStart = resultPointer / bytesPerCoefficient;
        return Array.from(
            module.HEAPF64.subarray(resultStart, resultStart + resultSize));
    }
    finally
    {
        module._free(resultPointer);

        for (let index = inputPointers.length - 1; index >= 0; --index)
        {
            module._free(inputPointers[index]);
        }
    }
}

export function addPolynomials(left, right)
{
    const lhs = normalizeCoeffs(left);
    const rhs = normalizeCoeffs(right);
    return runWasmOperation(
        "_polycalc_add",
        [lhs, rhs],
        Math.max(lhs.length, rhs.length));
}

export function subtractPolynomials(left, right)
{
    const lhs = normalizeCoeffs(left);
    const rhs = normalizeCoeffs(right);
    return runWasmOperation(
        "_polycalc_subtract",
        [lhs, rhs],
        Math.max(lhs.length, rhs.length));
}

export function multiplyPolynomials(left, right)
{
    const lhs = normalizeCoeffs(left);
    const rhs = normalizeCoeffs(right);
    return runWasmOperation(
        "_polycalc_multiply",
        [lhs, rhs],
        lhs.length + rhs.length - 1);
}

export async function dividePolynomials(left, right)
{
    const lhs = normalizeCoeffs(left);
    const rhs = normalizeCoeffs(right);

    if (rhs.length === 1 && rhs[0] === 0)
    {
        throw new Error("Cannot divide by the zero polynomial");
    }

    const quotientCapacity =
        Math.max(1, lhs.length - rhs.length + 1);

    const remainderCapacity = Math.min(
        lhs.length,
        Math.max(1, rhs.length - 1));

    const packed = await runWasmOperation(
        "_polycalc_divmod",
        [lhs, rhs],
        2 + quotientCapacity + remainderCapacity);

    const quotientSize = packed[0];
    const remainderSize = packed[1];
    const remainderStart = 2 + quotientSize;

    return {
        quotient: packed.slice(2, remainderStart),
        remainder: packed.slice(
            remainderStart,
            remainderStart + remainderSize)
    };
}

async function runComplexPolynomialOperation(
    operationName,
    left,
    right,
    getResultCapacity)
{
    const lhs = normalizeComplexPolynomial(left);
    const rhs = normalizeComplexPolynomial(right);
    const packed = await runWasmOperation(
        operationName,
        [lhs.real, lhs.imaginary, rhs.real, rhs.imaginary],
        getResultCapacity(lhs, rhs));

    return unpackComplexPolynomial(packed);
}

export function addComplexPolynomials(left, right)
{
    return runComplexPolynomialOperation(
        "_polycalc_complex_add",
        left,
        right,
        (lhs, rhs) =>
            2
            + Math.max(lhs.real.length, rhs.real.length)
            + Math.max(lhs.imaginary.length, rhs.imaginary.length));
}

export function subtractComplexPolynomials(left, right)
{
    return runComplexPolynomialOperation(
        "_polycalc_complex_subtract",
        left,
        right,
        (lhs, rhs) =>
            2
            + Math.max(lhs.real.length, rhs.real.length)
            + Math.max(lhs.imaginary.length, rhs.imaginary.length));
}

export function multiplyComplexPolynomials(left, right)
{
    return runComplexPolynomialOperation(
        "_polycalc_complex_multiply",
        left,
        right,
        (lhs, rhs) =>
        {
            const resultCoefficientCapacity =
                complexProductCoefficientCapacity(lhs, rhs);

            return 2 + 2 * resultCoefficientCapacity;
        });
}

export async function divideComplexPolynomials(left, right)
{
    const lhs = normalizeComplexPolynomial(left);
    const rhs = normalizeComplexPolynomial(right);

    if (isZeroComplexPolynomial(rhs))
    {
        throw new Error("Cannot divide by the zero complex polynomial");
    }

    const lhsCoefficientCount = complexCoefficientCount(lhs);
    const rhsCoefficientCount = complexCoefficientCount(rhs);
    const quotientCapacity = Math.max(
        1,
        lhsCoefficientCount - rhsCoefficientCount + 1);
    const remainderCapacity = Math.min(
        lhsCoefficientCount,
        Math.max(1, rhsCoefficientCount - 1));

    const packed = await runWasmOperation(
        "_polycalc_complex_divmod",
        [lhs.real, lhs.imaginary, rhs.real, rhs.imaginary],
        4 + 2 * quotientCapacity + 2 * remainderCapacity);

    const quotientRealSize = packed[0];
    const quotientImaginarySize = packed[1];
    const remainderRealSize = packed[2];
    const remainderImaginarySize = packed[3];
    const quotientRealStart = 4;
    const quotientImaginaryStart =
        quotientRealStart + quotientRealSize;
    const remainderRealStart =
        quotientImaginaryStart + quotientImaginarySize;
    const remainderImaginaryStart =
        remainderRealStart + remainderRealSize;

    return {
        quotient: {
            real: packed.slice(
                quotientRealStart,
                quotientImaginaryStart),
            imaginary: packed.slice(
                quotientImaginaryStart,
                remainderRealStart)
        },
        remainder: {
            real: packed.slice(
                remainderRealStart,
                remainderImaginaryStart),
            imaginary: packed.slice(
                remainderImaginaryStart,
                remainderImaginaryStart + remainderImaginarySize)
        }
    };
}

function rationalFunctionInputs(value)
{
    return [
        value.numerator.real,
        value.numerator.imaginary,
        value.denominator.real,
        value.denominator.imaginary
    ];
}

function rationalResultCapacity(
    numeratorCoefficientCapacity,
    denominatorCoefficientCapacity)
{
    return 4
        + 2 * numeratorCoefficientCapacity
        + 2 * denominatorCoefficientCapacity;
}

function additiveRationalResultCapacity(lhs, rhs)
{
    const numeratorCapacity = Math.max(
        complexProductCoefficientCapacity(
            lhs.numerator,
            rhs.denominator),
        complexProductCoefficientCapacity(
            rhs.numerator,
            lhs.denominator));
    const denominatorCapacity =
        complexProductCoefficientCapacity(
            lhs.denominator,
            rhs.denominator);

    return rationalResultCapacity(
        numeratorCapacity,
        denominatorCapacity);
}

async function runRationalFunctionOperation(
    operationName,
    left,
    right,
    getResultCapacity,
    requireNonzeroRightNumerator = false)
{
    const lhs = normalizeRationalFunction(left);
    const rhs = normalizeRationalFunction(right);

    if (
        requireNonzeroRightNumerator &&
        isZeroComplexPolynomial(rhs.numerator))
    {
        throw new Error("Cannot divide by a zero rational function");
    }

    const packed = await runWasmOperation(
        operationName,
        [...rationalFunctionInputs(lhs), ...rationalFunctionInputs(rhs)],
        getResultCapacity(lhs, rhs));

    return unpackRationalFunction(packed);
}

export function addRationalFunctions(left, right)
{
    return runRationalFunctionOperation(
        "_polycalc_rational_add",
        left,
        right,
        additiveRationalResultCapacity);
}

export function subtractRationalFunctions(left, right)
{
    return runRationalFunctionOperation(
        "_polycalc_rational_subtract",
        left,
        right,
        additiveRationalResultCapacity);
}

export function multiplyRationalFunctions(left, right)
{
    return runRationalFunctionOperation(
        "_polycalc_rational_multiply",
        left,
        right,
        (lhs, rhs) => rationalResultCapacity(
            complexProductCoefficientCapacity(
                lhs.numerator,
                rhs.numerator),
            complexProductCoefficientCapacity(
                lhs.denominator,
                rhs.denominator)));
}

export function divideRationalFunctions(left, right)
{
    return runRationalFunctionOperation(
        "_polycalc_rational_divide",
        left,
        right,
        (lhs, rhs) => rationalResultCapacity(
            complexProductCoefficientCapacity(
                lhs.numerator,
                rhs.denominator),
            complexProductCoefficientCapacity(
                lhs.denominator,
                rhs.numerator)),
        true);
}
