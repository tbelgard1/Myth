#ifndef __UPDATE_DAT_H
#define __UPDATE_DAT_H

// Application entry structure
struct update_app_entry {
    long version_number;
    long platform_type;
    long size;
    char* path;
};

// Patch entry structure
struct update_patch_entry {
    long patch_number;
    long size;
    char* description;
    char* path;
};

// Platform types
enum {
    _platform_windows = 0,
    _platform_mac,
    _platform_linux
};

// Country codes
enum {
    _country_us = 0,
    _country_uk,
    _country_france,
    _country_germany,
    _country_italy,
    _country_spain,
    _country_japan
};

#endif // __UPDATE_DAT_H