#include "errors.hpp"
#include <regex>
#include <sstream>
#include <vector>

class LineIterator {
  public:
    LineIterator(const std::string& multi_line_str, int start_line_no, int end_line_no)
        : stream(multi_line_str), current_line_no(1), start_line_no(start_line_no),
          end_line_no(end_line_no) { // Skip lines until we reach start_line_no
        while(current_line_no < start_line_no && std::getline(stream, current_line)) {
            ++current_line_no;
            this->before_start_line = current_line + "\n";
        }
    }

    bool has_next() {
        if(current_line_no <= end_line_no) {
            std::getline(stream, current_line);
            return true;
        } else {
            return false;
        }
    }

    std::string next() {
        current_line_no++;
        return current_line;
    }

    const std::string& get_before_start_line() const { return before_start_line; }

    std::string get_after_end_line() {
        std::getline(stream, after_end_line);
        return after_end_line;
    }

  private:
    std::istringstream stream;
    std::string current_line;
    std::string before_start_line;
    std::string after_end_line;
    int current_line_no;
    int start_line_no;
    int end_line_no;
};

void errors::Error::raise(bool terminate) {
    // Create a banner with a centered "Syntax Error" label
    const std::string banner = std::string(30, '=') + " Error " + std::string(30, '=');

    // Print banner in magenta with bold text
    std::cerr << "\n\n\033[1;35m" << banner << "\033[0m\n\n";

    // Print error message in bold red
    std::cerr << "\033[1;31m\033[1mError:\033[0m \033[0;97m" << message << "\033[0m\n\n";

    // Print source context in bold cyan
    std::cerr << "\033[1;36m\033[1mSource Context:\033[0m\n";
    LineIterator lineIterator(source, st_line, end_line);
    int c_line = st_line;
    if(st_line > 1) {
        std::cerr << "\033[1;34m" << st_line - 1 << " |\033[0m \033[0;97m" << lineIterator.get_before_start_line() << "\033[0m";
    }
    while(lineIterator.has_next()) {
        // Print line number in bold blue and source content in white
        std::cerr << "\033[1;34m" << c_line << " |\033[0m \033[0;97m" << lineIterator.next() << "\033[0m\n";
        c_line++;
    }
    if(end_line < getNumberOfLines(source)) {
        std::cerr << "\033[1;34m" << end_line + 1 << " |\033[0m \033[0;97m" << lineIterator.get_after_end_line() << "\033[0m\n";
    }

    // Print suggested fix in bold yellow, if provided
    if(!suggestedFix.empty()) {
        std::cerr << "\n\033[1;33m\033[1mSuggested Fix:\033[0m \033[0;97m" << suggestedFix << "\033[0m\n\n";
    }

    // Terminate the program if required, with a bold red termination line
    if(terminate) {
        std::cerr << "\033[1;31m" << std::string(70, '=') << "\033[0m\n";
        exit(EXIT_FAILURE);
    }
}

void errors::NoPrefixParseFnError::raise(bool terminate) {
    std::cerr << "\n\n\033[1;35m" << std::string(60, '=') << "\033[0m\n\n";
    // Print error message in red color
    std::cerr << "\033[1;31m"
              << "NoPrefixParseFnError: \033[0m"
              << "\033[1;97m" << message << "\033[0m\n";
    std::cerr << "\033[1;36mSource Context:\033[0m\n";
    LineIterator lineIterator(source, token.line_no, token.line_no);
    int c_line = token.line_no;
    if(token.line_no > 1) {
        std::cerr << "\033[0;32m" << token.line_no - 1 << " | \033[0m" << lineIterator.get_before_start_line();
    }
    while(lineIterator.has_next()) {
        // Print line number in bold blue and source content in white
        std::cerr << "\033[0;32m" << c_line << " | \033[0m" << lineIterator.next() << "\n";
        c_line++;
    }
    if(token.line_no < getNumberOfLines(source)) {
        std::cerr << "\033[0;32m" << token.line_no + 1 << " | \033[0m" << lineIterator.get_after_end_line();
    }
    //  Print suggested fix if it's not empty
    if(!suggestedFix.empty()) {
        std::cerr << "\033[1;33m"
                  << "Suggested fix: " << suggestedFix << "\033[0m\n\n";
    }
    if(terminate)
        exit(EXIT_FAILURE);
}

void errors::SyntaxError::raise(bool terminate) {
    const std::string banner = std::string(30, '=') + " Syntax Error " + std::string(30, '=');
    std::cerr << "\n\n\033[1;35m" << banner << "\033[0m\n\n";
    // Print error message in red color
    std::cerr << "\033[1;31m"
              << "SyntaxError: \033[0m"
              << "\033[0;97m" << message << "\033[0m\n";

    std::cerr << "\033[1;36mSource Context:\033[0m\n";

    LineIterator lineIterator(source, token.line_no, token.line_no + getNumberOfLines(token.literal) - 1);
    int c_line = token.line_no;
    if(token.line_no > 1) {
        std::cerr << "\033[0;32m" << token.line_no - 1 << " | \033[0m" << lineIterator.get_before_start_line();
    }
    bool is_first_line = true;
    while(lineIterator.has_next()) {
        // Print line number in bold blue and source content in white
        auto line = lineIterator.next();
        std::cerr << "\033[0;32m" << c_line << " | \033[0m" << line << "\n";
        std::string underline;
        if(is_first_line) {
            underline = std::string(token.col_no, ' ') + std::string(token.end_col_no - token.col_no, '^');
        } else {
            underline = std::string(line.length(), '^');
        }
        // if length of underline is greater than line length, then we need to split the underline and add the remaining to next line
        std::cerr << "\033[0;32m  | \033[0m\033[1;31m" << underline.substr(0, line.length()) << "\033[0m\n";
        c_line++;
        is_first_line = false;
    }
    if(token.line_no + getNumberOfLines(token.literal) - 1 < getNumberOfLines(source)) {
        std::cerr << "\033[0;32m" << token.line_no + 1 << " | \033[0m" << lineIterator.get_after_end_line() << "\n";
    }

    //  Print suggested fix if it's not empty
    if(!suggestedFix.empty()) {
        std::cerr << "\033[1;33m"
                  << "Suggested fix: " << suggestedFix << "\033[0m\n\n";
    }
    if(terminate)
        exit(EXIT_FAILURE);
}

void errors::CompletionError::raise(bool terminate) {
    const std::string banner = std::string(30, '=') + " Completion Error " + std::string(30, '=');
    std::cerr << "\n\n\033[1;35m" << banner << "\033[0m\n\n";
    // Print error message in red color
    std::cerr << "\033[1;31m"
              << "CompletionError: \033[0m"
              << "\033[0;97m" << message << "\033[0m\n";
    std::cerr << "\033[1;36mSource Context:\033[0m\n";
    LineIterator lineIterator(source, this->st_line, this->end_line);
    int c_line = this->st_line;
    while(lineIterator.has_next()) {
        // Print line number in bold blue and source content in white
        std::cerr << "\033[0;32m" << c_line << " | \033[0m" << lineIterator.next() << "\n";
        c_line++;
    }
    if(this->end_line < getNumberOfLines(source)) {
        std::cerr << "\033[0;32m" << this->end_line + 1 << " | \033[0m" << lineIterator.get_after_end_line() << "\n";
    }
    if(!suggestedFix.empty()) {
        std::cerr << "\033[1;33m"
                  << "Suggested fix: " << suggestedFix << "\033[0m\n\n";
    }
    if(terminate)
        exit(EXIT_FAILURE);
}

void errors::raiseSyntaxError(const std::string& type, const std::string& source, const token::Token& token, const std::string& message, const std::string& suggestedFix) {
    errors::SyntaxError error(type, source, token, message, suggestedFix);
    error.raise();
}