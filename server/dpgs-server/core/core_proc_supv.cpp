#include "core_proc_supv.h"
#include "ai_engine.h"

#include <iostream>
#include <sys/wait.h>
#include <unistd.h>


// === Signal Handler ===
static AIEngine* g_engine = nullptr;

void sigusr2_handler(int) {
    if (g_engine) {
        std::cout << "[PROC_AI] sig_handler: SIGTERM receviced. Stopping AIEngine...\n";
        g_engine->stop();
    }
}

void block_all_except_sigusr2() {
    sigset_t sigset;
    sigfillset(&sigset);
    sigdelset(&sigset, SIGUSR2);
    pthread_sigmask(SIG_SETMASK, &sigset, nullptr);
}
// ===================


// === CoreProcSupv ===
CoreProcSupv::CoreProcSupv(FrameBuffer& _fb, MapManager& _map_mgr)
    : fb(_fb), map_mgr(_map_mgr) {
}

CoreProcSupv::~CoreProcSupv() {
}


bool CoreProcSupv::initialize() {
    std::cout << "[PROC_SUPV] Start to initialize...\n";

    std::cout << "[PROC_SUPV] Success: Process Supervisor initialized\n";
    return true;
}


void CoreProcSupv::start() {
    std::cout << "[PROC_SUPV] Start Process Supervisor\n";

    pid_t pid = fork();

    if (pid == 0) {
        std::cout << "[PROC_AI] Sub-process startd (PID: " << getpid() << ")\n";

        block_all_except_sigusr2();

        AIEngine engine(fb, map_mgr);
        if (!engine.initialize()) {
            std::cout << "[PROC_AI] Error: Failed to initialize AI Engine\n";
            ::exit(0);
        }
        g_engine = &engine;

        engine.run();

        std::cout << "[PROC_AI] Sub-process exiting...\n";
        ::exit(0);
    }
    else if (pid > 0) {
        pid_ai = pid;
        is_running_ai = true;
        std::cout << "[PROC_SUPV] Forked AIEngine with PID: " << pid_ai << "\n";
    }
    else {
        perror("fork");
    }

}


void CoreProcSupv::stop() {
    std::cout << "[PROC_SUPV] Process Supervisor Terminating...\n";

    if (is_running_ai && (pid_ai > 0)) {
        std::cout << "[PROC_SUPV] stop: Sending SIGTERM to AIEngine (PID: " << pid_ai << ")\n";
        kill(pid_ai, SIGUSR2);
        waitpid(pid_ai, nullptr, 0);
        is_running_ai = false;
    }

    clear();

    std::cout << "[PROC_SUPV] Process Supervisor Terminated\n";
}


bool CoreProcSupv::monitor() {
    if (!is_running_ai || pid_ai <= 0) return false;

    int status;
    pid_t result = waitpid(pid_ai, &status, WNOHANG);

    if (result == 0) return true;
    if (result == pid_ai) {
        is_running_ai = false;

        if (WIFEXITED(status)) {
            std::cout << "[PROC_SUPV] AIEngine exited normally with code: " << WEXITSTATUS(status) << "\n";
        }
        else if (WIFSIGNALED(status)) {
            std::cout << "[PROC_SUPV] AIEngine terminated by signal: " << WTERMSIG(status) << "\n";
        }
        else {
            std::cout << "[PROC_SUPV] AIEngine exited with unknown status\n";
        }

    }
    else {
        perror("waitpid");
    }

    return is_running_ai;
}


void CoreProcSupv::clear() {
    std::cout << "[PROC_SUPV] clear: Cleanning...\n";

    std::cout << "[PROC_SUPV] clear: Cleanning Success\n";
}

