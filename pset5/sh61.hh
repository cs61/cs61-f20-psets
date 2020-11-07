#ifndef SH61_HH
#define SH61_HH
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <fcntl.h>
#include <unistd.h>
#include <string>

#define TYPE_NORMAL        0   // normal command word
#define TYPE_REDIRECT_OP   1   // redirection operator (>, <, 2>)

// All other tokens are control operators that terminate the current command.
#define TYPE_SEQUENCE      2   // `;` sequence operator
#define TYPE_BACKGROUND    3   // `&` background operator
#define TYPE_PIPE          4   // `|` pipe operator
#define TYPE_AND           5   // `&&` operator
#define TYPE_OR            6   // `||` operator

// If you want to handle an extended shell syntax for extra credit, here are
// some other token types to get you started.
#define TYPE_LPAREN        7   // `(` operator
#define TYPE_RPAREN        8   // `)` operator
#define TYPE_OTHER         -1

struct shell_token_iterator;


// shell_parser
//    `shell_parser` objects represent a command line.
//    See `shell_token_iterator` for more.

struct shell_parser {
    shell_parser(const char* str);

    inline shell_token_iterator begin() const;
    inline shell_token_iterator end() const;

private:
    const char* _str;
    const char* _estr;
};


// shell_token_iterator
//    `shell_token_iterator` represents a token in a command line via a
//    C++ iterator-like interface. Use it as follows:
//
//    ```
//    shell_parser parser("... command line string ...");
//    for (auto it = parser.begin(); it != parser.end(); ++it) {
//        // `it` is a shell_token_iterator; it iterates over the words
//        // and operators in the command line.
//        // `it.type()` returns the current token’s type, which is one of
//        // the `TYPE_` constants.
//        // `it.str()` returns a C++ string holding the current token’s
//        // character contents.
//    }
//    ```

struct shell_token_iterator {
    std::string str() const;    // current token’s character contents
    inline int type() const;    // current token’s type

    // compare iterators
    inline bool operator==(const shell_token_iterator& x) const;
    inline bool operator!=(const shell_token_iterator& x) const;

    inline void operator++();   // advance `this` to next token

private:
    const char* _s;
    unsigned short _type;
    bool _quoted;
    unsigned _len;

    inline shell_token_iterator(const char* s);
    void update();
    friend struct shell_parser;
};

// claim_foreground(pgid)
//    Mark `pgid` as the current foreground process group.
int claim_foreground(pid_t pgid);

// set_signal_handler(signo, handler)
//    Install handler `handler` for signal `signo`. `handler` can be SIG_DFL
//    to install the default handler, or SIG_IGN to ignore the signal. Return
//    0 on success, -1 on failure. See `man 2 sigaction` or `man 3 signal`.
inline int set_signal_handler(int signo, void (*handler)(int)) {
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    return sigaction(signo, &sa, NULL);
}

inline shell_token_iterator shell_parser::begin() const {
    return shell_token_iterator(_str);
}

inline shell_token_iterator shell_parser::end() const {
    return shell_token_iterator(_estr);
}

inline shell_token_iterator::shell_token_iterator(const char* s)
    : _s(s) {
    update();
}

inline int shell_token_iterator::type() const {
    return _type;
}

inline bool shell_token_iterator::operator==(const shell_token_iterator& x) const {
    return _s == x._s;
}

inline bool shell_token_iterator::operator!=(const shell_token_iterator& x) const {
    return _s != x._s;
}

inline void shell_token_iterator::operator++() {
    _s += _len;
    update();
}

#endif
