#ifndef _ALU_CLIENT_LOG_HEADER_H_
#define _ALU_CLIENT_LOG_HEADER_H_

#if __cplusplus
extern "C" {
#endif

#define ALU_CLIENT_LOG(logger, level, fmt, ...) alu_client_log(logger, level, __FUNCTION__, __LINE__, fmt "\n", ##__VA_ARGS__)

enum
{
    ALU_CLIENT_LOG_LEVEL_EMERG = 0,
    ALU_CLIENT_LOG_LEVEL_ALERT = 1,
    ALU_CLIENT_LOG_LEVEL_CRIT = 2,
    ALU_CLIENT_LOG_LEVEL_ERROR = 3,
    ALU_CLIENT_LOG_LEVEL_WARN = 4,
    ALU_CLIENT_LOG_LEVEL_NOTICE = 5,
    ALU_CLIENT_LOG_LEVEL_INFO = 6,
    ALU_CLIENT_LOG_LEVEL_DEBUG = 7,
    ALU_CLIENT_LOG_LEVEL_UNKNOWN = 8,
    ALU_CLIENT_LOG_LEVEL_LAST = ALU_CLIENT_LOG_LEVEL_UNKNOWN
};

void alu_client_log(const alu_client_logger_t *logger, int level, const char *func, int line, const char *fmt, ...);

#if __cplusplus
}
#endif

#endif /* _ALU_CLIENT_LOG_HEADER_H_ */

