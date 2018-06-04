/***
 * Assignment 1, Task 5
 * Authors:	Vasileios Charalampidis,
 * 			Karl Gäfvert
 * 			(Group 20)
 ***/

#include <iostream>
#include <gecode/driver.hh>
#include <gecode/int.hh>

#define SIZE 9
#define FRAC 3

/* Sudoku specifications
 *
 * Each specification gives the initial positions that are filled in,
 * with blank squares represented as zeroes.
 */
const static int examples[][9][9] = {
    {
	{0,0,0, 2,0,5, 0,0,0},
	{0,9,0, 0,0,0, 7,3,0},
	{0,0,2, 0,0,9, 0,6,0},
	
	{2,0,0, 0,0,0, 4,0,9},
	{0,0,0, 0,7,0, 0,0,0},
	{6,0,9, 0,0,0, 0,0,1},
    
	{0,8,0, 4,0,0, 1,0,0},
	{0,6,3, 0,0,0, 0,8,0},
	{0,0,0, 6,0,8, 0,0,0}
    },{
	{3,0,0, 9,0,4, 0,0,1},
	{0,0,2, 0,0,0, 4,0,0},
	{0,6,1, 0,0,0, 7,9,0},

	{6,0,0, 2,4,7, 0,0,5},
	{0,0,0, 0,0,0, 0,0,0},
	{2,0,0, 8,3,6, 0,0,4},
    
	{0,4,6, 0,0,0, 2,3,0},
	{0,0,9, 0,0,0, 6,0,0},
	{5,0,0, 3,0,9, 0,0,8}
    },{
	{0,0,0, 0,1,0, 0,0,0},
	{3,0,1, 4,0,0, 8,6,0},
	{9,0,0, 5,0,0, 2,0,0},
    
	{7,0,0, 1,6,0, 0,0,0},
	{0,2,0, 8,0,5, 0,1,0},
	{0,0,0, 0,9,7, 0,0,4},
    
	{0,0,3, 0,0,4, 0,0,6},
	{0,4,8, 0,0,6, 9,0,7},
	{0,0,0, 0,8,0, 0,0,0}
    },{	// Fiendish puzzle April 21, 2005 Times London
	{0,0,4, 0,0,3, 0,7,0},
	{0,8,0, 0,7,0, 0,0,0},
	{0,7,0, 0,0,8, 2,0,5},
    
	{4,0,0, 0,0,0, 3,1,0},
	{9,0,0, 0,0,0, 0,0,8},
	{0,1,5, 0,0,0, 0,0,4},
    
	{1,0,6, 9,0,0, 0,3,0},
	{0,0,0, 0,2,0, 0,6,0},
	{0,2,0, 4,0,0, 5,0,0}
    },{	// This one requires search
	{0,4,3, 0,8,0, 2,5,0},
	{6,0,0, 0,0,0, 0,0,0},
	{0,0,0, 0,0,1, 0,9,4},
    
	{9,0,0, 0,0,4, 0,7,0},
	{0,0,0, 6,0,8, 0,0,0},
	{0,1,0, 2,0,0, 0,0,3},
    
	{8,2,0, 5,0,0, 0,0,0},
	{0,0,0, 0,0,0, 0,0,5},
	{0,3,4, 0,9,0, 7,1,0}
    },{	// Hard one from http://www.cs.mu.oz.au/671/proj3/node5.html
	{0,0,0, 0,0,3, 0,6,0},
	{0,0,0, 0,0,0, 0,1,0},
	{0,9,7, 5,0,0, 0,8,0},

	{0,0,0, 0,9,0, 2,0,0},
	{0,0,8, 0,7,0, 4,0,0},
	{0,0,3, 0,6,0, 0,0,0},

	{0,1,0, 0,0,2, 8,9,0},
	{0,4,0, 0,0,0, 0,0,0},
	{0,5,0, 1,0,0, 0,0,0}
    },{ // Puzzle 1 from http://www.sudoku.org.uk/bifurcation.htm
	{1,0,0, 9,0,7, 0,0,3},
	{0,8,0, 0,0,0, 0,7,0},
	{0,0,9, 0,0,0, 6,0,0},
	{0,0,7, 2,0,9, 4,0,0},
	{4,1,0, 0,0,0, 0,9,5},
	{0,0,8, 5,0,4, 3,0,0},
	{0,0,3, 0,0,0, 7,0,0},
	{0,5,0, 0,0,0, 0,4,0},
	{2,0,0, 8,0,6, 0,0,9}
    },{ // Puzzle 2 from http://www.sudoku.org.uk/bifurcation.htm
	{0,0,0, 3,0,2, 0,0,0},
	{0,5,0, 7,9,8, 0,3,0},
	{0,0,7, 0,0,0, 8,0,0},
	{0,0,8, 6,0,7, 3,0,0},
	{0,7,0, 0,0,0, 0,6,0},
	{0,0,3, 5,0,4, 1,0,0},
	{0,0,5, 0,0,0, 6,0,0},
	{0,2,0, 4,1,9, 0,5,0},
	{0,0,0, 8,0,6, 0,0,0}
    },{ // Puzzle 3 from http://www.sudoku.org.uk/bifurcation.htm
	{0,0,0, 8,0,0, 0,0,6},
	{0,0,1, 6,2,0, 4,3,0},
	{4,0,0, 0,7,1, 0,0,2},
	{0,0,7, 2,0,0, 0,8,0},
	{0,0,0, 0,1,0, 0,0,0},
	{0,1,0, 0,0,6, 2,0,0},
	{1,0,0, 7,3,0, 0,0,4},
	{0,2,6, 0,4,8, 1,0,0},
	{3,0,0, 0,0,5, 0,0,0}
    },{ // Puzzle 4 from http://www.sudoku.org.uk/bifurcation.htm
	{3,0,5, 0,0,4, 0,7,0},
	{0,7,0, 0,0,0, 0,0,1},
	{0,4,0, 9,0,0, 0,3,0},
	{4,0,0, 0,5,1, 0,0,6},
	{0,9,0, 0,0,0, 0,4,0},
	{2,0,0, 8,4,0, 0,0,7},
	{0,2,0, 0,0,7, 0,6,0},
	{8,0,0, 0,0,0, 0,9,0},
	{0,6,0, 4,0,0, 2,0,8}
    },{ // Puzzle 5 from http://www.sudoku.org.uk/bifurcation.htm
	{0,0,0, 7,0,0, 3,0,0},
	{0,6,0, 0,0,0, 5,7,0},
	{0,7,3, 8,0,0, 4,1,0},
	{0,0,9, 2,8,0, 0,0,0},
	{5,0,0, 0,0,0, 0,0,9},
	{0,0,0, 0,9,3, 6,0,0},
	{0,9,8, 0,0,7, 1,5,0},
	{0,5,4, 0,0,0, 0,6,0},
	{0,0,1, 0,0,9, 0,0,0}
    },{ // Puzzle 6 from http://www.sudoku.org.uk/bifurcation.htm
	{0,0,0, 6,0,0, 0,0,4},
	{0,3,0, 0,9,0, 0,2,0},
	{0,6,0, 8,0,0, 7,0,0},
	{0,0,5, 0,6,0, 0,0,1},
	{6,7,0, 3,0,1, 0,5,8},
	{9,0,0, 0,5,0, 4,0,0},
	{0,0,6, 0,0,3, 0,9,0},
	{0,1,0, 0,8,0, 0,6,0},
	{2,0,0, 0,0,6, 0,0,0}
    },{ // Puzzle 7 from http://www.sudoku.org.uk/bifurcation.htm
	{8,0,0, 0,0,1, 0,4,0},
	{2,0,6, 0,9,0, 0,1,0},
	{0,0,9, 0,0,6, 0,8,0},
	{1,2,4, 0,0,0, 0,0,9},
	{0,0,0, 0,0,0, 0,0,0},
	{9,0,0, 0,0,0, 8,2,4},
	{0,5,0, 4,0,0, 1,0,0},
	{0,8,0, 0,7,0, 2,0,5},
	{0,9,0, 5,0,0, 0,0,7}
    },{ // Puzzle 8 from http://www.sudoku.org.uk/bifurcation.htm
	{6,5,2, 0,4,8, 0,0,7},
	{0,7,0, 2,0,5, 4,0,0},
	{0,0,0, 0,0,0, 0,0,0},
	{0,6,4, 1,0,0, 0,7,0},
	{0,0,0, 0,8,0, 0,0,0},
	{0,8,0, 0,0,4, 5,6,0},
	{0,0,0, 0,0,0, 0,0,0},
	{0,0,8, 6,0,7, 0,2,0},
	{2,0,0, 8,9,0, 7,5,1}
    },{ // Puzzle 9 from http://www.sudoku.org.uk/bifurcation.htm
	{0,0,6, 0,0,2, 0,0,9},
	{1,0,0, 5,0,0, 0,2,0},
	{0,4,7, 3,0,6, 0,0,1},
	{0,0,0, 0,0,8, 0,4,0},
	{0,3,0, 0,0,0, 0,7,0},
	{0,1,0, 6,0,0, 0,0,0},
	{4,0,0, 8,0,3, 2,1,0},
	{0,6,0, 0,0,1, 0,0,4},
	{3,0,0, 4,0,0, 9,0,0}
    },{ // Puzzle 10 from http://www.sudoku.org.uk/bifurcation.htm
	{0,0,4, 0,5,0, 9,0,0},
	{0,0,0, 0,7,0, 0,0,6},
	{3,7,0, 0,0,0, 0,0,2},
	{0,0,9, 5,0,0, 0,8,0},
	{0,0,1, 2,0,4, 3,0,0},
	{0,6,0, 0,0,9, 2,0,0},
	{2,0,0, 0,0,0, 0,9,3},
	{1,0,0, 0,4,0, 0,0,0},
	{0,0,6, 0,2,0, 7,0,0}
    },{ // Puzzle 11 from http://www.sudoku.org.uk/bifurcation.htm
	{0,0,0, 0,3,0, 7,9,0},
	{3,0,0, 0,0,0, 0,0,5},
	{0,0,0, 4,0,7, 3,0,6},
	{0,5,3, 0,9,4, 0,7,0},
	{0,0,0, 0,7,0, 0,0,0},
	{0,1,0, 8,2,0, 6,4,0},
	{7,0,1, 9,0,8, 0,0,0},
	{8,0,0, 0,0,0, 0,0,1},
	{0,9,4, 0,1,0, 0,0,0}
    },{ // From http://www.sudoku.org.uk/discus/messages/29/51.html?1131034031
	{2,5,8, 1,0,4, 0,3,7},
	{9,3,6, 8,2,7, 5,1,4},
	{4,7,1, 5,3,0, 2,8,0},

	{7,1,5, 2,0,3, 0,4,0},
	{8,4,9, 6,7,5, 3,2,1},
	{3,6,2, 4,1,0, 0,7,5},

	{1,2,4, 9,0,0, 7,5,3},
	{5,9,3, 7,4,2, 1,6,8},
	{6,8,7, 3,5,1, 4,9,2}
    }
};

const static int examples_size = sizeof(examples) / sizeof(examples[0]);

using namespace Gecode;

class Sudoku : public Script {
	private:
		IntVarArray p;
	
	public:
		Sudoku(const SizeOptions& opt) : Script(opt), p(*this, SIZE*SIZE, 1, 9)
		{	
			// Wrap array p in a SIZExSIZE matrix
			Matrix<IntVarArray> puzzle = Matrix<IntVarArray>(p, SIZE);
			
			// Assign the non zero values to p array
			const int example = opt.size();
			for (int row = 0; row < SIZE; row++)
				for (int col = 0; col < SIZE; col++)
					if (examples[example][row][col] != 0)
						rel(*this, puzzle(col, row) == examples[example][row][col], opt.ipl());
			
			// Distinct values for the same row and same column of the puzzle matrix
			for (int i = 0; i < SIZE; i++) {
				distinct(*this, puzzle.row(i), opt.ipl());
				distinct(*this, puzzle.col(i), opt.ipl());
			}
			
			// Distinct values for each 3x3 block of the puzzle matrix
			for (int bx = 0; bx < SIZE; bx += SIZE/FRAC)
				for (int by = 0; by < SIZE; by += SIZE/FRAC)
					distinct(*this, puzzle.slice(bx, bx+FRAC, by, by+FRAC), opt.ipl());
			
			// The branching part
			branch(*this, p, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
		}

		Sudoku(Sudoku& s) : Script(s) {
			p.update(*this, s.p);
		}

		virtual Space* copy(void) {
			return new Sudoku(*this);
		}

		virtual void print(std::ostream& os) const {
			for (int row = 0; row < SIZE; row++) {
				for (int col = 0; col < SIZE; col++)
					os << p[row*SIZE + col] << ' ';
				os << std::endl;
			}
		}
};

int main(int argc, char* argv[]) {
	SizeOptions opt = SizeOptions("Sudoku");
	opt.ipl(IPL_DOM);
	opt.size(0);
	opt.parse(argc, argv);
	if (opt.size() < 0 || opt.size() > (examples_size-1) ) {
		std::cerr << "Not a valid quiz chosen.";
		return -1;
	}
	Script::run<Sudoku, DFS, SizeOptions>(opt);
	return 0;
}
