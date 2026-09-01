import assert from "node:assert/strict";
import test from "node:test";
import {
    evaluateExpression,
    formatRationalFunction,
    normalizePastedExpression,
    parseExpression,
    rationalConstant,
    toAsciiExpression,
    tokenizeExpression
} from "../src/expression.js";

function scalarValue(value)
{
    return (
        value.numerator.real[0] /
        value.denominator.real[0]);
}

function scalarOperation(operation)
{
    return async (left, right) =>
        rationalConstant(
            operation(scalarValue(left), scalarValue(right)));
}

const scalarOperations = Object.freeze({
    add: scalarOperation((left, right) => left + right),
    subtract: scalarOperation((left, right) => left - right),
    multiply: scalarOperation((left, right) => left * right),
    divide: scalarOperation((left, right) =>
    {
        if (right === 0)
        {
            throw new Error("division by zero");
        }
        return left / right;
    })
});

test("ASCII exponent paste becomes visible superscript", () =>
{
    assert.equal(
        normalizePastedExpression("(x^12+1)÷2"),
        "(x¹²+1)/2");
    assert.equal(toAsciiExpression("x⁻²"), "x^-2");
});

test("tokenizer records a signed superscript exponent", () =>
{
    assert.deepEqual(
        tokenizeExpression("x⁻²").map(token => token.type),
        ["x", "exponent", "end"]);
    assert.equal(tokenizeExpression("x⁻²")[1].value, -2);
});

test("multiplication binds more tightly than addition", async () =>
{
    const result = await evaluateExpression(
        "2+3*4",
        scalarOperations);
    assert.equal(scalarValue(result), 14);
});

test("division is left associative", async () =>
{
    const result = await evaluateExpression(
        "8/4/2",
        scalarOperations);
    assert.equal(scalarValue(result), 1);
});

test("integer superscript powers support negative values", async () =>
{
    const squared = await evaluateExpression(
        "3²",
        scalarOperations);
    const reciprocal = await evaluateExpression(
        "2⁻¹",
        scalarOperations);

    assert.equal(scalarValue(squared), 9);
    assert.equal(scalarValue(reciprocal), 0.5);
});

test("unary minus applies after exponentiation", () =>
{
    const negativeSquare = parseExpression("-x²");
    const groupedSquare = parseExpression("(-x)²");

    assert.equal(negativeSquare.type, "unary");
    assert.equal(negativeSquare.operand.type, "power");
    assert.equal(groupedSquare.type, "power");
    assert.equal(groupedSquare.base.type, "unary");
});

test("unsupported syntax fails instead of changing its meaning", () =>
{
    assert.throws(
        () => parseExpression("y+1"),
        /Only x and i are supported characters/);
    assert.throws(
        () => parseExpression("sqrt\(x\)"),
        /Only x and i are supported characters/);
    assert.throws(
        () => parseExpression("x^2"),
        /superscript exponent/);
});

test("implicit multiplication is parsed between adjacent values", () =>
{
    for (const expression of [
        "5x",
        "ix",
        "3i",
        "2(x+1)",
        "(x+1)(x-1)",
        "x²i"
    ])
    {
        assert.doesNotThrow(() => parseExpression(expression));
    }
});

test("implicit multiplication binds the denominator together", () =>
{
    const parsed = parseExpression("1/ix");

    assert.equal(parsed.type, "binary");
    assert.equal(parsed.operator, "/");
    assert.equal(parsed.right.type, "binary");
    assert.equal(parsed.right.operator, "*");
});

test("rational results format as reusable expressions", () =>
{
    const polynomial = {
        numerator: {
            real: [1, 1],
            imaginary: [0]
        },
        denominator: {
            real: [1],
            imaginary: [0]
        }
    };
    const rational = {
        numerator: {
            real: [1],
            imaginary: [0]
        },
        denominator: {
            real: [-1, 1],
            imaginary: [0]
        }
    };

    assert.equal(formatRationalFunction(polynomial), "x + 1");
    assert.equal(
        formatRationalFunction(rational),
        "(1) / (x - 1)");
});

test("complex constants format without unnecessary parentheses", () =>
{
    const imaginaryUnit = {
        numerator: {
            real: [0],
            imaginary: [1]
        },
        denominator: {
            real: [1],
            imaginary: [0]
        }
    };
    const complexConstant = {
        numerator: {
            real: [2],
            imaginary: [3]
        },
        denominator: {
            real: [1],
            imaginary: [0]
        }
    };

    assert.equal(formatRationalFunction(imaginaryUnit), "i");
    assert.equal(formatRationalFunction(complexConstant), "2 + 3*i");
});

test("formatted doubles round-trip without precision loss", () =>
{
    const value = 0.123456789012345;
    const formatted = formatRationalFunction({
        numerator: {
            real: [value],
            imaginary: [0]
        },
        denominator: {
            real: [1],
            imaginary: [0]
        }
    });

    assert.equal(parseExpression(formatted).value, value);
});

test("formatted high-degree results remain parseable", () =>
{
    const coefficients = Array(2049).fill(0);
    coefficients[2048] = 1;
    const formatted = formatRationalFunction({
        numerator: {
            real: coefficients,
            imaginary: [0]
        },
        denominator: {
            real: [1],
            imaginary: [0]
        }
    });

    assert.doesNotThrow(() => parseExpression(formatted));
});
