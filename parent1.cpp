#include <string>
#include <unistd.h>
#include <sys/wait.h>

std::string read_line() {
    std::string line;
    char c;
       while (read(STDIN_FILENO, &c, 1) == 1) {
        if (c == '\n') break;
        line += c;
    }
    return line;
}


std::string my_to_string(int n) {
    if (n == 0) return "0";
    std::string res;
    while (n > 0) {
        res = char('0' + (n % 10)) + res;
        n /= 10;
    }
    return res;
}

int main() {
    write(STDOUT_FILENO, "Enter filename for child1: ", 27);
    std::string filename1 = read_line();

    write(STDOUT_FILENO, "Enter filename for child2: ", 27);
    std::string filename2 = read_line();

    int pipe1[2], pipe2[2];
    pipe(pipe1);
    pipe(pipe2);

    pid_t pid1 = fork();
    if (pid1 == 0) {
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);

        std::string fd_str = my_to_string(pipe1[0]);
        
        char *args[] = {
            (char*)"./child1",
            (char*)"1",
            (char*)filename1.c_str(),
            (char*)fd_str.c_str(),
            NULL
        };
        execv("./child1", args);
        return 1;
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        close(pipe2[1]);
        close(pipe1[0]);
        close(pipe1[1]);

        std::string fd_str = my_to_string(pipe2[0]);

        char *args[] = {
            (char*)"./child1",
            (char*)"2",
            (char*)filename2.c_str(),
            (char*)fd_str.c_str(),
            NULL
        };
        execv("./child1", args);
        return 1;
    }

    close(pipe1[0]);
    close(pipe2[0]);

    write(STDOUT_FILENO, "Enter strings (empty line to quit):\n", 37);
    
    while (true) {
        write(STDOUT_FILENO, "> ", 2);
        std::string line = read_line();

        if (line.empty()) break;

        if (line.length() > 10) {
            write(pipe2[1], line.c_str(), line.size());
            write(pipe2[1], "\n", 1);
        } else {
            write(pipe1[1], line.c_str(), line.size());
            write(pipe1[1], "\n", 1);
        }
    }

    close(pipe1[1]);
    close(pipe2[1]);

    wait(NULL);
    wait(NULL);

    return 0;
}
