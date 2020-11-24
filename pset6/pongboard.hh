#ifndef PONGBOARD_HH
#define PONGBOARD_HH
#include <cassert>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "helpers.hh"
struct pong_ball;
int random_int(int min, int max);


enum pong_celltype {
    cell_empty,
    cell_sticky,
    cell_obstacle,
    cell_hole
};


struct pong_cell {
    pong_celltype type_ = cell_empty;  // type of cell
    pong_ball* ball_ = nullptr;        // pointer to ball currently in cell
};


struct pong_board {
    int width_;
    int height_;
    std::vector<pong_cell> cells_;     // `width_ * height_`, row-major order
    pong_cell obstacle_cell_;          // represents off-board positions
    unsigned long ncollisions_ = 0;


    // pong_board(width, height)
    //    Construct a new `width x height` pong board with all empty cells.
    pong_board(int width, int height)
        : width_(width), height_(height),
          cells_(width * height, pong_cell()) {
        obstacle_cell_.type_ = cell_obstacle;
    }

    // destroy a pong_board
    ~pong_board() {
    }

    // boards can't be copied, moved, or assigned
    pong_board(const pong_board&) = delete;
    pong_board& operator=(const pong_board&) = delete;


    // cell(x, y)
    //    Return a reference to the cell at position `x, y`. If there is
    //    no such position, returns `obstacle_cell_`, a cell containing an
    //    obstacle.
    pong_cell& cell(int x, int y) {
        if (x < 0 || x >= this->width_ || y < 0 || y >= this->height_) {
            return obstacle_cell_;
        } else {
            return this->cells_[y * this->width_ + x];
        }
    }
};



struct pong_ball {
    pong_board& board_;
    bool placed_ = false;
    int x_ = -1;
    int y_ = -1;
    int dx_ = 0;
    int dy_ = 0;


    // pong_ball(board)
    //    Construct a new ball on `board`.
    pong_ball(pong_board& board)
        : board_(board) {
    }

    // pong_ball(board, x, y, dx, dy)
    //    Construct a new ball on `board` at a known position.
    pong_ball(pong_board& board, int x, int y, int dx, int dy)
        : board_(board), placed_(true), x_(x), y_(y), dx_(dx), dy_(dy) {
        assert(x >= 0 && x < board_.width_ && y >= 0 && y < board_.height_);
        assert(board_.cell(x, y).ball_ == nullptr);
        board_.cell(x, y).ball_ = this;
    }

    // balls can't be copied, moved, or assigned
    pong_ball(const pong_ball&) = delete;
    pong_ball& operator=(const pong_ball&) = delete;


    // place()
    //    Place this ball onto the board at a random empty or sticky position,
    //    moving in a random direction.
    void place() {
        pong_board& board = this->board_;

        // pick a random direction
        this->dx_ = random_int(0, 1) ? 1 : -1;
        this->dy_ = random_int(0, 1) ? 1 : -1;

        // pick random positions until a suitable position is found
        while (!this->placed_) {
            int x = random_int(0, board.width_ - 1);
            int y = random_int(0, board.height_ - 1);
            pong_cell& cell = board.cell(x, y);

            if ((cell.type_ == cell_empty || cell.type_ == cell_sticky)
                && !cell.ball_) {
                this->x_ = x;
                this->y_ = y;
                cell.ball_ = this;
                this->placed_ = true;
            }
        }
    }


    // move()
    //    Move this ball once on its board.
    //
    //    * Returns 1 if the ball successfully moved to a new board cell.
    //    * Returns 0 if the ball did not move, but is still on the board.
    //    * Returns -1 if the ball fell off the board.
    //
    //    This function is complex because it must consider obstacles,
    //    collisions, holes, and sticky cells.
    //
    //    You should preserve its current logic while adding sufficient
    //    synchronization to make it thread-safe.
    int move() {
        // return -1 if ball has been removed from board
        if (this->x_ < 0 || this->y_ < 0) {
            assert(this->x_ < 0 && this->y_ < 0
                   && this->dx_ == 0 && this->dy_ == 0);
            return -1;
        }

        // otherwise, ball is on board
        // assert that this ball is stored in the board correctly
        pong_board& board = this->board_;
        pong_cell& cur_cell = board.cell(this->x_, this->y_);
        assert(cur_cell.ball_ == this);

        // sticky cell: nothing to do
        if (this->dx_ == 0 && this->dy_ == 0) {
            return 0;
        }

        // obstacle: change direction on hitting a board edge
        if (board.cell(this->x_ + this->dx_, this->y_).type_ == cell_obstacle) {
            this->dx_ = -this->dx_;
        }
        if (board.cell(this->x_, this->y_ + this->dy_).type_ == cell_obstacle) {
            this->dy_ = -this->dy_;
        }

        // check next cell
        pong_cell& next_cell = board.cell(this->x_ + this->dx_,
                                          this->y_ + this->dy_);
        if (next_cell.ball_) {
            // collision: change both balls' directions without moving them
            if (next_cell.ball_->dx_ != this->dx_) {
                next_cell.ball_->dx_ = this->dx_;
                this->dx_ = -this->dx_;
            }
            if (next_cell.ball_->dy_ != this->dy_) {
                next_cell.ball_->dy_ = this->dy_;
                this->dy_ = -this->dy_;
            }
            ++board.ncollisions_;
            return 0;
        } else if (next_cell.type_ == cell_obstacle) {
            // obstacle: reverse direction
            this->dx_ = -this->dx_;
            this->dy_ = -this->dy_;
            return 0;
        } else if (next_cell.type_ == cell_hole) {
            // hole: fall off board
            this->x_ = this->y_ = -1;
            this->dx_ = this->dy_ = 0;
            this->placed_ = false;
            cur_cell.ball_ = nullptr;
            return -1;
        } else {
            // otherwise, move into the next cell
            this->x_ += this->dx_;
            this->y_ += this->dy_;
            cur_cell.ball_ = nullptr;
            next_cell.ball_ = this;
            // stop if the next cell is sticky
            if (next_cell.type_ == cell_sticky) {
                this->dx_ = this->dy_ = 0;
            }
            return 1;
        }
    }
};

#endif
