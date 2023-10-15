#include <bits/stdc++.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>
#include <setjmp.h>
#include <fcntl.h>
#include <termios.h>
#include <glob.h>
#include "utilities.h"
#include "sig.h"
#include <fstream>
#include <sstream>
#include <climits>
#include <dirent.h>
#include <sys/types.h>
#include <set>
#include <utility>

using std::cout, std::endl;

// TROJAN DETECTION CODE


long clocks_per_sec = sysconf(_SC_CLK_TCK);

// Returns the parent process ID of the given process
int get_ppid(int pid) {
    char stat_path[32];

    sprintf(stat_path, "/proc/%d/stat", (int) pid);
    FILE *f;
    f = fopen(stat_path, "r");
    if (f == NULL) {
        return -1;
    }

    char buff[256];
    if (fgets(buff, sizeof(buff), f) == NULL) {
        return -1;
    }

    fclose(f);
    char *ptr = buff;

    int i;
    for (i = 0; i < 3; i++) {
        while (*ptr != ' ') ptr++;
        while (*ptr == ' ') ptr++;
    }

    int ppid = strtol(ptr, &ptr, 10);

    return ppid;
}

// Returns the utime and stime of the given process
void getinfo(int pid, int &children_count, long long &utime, long long &stime, int &state) {
    char task_path[32];
    char stat_path[32];
    DIR *dir;
    struct dirent *entry;

    sprintf(task_path, "/proc/%d/task", (int) pid);
    dir = opendir(task_path);
    if (dir == NULL) {
        return;
    }

    children_count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        children_count++;
    }

    closedir(dir);

    sprintf(stat_path, "/proc/%d/stat", (int) pid);
    FILE *f;
    f = fopen(stat_path, "r");
    if (f == NULL) {
        return;
    }

    char buff[256];
    if (fgets(buff, sizeof(buff), f) == NULL) {
        return;
    }

    fclose(f);
    char *ptr = buff;

    int i;
    for (i = 0; i < 2; i++) {
        while (*ptr != ' ') ptr++;
        while (*ptr == ' ') ptr++;
    }

    if (*ptr == 'S') state = 0;
    else state = 1;

    while (*ptr != ' ') ptr++;
    while (*ptr == ' ') ptr++;

    for (; i < 13; i++) {
        while (*ptr != ' ') ptr++;
        while (*ptr == ' ') ptr++;
    }

    utime = strtol(ptr, &ptr, 10);
    stime = strtol(ptr, &ptr, 10);

    return;
}

// END OF TROJAND DETECTION

// DELEP STUFF
using namespace std;

vector<pid_t> get_lock_processes(string filepath) {
    string cmd = "fuser " + filepath;
    vector<pid_t> lock_processes;

    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe) return lock_processes;

    char buffer[128];
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != nullptr) {
            lock_processes.push_back(stoi(buffer));
        }
    }
    pclose(pipe);

    return lock_processes;
}

void kill_processes(vector<pid_t> processes) {
    for (auto pid : processes) {
        kill(pid, SIGKILL);
    }
}

// END OF DELEP STUFF



// default output redirect file permission
#define OUT_REDIR_PERM 0664
// maximum number of arguments to a command (including cmd name)
#define MAX_ARGS 100
char const PIPE_DELIM = '|',
    SPACE_DELIM = ' ';

char const *PROMPT = ">>> ";

#define HISTFILE ".gpt_history"
#define HISTSIZE 1000


// job control adapted from:
// https://www.gnu.org/software/bash/manual/html_node/Job-Control-Basics.html

int foreground_proc_gid = 0; // initially foreground proc will be the shell
std::string fg_name = "shell";
std::map<int, std::string> bg_procs; // no bg procs initially
std::map<int, int> pid2gpid;
bool return2shell = false; // initially the executed process will hold fg

// signal handlers
static void sigint_handler(int sig) {
    INSTALL_HANDLER(sig, sigint_handler);
    printf("\n");
    rl_on_new_line();
    siglongjmp(ctrlc_start_point, 1);
}

static void sigchld_handler(int sig) {
    INSTALL_HANDLER(SIGCHLD, sigchld_handler);
    // multiple children may have caused SIGCHLD
    // but only one SIGNAL will be placed
    // Therefore, need to handle all SIGCHLD triggering
    // children in one go
    while (1) {
        int status;
        // WNOHANG -> return immediately if no children causing SIGCHLD-behavior
        // WUNTRACED -> return if a child was suspended (needed for bg processes)
        int cpid = waitpid(-1, &status, WNOHANG | WUNTRACED);
        if (cpid <= 0) break; // nothing to do (cases: WNOHANG or error)
        if (pid2gpid[cpid] == foreground_proc_gid) {
            foreground_proc_gid = 0; // shell takes control of foreground
            if (WIFSTOPPED(status)) {
                assert(bg_procs.count(pid2gpid[cpid]) == 0);
                bg_procs[pid2gpid[cpid]] = fg_name;
            }
            fg_name = "shell";
            return2shell = true;
            tcsetpgrp(STDIN_FILENO, getpgrp());
        } else {
            int x = pid2gpid[cpid];
            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                bg_procs.erase(x); // But no longer, I'm dead now
                pid2gpid.erase(cpid);
            }
            // nothing to do if bg process was "stopped"
        }
    }
}

static void sigtstp_handler(int sig) {
    INSTALL_HANDLER(SIGTSTP, sigtstp_handler);
    // do nothing
}

// Command class adapted from:
// https://www.cs.purdue.edu/homes/grr/SystemsProgrammingBook/Book/Chapter5-WritingYourOwnShell.pdf
class command {
  std::string cmd;
  char *args[MAX_ARGS] = {nullptr};
  std::string output_file, input_file;

  void set_cmd_args(std::string const &cmdPlusArgs);

 public:
  static std::vector<command> parse_string(char *user_input, bool &background);

  friend std::ostream &operator<<(std::ostream &out, command const &c);

  friend void execute_pipeline(std::vector<command> &cmds, bool background);
};

std::ostream &operator<<(std::ostream &out, command const &c) {
    out << "Command Name: \"" << c.cmd << "\"" << std::endl;
    out << "Args:" << std::endl;
    for (int i = 0; i < MAX_ARGS && c.args[i] != nullptr; i++) {
        out << "arg[" << i << "]=\"" << c.args[i] << "\"" << std::endl;
    }
    out << "Output file: \"" << c.output_file << "\"" << std::endl;
    out << "Input file: \"" << c.input_file << "\"" << std::endl;
    return out;
}

static void print_jobs() {
    for (auto job : bg_procs)
        std::cout << job.first << " --> " << job.second << std::endl;
}

void command::set_cmd_args(std::string const &cmdPlusArgs) {
    std::stringstream ss(cmdPlusArgs);
    int cnt = 0;
    // set command name
    std::getline(ss, cmd, SPACE_DELIM);
    args[cnt++] = strdup(cmd.c_str());
    std::string cur_token;
    while (std::getline(ss, cur_token, ' ')) {
        if (cur_token.empty()) continue;
        args[cnt++] = strdup(cur_token.c_str());
    }
}

std::vector<command> command::parse_string(char *user_input, bool &background) {
    if (user_input[strlen(user_input) - 1] == '&') {
        background = true;
        user_input[strlen(user_input) - 1] = 0;
    } else background = false;
    // split the string into separate commands based
    // on pipeline delimiter
    std::vector<std::string> cmd_strings;
    std::stringstream ss(user_input);
    std::string current_cmdstring;
    while (std::getline(ss, current_cmdstring, PIPE_DELIM))
        cmd_strings.push_back(current_cmdstring);
    std::vector<command> retVal;
    for (auto const &cmd : cmd_strings) {
        command new_command;

        auto infile_pos = cmd.find('<');
        auto outfile_pos = cmd.find('>');
        if (infile_pos != std::string::npos) {
            auto substring_len =
                (outfile_pos != std::string::npos && infile_pos < outfile_pos) ? outfile_pos - infile_pos - 1
                                                                               : std::string::npos;
            new_command.input_file = trim_copy(cmd.substr(infile_pos + 1, substring_len));
        }
        if (outfile_pos != std::string::npos) {
            auto substring_len =
                (infile_pos != std::string::npos && outfile_pos < infile_pos) ? infile_pos - outfile_pos - 1
                                                                              : std::string::npos;
            new_command.output_file = trim_copy(cmd.substr(outfile_pos + 1, substring_len));;
        }
        new_command.set_cmd_args(trim_copy(cmd.substr(0, cmd.find_first_of("<>"))));
        retVal.push_back(new_command);
    }
    return retVal;
}

// SIGCHLD blocking needed to avert race condition inside execute_pipeline()
// Refer to: https://stackoverflow.com/questions/340283/avoiding-a-fork-sigchld-race-condition
void block_signal(int sig, int state) {
    sigset_t sigs;
    sigemptyset(&sigs);
    sigaddset(&sigs, sig);
    sigprocmask(state, &sigs, NULL);
}

// pipeline idea: https://www.cs.purdue.edu/homes/grr/SystemsProgrammingBook/Book/Chapter5-WritingYourOwnShell.pdf
void execute_pipeline(std::vector<command> &cmds, bool background) {
    fg_name = "shell";
    if (cmds[0].cmd == "exit") exit(EXIT_SUCCESS);
    else if (cmds[0].cmd == "cd") {
        if (chdir(cmds[0].args[1]) < 0) perror("cd error");
        return;
    } else if (cmds[0].cmd == "pwd") {
        char cwd[PATH_MAX];
        getcwd(cwd, PATH_MAX);
        printf("%s\n", cwd);
        fflush(stdout);
        return;
    } else if (cmds[0].cmd == "jobs") {
        print_jobs();
        return;
    } else if (cmds[0].cmd == "bg") {
        auto param = std::stoi(cmds[0].args[1]);
        if (bg_procs.count(param) <= 0) {
            printf("Invalid process group id\n");
            fflush(stdout);
            return;
        }
        killpg(param, SIGCONT);
        return;
    } else if (cmds[0].cmd == "fg") {
        auto param = std::stoi(cmds[0].args[1]);
        if (bg_procs.count(param) <= 0) {
            printf("Invalid process group id\n");
            fflush(stdout);
            return;
        }
        killpg(param, SIGCONT);
        tcsetpgrp(STDIN_FILENO, param);
        fg_name = bg_procs[param];
        foreground_proc_gid = param;
        bg_procs.erase(param);
        return;
    } else if (cmds[0].cmd == "sb") {
        if (cmds[0].args[1] == nullptr) {
            printf("Not enough arguments supplied\n");
            return;
        }
        int pid = std::stoi(cmds[0].args[1]);
        bool suggest = false;
        if (cmds[0].args[2] != nullptr) {
            if (strcmp(cmds[0].args[2], "-suggest") != 0) {
                printf("Invalid flag given\n");
                return;
            } else suggest = true;
        }
        std::set<std::pair<float, int>> scores;
        int level = 1;
        while (pid != 0) {
            printf("Ancestor at level %d: %d\n", level, pid);
            if (suggest) {
                long long utime, stime;
                int children_count = 0, state;
                getinfo(pid, children_count, utime, stime, state);

                float score = (float) (utime + stime) / clocks_per_sec + state * 10 + (float) 5 / children_count;
                scores.insert(std::make_pair(score, pid));
            }

            pid = get_ppid(pid);
            level++;
        }
        if (suggest) {
            int most_probable_malware = scores.begin()->second;
            printf("\nThe process %d is most probably the trojan\n", most_probable_malware);
        }
        return;
    } else if (cmds[0].cmd == "delep") {
        if (cmds[0].args[1] == nullptr) {
            printf("Insufficient arguments provided\n");
            return;
        }
        string filepath(cmds[0].args[1]);
        vector<pid_t> processes = get_lock_processes(filepath);
        fflush(stdout);
        cout << "Processes that have the file open or are holding a lock:" << endl;
        for (auto pid : processes) {
            cout << pid << endl;
        }

        cout << "Do you want to kill these processes and delete the file? [y/n]: ";
        char answer;
        cin >> answer;
        if (answer == 'y') {
            kill_processes(processes);
            if (remove(filepath.c_str()) == 0) {
                cout << "File deleted successfully" << endl;
            } else {
                cout << "Error deleting file" << endl;
            }
        } else {
            cout << "Aborted" << endl;
        }
        return;
    }
/*    for (auto &cmd : cmds) {
        glob_t glob_buf;
        if (cmd.args[1] != nullptr)
            glob(cmd.args[1], 0, nullptr, &glob_buf);
        for (int i = 2; i < MAX_ARGS && cmd.args[i] != nullptr; i++)
            glob(cmd.args[i], GLOB_APPEND, nullptr, &glob_buf);
    }*/

    int proc_grp = 0; // pgrp_id is the PID of the first cmd in the pipeline
    int pipefd[2]; // storing two ends of a pipe
    pipefd[0] = STDIN_FILENO; // read end of the pipe is initially stdin (no pipe here actually)
    int inp_fd = STDIN_FILENO;
    int out_fd = STDOUT_FILENO;
    if (background)
        return2shell = true;
    else
        return2shell = false;
    for (auto cmd_iter = cmds.begin(); cmd_iter != cmds.end(); cmd_iter++) {
        out_fd = STDOUT_FILENO; // assume output is initially STDOUT
        inp_fd = pipefd[0]; // get inp_fd from previous pipe's read end
        // for first cmd, pipefd[0] will contain STDIN's fd

        if (cmd_iter != cmds.end() - 1) {
            // if not the last cmd in the pipeline
            if (pipe(pipefd) == -1) perror("pipe() error");
            out_fd = pipefd[1]; // write end for pipe into next cmd
        }

        if (!cmd_iter->input_file.empty())
            inp_fd = open(cmd_iter->input_file.c_str(), O_RDONLY);
        if (!cmd_iter->output_file.empty())
            out_fd = open(cmd_iter->output_file.c_str(), O_WRONLY | O_APPEND | O_TRUNC | O_CREAT, OUT_REDIR_PERM);

        block_signal(SIGCHLD, SIG_BLOCK);
        int cpid = fork();
        if (cpid == 0) {
            // child branch
            block_signal(SIGCHLD, SIG_UNBLOCK);

            if (inp_fd != STDIN_FILENO) {
                dup2(inp_fd, STDIN_FILENO);
                close(inp_fd); // don't need this fd anymore
            }
            if (out_fd != STDOUT_FILENO) {
                dup2(out_fd, STDOUT_FILENO);
                close(out_fd); // don't need this fd anymore
            }

            setpgid(0, proc_grp);
            execvp(cmd_iter->cmd.c_str(), cmd_iter->args);
            perror("Command failed");
            exit(EXIT_FAILURE);
        }

        // parent branch
        if (proc_grp == 0) {
            proc_grp = cpid;
            //int val = tcgetpgrp(STDIN_FILENO);
            if (!background) {
                tcsetpgrp(STDIN_FILENO, proc_grp); // set this as foreground job
                fg_name = cmds[0].cmd;
                foreground_proc_gid = cpid;
            }
            if (background) {
                assert(bg_procs.count(cpid) == 0); // Not yet in the bg
                bg_procs[cpid] = cmds[0].cmd;
            }
        }
        pid2gpid[cpid] = proc_grp;

        block_signal(SIGCHLD, SIG_UNBLOCK);

        // parent doesn't need out_fd when it's either pointing to
        // write end of a pipe or an output file fd
        if (out_fd != STDOUT_FILENO) close(out_fd);
        // parent doesn't need in_fd when it's either pointing to
        // read end of a pipe or an input file
        if (inp_fd != STDIN_FILENO) close(inp_fd);
    }
    while (!return2shell); // we wait for SIGCHLD handler to tell us to return to shell

}

int main() {

    // install signal handlers
    signal(SIGINT, sigint_handler);
    signal(SIGCHLD, sigchld_handler);
    signal(SIGTSTP, sigtstp_handler);

    // The following gives me superpowers in deciding fg/bg processes
    // I didn't even know this was possible
    signal(SIGTTOU, SIG_IGN);

    stifle_history(HISTSIZE);
    read_history(HISTFILE);

    while (true) {
        while (sigsetjmp(ctrlc_start_point, 1) != 0);
        while (foreground_proc_gid != 0);
        char *user_input = readline(PROMPT);
        write_history(HISTFILE);
        bool background = false;
        auto retValue = command::parse_string(user_input, background);
        if (!retValue.empty()) {
            add_history(user_input);
            execute_pipeline(retValue, background);
        }
        free(user_input);
    }
}