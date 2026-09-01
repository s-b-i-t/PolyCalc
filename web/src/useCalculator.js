import {computed, nextTick, onMounted, ref} from "vue";
import {
    addRationalFunctions,
    divideRationalFunctions,
    multiplyRationalFunctions,
    subtractRationalFunctions
} from "../engine.js";
import {
    evaluateExpression,
    formatRationalFunction,
    normalizePastedExpression,
    toAsciiExpression,
    toSuperscript
} from "./expression.js";

export function useCalculator()
{
    const operations = Object.freeze({
        add: addRationalFunctions,
        subtract: subtractRationalFunctions,
        multiply: multiplyRationalFunctions,
        divide: divideRationalFunctions
    });

    const editor = ref(null);
    const expression = ref("(x²-1)/(x-1)");
    const exponentMode = ref(false);
    const exponentHasDigits = ref(false);
    const exponentHasSign = ref(false);
    const status = ref("idle");
    const error = ref("");
    const result = ref(null);
    const lastResultText = ref("");
    let calculationRevision = 0;

    const keypad = Object.freeze([
        {label: "x²", action: "insert", value: "x²", name: "Insert x squared"},
        {label: "xⁿ", action: "exponent", name: "Enter exponent"},
        {label: "(", action: "insert", value: "(", name: "Open parenthesis"},
        {label: ")", action: "insert", value: ")", name: "Close parenthesis"},
        {label: "i", action: "insert", value: "i", name: "Insert imaginary unit"},
        {label: "Clear", action: "clear", name: "Clear expression"},

        {label: "7", action: "insert", value: "7"},
        {label: "8", action: "insert", value: "8"},
        {label: "9", action: "insert", value: "9"},
        {label: "x", action: "insert", value: "x", name: "Insert x"},
        {label: "Ans", action: "answer", name: "Insert previous result"},
        {label: "÷", action: "insert", value: "/", name: "Divide"},

        {label: "4", action: "insert", value: "4"},
        {label: "5", action: "insert", value: "5"},
        {label: "6", action: "insert", value: "6"},
        {label: ".", action: "insert", value: ".", name: "Decimal point"},
        {label: "⌫", action: "backspace", name: "Backspace"},
        {label: "×", action: "insert", value: "*", name: "Multiply"},

        {label: "1", action: "insert", value: "1"},
        {label: "2", action: "insert", value: "2"},
        {label: "3", action: "insert", value: "3"},
        {label: "0", action: "insert", value: "0"},
        {label: "−", action: "insert", value: "-", name: "Subtract"},
        {label: "+", action: "insert", value: "+", name: "Add"}
    ]);

    const isWorking = computed(() => status.value === "working");
    const resultText = computed(() =>
        result.value === null ? "" : formatRationalFunction(result.value));
    const accessibleResult = computed(() =>
        result.value === null ? "" : toAsciiExpression(resultText.value));

    function stopExponentMode()
    {
        exponentMode.value = false;
        exponentHasDigits.value = false;
        exponentHasSign.value = false;
    }

    function startExponentMode()
    {
        exponentMode.value = true;
        exponentHasDigits.value = false;
        exponentHasSign.value = false;
        editor.value?.focus();
    }

    function markExpressionChanged()
    {
        ++calculationRevision;
        status.value = "idle";
        error.value = "";
        result.value = null;
    }

    async function restoreSelection(position)
    {
        await nextTick();
        editor.value?.focus();
        editor.value?.setSelectionRange(position, position);
    }

    function replaceSelection(text)
    {
        const input = editor.value;
        const start = input?.selectionStart ?? expression.value.length;
        const end = input?.selectionEnd ?? start;

        expression.value =
            expression.value.slice(0, start) +
            text +
            expression.value.slice(end);
        markExpressionChanged();
        restoreSelection(start + text.length);
    }

    function backspace()
    {
        stopExponentMode();
        const input = editor.value;
        const start = input?.selectionStart ?? expression.value.length;
        const end = input?.selectionEnd ?? start;

        if (start !== end)
        {
            expression.value =
                expression.value.slice(0, start) +
                expression.value.slice(end);
            markExpressionChanged();
            restoreSelection(start);
            return;
        }

        if (start > 0)
        {
            expression.value =
                expression.value.slice(0, start - 1) +
                expression.value.slice(start);
            markExpressionChanged();
            restoreSelection(start - 1);
        }
    }

    function insertExponentCharacter(value)
    {
        if (/^\d$/.test(value))
        {
            replaceSelection(toSuperscript(value));
            exponentHasDigits.value = true;
            return true;
        }

        if (
            (value === "+" || value === "-") &&
            !exponentHasDigits.value &&
            !exponentHasSign.value)
        {
            replaceSelection(toSuperscript(value));
            exponentHasSign.value = true;
            return true;
        }

        return false;
    }

    function insertValue(value)
    {
        if (
            exponentMode.value &&
            insertExponentCharacter(value))
        {
            return;
        }

        stopExponentMode();
        replaceSelection(value);
    }

    function pressKey(key)
    {
        if (key.action === "insert")
        {
            insertValue(key.value);
        }
        else if (key.action === "exponent")
        {
            startExponentMode();
        }
        else if (key.action === "backspace")
        {
            backspace();
        }
        else if (key.action === "clear")
        {
            stopExponentMode();
            expression.value = "";
            markExpressionChanged();
            restoreSelection(0);
        }
        else if (key.action === "answer" && lastResultText.value !== "")
        {
            insertValue("(" + lastResultText.value + ")");
        }
    }

    function handleKeydown(event)
    {
        if (event.ctrlKey || event.metaKey || event.altKey)
        {
            return;
        }

        if (event.key === "Enter")
        {
            event.preventDefault();
            calculate();
            return;
        }

        if (event.key === "^")
        {
            event.preventDefault();
            startExponentMode();
            return;
        }

        if (!exponentMode.value)
        {
            return;
        }

        if (
            /^\d$/.test(event.key) ||
            event.key === "+" ||
            event.key === "-")
        {
            if (insertExponentCharacter(event.key))
            {
                event.preventDefault();
                return;
            }
        }

        if (event.key === "Backspace")
        {
            stopExponentMode();
            return;
        }

        if (event.key !== "Shift")
        {
            stopExponentMode();
        }
    }

    function handleInput(event)
    {
        let nextExpression = normalizePastedExpression(event.target.value);

        if (nextExpression.includes("^"))
        {
            nextExpression = nextExpression.replaceAll("^", "");
            startExponentMode();
        }

        expression.value = nextExpression;
        markExpressionChanged();
    }

    function handlePaste(event)
    {
        event.preventDefault();
        const pasted = normalizePastedExpression(
            event.clipboardData.getData("text"));

        if (pasted.includes("^"))
        {
            error.value = "A pasted exponent needs an integer after ^";
            status.value = "error";
            return;
        }

        stopExponentMode();
        replaceSelection(pasted);
    }

    async function calculate()
    {
        if (isWorking.value)
        {
            return;
        }

        const revision = ++calculationRevision;
        stopExponentMode();
        status.value = "working";
        error.value = "";

        try
        {
            const calculated = await evaluateExpression(
                expression.value,
                operations);

            if (revision !== calculationRevision)
            {
                return;
            }

            result.value = calculated;
            lastResultText.value = formatRationalFunction(calculated);
            status.value = "success";
        }
        catch (caught)
        {
            if (revision !== calculationRevision)
            {
                return;
            }

            result.value = null;
            error.value =
                caught instanceof Error ?
                    caught.message :
                    "The expression could not be calculated";
            status.value = "error";
        }
    }

    async function copyResult()
    {
        if (result.value === null)
        {
            return;
        }

        try
        {
            await navigator.clipboard.writeText(resultText.value);
        }
        catch
        {
            error.value = "Copy is unavailable in this browser";
            status.value = "error";
        }
    }

    function useResult()
    {
        if (lastResultText.value === "")
        {
            return;
        }

        expression.value = lastResultText.value;
        markExpressionChanged();
        restoreSelection(expression.value.length);
    }

    onMounted(calculate);

    return {
        accessibleResult,
        backspace,
        calculate,
        copyResult,
        editor,
        error,
        exponentMode,
        expression,
        handleInput,
        handleKeydown,
        handlePaste,
        isWorking,
        keypad,
        pressKey,
        result,
        resultText,
        status,
        useResult
    };
}
