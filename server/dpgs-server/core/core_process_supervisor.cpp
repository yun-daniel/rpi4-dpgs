#include "core_process_supervisor.h"
#include "ai_engine.h"

#include <iostream>
#include <sys/wait.h>
#include <unistd.h>


CoreProcessSupervisor::CoreProcessSupervisor(FrameBuffer& _fb, MapManager& _map_mgr)
    : fb(_fb), map_mgr(_map_mgr) {
}

CoreProcessSupervisor::~CoreProcessSupervisor() {
}


void CoreProcessSupervisor::start() {
    std::cout << "[PROC_SUPV] Start Process Supervisor\n";

    pid_t pid = fork();

    if (pid == 0) {
        std::cout << "[PROC_AI] Subprocess startd (PID: " << getpid() << ")\n";

        AIEngine engine(fb, map_mgr);
        engine.run();

        std::cout << "[PROC_AI] Subprocess exiting...\n";
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


void CoreProcessSupervisor::stop() {
    std::cout << "[PROC_SUPV] Stop Process Supervisor\n";

}


bool CoreProcessSupervisor::monitor() {
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


bool CoreProcessSupervisor::create_process() {
    std::cout << "[PROC_SUPV] Create Process...\n";


    std::cout << "[PROC_SUPV] Success: Process created\n";
    return true;
}


bool CoreProcessSupervisor::clear() {
    std::cout << "[PROC_SUPV] Cleanning...\n";


    std::cout << "[PROC_SUPV] Success: Process clear\n";
    return true;
}

