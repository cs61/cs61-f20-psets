#include "pongboard.hh"
#include "helpers.hh"
#include <unistd.h>
#include <sys/time.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <csignal>
#include <cassert>
#include <atomic>
#include <thread>
#include <random>
#include <deque>


// pong board
pong_board* main_board;

// delay between moves, in microseconds
static unsigned long delay;

// balls waiting to run
static std::deque<pong_ball*> ball_reserve;

// number of running threads
static std::atomic<unsigned long> nstarted;
static std::atomic<long> nrunning;


// ball_thread(ball)
//    1. Obtain a ball from the `ball_reserve` and place it
//       on the board.
//    2. Move the ball until it falls down a hole.
//    3. Put it back on the `ball_reserve`.

void ball_thread() {
    pong_ball* ball = nullptr;
    while (!ball) {
        if (!ball_reserve.empty()) {
            ball = ball_reserve.front();
            ball_reserve.pop_front();
        }
    }
    ball->place();

    while (true) {
        int mval = ball->move();
        if (mval > 0) {
            // ball successfully moved; wait `delay` to move it again
            if (delay > 0) {
                usleep(delay);
            }
        } else if (mval < 0) {
            // ball fell down hole; exit
            break;
        }
    }

    ball_reserve.push_back(ball);
    --nrunning;
}


// HELPER FUNCTIONS

// usage()
//    Explain how simpong61 should be run.
static void usage() {
    fprintf(stderr, "\
Usage: ./simpong61 [-1] [-w WIDTH] [-h HEIGHT] [-b NBALLS] [-s NSTICKY]\n\
                   [-H NHOLES] [-j NTHREADS] [-d MOVEPAUSE] [-p PRINTTIMER]\n");
    exit(1);
}


// summary_handler
//    Runs when `SIGUSR2` is received; prints out summary statistics for the
//    board to standard output.
void summary_handler(int) {
    char buf[BUFSIZ];
    ssize_t nw = 0;
    if (main_board) {
        simple_printer pr(buf, sizeof(buf));
        pr << nstarted << " threads started, "
           << nrunning << " running, "
           << main_board->ncollisions_ << " collisions\n";
        nw = write(STDOUT_FILENO, pr.data(), pr.length());
    }
    assert(nw >= 0);
}

// signal_handler
//    Runs when `SIGUSR1` is received; prints out the current state of the
//    board to standard output.
//
//    NOTE: This function accesses the board in a thread-unsafe way. There is no
//    easy way to fix this and you aren't expected to fix it.
void signal_handler(int) {
    char buf[BUFSIZ];
    ssize_t nw = 0;
    if (main_board) {
        summary_handler(0);

        for (int y = 0; y < main_board->height_; ++y) {
            for (int x = 0; x < main_board->width_; ++x) {
                pong_cell& c = main_board->cell(x, y);
                char ch = '?';
                if (c.ball_) {
                    assert(c.type_ == cell_empty || c.type_ == cell_sticky);
                    ch = 'O';
                } else if (c.type_ == cell_empty) {
                    ch = '.';
                } else if (c.type_ == cell_sticky) {
                    ch = '_';
                } else if (c.type_ == cell_obstacle) {
                    ch = 'X';
                } else if (c.type_ == cell_hole) {
                    ch = '#';
                }
                if (x < BUFSIZ - 1) {
                    buf[x] = ch;
                }
            }
            buf[std::min(main_board->width_, BUFSIZ - 1)] = '\n';
            nw = write(STDOUT_FILENO, buf,
                       std::min(main_board->width_ + 1, BUFSIZ));
        }
        nw = write(STDOUT_FILENO, "\n", 1);
    }
    assert(nw >= 0);
}


// main(argc, argv)
//    The main loop.
int main(int argc, char** argv) {
    // print information on receiving a signal
    {
        struct sigaction sa;
        sa.sa_handler = signal_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        int r = sigaction(SIGUSR1, &sa, nullptr);
        assert(r == 0);
        r = sigaction(SIGALRM, &sa, nullptr);
        assert(r == 0);
        sa.sa_handler = summary_handler;
        r = sigaction(SIGUSR2, &sa, nullptr);
        assert(r == 0);
    }

    // parse arguments and check size invariants
    int width = 100, height = 31, nballs = 24, nsticky = 12,
        nholes = 0, nthreads = -1;
    long print_interval = 0;
    bool single_threaded = false;
    int ch;
    while ((ch = getopt(argc, argv, "w:h:b:s:d:p:H:j:1")) != -1) {
        if (ch == 'w' && is_integer_string(optarg)) {
            width = strtol(optarg, nullptr, 10);
        } else if (ch == 'h' && is_integer_string(optarg)) {
            height = strtol(optarg, nullptr, 10);
        } else if (ch == 'b' && is_integer_string(optarg)) {
            nballs = strtol(optarg, nullptr, 10);
        } else if (ch == 's' && is_integer_string(optarg)) {
            nsticky = strtol(optarg, nullptr, 10);
        } else if (ch == 'H' && is_integer_string(optarg)) {
            nholes = strtol(optarg, nullptr, 10);
        } else if (ch == 'j' && is_integer_string(optarg)) {
            nthreads = strtol(optarg, nullptr, 10);
        } else if (ch == 'd' && is_real_string(optarg)) {
            delay = (unsigned long) (strtod(optarg, nullptr) * 1000000);
        } else if (ch == 'p' && is_real_string(optarg)) {
            print_interval = (long) (strtod(optarg, nullptr) * 1000000);
        } else if (ch == '1') {
            single_threaded = true;
        } else {
            usage();
        }
    }
    if (nthreads < 0) {
        nthreads = nballs;
    }
    if (optind != argc
        || width < 2
        || height < 2
        || std::min(nthreads, nballs) + nsticky + nholes >= width * height
        || nthreads == 0
        || nthreads > nballs) {
        usage();
    }
    if (print_interval > 0) {
        struct itimerval it;
        it.it_interval.tv_sec = print_interval / 1000000;
        it.it_interval.tv_usec = print_interval % 1000000;
        it.it_value = it.it_interval;
        int r = setitimer(ITIMER_REAL, &it, nullptr);
        assert(r == 0);
    }

    // create pong board
    pong_board board(width, height);
    main_board = &board;

    // create sticky locations
    for (int n = 0; n < nsticky; ++n) {
        int x, y;
        do {
            x = random_int(0, width - 1);
            y = random_int(0, height - 1);
        } while (board.cell(x, y).type_ != cell_empty);
        board.cell(x, y).type_ = cell_sticky;
    }

    // create holes
    for (int n = 0; n < nholes; ++n) {
        int x, y;
        do {
            x = random_int(0, width - 1);
            y = random_int(0, height - 1);
        } while (board.cell(x, y).type_ != cell_empty);
        board.cell(x, y).type_ = cell_hole;
    }

    // create balls
    for (int n = 0; n < nballs; ++n) {
        ball_reserve.push_back(new pong_ball(board));
    }

    if (!single_threaded) {
        // initial ball threads
        for (int i = 0; i < nthreads; ++i) {
            std::thread t(ball_thread);
            t.detach();
            ++nstarted;
            ++nrunning;
        }

        // main thread
        if (nholes == 0) {
            // if no holes, block forever
            while (true) {
                select(0, nullptr, nullptr, nullptr, nullptr);
            }
        } else {
            // otherwise, start new threads periodically as ball threads exit
            while (true) {
                if (nrunning < nthreads) {
                    std::thread t(ball_thread);
                    t.detach();
                    ++nstarted;
                    ++nrunning;
                    usleep(1000);
                }
            }
        }
    } else {
        // single threaded mode: one thread runs all balls
        // (single threaded mode does not handle holes)
        assert(nholes == 0);
        while (true) {
            for (int n = 0; n < nballs; ++n) {
                ball_reserve[n]->move();
            }
            if (delay) {
                usleep(delay);
            }
        }
    }
}
