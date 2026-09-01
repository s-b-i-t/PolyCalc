import assert from "node:assert/strict";
import {readFile} from "node:fs/promises";
import test from "node:test";
import {
    evaluateExpression,
    formatRationalFunction
} from "../src/expression.js";

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

const engine = await import("../engine.js");
const operations = Object.freeze({
    add: engine.addRationalFunctions,
    subtract: engine.subtractRationalFunctions,
    multiply: engine.multiplyRationalFunctions,
    divide: engine.divideRationalFunctions
});

function complexPolynomial(real, imaginary = [0])
{
    return {real, imaginary};
}

function rational(numerator, denominator = complexPolynomial([1]))
{
    return {numerator, denominator};
}

async function calculate(expression)
{
    return evaluateExpression(expression, operations);
}

test("default expression cancels its common factor", async () =>
{
    assert.deepEqual(
        await calculate("(x²-1)/(x-1)"),
        rational(complexPolynomial([1, 1])));
});

test("rational addition preserves the denominator", async () =>
{
    assert.deepEqual(
        await calculate("1/(x+1)+1/(x-1)"),
        rational(
            complexPolynomial([0, 2]),
            complexPolynomial([-1, 0, 1])));
});

test("negative powers lower to rational division", async () =>
{
    assert.deepEqual(
        await calculate("x⁻¹+1"),
        rational(
            complexPolynomial([1, 1]),
            complexPolynomial([0, 1])));
});

test("integer scalar content reduces without decimalizing fractions", async () =>
{
    assert.deepEqual(
        await calculate("6/3"),
        rational(complexPolynomial([2])));
    assert.deepEqual(
        await calculate("2/4"),
        rational(
            complexPolynomial([1]),
            complexPolynomial([2])));
    assert.deepEqual(
        await calculate("(6x-3)/3"),
        rational(complexPolynomial([-1, 2])));
    assert.deepEqual(
        await calculate("1/-2"),
        rational(
            complexPolynomial([-1]),
            complexPolynomial([2])));
    assert.deepEqual(
        await calculate("1/3"),
        rational(
            complexPolynomial([1]),
            complexPolynomial([3])));

    assert.equal(formatRationalFunction(await calculate("6/3")), "2");
    assert.equal(
        formatRationalFunction(await calculate("2/4")),
        "(1) / (2)");
    assert.equal(
        formatRationalFunction(await calculate("(6x-3)/3")),
        "2*x - 1");
    assert.equal(
        formatRationalFunction(await calculate("1/-2")),
        "(-1) / (2)");
    assert.equal(
        formatRationalFunction(await calculate("1/3")),
        "(1) / (3)");
});

test("imaginary arithmetic uses the complex core", async () =>
{
    assert.deepEqual(
        await calculate("i*i"),
        rational(complexPolynomial([-1])));
    assert.deepEqual(
        await calculate("(x+i)*(x-i)"),
        rational(complexPolynomial([1, 0, 1])));
});

test("implicit multiplication lowers identically to explicit multiplication", async () =>
{
    assert.deepEqual(
        await calculate("5x"),
        await calculate("5*x"));
    assert.deepEqual(
        await calculate("ix"),
        await calculate("i*x"));
    assert.deepEqual(
        await calculate("3i"),
        await calculate("3*i"));
    assert.deepEqual(
        await calculate("2(x+1)"),
        await calculate("2*(x+1)"));
    assert.deepEqual(
        await calculate("(x+1)(x-1)"),
        await calculate("(x+1)*(x-1)"));
    assert.deepEqual(
        await calculate("x²i"),
        await calculate("x²*i"));
    assert.deepEqual(
        await calculate("1/ix"),
        await calculate("1/(i*x)"));
});

test("division by zero reaches the existing engine guard", async () =>
{
    await assert.rejects(
        calculate("1/0"),
        /Cannot divide by a zero rational function/);
});
