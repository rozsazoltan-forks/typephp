<?php

namespace TypePhp\Analysis;

/**
 * Usage information collected while TypePHP emits the final C++ program.
 *
 * This collector is intentionally independent from Nano. Build modules may
 * reuse it for dependency selection, diagnostics, or compilation reports.
 */
final class CompilationStatistics
{
    public const string FUNCTIONS = 'functions';
    public const string DIRECT_FUNCTIONS = 'direct-functions';
    public const string RUNTIME_FUNCTIONS = 'runtime-functions';
    public const string CLASSES = 'classes';
    public const string TYPES = 'types';
    public const string DYNAMIC_CAPABILITIES = 'dynamic-capabilities';

    /** @var array<string, array<string, int>> */
    private array $counters = [];

    private bool $collecting = false;

    public function begin(): void
    {
        $this->counters = [];
        $this->collecting = true;
    }

    public function finish(): void
    {
        $this->collecting = false;
    }

    public function isCollecting(): bool
    {
        return $this->collecting;
    }

    public function record(string $category, string $name): void
    {
        if (!$this->collecting || $category === '' || $name === '') {
            return;
        }
        $this->counters[$category][$name] = ($this->counters[$category][$name] ?? 0) + 1;
    }

    /** @return array<string, int> */
    public function get(string $category): array
    {
        $values = $this->counters[$category] ?? [];
        ksort($values, SORT_STRING);
        return $values;
    }

    public function has(string $category, string $name): bool
    {
        return isset($this->counters[$category][$name]);
    }

    /** @return array<string, array<string, int>> */
    public function all(): array
    {
        $result = $this->counters;
        ksort($result, SORT_STRING);
        foreach ($result as &$values) {
            ksort($values, SORT_STRING);
        }
        unset($values);
        return $result;
    }
}
