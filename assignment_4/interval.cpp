/*
 *  Main authors:
 *     Christian Schulte <cschulte@kth.se>
 *
 *  Copyright:
 *     Christian Schulte, 2009
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

/*
 * Assignment 4, Task 2
 * Authors: Vasileios Charalampidis,
 * 		    Karl Gäfvert
 * 		    (Group 20)
 */

#include <gecode/int.hh>
#include <string>

using namespace std;

using namespace Gecode;

using namespace Gecode::Int;

/*
 * Custom brancher for forcing mandatory parts
 *
 */
class IntervalBrancher : public Brancher {
protected:
  // Views for x-coordinates (or y-coordinates)
  ViewArray<IntView> x;
  // Width (or height) of rectangles
  int* w;
  // Percentage for obligatory part
  double p;
  // Cache of first unassigned view
  mutable int start;
  // Description
  class Description : public Choice {
  public:
    // Position of view
    int pos;
    // You might need more information, please add here
    int split_pos; // refers to the exact position of splitting the coordinate interval

    /* Initialize description for brancher b, number of
     *  alternatives a, position p, and ???. --> and split position
     */
    Description(const Brancher& b, unsigned int a, int p, int sp)
      : Choice(b,a), pos(p), split_pos(sp) {}
    // Report size occupied
    virtual size_t size(void) const {
      return sizeof(Description);
    }
    // Archive the choice's information in e
    virtual void archive(Archive& e) const {
      Choice::archive(e);
      // You must also archive the additional information --> added the split position
      e << pos << split_pos;
    }
  };
public:
  // Construct branching
  IntervalBrancher(Home home, 
                   ViewArray<IntView>& x0, int w0[], double p0)
    : Brancher(home), x(x0), w(w0), p(p0), start(0) {}
  // Post branching
  static void post(Home home, ViewArray<IntView>& x, int w[], double p) {
    (void) new (home) IntervalBrancher(home,x,w,p);
  }

  // Copy constructor used during cloning of b
  IntervalBrancher(Space& home, IntervalBrancher& b)
    : Brancher(home, b), p(b.p), start(b.start) {
    x.update(home, b.x);
    w = home.alloc<int>(x.size());
    for (int i=x.size(); i--; )
      w[i]=b.w[i];
  }
  // Copy brancher
  virtual Actor* copy(Space& home) {
    return new (home) IntervalBrancher(home, *this);
  }

  // Check status of brancher, return true if alternatives left
  virtual bool status(const Space& home) const {

    // FILL IN HERE

    /** 
     * We aim to return 'true' as status when the x-view array is not yet assigned 
     * and when the x range is larger than the size needed for an obligatory part. 
     * The obligatory part has size p*w[i], where p the pecentage of the total square dimension w.  
     * We iterate from first unassigned view of x-coordinate view array until the end of it.
     */
    for (int i = start; i < x.size(); i++){
        if ((!x[i].assigned()) && (x[i].max() - x[i].min() > w[i] - ceil(p * w[i]))){
            start = i; //assign new start value for branching
            return true;
        }
    }
    return false; //no more alternatives left
  }
  // Return choice as description
  virtual const Choice* choice(Space& home) {

    // FILL IN HERE

    /**
     * We aim to return choice as description where we provide the number of alternatives,
     * the current branching view for x-view array and the split position for the x range.  
     */
    int split_position = x[start].min() + w[start] - ceil(p * w[start]);
    return new Description(*this, 2, start, split_position);
  }
  // Construct choice from archive e
  virtual const Choice* choice(const Space&, Archive& e) {
    // Again, you have to take care of the additional information --> add the split_pos
    int pos, split_pos;
    e >> pos >> split_pos;
    return new Description(*this, 2, pos, split_pos);
  }
  // Perform commit for choice c and alternative a
  virtual ExecStatus commit(Space& home, 
                            const Choice& c,
                            unsigned int a) {
    const Description& d = static_cast<const Description&>(c);

    // FILL IN HERE

    /**
     * We check the corresponding alternative. We have 2 in total. 
     * The first one has range [x[pos].min...split_pos] and the second one (split_pos...x[pos].max].
     * If the modification operation returns a modification event signaling failure,
     * me_failed is true and hence the execution status ES_FAILED is returned. 
     * Otherwise, the ES_OK is returned.
     */
    if (a == 0) {
        return me_failed(x[d.pos].lq(home, d.split_pos)) ? ES_FAILED : ES_OK;
    }
    else {
        return me_failed(x[d.pos].gr(home, d.split_pos)) ? ES_FAILED : ES_OK;
    }
  }
  // Print some information on stream o (used by Gist, from Gecode 4.0.1 on)
  virtual void print(const Space& home, const Choice& c, unsigned int b,
                     std::ostream& o) const {

    // FILL IN HERE
    const Description& descr = static_cast<const Description&>(c);
    int pos = descr.pos;
    int split_position = descr.split_pos;
    // First alternative
    if (b == 0){
        o << "First alternative" << std::endl;
        o << "x[ " << pos << " ], with interval: [ " << x[pos].min() << "..." << split_position << " ]" << std::endl;
    }
    // Second alternative
    else {
        o << "Second alternative" << std::endl;
        o << "x[ " << pos << " ], with interval: ( " << split_position << "..." << x[pos].max() << " ]" << std::endl;
    }
  }
};

// This posts the interval branching
void interval(Home home, const IntVarArgs& x, const IntArgs& w, double p) {
  // Check whether arguments make sense
  if (x.size() != w.size())
    throw ArgumentSizeMismatch("interval");
  // Never post a branching in a failed space
  if (home.failed()) return;
  // Create an array of integer views
  ViewArray<IntView> vx(home,x);
  // Create an array of integers
  int* wc = static_cast<Space&>(home).alloc<int>(x.size());
  for (int i=x.size(); i--; )
    wc[i]=w[i];
  // Post the brancher
  IntervalBrancher::post(home,vx,wc,p);
}


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
        for (int i = 0; i < ni; i++) {
            if (sizes[i] == 2 || sizes[i] == 4) {
                empty_strip(2, i);
            }
            else if (sizes[i] == 3 || (sizes[i] >= 5 && sizes[i] <= 8)) {
                empty_strip(3, i);
            }
            else if (sizes[i] <= 11) {
                empty_strip(4, i);
            }
            else if (sizes[i] <= 17) {
                empty_strip(5, i);
            }
            else if (sizes[i] <= 21) {
                empty_strip(6, i);
            }
            else if (sizes[i] <= 29) {
                empty_strip(7, i);
            }
            else if (sizes[i] <= 34) {
                empty_strip(8, i);
            }
            else if (sizes[i] <= 44) {
                empty_strip(9, i);
            }
            else if (sizes[i] <= 45) {
                empty_strip(10, i);
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
        // Try the interval branch
        interval(*this, x, a_sizes, 0.6);
        interval(*this, y, a_sizes, 0.6);
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

