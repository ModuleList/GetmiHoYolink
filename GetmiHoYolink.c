#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>

FILE *logFile;

#define LOG_FORMAT "[%s]%s\n"

char* getCurrentTime() {
    time_t currentTime;
    time(&currentTime);

    // 定义一个缓冲区，用于存储格式化后的时间字符串
    static char timeString[20]; // 大小足够存储 "YYYY/MM/DD HH:MM\0"

    // 使用strftime函数格式化时间
    strftime(timeString, sizeof(timeString), "%Y/%m/%d %H:%M", localtime(&currentTime));

    char* newlinePos = strchr(timeString, '\n');
    if (newlinePos != NULL) {
        *newlinePos = '\0';  // 替换换行符为字符串结束符
    }
    return timeString;
}

int initLog() {
    // 清空并创建日志文件
    FILE *newLogFile = fopen("run.log", "w");
    if (newLogFile == NULL) {
        fprintf(stderr, "Error: Failed to clear and create log file.\n");
        return 1;
    } else {
        fclose(newLogFile);
    }
    logFile = fopen("run.log", "a+");
    // 检查文件是否成功打开
    if (logFile == NULL) {
        fprintf(stderr, "Error: Failed to open log file.\n");
        return 1;
    }

    return 0;
}


void writeLog(const char *text) {
    fprintf(logFile, LOG_FORMAT, getCurrentTime(), text);
    fflush(logFile);
    printf(LOG_FORMAT, getCurrentTime(), text);
}

void closeLog() {
    fclose(logFile);
}

void openUrl(const char *url) {
    writeLog("跳转浏览器");
    char command[1024];
    snprintf(command, sizeof(command), "am start -a android.intent.action.VIEW -d \"%s\"", url);
    system(command);
}

void processUrl(const char *url) {
    if (url && strlen(url) > 0) {
        openUrl(url);
    } else {
        fprintf(stderr, "Error: Empty URL\n");
    }
}

int start() {
    char buffer[1024];
    
    FILE *logcatFile = popen("logcat -m 1 -e \"OnGetWebViewPageFinish.+https.+\"", "r");
    writeLog("logcat运行");
    if (!logcatFile) {
        fprintf(stderr, "Error: Failed to open logcat\n");
        return 1;
    }

    while (fgets(buffer, sizeof(buffer), logcatFile) != NULL) {
        char *url = strstr(buffer, "https://webstatic.mihoyo.com");
        if (url) {
            url[strcspn(url, "\r\n")] = '\0';  // 移除换行符
            writeLog("已获取到链接:");
            writeLog(url);
            processUrl(url);
        }
    }

    pclose(logcatFile);
    return 0;
}

int isProcessRunning(const char *PackageName) {
    DIR *dir = opendir("/proc");
    struct dirent *ptr = NULL;
    char filepath[32];
    
    if (NULL != dir) {
        while ((ptr = readdir(dir)) != NULL) {
            if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
                continue;
            
            if (ptr->d_type != DT_DIR)
                continue;
                
            snprintf(filepath, sizeof(filepath), "/proc/%s/cmdline", ptr->d_name);
            // 使用动态分配的内存来存储命令行参数
            char *filetext = NULL;
            FILE *fp = fopen(filepath, "r");
            
            if (NULL != fp) {
                size_t len = 0;
                ssize_t read = getline(&filetext, &len, fp);
                fclose(fp); // 关闭文件
                
                if (read != -1) {
                    // 移除命令行参数中的换行符
                    char *newline = strchr(filetext, '\n');
                    if (newline) {
                        *newline = '\0';
                    }
                    
                    if (strcmp(filetext, PackageName) == 0) {
                        free(filetext); // 释放动态分配的内存
                        closedir(dir);
                        return atoi(ptr->d_name); // 返回进程的PID
                    }
                }
                
                free(filetext); // 释放动态分配的内存
            }
        }
        closedir(dir);
    }
    
    return 0; // 如果未找到进程，返回0
}

int main(int argc, char *argv[]) {
    initLog();
    if (argc > 1 && strcmp(argv[1], "start") == 0) {
        writeLog("当前为:监听模式");
        int alreadyRun = 0;  // 重置
        
        while (1) {
            int running = 0;
            
            if (isProcessRunning("com.miHoYo.Yuanshen") || isProcessRunning("com.miHoYo.hkrpg")) {
                writeLog("监听到游戏已启动");
                running = 1;
            }
            
            if (running && !alreadyRun) {
                start();
                alreadyRun = 1;  // 已运行
            } else if (!running) {
                alreadyRun = 0;  // 重置
            }
            writeLog("休眠");
            sleep(60); // 暂停一分钟
        }
    } else {
        writeLog("当前为:立即执行模式");
        start();
    }
    closeLog();
    return 0;
}