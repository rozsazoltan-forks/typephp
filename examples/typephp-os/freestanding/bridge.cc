#include <typephp_os_abi.h>
#include <php_nano_extension.h>
#include <phpx.h>
#include <phpx_helper.h>

void php_main();

extern "C" {
#include <zend_exceptions.h>
}

extern "C" void kernel_clear_c(int color);
extern "C" void kernel_put_char_c(int ascii, int color);
extern "C" void kernel_halt_c();
extern "C" unsigned long physical_memory_megabytes();
extern "C" unsigned long physical_chunk_smoke_test();

static void exception_model_smoke_test()
{
    php::enableDebugInfo(true);
    php::pushDebugFrame("kernel-smoke.php", 42, "Kernel::smoke");
    zend_throw_exception_ex(zend_ce_type_error, 0, "%s", "kernel exception");
    if (EG(exception) == nullptr || EG(exception)->ce != zend_ce_type_error) {
        typephp_os_panic("Zend TypeError construction failed");
    }

    php::augmentException();
    zval value;
    zval *file = zend_read_property_ex(
        zend_get_exception_base(EG(exception)), EG(exception), ZSTR_KNOWN(ZEND_STR_FILE), true, &value);
    if (Z_TYPE_P(file) != IS_STRING || !zend_string_equals_literal(Z_STR_P(file), "kernel-smoke.php")) {
        typephp_os_panic("PHPX exception augmentation failed");
    }

    zend_clear_exception();
    php::popDebugFrame();
    php::enableDebugInfo(false);
}

extern "C" void typephp_kernel_main()
{
    if (php_nano_startup_composer_extensions() != SUCCESS) {
        typephp_os_panic("Unable to start Nano extensions");
    }
    exception_model_smoke_test();
    php_main();
}

void php_kernel_clear(php::Int color)
{
    kernel_clear_c(static_cast<int>(color));
}

void php_kernel_put_char(php::Int ascii, php::Int color)
{
    kernel_put_char_c(static_cast<int>(ascii), static_cast<int>(color));
}

void php_kernel_write(php::Str text, php::Int color)
{
    for (size_t i = 0; i < text.length(); ++i) {
        kernel_put_char_c(static_cast<unsigned char>(text.data()[i]), static_cast<int>(color));
    }
}

php::Int php_kernel_word_bits()
{
    return static_cast<php::Int>(sizeof(php::Int) * 8);
}

php::Int php_kernel_memory_megabytes()
{
    return static_cast<php::Int>(physical_memory_megabytes());
}

php::Int php_kernel_chunk_smoke_test()
{
    return static_cast<php::Int>(physical_chunk_smoke_test());
}

void php_kernel_halt()
{
    kernel_halt_c();
}
