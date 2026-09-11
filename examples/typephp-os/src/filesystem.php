<?php

final class KernelOpenFile
{
    public string $name;
    public string $contents;
    public int $offset;
    public bool $writable;
    public bool $dirty;

    public function __construct(string $name, string $contents, int $offset, bool $writable)
    {
        $this->name = $name;
        $this->contents = $contents;
        $this->offset = $offset;
        $this->writable = $writable;
        $this->dirty = false;
    }
}

/*
 * Process-level filesystem state. The POSIX shim forwards here, so PHP's
 * unchanged plain-stream implementation ultimately executes TypePHP code.
 */
final class KernelFileSystem
{
    private Fat16Volume $volume;
    private bool $mounted = false;
    private array $files = [];

    public function initialize(): bool
    {
        if ($this->mounted) {
            return true;
        }
        $device = new AtaBlockDevice();
        if (!$device->available()) {
            return false;
        }
        $this->volume = new Fat16Volume($device);
        $volume = $this->volume->toObject(Fat16Volume::class);
        $this->mounted = $volume->mount();
        return $this->mounted;
    }

    private function rootName(string $path): string
    {
        $name = $path;
        while (strlen($name) > 0 && $name[0] === '/') {
            $name = substr($name, 1);
        }
        if (strlen($name) === 0) {
            return '';
        }
        for ($index = 0; $index < strlen($name); $index++) {
            if ($name[$index] === '/') {
                return '';
            }
        }
        return $name;
    }

    private function opened(int $fd): KernelOpenFile
    {
        return $this->files[$fd]->toObject(KernelOpenFile::class);
    }

    public function open(string $path, int $flags): int
    {
        if (!$this->initialize()) {
            return -5;
        }
        $name = $this->rootName($path);
        if (strlen($name) === 0) {
            return -21;
        }
        $volume = $this->volume->toObject(Fat16Volume::class);
        $type = $volume->rootEntryType($name);
        if ($type === 2) {
            return -21;
        }
        $create = ($flags & 64) !== 0;
        $truncate = ($flags & 512) !== 0;
        $append = ($flags & 1024) !== 0;
        $writable = ($flags & 3) !== 0;
        if ($type === 0 && !$create) {
            return -2;
        }
        $contents = $type === 1 ? $volume->readRootFile($name) : '';
        if ($truncate && $writable) {
            $contents = '';
        }
        $offset = $append ? strlen($contents) : 0;
        $fd = 3;
        while (isset($this->files[$fd])) {
            $fd++;
            if ($fd >= 64) {
                return -24;
            }
        }
        $file = new KernelOpenFile($name, $contents, $offset, $writable);
        $file->dirty = $type === 0 || ($truncate && $writable);
        $this->files[$fd] = $file;
        return $fd;
    }

    public function close(int $fd): int
    {
        if (!isset($this->files[$fd])) {
            return -9;
        }
        $file = $this->opened($fd);
        $volume = $this->volume->toObject(Fat16Volume::class);
        if ($file->dirty && !$volume->writeRootFile($file->name, $file->contents)) {
            return -5;
        }
        unset($this->files[$fd]);
        return 0;
    }

    public function read(int $fd, int $count): string
    {
        if (!isset($this->files[$fd]) || $count <= 0) {
            return '';
        }
        $file = $this->opened($fd);
        $data = substr($file->contents, $file->offset, $count);
        $file->offset += strlen($data);
        return $data;
    }

    public function write(int $fd, string $data): int
    {
        if (!isset($this->files[$fd])) {
            return -9;
        }
        $file = $this->opened($fd);
        if (!$file->writable) {
            return -9;
        }
        while (strlen($file->contents) < $file->offset) {
            $file->contents .= chr(0);
        }
        $prefix = substr($file->contents, 0, $file->offset);
        $after = $file->offset + strlen($data);
        $suffix = $after < strlen($file->contents) ? substr($file->contents, $after) : '';
        $file->contents = $prefix . $data . $suffix;
        $file->offset += strlen($data);
        $file->dirty = true;
        return strlen($data);
    }

    public function seek(int $fd, int $offset, int $whence): int
    {
        if (!isset($this->files[$fd])) {
            return -9;
        }
        $file = $this->opened($fd);
        $position = $offset;
        if ($whence === 1) {
            $position = $file->offset + $offset;
        } elseif ($whence === 2) {
            $position = strlen($file->contents) + $offset;
        } elseif ($whence !== 0) {
            return -22;
        }
        if ($position < 0) {
            return -22;
        }
        $file->offset = $position;
        return $position;
    }

    public function flush(int $fd): int
    {
        if (!isset($this->files[$fd])) {
            return -9;
        }
        $file = $this->opened($fd);
        if ($file->dirty) {
            $volume = $this->volume->toObject(Fat16Volume::class);
            if (!$volume->writeRootFile($file->name, $file->contents)) {
                return -5;
            }
            $file->dirty = false;
        }
        return 0;
    }

    public function truncate(int $fd, int $size): int
    {
        if (!isset($this->files[$fd]) || $size < 0) {
            return -22;
        }
        $file = $this->opened($fd);
        if (!$file->writable) {
            return -9;
        }
        if ($size < strlen($file->contents)) {
            $file->contents = substr($file->contents, 0, $size);
        } else {
            while (strlen($file->contents) < $size) {
                $file->contents .= chr(0);
            }
        }
        $file->dirty = true;
        return 0;
    }

    public function fileSize(string $path): int
    {
        if (!$this->initialize()) {
            return -5;
        }
        if ($path === '/' || $path === '') {
            return 0;
        }
        $name = $this->rootName($path);
        $volume = $this->volume->toObject(Fat16Volume::class);
        if (strlen($name) === 0 || !$volume->hasRootEntry($name)) {
            return -2;
        }
        return $volume->rootFileSize($name);
    }

    public function pathType(string $path): int
    {
        if (!$this->initialize()) {
            return 0;
        }
        if ($path === '/' || $path === '' || $path === '.') {
            return 2;
        }
        $name = $this->rootName($path);
        $volume = $this->volume->toObject(Fat16Volume::class);
        return strlen($name) === 0 ? 0 : $volume->rootEntryType($name);
    }

    public function fdSize(int $fd): int
    {
        return isset($this->files[$fd]) ? strlen($this->opened($fd)->contents) : -9;
    }

    public function makeDirectory(string $path): int
    {
        $name = $this->rootName($path);
        if (!$this->initialize() || strlen($name) === 0) {
            return -22;
        }
        $volume = $this->volume->toObject(Fat16Volume::class);
        if ($volume->hasRootEntry($name)) {
            return -17;
        }
        return $volume->makeRootDirectory($name) ? 0 : -5;
    }

    public function removeDirectory(string $path): int
    {
        $name = $this->rootName($path);
        if (!$this->initialize() || strlen($name) === 0) {
            return -22;
        }
        $volume = $this->volume->toObject(Fat16Volume::class);
        return $volume->removeRootDirectory($name) ? 0 : -39;
    }

    public function removeFile(string $path): int
    {
        $name = $this->rootName($path);
        if (!$this->initialize() || strlen($name) === 0) {
            return -22;
        }
        $volume = $this->volume->toObject(Fat16Volume::class);
        return $volume->removeRootFile($name) ? 0 : -2;
    }

    public function rename(string $oldPath, string $newPath): int
    {
        $oldName = $this->rootName($oldPath);
        $newName = $this->rootName($newPath);
        if (!$this->initialize() || strlen($oldName) === 0 || strlen($newName) === 0) {
            return -22;
        }
        $volume = $this->volume->toObject(Fat16Volume::class);
        return $volume->renameRootEntry($oldName, $newName) ? 0 : -2;
    }

    public function entries(string $path): string
    {
        if (!$this->initialize() || $this->pathType($path) !== 2) {
            return '';
        }
        if ($path !== '/' && $path !== '' && $path !== '.') {
            /* The first FAT16 slice has root directory entries but no nested
             * traversal yet. A root child directory is nevertheless a valid
             * working directory and contains its logical dot entries. */
            return ".\n..\n";
        }
        $volume = $this->volume->toObject(Fat16Volume::class);
        return ".\n..\n" . $volume->rootEntryNames();
    }
}
