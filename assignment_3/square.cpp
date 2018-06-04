#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;

class Squares : public Script {
  private:
    IntVar s;      // Size of enclosing square
    IntVarArray x; // x coordinates
    IntVarArray y; // y coordinates
    static int n;
    static int ni;
    static int sum;
    static int *sizes; 
    
  public:
    Squares(const SizeOptions &opt): Script(opt),
        s(*this, n, sum),
        x(*this, ni, 0, sum), 
        y(*this, ni, 0, sum) 
    {
        // Empty strip dominance
		// Tested for Linux. Some modifications needed for Windows
        for (int i = 0; i < ni; i++) {
            switch(sizes[i]) {
                case 2:
                case 4:
                    empty_strip(2, i);
                    break;
                case 3:
                case 5 ... 8:
                    empty_strip(3, i);
                    break;
                case 9 ... 11:
                    empty_strip(4, i);
                    break;
                case 12 ... 17:
                    empty_strip(5, i);
                    break;
                case 18 ... 21:
                    empty_strip(6, i);
                    break;
                case 22 ... 29:
                    empty_strip(7, i);
                    break;
                case 30 ... 34:
                    empty_strip(8, i);
                case 35 ... 44:
                    empty_strip(9, i);
                    break;
                case 45:
                    empty_strip(10, i);
                    break;
            }
        }

        // Symmetry removal
        rel(*this, x[0] <= (1 + (s - sizes[0]) / 2));
        rel(*this, y[0] <= x[0]);

        // Dont allow the smaller squares to be placed outside the border of S.
        for (int i = 0; i < ni; i++) {
            rel(*this, x[i] + sizes[i] <= s);
            rel(*this, y[i] + sizes[i] <= s);
        }

        // A square S1 doesn't overlap another sqaure S2 if
        // S1(x) + S1(size) <= S2(x) or
        // S2(x) + S2(size) <= S1(x) or
        // S1(y) + S1(size) <= S2(y) or
        // S2(y) + S2(size) <= S1(y)
        // Add these four conditions for every pair of squares.
        const int s_max_sum = s.max() * (s.max() + 1) / 2;
        for (int i = 0; i < (ni - 1); i++) {
            for (int j = (i + 1); j < ni; j++) {
                IntVar s1_left = IntVar(*this, 0, s_max_sum);
                IntVar s2_left = IntVar(*this, 0, s_max_sum);
                IntVar s1_top = IntVar(*this, 0, s_max_sum);
                IntVar s2_top = IntVar(*this, 0, s_max_sum);

                rel(*this, s1_left == x[i] + sizes[i]);
                rel(*this, s2_left == x[j] + sizes[j]);
                rel(*this, s1_top == y[i] + sizes[i]);
                rel(*this, s2_top == y[j] + sizes[j]);

                BoolVarArray b = BoolVarArray(*this, 4, 0, 1);
                rel(*this, s1_left, IRT_LQ, x[j], b[0]);
                rel(*this, s2_left, IRT_LQ, x[i], b[1]);
                rel(*this, s1_top, IRT_LQ, y[j], b[2]);
                rel(*this, s2_top, IRT_LQ, y[i], b[3]);

                linear(*this, b, IRT_GQ, 1);
            }
        }
        
        // Base cordinates are in the top left corner.
        //  x,y ------- x+size,y  
        //  |           |
        //  |           |
        //  |           |
        //  x,y+size -- x+size,y+size
        //
        // Overlap is calculated for every row and column of s
        // by adding constraints with all possible squares' starting 
        // positions that encompases that particular row or column. 
        // If a square is located in that row or column the 
        // corresponding binary value will be set to 1.
        // The final linear equation constraint sum the sizes of the 
        // squares which boolean value equals to 1. 
        const IntArgs a_sizes = IntArgs(ni, sizes);
        for (int i = 0; i < s.max(); i++) {
            BoolVarArray xb = BoolVarArray(*this, ni, 0, 1);
            BoolVarArray yb = BoolVarArray(*this, ni, 0, 1);
            for (int j = 0; j < ni; j++) {
                dom(*this, x[j], (i - sizes[j]), i, xb[j]);
                dom(*this, y[j], (i - sizes[j]), i, yb[j]);
            }
            linear(*this, a_sizes, xb, IRT_LQ, s);
            linear(*this, a_sizes, yb, IRT_LQ, s);
        }

        // Branching should be performed in the order they posted
        // (declared) meaning that our brancing order is: s, x, y. 
        // Try smallest possible values of s to find the smallest
        // encompassing square.
        // INT_VAR_NONE corresponds chossing to the first unassigned 
        // value (index: 0, 1, ..), so larger squares are placed first.
        // INT_VAL_MIN will try smaller cordinates first, so the 
        // placing order is from 0,0 and increasing (top to bottom
        // and left to right).
        branch(*this, s, INT_VAL_MIN());
        branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
        branch(*this, y, INT_VAR_NONE(), INT_VAL_MIN());
    }

    // Constructor for cloning
    Squares(Squares& sq) : Script(sq) {
      s.update(*this, sq.s);
      x.update(*this, sq.x);
      y.update(*this, sq.y);
    }

    // Perform copying during cloning
    virtual Space* copy(void) {
      return new Squares(*this);
    }

    // Print solution
    virtual void print(std::ostream& os) const {
        for (int i = 0; i < ni; i++)
			os << sizes[i] << 'x' << sizes[i] << ", x: " << x[i] << ", y: " << y[i] << std::endl;
        os << std::endl << "Enclosing square (S):" << std::endl << s << 'x' << s << std::endl;
    }

    inline void empty_strip(const int strip, const int index) {
        rel(*this, strip != x[index]);
        rel(*this, strip != y[index]);
    }

    static void set_n(const int argn) {
        n = argn;
        ni = n - 1;
        sum = n * (n + 1) / 2;
        delete [] sizes;
        sizes = new int[ni];
        for (int i = 0; i < ni; i++)
            sizes[i] = n - i;
    }
};

// Make compiler happy
int Squares::n = 3;
int Squares::ni = 2;
int Squares::sum = 5;
int *Squares::sizes = new int[2] {3, 2};

int main(int argc, char* argv[]) {
    SizeOptions opt("Squares");
    opt.ipl(IPL_DOM);
    opt.size(3);
    opt.parse(argc, argv);
    if (opt.size() < 3) {
        std::cerr << "The minimum number of squares is 3";
        return -1;
    }
    Squares::set_n(opt.size());
    Script::run<Squares, DFS, SizeOptions>(opt);
    return 0;
}
