<?php

final class KernelGreeting
{
    public function render(string $now): string
    {
        return $now . ' Hello TypePHP-OS!';
    }
}

function writeLine(string $text, int $color): void
{
    kernel_write($text, $color);
    kernel_put_char(10, $color);
}

function writeNumber(int $number, int $color): void
{
    if ($number === 0) {
        kernel_put_char(48, $color);
        return;
    }

    $digits = std::vector(Type::Int);
    while ($number > 0) {
        $digit = $number % 10;
        $digits[] = $digit;
        $number = (int) (($number - $digit) / 10);
    }

    for ($index = count($digits) - 1; $index >= 0; $index--) {
        kernel_put_char(48 + $digits[$index], $color);
    }
}

function isPrime(int $number): bool
{
    if ($number < 2) {
        return false;
    }
    for ($divisor = 2; $divisor * $divisor <= $number; $divisor++) {
        if ($number % $divisor === 0) {
            return false;
        }
    }
    return true;
}

function runFeatureSelfCheck(): void
{
    kernel_clear(0);
    writeLine('TypePHP OS POC', 10);

    kernel_write('Int bits: ', 15);
    writeNumber(kernel_word_bits(), 15);
    kernel_put_char(10, 15);

    kernel_write('RAM MiB: ', 15);
    writeNumber(kernel_memory_megabytes(), 15);
    kernel_put_char(10, 15);

    kernel_write('Zend MiB: ', 15);
    writeNumber(kernel_chunk_smoke_test(), 15);
    kernel_put_char(10, 15);

    $checks = ['Zend string/array: OK', 'Kernel is!'];
    foreach ($checks as $check) {
        writeLine($check, 10);
    }
}

function runPrimeDemo(int $limit): void
{
    $primes = std::vector(Type::Int);
    for ($candidate = 0; $candidate <= $limit; $candidate++) {
        if (isPrime($candidate)) {
            $primes[] = $candidate;
        }
    }

    kernel_write('Calculate primes: 0-', 11);
    writeNumber($limit, 11);
    kernel_put_char(10, 11);

    kernel_write('Prime count: ', 15);
    writeNumber(count($primes), 15);
    kernel_put_char(10, 15);

    kernel_write('Prime list: ', 15);
    for ($index = 0; $index < count($primes); $index++) {
        if ($index !== 0) {
            kernel_write(', ', 15);
        }
        writeNumber($primes[$index], 15);
    }
    kernel_put_char(10, 15);
}

function main(): void
{
    runFeatureSelfCheck();
    runPrimeDemo(100);

    $greeting = new KernelGreeting();
    while (true) {
        sleep(2);
        kernel_write($greeting->render(date('Y-m-d H:i:s')), 15);
        kernel_put_char(10, 15);
    }
}
