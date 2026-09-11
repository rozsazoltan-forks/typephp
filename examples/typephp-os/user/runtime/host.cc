#include <php_nano_extension.h>
#include <typephp_os_abi.h>

#include <cstdint>
#include <cstring>
#include <sys/syscall.h>
#include <unistd.h>

extern "C" int typephp_nano_project_main();
extern "C" char **environ;

namespace {

/* Zend MM acquires aligned 2 MiB chunks while PHPX's native object GC also
 * keeps process-lifetime metadata. A 48 MiB arena leaves enough headroom for
 * the complete Nano module startup in the current 512 MiB QEMU machine. */
constexpr std::uintptr_t arena_size = 48u * 1024u * 1024u;

[[noreturn]] void raw_exit(int status)
{
    (void) syscall(SYS_exit, status);
    for (;;) {
        __asm__ volatile("pause");
    }
}

} // namespace

extern "C" void typephp_os_write(const char *data, size_t size)
{
    (void) syscall(SYS_write, STDOUT_FILENO, data, size);
}

extern "C" void typephp_os_panic(const char *message)
{
    static constexpr char prefix[] = "TypePHP user panic: ";
    typephp_os_write(prefix, sizeof(prefix) - 1);
    typephp_os_write(message, std::strlen(message));
    typephp_os_write("\n", 1);
    raw_exit(127);
}

extern "C" void phpx_no_exception_abort(const char *message)
{
    typephp_os_panic(message);
}

extern "C" void php_nano_host_system_time(
    std::int64_t *seconds, std::int32_t *microseconds)
{
    *seconds = static_cast<std::int64_t>(syscall(SYS_time, nullptr));
    *microseconds = 0;
}

extern "C" std::uint64_t php_nano_host_monotonic_nanoseconds()
{
    return static_cast<std::uint64_t>(syscall(SYS_time, nullptr))
        * UINT64_C(1000000000);
}

extern "C" void php_nano_host_sleep(
    std::uint64_t seconds, std::uint32_t nanoseconds)
{
    const auto now = static_cast<std::uint64_t>(syscall(SYS_time, nullptr));
    const auto deadline = now + seconds + (nanoseconds != 0 ? 1u : 0u);
    while (static_cast<std::uint64_t>(syscall(SYS_time, nullptr)) < deadline) {
        __asm__ volatile("pause");
    }
}

int main(int argc, char **argv)
{
    const auto begin = static_cast<std::uintptr_t>(syscall(SYS_brk, 0));
    const auto end = begin + arena_size;
    if (begin == static_cast<std::uintptr_t>(-1)
        || end < begin
        || static_cast<std::uintptr_t>(syscall(SYS_brk, end)) != end) {
        typephp_os_panic("unable to allocate the Nano runtime arena");
    }
    typephp_os_memory_init(reinterpret_cast<void *>(begin), arena_size);
    environ = argv + argc + 1;
    php_nano_set_cli_arguments(argc, argv);
    if (php_nano_startup_composer_extensions() != SUCCESS) {
        typephp_os_panic("unable to start PHP Nano extensions");
    }
    const int status = typephp_nano_project_main();
    php_nano_shutdown_composer_extensions();
    return status;
}
