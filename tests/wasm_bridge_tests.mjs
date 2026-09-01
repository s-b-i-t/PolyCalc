import assert from "node:assert/strict";
import {readFile} from "node:fs/promises";

const nativeFetch = globalThis.fetch;

globalThis.fetch = async (input, init) =>
{
    const url = typeof input === "string" ? input : input.url;

    if (url.startsWith("file:"))
    {
        return new Response(await readFile(new URL(url)), {
            headers: {"Content-Type": "application/wasm"}
        });
    }

    return nativeFetch(input, init);
};

const engine = await import("../web/engine.js");

function complexPolynomial(real, imaginary = [0])
{
    return {real, imaginary};
}

function rationalFunction(numerator, denominator)
{
    return {numerator, denominator};
}

function realRationalFunction(numerator, denominator)
{
    return rationalFunction(
        complexPolynomial(numerator),
        complexPolynomial(denominator));
}

assert.deepEqual(engine.normalizeCoeffs([3, 2, 0, 0]), [3, 2]);
assert.deepEqual(engine.normalizeCoeffs([0, 0, 0]), [0]);

assert.deepEqual(
    await engine.addPolynomials([1, 2, 0], [4, -2, 0]),
    [5]);

assert.deepEqual(
    await engine.subtractPolynomials([1, 2, 3], [4, -2]),
    [-3, 4, 3]);

assert.deepEqual(
    await engine.multiplyPolynomials([1, 2], [3, 4]),
    [3, 10, 8]);

assert.deepEqual(
    await engine.dividePolynomials([-1, 0, 1], [-1, 1]),
    {quotient: [1, 1], remainder: [0]});

assert.deepEqual(
    await engine.dividePolynomials([-4, 0, -2, 1], [-3, 1]),
    {quotient: [3, 1, 1], remainder: [5]});

assert.deepEqual(
    await engine.dividePolynomials([1, 2], [1, 0, 1]),
    {quotient: [0], remainder: [1, 2]});

assert.deepEqual(
    await engine.dividePolynomials([2, 4], [2]),
    {quotient: [1, 2], remainder: [0]});

assert.deepEqual(
    await engine.dividePolynomials([1, 2, 3], [1, 0, 0]),
    {quotient: [1, 2, 3], remainder: [0]});

assert.deepEqual(
    await engine.dividePolynomials([0, 0], [1, 1]),
    {quotient: [0], remainder: [0]});

await assert.rejects(
    async () => engine.dividePolynomials([1, 2], [0, 0]),
    /Cannot divide by the zero polynomial/);

assert.throws(
    () => engine.multiplyPolynomials([], [1]),
    /at least 1 coefficient/);

assert.deepEqual(
    await engine.addPolynomials([0.5, 1.25], [1.5, -0.25]),
    [2, 1]);

const immutableRealLeft = Object.freeze([1, 2, 0]);
const immutableRealRight = Object.freeze([3, -1]);
await engine.multiplyPolynomials(immutableRealLeft, immutableRealRight);
assert.deepEqual(immutableRealLeft, [1, 2, 0]);
assert.deepEqual(immutableRealRight, [3, -1]);

assert.deepEqual(
    engine.normalizeComplexPolynomial({
        real: [1, 2, 0, 0],
        imaginary: [3, 0]
    }),
    {real: [1, 2], imaginary: [3]});

assert.throws(
    () => engine.normalizeComplexPolynomial({real: [1]}),
    /requires real and imaginary coefficients/);

assert.deepEqual(
    engine.normalizeRationalFunction(rationalFunction(
        complexPolynomial([2, 0, 0], [0, 0]),
        complexPolynomial([3, 0], [0]))),
    realRationalFunction([2], [3]));

assert.throws(
    () => engine.normalizeRationalFunction({numerator: complexPolynomial([1])}),
    /requires a numerator and denominator/);

assert.throws(
    () => engine.normalizeRationalFunction(
        realRationalFunction([1], [0, 0])),
    /denominator cannot be zero/);

assert.throws(
    () => engine.normalizeRationalFunction(rationalFunction(
        complexPolynomial([], [0]),
        complexPolynomial([1]))),
    /at least 1 coefficient/);

const complexA = {
    real: [1, 2],
    imaginary: [3]
};
const complexB = {
    real: [-4, 0, 5],
    imaginary: [2, -1]
};

assert.deepEqual(
    await engine.addComplexPolynomials(complexA, complexB),
    {real: [-3, 2, 5], imaginary: [5, -1]});

assert.deepEqual(
    await engine.subtractComplexPolynomials(complexA, complexB),
    {real: [5, 2, -5], imaginary: [1, 1]});

assert.deepEqual(
    await engine.multiplyComplexPolynomials(
        {real: [1], imaginary: [2]},
        {real: [3], imaginary: [-4]}),
    {real: [11], imaginary: [2]});

assert.deepEqual(
    await engine.multiplyComplexPolynomials(
        {real: [0], imaginary: [1]},
        {real: [0], imaginary: [1]}),
    {real: [-1], imaginary: [0]});

assert.deepEqual(
    await engine.multiplyComplexPolynomials(
        {real: [1, 2], imaginary: [3, -1]},
        {real: [-2, 1], imaginary: [4, 2]}),
    {real: [-14, -5, 4], imaginary: [-2, 15, 3]});

const complexDividend = {
    real: [6, 2, -1],
    imaginary: [-1, 4, 3]
};
const complexDivisor = {
    real: [1, 1],
    imaginary: [1, 1]
};
const expectedComplexDivision = {
    quotient: {real: [2, 1], imaginary: [-1, 2]},
    remainder: {real: [3], imaginary: [-2]}
};

assert.deepEqual(
    await engine.divideComplexPolynomials(
        complexDividend,
        complexDivisor),
    expectedComplexDivision);

assert.deepEqual(
    await engine.divideComplexPolynomials(
        {real: [1, 0, 1], imaginary: [0]},
        {real: [1], imaginary: [0, 1]}),
    {
        quotient: {real: [1], imaginary: [0, -1]},
        remainder: {real: [0], imaginary: [0]}
    });

assert.deepEqual(
    await engine.divideComplexPolynomials(
        {real: [1], imaginary: [0]},
        {real: [1], imaginary: [1]}),
    {
        quotient: {real: [0.5], imaginary: [-0.5]},
        remainder: {real: [0], imaginary: [0]}
    });

assert.deepEqual(
    await engine.divideComplexPolynomials(
        {real: [2], imaginary: [3]},
        {real: [1, 1], imaginary: [1, 1]}),
    {
        quotient: {real: [0], imaginary: [0]},
        remainder: {real: [2], imaginary: [3]}
    });

await assert.rejects(
    async () => engine.divideComplexPolynomials(
        {real: [1], imaginary: [1]},
        {real: [0, 0], imaginary: [0, 0]}),
    /Cannot divide by the zero complex polynomial/);

const immutableComplexLeft = Object.freeze({
    real: Object.freeze([1, 2, 0]),
    imaginary: Object.freeze([3, 0])
});
const immutableComplexRight = Object.freeze({
    real: Object.freeze([-2, 1]),
    imaginary: Object.freeze([4, 2])
});

await engine.multiplyComplexPolynomials(
    immutableComplexLeft,
    immutableComplexRight);
assert.deepEqual(
    immutableComplexLeft,
    {real: [1, 2, 0], imaginary: [3, 0]});
assert.deepEqual(
    immutableComplexRight,
    {real: [-2, 1], imaginary: [4, 2]});

const rationalA = realRationalFunction([1, 1], [1, -1]);
const rationalB = realRationalFunction([2], [1, 1]);

assert.deepEqual(
    await engine.addRationalFunctions(rationalA, rationalB),
    realRationalFunction([3, 0, 1], [1, 0, -1]));

assert.deepEqual(
    await engine.subtractRationalFunctions(rationalA, rationalB),
    realRationalFunction([-1, 4, 1], [1, 0, -1]));

const expectedRationalProduct = realRationalFunction([2], [1, -1]);
assert.deepEqual(
    await engine.multiplyRationalFunctions(rationalA, rationalB),
    expectedRationalProduct);

assert.deepEqual(
    await engine.divideRationalFunctions(rationalA, rationalB),
    realRationalFunction([1, 2, 1], [2, -2]));

assert.deepEqual(
    await engine.subtractRationalFunctions(rationalA, rationalA),
    realRationalFunction([0], [1]));

const zeroRational = realRationalFunction([0], [1]);
assert.deepEqual(
    await engine.addRationalFunctions(
        realRationalFunction([2, 2], [4, 4]),
        zeroRational),
    realRationalFunction([1], [2]));

assert.deepEqual(
    await engine.multiplyRationalFunctions(
        realRationalFunction([1], [3]),
        realRationalFunction([1], [1])),
    realRationalFunction([1], [3]));

assert.deepEqual(
    await engine.divideRationalFunctions(
        realRationalFunction([6], [1]),
        realRationalFunction([3], [1])),
    realRationalFunction([2], [1]));

assert.deepEqual(
    await engine.divideRationalFunctions(
        realRationalFunction([-3, 6], [1]),
        realRationalFunction([3], [1])),
    realRationalFunction([-1, 2], [1]));

assert.deepEqual(
    await engine.divideRationalFunctions(
        realRationalFunction([1], [1]),
        realRationalFunction([-2], [1])),
    realRationalFunction([-1], [2]));

const complexRationalA = rationalFunction(
    complexPolynomial([1], [1]),
    complexPolynomial([2], [-1]));
const complexRationalB = rationalFunction(
    complexPolynomial([3], [-2]),
    complexPolynomial([1], [1]));

assert.deepEqual(
    await engine.addRationalFunctions(
        complexRationalA,
        complexRationalB),
    rationalFunction(
        complexPolynomial([4], [-5]),
        complexPolynomial([3], [1])));

assert.deepEqual(
    await engine.subtractRationalFunctions(
        complexRationalA,
        complexRationalB),
    rationalFunction(
        complexPolynomial([-4], [9]),
        complexPolynomial([3], [1])));

assert.deepEqual(
    await engine.multiplyRationalFunctions(
        complexRationalA,
        complexRationalB),
    rationalFunction(
        complexPolynomial([5], [1]),
        complexPolynomial([3], [1])));

assert.deepEqual(
    await engine.divideRationalFunctions(
        complexRationalA,
        complexRationalB),
    rationalFunction(
        complexPolynomial([0], [2]),
        complexPolynomial([4], [-7])));

const reducibleComplexRational = rationalFunction(
    complexPolynomial([0, 1, 1], [1, 1]),
    complexPolynomial([2, 0, 1], [0, -1]));
assert.deepEqual(
    await engine.addRationalFunctions(
        reducibleComplexRational,
        zeroRational),
    rationalFunction(
        complexPolynomial([1, 1], [0]),
        complexPolynomial([0, 1], [-2])));

await assert.rejects(
    async () => engine.addRationalFunctions(
        realRationalFunction([1], [0]),
        rationalA),
    /denominator cannot be zero/);

await assert.rejects(
    async () => engine.multiplyRationalFunctions(
        rationalA,
        realRationalFunction([1], [0, 0])),
    /denominator cannot be zero/);

await assert.rejects(
    async () => engine.divideRationalFunctions(
        rationalA,
        realRationalFunction([0, 0], [1])),
    /Cannot divide by a zero rational function/);

assert.deepEqual(
    await engine.addRationalFunctions(
        realRationalFunction([1], [2]),
        rationalFunction(
            complexPolynomial([0], [1]),
            complexPolynomial([1]))),
    rationalFunction(
        complexPolynomial([1], [2]),
        complexPolynomial([2], [0])));

const immutableRationalLeft = Object.freeze({
    numerator: Object.freeze({
        real: Object.freeze([1, 1, 0]),
        imaginary: Object.freeze([0])
    }),
    denominator: Object.freeze({
        real: Object.freeze([1, -1]),
        imaginary: Object.freeze([0])
    })
});
const immutableRationalRight = Object.freeze({
    numerator: Object.freeze({
        real: Object.freeze([2]),
        imaginary: Object.freeze([0])
    }),
    denominator: Object.freeze({
        real: Object.freeze([1, 1]),
        imaginary: Object.freeze([0])
    })
});

await engine.multiplyRationalFunctions(
    immutableRationalLeft,
    immutableRationalRight);
assert.deepEqual(immutableRationalLeft, {
    numerator: {real: [1, 1, 0], imaginary: [0]},
    denominator: {real: [1, -1], imaginary: [0]}
});
assert.deepEqual(immutableRationalRight, {
    numerator: {real: [2], imaginary: [0]},
    denominator: {real: [1, 1], imaginary: [0]}
});

for (let iteration = 0; iteration < 100; ++iteration)
{
    assert.deepEqual(
        await engine.dividePolynomials([-4, 0, -2, 1], [-3, 1]),
        {quotient: [3, 1, 1], remainder: [5]});

    assert.deepEqual(
        await engine.divideComplexPolynomials(
            complexDividend,
            complexDivisor),
        expectedComplexDivision);

    assert.deepEqual(
        await engine.multiplyRationalFunctions(rationalA, rationalB),
        expectedRationalProduct);
}

console.log("WASM/JS bridge checks passed");
