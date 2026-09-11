<?php

#[NativeFunction]
function kernel_clear(int $color): void {}

#[NativeFunction]
function kernel_put_char(int $ascii, int $color): void {}

#[NativeFunction]
function kernel_write(string $text, int $color): void {}

#[NativeFunction]
function kernel_word_bits(): int { return 0; }

#[NativeFunction]
function kernel_memory_megabytes(): int { return 0; }

#[NativeFunction]
function kernel_chunk_smoke_test(): int { return 0; }

#[NativeFunction]
function kernel_halt(): void {}
