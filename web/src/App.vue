<script setup>
import {useCalculator} from "./useCalculator.js";

const {
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
} = useCalculator();
</script>

<template>
    <div class="page">
        <main class="calculator">
            <header class="header">
                <div class="brand">
                    <span class="mark" aria-hidden="true">P(x)</span>
                    <span>
                        <strong>PolyCalc</strong>
                        <small>Polynomial · complex · rational</small>
                    </span>
                </div>
            </header>

            <div class="workspace">
                <form class="editor" @submit.prevent="calculate">
                    <div class="field-heading">
                        <label for="expression">Expression</label>
                    </div>

                    <div class="input-row">
                        <input
                            id="expression"
                            ref="editor"
                            :value="expression"
                            type="text"
                            autocomplete="off"
                            autocapitalize="off"
                            spellcheck="false"
                            @input="handleInput"
                            @keydown="handleKeydown"
                            @paste="handlePaste"
                        >
                        <button
                            type="button"
                            class="backspace"
                            aria-label="Backspace"
                            @mousedown.prevent
                            @click="backspace"
                        >
                            ⌫
                        </button>
                        <button
                            type="submit"
                            class="calculate"
                            :disabled="isWorking"
                        >
                            {{ isWorking ? "Working…" : "Calculate" }}
                        </button>
                    </div>
                    <p v-if="error" class="error" role="alert">{{ error }}</p>
                    <p class="screen-reader-only" role="status" aria-live="polite">
                        {{
                            status === "success" ?
                                "Result: " + accessibleResult :
                                ""
                        }}
                    </p>
                </form>

                <section class="keypad" aria-label="Calculator keypad">
                    <button
                        v-for="key in keypad"
                        :key="key.label"
                        type="button"
                        :class="[
                            'key',
                            key.action === 'exponent' && exponentMode ?
                                'key-active' :
                                '',
                            ['+', '−', '×', '÷'].includes(key.label) ?
                                'key-operator' :
                                ''
                        ]"
                        :aria-label="key.name || key.label"
                        :aria-pressed="
                            key.action === 'exponent' ?
                                exponentMode :
                                undefined
                        "
                        @mousedown.prevent
                        @click="pressKey(key)"
                    >
                        {{
                            key.action === "exponent" && exponentMode ?
                                "Exponent…" :
                                key.label
                        }}
                    </button>
                </section>

                <section
                    v-if="result"
                    class="result"
                    :aria-label="'Result: ' + accessibleResult"
                >
                    <strong>Result</strong>

                    <output>{{ resultText }}</output>

                    <div class="result-actions">
                        <button type="button" @click="useResult">Use result</button>
                        <button type="button" @click="copyResult">Copy</button>
                    </div>

                    <details>
                        <summary>Coefficient representation</summary>
                        <dl>
                            <dt>Numerator real</dt>
                            <dd>{{ result.numerator.real }}</dd>
                            <dt>Numerator imaginary</dt>
                            <dd>{{ result.numerator.imaginary }}</dd>
                            <dt>Denominator real</dt>
                            <dd>{{ result.denominator.real }}</dd>
                            <dt>Denominator imaginary</dt>
                            <dd>{{ result.denominator.imaginary }}</dd>
                        </dl>
                    </details>
                </section>
            </div>
        </main>
    </div>
</template>

<style src="./App.css"></style>
