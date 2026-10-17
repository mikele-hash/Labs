#include <string>
#include <unistd.h>
#include <fcntl.h>

int my_stoi(const std::string& s) {
    int res = 0;
    for (char c : s) {
        res = res * 10 + (c - '0');
    }
    return res;
}

bool isVowel(char c) {
    return c=='a'||c=='e'||c=='i'||c=='o'||c=='u'||c=='y'||
           c=='A'||c=='E'||c=='I'||c=='O'||c=='U'||c=='Y';
}

int main(int argc, char *argv[]) {
    std::string filename = argv[2];
    int pipe_fd = my_stoi(argv[3]);

    int file_fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);

    char c;
    std::string line;

    while (read(pipe_fd, &c, 1) == 1) {
        if (c == '\n') {
            std::string result;
            for (char ch : line) {
                if (!isVowel(ch)) {
                    result += ch;
                }
            }

            std::string out_msg = "Child";
            out_msg += argv[1];
            out_msg += ": ";
            out_msg += result;
            out_msg += "\n";

            write(STDOUT_FILENO, out_msg.c_str(), out_msg.size());


            write(file_fd, result.c_str(), result.size());
            write(file_fd, "\n", 1);

            line.clear();
        } else {
            line += c;
        }
    }

    close(file_fd);
    return 0;
}
