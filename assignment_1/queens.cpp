/***
 * Assignment 1, Task 7
 * Authors:	Vasileios Charalampidis,
 * 			Karl Gäfvert
 * 			(Group 20)
 ***/

#include <iostream>
#include <gecode/driver.hh>
#include <gecode/int.hh>

using namespace Gecode;

/* 
  Memory: ~N^2
  Constraints: 2*N+2*(2*N-3) = 6N-6
*/
class NQueens : public Script {
	private:
		IntVarArray b;
        int size;
	
	public:
		NQueens(const SizeOptions& opt) : Script(opt), 
            b(*this, opt.size()*opt.size(), 0, 1)
		{	
			// Wrap array b in a opt.size() x opt.size() matrix
			// board(col, row)
		    size = opt.size();
			Matrix<IntVarArray> board = Matrix<IntVarArray>(b, size);

			/* Sum of each row and column must = 1
			   Horizontal and vertical constraints */
			for (int i = 0; i < size; i++) {
				linear(*this, board.row(i), IRT_EQ, 1, opt.ipl());
				linear(*this, board.col(i), IRT_EQ, 1, opt.ipl());
			}

			/* Sum of each diagonal must <= 1
			   First half diagonal constraints
			   dv = x / / /  dh = \ x x x
			        / / / x       \ \ x x
			        / / x x       \ \ \ x
			        / x x x       x \ \ \  */
			for (int i = 1; i < size; i++) {
				IntVarArray dv = IntVarArray(*this, i+1);
				IntVarArray dh = IntVarArray(*this, i+1);

				for (int j = 0; j < i+1; j++) {
					dv[j] = board(j, i-j);
					dh[j] = board(i-j, size-1-j);
				}

				linear(*this, dv, IRT_LQ, 1, opt.ipl());
				linear(*this, dh, IRT_LQ, 1, opt.ipl());
			}

			/* Second half diagonal constraints
			   dv = x x x x  dh = x \ \ x
			        x x x /       x x \ \ 
			        x x / /       x x x \ 
			        x / / x       x x x x  */
			for (int i = 1; i < size-1; i++) {
				IntVarArray dv = IntVarArray(*this, i+1);
				IntVarArray dh = IntVarArray(*this, i+1);

				for (int j = 0; j < i+1; j++) {
					dv[j] = board(size-1-i+j, size-1-j);
					dh[j] = board(size-1-j, i-j);
				}

				linear(*this, dv, IRT_LQ, 1, opt.ipl());
				linear(*this, dh, IRT_LQ, 1, opt.ipl());
			}
			
			// Branching
			branch(*this, b, INT_VAR_SIZE_MIN(), INT_VAL_MAX());
		}

		NQueens(NQueens& q) : Script(q) {
			b.update(*this, q.b);
			size = q.size;
		}

		virtual Space* copy(void) {
			return new NQueens(*this);
		}

		virtual void print(std::ostream& os) const {
			for (int row = 0; row < size; row++) {
				for (int col = 0; col < size; col++)
					os << b[row*size + col] << ' ';
				os << std::endl;
			}
		}
};

int main(int argc, char* argv[]) {
  	SizeOptions opt("Queens");
	opt.ipl(IPL_DOM);
	opt.size(8);
	opt.parse(argc, argv);
	if (opt.size() < 4) {
		std::cerr << "The minimum number of queens is 4";
		return -1;
	}
	Script::run<NQueens, DFS, SizeOptions>(opt);
	return 0;
}

/// Advantages
//  - 	Simple solution 
//  -   Fewer constraints (6N-6 here, comparing to (3/2)N(N-1) from lecture)
/// Disadvantages 
//  -   Many variables (n^2)
//  -   Does not consider trivial soulutions (rotation etc.)
//  -   Many propagations for high N >~30 because of n^2 variable space
