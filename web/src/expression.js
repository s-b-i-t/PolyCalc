const SUPERSCRIPT_FROM_NORMAL = Object.freeze({
    "0": "⁰",
    "1": "¹",
    "2": "²",
    "3": "³",
    "4": "⁴",
    "5": "⁵",
    "6": "⁶",
    "7": "⁷",
    "8": "⁸",
    "9": "⁹",
    "+": "⁺",
    "-": "⁻"
});

const NORMAL_FROM_SUPERSCRIPT = Object.freeze(
    Object.fromEntries(
        Object.entries(SUPERSCRIPT_FROM_NORMAL)
            .map(([normal, superscript]) => [superscript, normal])));

export const MAX_EXPONENT_MAGNITUDE = 1024;

function expressionError(message, position)
{
    if (position === undefined)
    {
        return new Error(message);
    }

    return new Error(message + " at character " + (position + 1));
}

export function toSuperscript(value)
{
    const characters = String(value).split("");
    if (characters.some(character =>
        SUPERSCRIPT_FROM_NORMAL[character] === undefined))
    {
        throw new Error("Only signed integer exponents are supported");
    }

    return characters
        .map(character => SUPERSCRIPT_FROM_NORMAL[character])
        .join("");
}

export function normalizePastedExpression(value)
{
    return value
        .replaceAll("×", "*")
        .replaceAll("÷", "/")
        .replace(
            /\^([+-]?\d+)/g,
            (match, exponent) => toSuperscript(exponent));
}

export function toAsciiExpression(value)
{
    let output = "";
    let readingExponent = false;

    for (const character of value)
    {
        const normal = NORMAL_FROM_SUPERSCRIPT[character];
        if (normal !== undefined)
        {
            if (!readingExponent)
            {
                output += "^";
                readingExponent = true;
            }
            output += normal;
        }
        else
        {
            readingExponent = false;
            output += character;
        }
    }

    return output;
}

function readSuperscript(source, start)
{
    let end = start;
    let normal = "";

    while (
        end < source.length &&
        NORMAL_FROM_SUPERSCRIPT[source[end]] !== undefined)
    {
        normal += NORMAL_FROM_SUPERSCRIPT[source[end]];
        ++end;
    }

    if (!/^[+-]?\d+$/.test(normal))
    {
        throw expressionError("Malformed superscript exponent", start);
    }

    const value = Number(normal);
    if (
        !Number.isSafeInteger(value) ||
        Math.abs(value) > MAX_EXPONENT_MAGNITUDE)
    {
        throw expressionError(
            "Exponent magnitude must be " +
                MAX_EXPONENT_MAGNITUDE +
                " or less",
            start);
    }

    return {end, value};
}

export function tokenizeExpression(source)
{
    const tokens = [];
    let position = 0;

    while (position < source.length)
    {
        const character = source[position];

        if (/\s/.test(character))
        {
            ++position;
            continue;
        }

        const superscript = NORMAL_FROM_SUPERSCRIPT[character];
        if (superscript !== undefined)
        {
            const exponent = readSuperscript(source, position);
            tokens.push({
                type: "exponent",
                value: exponent.value,
                position
            });
            position = exponent.end;
            continue;
        }

        if (character === "^")
        {
            throw expressionError(
                "Use a superscript exponent instead of a visible ^",
                position);
        }

        const numberMatch = source.slice(position).match(
            /^(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?/);
        if (numberMatch)
        {
            const value = Number(numberMatch[0]);
            if (!Number.isFinite(value))
            {
                throw expressionError("Number is outside the supported range", position);
            }

            tokens.push({type: "number", value, position});
            position += numberMatch[0].length;
            continue;
        }

        if (character === "x" || character === "i")
        {
            tokens.push({type: character, position});
            ++position;
            continue;
        }

        if (/[A-Za-z]/.test(character))
        {
            throw expressionError(
                "Only x and i are supported characters");

        }

        const normalizedOperator =
            character === "×" ? "*" :
            character === "÷" ? "/" :
            character;

        if ("+-*/()".includes(normalizedOperator))
        {
            tokens.push({type: normalizedOperator, position});
            ++position;
            continue;
        }

        throw expressionError(
            "Unsupported character " + JSON.stringify(character),
            position);
    }

    tokens.push({type: "end", position: source.length});
    return tokens;
}

class Parser
{
    constructor(tokens)
    {
        this.tokens = tokens;
        this.index = 0;
    }

    current()
    {
        return this.tokens[this.index];
    }

    match(type)
    {
        if (this.current().type !== type)
        {
            return null;
        }

        return this.tokens[this.index++];
    }

    require(type, message)
    {
        const token = this.match(type);
        if (token === null)
        {
            throw expressionError(message, this.current().position);
        }
        return token;
    }

    parse()
    {
        const node = this.parseSum();
        const remaining = this.current();

        if (remaining.type !== "end")
        {
            if (
                remaining.type === "number" ||
                remaining.type === "x" ||
                remaining.type === "i" ||
                remaining.type === "(")
            {
                throw expressionError(
                    "Invalid");
            }

            throw expressionError("Unexpected token", remaining.position);
        }

        return node;
    }

    parseSum()
    {
        let node = this.parseProduct();

        while (
            this.current().type === "+" ||
            this.current().type === "-")
        {
            const operator = this.tokens[this.index++].type;
            node = {
                type: "binary",
                operator,
                left: node,
                right: this.parseProduct()
            };
        }

        return node;
    }

    parseProduct()
    {
        let node = this.parseImplicitProduct();

        while (
            this.current().type === "*" ||
            this.current().type === "/")
        {
            const operator = this.tokens[this.index++].type;
            node = {
                type: "binary",
                operator,
                left: node,
                right: this.parseImplicitProduct()
            };
        }

        return node;
    }

    parseImplicitProduct()
    {
        let node = this.parseUnary();

        while (
            this.current().type === "x" ||
            this.current().type === "i" ||
            this.current().type === "(")
        {
            node = {
                type: "binary",
                operator: "*",
                left: node,
                right: this.parseUnary()
            };
        }

        return node;
    }

    parseUnary()
    {
        if (
            this.current().type === "+" ||
            this.current().type === "-")
        {
            const operator = this.tokens[this.index++].type;
            return {
                type: "unary",
                operator,
                operand: this.parseUnary()
            };
        }

        return this.parsePower();
    }

    parsePower()
    {
        const base = this.parsePrimary();
        const exponent = this.match("exponent");

        if (exponent === null)
        {
            return base;
        }

        if (this.current().type === "exponent")
        {
            throw expressionError(
                "Wrap a powered value in parentheses before another exponent",
                this.current().position);
        }

        return {
            type: "power",
            base,
            exponent: exponent.value
        };
    }

    parsePrimary()
    {
        const token = this.current();

        if (this.match("number"))
        {
            return {type: "number", value: token.value};
        }

        if (this.match("x"))
        {
            return {type: "x"};
        }

        if (this.match("i"))
        {
            return {type: "i"};
        }

        if (this.match("("))
        {
            const node = this.parseSum();
            this.require(")", "Missing closing parenthesis");
            return node;
        }

        if (token.type === "exponent")
        {
            throw expressionError("An exponent needs a base", token.position);
        }

        throw expressionError(
            "Expected a number, x, i, or (",
            token.position);
    }
}

export function parseExpression(source)
{
    if (source.trim() === "")
    {
        throw new Error("Enter an expression");
    }

    return new Parser(tokenizeExpression(source)).parse();
}

function complexPolynomial(real, imaginary)
{
    return {real, imaginary};
}

export function rationalConstant(real, imaginary = 0)
{
    return {
        numerator: complexPolynomial([real], [imaginary]),
        denominator: complexPolynomial([1], [0])
    };
}

function rationalVariable()
{
    return {
        numerator: complexPolynomial([0, 1], [0]),
        denominator: complexPolynomial([1], [0])
    };
}

async function raiseToIntegerPower(base, exponent, operations)
{
    if (exponent === 0)
    {
        return rationalConstant(1);
    }

    let remaining = Math.abs(exponent);
    let result = rationalConstant(1);
    let factor = base;

    while (remaining > 0)
    {
        if (remaining % 2 === 1)
        {
            result = await operations.multiply(result, factor);
        }

        remaining = Math.floor(remaining / 2);
        if (remaining > 0)
        {
            factor = await operations.multiply(factor, factor);
        }
    }

    if (exponent < 0)
    {
        return operations.divide(rationalConstant(1), result);
    }

    return result;
}

export async function evaluateAst(node, operations)
{
    if (node.type === "number")
    {
        return rationalConstant(node.value);
    }

    if (node.type === "x")
    {
        return rationalVariable();
    }

    if (node.type === "i")
    {
        return rationalConstant(0, 1);
    }

    if (node.type === "unary")
    {
        const operand = await evaluateAst(node.operand, operations);
        if (node.operator === "+")
        {
            return operand;
        }
        return operations.subtract(rationalConstant(0), operand);
    }

    if (node.type === "power")
    {
        const base = await evaluateAst(node.base, operations);
        return raiseToIntegerPower(base, node.exponent, operations);
    }

    if (node.type === "binary")
    {
        const [left, right] = await Promise.all([
            evaluateAst(node.left, operations),
            evaluateAst(node.right, operations)
        ]);

        const operation = {
            "+": operations.add,
            "-": operations.subtract,
            "*": operations.multiply,
            "/": operations.divide
        }[node.operator];

        return operation(left, right);
    }

    throw new Error("Unknown expression node");
}

export function evaluateExpression(source, operations)
{
    return evaluateAst(parseExpression(source), operations);
}

function formatNumber(value)
{
    if (Object.is(value, -0))
    {
        return "0";
    }

    return String(value);
}

function polynomialIsZero(coefficients)
{
    return coefficients.every(coefficient => coefficient === 0);
}

function formatVariablePower(degree)
{
    if (degree === 1)
    {
        return "x";
    }

    if (degree <= MAX_EXPONENT_MAGNITUDE)
    {
        return "x" + toSuperscript(degree);
    }

    const quotient = Math.floor(
        degree / MAX_EXPONENT_MAGNITUDE);
    const remainder =
        degree % MAX_EXPONENT_MAGNITUDE;
    const groupedPower =
        "(" +
        formatVariablePower(quotient) +
        ")" +
        toSuperscript(MAX_EXPONENT_MAGNITUDE);

    if (remainder === 0)
    {
        return groupedPower;
    }

    return (
        groupedPower +
        "*" +
        formatVariablePower(remainder));
}

function formatPolynomial(coefficients)
{
    const terms = [];

    for (let degree = coefficients.length - 1; degree >= 0; --degree)
    {
        const coefficient = coefficients[degree];
        if (coefficient === 0)
        {
            continue;
        }

        const magnitude = Math.abs(coefficient);
        let body;

        if (degree === 0)
        {
            body = formatNumber(magnitude);
        }
        else
        {
            const coefficientText =
                magnitude === 1 ? "" : formatNumber(magnitude) + "*";
            body = coefficientText + formatVariablePower(degree);
        }

        terms.push({negative: coefficient < 0, body});
    }

    if (terms.length === 0)
    {
        return "0";
    }

    return terms
        .map((term, index) =>
        {
            if (index === 0)
            {
                return term.negative ? "-" + term.body : term.body;
            }
            return term.negative ?
                " - " + term.body :
                " + " + term.body;
        })
        .join("");
}

function formatComplexPolynomial(value)
{
    const realIsZero = polynomialIsZero(value.real);
    const imaginaryIsZero = polynomialIsZero(value.imaginary);
    const real = formatPolynomial(value.real);

    if (imaginaryIsZero)
    {
        return real;
    }

    if (value.imaginary.length === 1)
    {
        const coefficient = value.imaginary[0];
        const magnitude = Math.abs(coefficient);
        const imaginaryTerm =
            magnitude === 1 ?
                "i" :
                formatNumber(magnitude) + "*i";

        if (realIsZero)
        {
            return coefficient < 0 ?
                "-" + imaginaryTerm :
                imaginaryTerm;
        }

        return coefficient < 0 ?
            real + " - " + imaginaryTerm :
            real + " + " + imaginaryTerm;
    }

    const imaginary = formatPolynomial(value.imaginary);
    if (realIsZero)
    {
        return "i*(" + imaginary + ")";
    }

    return "(" + real + ") + i*(" + imaginary + ")";
}

function denominatorIsOne(value)
{
    return (
        value.real.length === 1 &&
        value.real[0] === 1 &&
        value.imaginary.length === 1 &&
        value.imaginary[0] === 0);
}

export function formatRationalFunction(value)
{
    const numerator = formatComplexPolynomial(value.numerator);
    if (denominatorIsOne(value.denominator))
    {
        return numerator;
    }

    return (
        "(" +
        numerator +
        ") / (" +
        formatComplexPolynomial(value.denominator) +
        ")");
}

export function classifyRationalFunction(value)
{
    if (!denominatorIsOne(value.denominator))
    {
        return "Rational function";
    }

    if (!polynomialIsZero(value.numerator.imaginary))
    {
        return "Complex polynomial";
    }

    return "Polynomial";
}
