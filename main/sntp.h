/**
 * @file sntp.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-07-08
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "esp_log.h"
#include "esp_err.h"


class SyncTime {
private:
    struct tm datetime_ = {};

public:
    esp_err_t get_local_time(void);
    esp_err_t sync(void);
};

