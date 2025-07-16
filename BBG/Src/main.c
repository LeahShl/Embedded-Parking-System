/**
 * @file main.c
 * @author Leah
 * @brief Parking system project file for BBG
 * @date 2025-07-14
 */
#include "config.h"
#include "i2c_process.h"
#include "eth_process.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>

#define CONFIG_PATH     "/etc/parksys/parksys.config"
#define SERVICE_PATH    "/etc/systemd/system/parksys.service"
#define SERVICE_NAME    "parksys.service"
#define LOG_PATH        "/var/log/parksys/parksys.log"
#define BIN_PATH        "/usr/sbin/parksys"

/**
 * @brief Installs the program on the system 
 * 
 * @return int 1 if a first time install occured, 0 otherwise
 */
static int first_time_install(void);

/**
 * @brief Display program status to stdout
 * 
 */
static void print_status();

/**
 * @brief Uninstall the program from the system
 * 
 */
static void uninstall();

int main(int argc, char *argv[])
{
    if (argc == 2 && strcmp(argv[1], "status") == 0)
    {
        print_status();
        return 0;
    }
    else if (argc == 2 && strcmp(argv[1], "uninstall") == 0)
    {
        if (geteuid() != 0)
        {
            fprintf(stderr, "uninstall must be run as root\n");
            return 1;
        }
        uninstall();
        return 0;
    }

    if (first_time_install() == 1)
    {
        printf("[INIT] First-time setup done.\n");
        printf("Please start the service manually with:\n");
        printf("  sudo systemctl start parksys\n");
        return EXIT_SUCCESS; 
    }

    Config cfg;
    if (load_config(&cfg) != 0)
    {
        fprintf(stderr, "Failed to load config\n");
        return EXIT_FAILURE;
    }

    printf("Loaded config:\n");
    printf("  i2c_bus     = %s\n", cfg.i2c_bus);
    printf("  i2c_addr    = 0x%02X\n", cfg.i2c_addr);
    printf("  server_ip   = %s\n", cfg.server_ip);
    printf("  server_port = %d\n", cfg.server_port);
    printf("  log_path    = %s\n", cfg.log_path);
    printf("  service_name= %s\n", cfg.service_name);
    printf("  service_path= %s\n\n", cfg.service_path);

    int pipefd[2];
    if (pipe(pipefd) < 0)
    {
        perror("pipe");
        return 1;
    }

    if (daemon(0, 0) < 0)
    {
        perror("daemon");
        return EXIT_FAILURE;
    }

    // Make sure systemd knows who's the parent
    FILE *pid_file = fopen("/run/parksys.pid", "w");
    if (pid_file)
    {
        fprintf(pid_file, "%d\n", getpid());
        fclose(pid_file);
    }


    pid_t i2c_pid = fork();
    if (i2c_pid == 0)
    {
        close(pipefd[0]);
        openlog("parksys-i2c", LOG_PID | LOG_CONS, LOG_DAEMON);
        run_i2c_process(pipefd[1], &cfg);

        exit(EXIT_FAILURE); // Shouldn't reach here
    }

    pid_t eth_pid = fork();
    if (eth_pid == 0)
    {
        /* 
          Temporary closed socket (for example, server briefly disconnects)
          will cause a SIGPIPE, which normally kills the eth proccess. Ignoring this
          signal will keep the system running even when the server is unavailable.
        */
        signal(SIGPIPE, SIG_IGN);

        close(pipefd[1]); 
        openlog("parksys-eth", LOG_PID | LOG_CONS, LOG_DAEMON);
        run_eth_process(pipefd[0], &cfg);

        exit(EXIT_FAILURE); // Shouldn't reach here
    }

    // Shouldn't reach here
    int status;
    waitpid(i2c_pid, &status, 0);
    waitpid(eth_pid, &status, 0);
    return EXIT_SUCCESS;
}

static int first_time_install(void)
{
    struct stat st;

    if (stat(CONFIG_PATH, &st) == 0 && stat(SERVICE_PATH, &st) == 0)
        return 0; // Already installed

    printf("[INIT] First-time setup: installing config and service...\n");

    // Create /etc/parksys and /var/log/parksys
    if (mkdir("/etc/parksys", 0755) < 0 && errno != EEXIST)
        perror("mkdir /etc/parksys");

    if (mkdir("/var/log/parksys", 0755) < 0 && errno != EEXIST)
        perror("mkdir /var/log/parksys");

    // Copy binary to /usr/sbin
    char self_path[1024];
    ssize_t len = readlink("/proc/self/exe", self_path, sizeof(self_path) - 1);
    if (len < 0)
    {
        perror("readlink");
        return -1;
    }
    self_path[len] = '\0';

    printf("[INIT] Copying binary from %s to %s\n", self_path, BIN_PATH);
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "cp \"%s\" " BIN_PATH, self_path);
    if (system(cmd) != 0)
    {
        fprintf(stderr, "Failed to copy binary to %s\n", BIN_PATH);
        return -1;
    }

    // Write default config
    FILE *fcfg = fopen(CONFIG_PATH, "w");
    if (!fcfg)
    {
        perror("fopen config");
        return -1;
    }
    fprintf(fcfg,
            "i2c_bus=/dev/i2c-1\n"
            "i2c_addr=0x10\n"
            "server_ip=192.168.1.71\n"
            "server_port=12321\n"
            "log_path=%s\n"
            "service_name=%s\n"
            "service_path=%s\n",
            LOG_PATH, SERVICE_NAME, SERVICE_PATH);
    fclose(fcfg);

    // Write service file
    FILE *fsvc = fopen(SERVICE_PATH, "w");
    if (!fsvc)
    {
        perror("fopen service");
        return -1;
    }

    fprintf(fsvc,
            "[Unit]\n"
            "Description=Parking System Daemon\n"
            "After=network.target\n\n"
            "[Service]\n"
            "ExecStart=/usr/sbin/parksys\n"
            "PIDFile=/run/parksys.pid\n"
            "Restart=always\n\n"
            "[Install]\n"
            "WantedBy=multi-user.target\n");
    fclose(fsvc);

    // Reload and enable service
    printf("[INIT] Enabling service...\n");
    system("systemctl daemon-reexec");
    system("systemctl daemon-reload");
    system("systemctl enable parksys.service");

    printf("[INIT] Setup complete. starting service.\n");

    return 1; // Signal that setup occurred
}

static void print_status()
{
    int status = system("systemctl is-active --quiet parksys.service");
    if (status == 0)
        printf("parksys is running\n");
    else
        printf("parksys is NOT running\n");
}

static void uninstall()
{
    printf("[UNINSTALL] Stopping and disabling service...\n");
    system("systemctl stop parksys.service");
    system("systemctl disable parksys.service");
    printf("[UNINSTALL] Removing files...\n");
    remove(SERVICE_PATH);
    remove(CONFIG_PATH);
    remove(LOG_PATH);
    remove(BIN_PATH);
    rmdir("/etc/parksys");
    rmdir("/var/log/parksys");
    printf("[UNINSTALL] Uninstall complete.\n");
}