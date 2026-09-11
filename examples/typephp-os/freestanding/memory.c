typedef unsigned int uint32_t;
typedef unsigned long uint64_t;

extern void typephp_os_memory_init(void *address, unsigned long size);

enum {
    MULTIBOOT_INFO_MEMORY_MAP = 1u << 6,
};

static const uint64_t CHUNK_SIZE = 2ul * 1024ul * 1024ul;
/* Keep the complete TypePHP + PHP Nano payload away from the page allocator. */
static const uint64_t KERNEL_RESERVED_END = 16ul * 1024ul * 1024ul;
static const uint64_t IDENTITY_MAP_END = 1024ul * 1024ul * 1024ul;

typedef struct __attribute__((packed)) {
    uint32_t size;
    uint64_t address;
    uint64_t length;
    uint32_t type;
} multiboot_memory_entry;

typedef struct __attribute__((packed)) {
    uint32_t flags;
    uint32_t memory_lower;
    uint32_t memory_upper;
    uint32_t boot_device;
    uint32_t command_line;
    uint32_t modules_count;
    uint32_t modules_address;
    uint32_t symbols[4];
    uint32_t memory_map_length;
    uint32_t memory_map_address;
} multiboot_info;

static uint64_t usable_memory_bytes;
static uint64_t chunk_cursor;
static uint64_t chunk_end;

static uint64_t align_up(uint64_t value, uint64_t alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

void physical_memory_init(uint64_t multiboot_info_address)
{
    const multiboot_info *info =
        (const multiboot_info *) (unsigned long) multiboot_info_address;
    usable_memory_bytes = 0;
    chunk_cursor = 0;
    chunk_end = 0;

    if ((info->flags & MULTIBOOT_INFO_MEMORY_MAP) == 0) {
        return;
    }

    uint64_t position = info->memory_map_address;
    const uint64_t end = position + info->memory_map_length;
    while (position + sizeof(multiboot_memory_entry) <= end) {
        const multiboot_memory_entry *entry =
            (const multiboot_memory_entry *) (unsigned long) position;
        if (entry->size < sizeof(multiboot_memory_entry) - sizeof(uint32_t)) {
            break;
        }

        if (entry->type == 1) {
            usable_memory_bytes += entry->length;
            uint64_t region_end = entry->address + entry->length;
            if (region_end > IDENTITY_MAP_END) {
                region_end = IDENTITY_MAP_END;
            }
            uint64_t region_start = entry->address;
            if (region_start < KERNEL_RESERVED_END) {
                region_start = KERNEL_RESERVED_END;
            }
            region_start = align_up(region_start, CHUNK_SIZE);
            if (region_start + CHUNK_SIZE <= region_end
                && region_end - region_start > chunk_end - chunk_cursor) {
                chunk_cursor = region_start;
                chunk_end = region_end;
            }
        }
        position += (uint64_t) entry->size + sizeof(entry->size);
    }

    if (chunk_cursor != 0 && chunk_end > chunk_cursor) {
        typephp_os_memory_init(
            (void *) (unsigned long) chunk_cursor,
            (unsigned long) (chunk_end - chunk_cursor));
    }
}

uint64_t physical_memory_megabytes(void)
{
    return usable_memory_bytes >> 20;
}
