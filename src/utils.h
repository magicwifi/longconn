#define PORT 80
#define USERAGENT "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/30.0.1599.114 Safari/537.36"
#define ACCEPTLANGUAGE "zh-CN,zh;q=0.8,en;q=0.6,en-US;q=0.4,en-GB;q=0.2"
#define ACCEPTENCODING "gzip,deflate,sdch"

#define MAX_BUF 8192
#define IPSTR "117.34.78.195"

#define LOCK_CONFIG() do { \
	pthread_mutex_lock(&config_mutex); \
} while (0)

#define UNLOCK_CONFIG() do { \
	pthread_mutex_unlock(&config_mutex); \
} while (0)


#define LOCK_GHBN() do { \
	pthread_mutex_lock(&ghbn_mutex); \
} while (0)

#define UNLOCK_GHBN() do { \
	pthread_mutex_unlock(&ghbn_mutex); \
} while (0)


int create_tcp_socket();
int fprintf_locked(FILE *fp, const char *format, ...);
pid_t safe_fork(void);
int execute(const char *cmd_line, int quiet);
