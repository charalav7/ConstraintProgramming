/*
 *  Main author:
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
 * Assignment 3
 * Authors: Vasileios Charalampidis,
 * 		    Karl Gäfvert
 * 		    (Group 20)
 */

#include <gecode/int.hh>
#include <gecode/driver.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;

using namespace Gecode;
using namespace Gecode::Int;

// The no-overlap propagator
class NoOverlap : public Propagator {
  protected:
    ViewArray<IntView> x; // The x-coordinates
    int* w;               // The width (array)
    ViewArray<IntView> y; // The y-coordinates
    int* h;               // The heights (array)

  public:
    // Create propagator and initialize
    NoOverlap(Home home, 
              ViewArray<IntView> &x0, int w0[], 
              ViewArray<IntView> &y0, int h0[]): Propagator(home), 
        x(x0), w(w0), y(y0), h(h0)
    {
        x.subscribe(home, *this, PC_INT_BND);
        y.subscribe(home, *this, PC_INT_BND);
    }

    // Post no-overlap propagator
    static ExecStatus post(Home home, 
                           ViewArray<IntView>& x, int w[], 
                           ViewArray<IntView>& y, int h[])
    {
        // Only if there is something to propagate
        if (x.size() > 1)
            (void) new (home) NoOverlap(home, x, w, y, h);

        return ES_OK;
    }

    // Copy constructor during cloning
    NoOverlap(Space &home, NoOverlap &p): Propagator(home, p) {
        x.update(home, p.x);
        y.update(home, p.y);
        
        // Also copy width and height arrays
        w = home.alloc<int>(x.size());
        h = home.alloc<int>(y.size());
        for (int i = x.size(); i--; ) {
            w[i] = p.w[i];
            h[i] = p.h[i];
        }
    }

    // Create copy during cloning
    virtual Propagator* copy(Space &home) {
        return new (home) NoOverlap(home, *this);
    }

    // Re-schedule function after propagator has been re-enabled
    virtual void reschedule(Space &home) {
        x.reschedule(home, *this, PC_INT_BND);
        y.reschedule(home, *this, PC_INT_BND);
    }

    // Return cost (defined as cheap quadratic)
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
        return PropCost::quadratic(PropCost::LO, 2*x.size());
    }

    // Macro for domain check and reduction
    #define nol(v1,cq,op,v2,i1) (me_failed(v1[j].cq(home, v1[i].val() op v2[i1])))

    // Perform propagation
    virtual ExecStatus propagate(Space &home, const ModEventDelta&) {
        const int n = x.size();

        // Detect early subsumption
		// This makes sence, however, only in the case when the 
        // propagator allready have been run atleast one time.
        // If not, all the x and y cordinates may have been assigned
        // but we can still have a overlap because that haven't been
        // checked yet by the code below.
//         for (int i = n; i--; )
//             if (!(x[i].assigned() && y[i].assigned()))
//                 goto NO_EARLY_SUBSUMPTION;    
        
//         return home.ES_SUBSUMED(*this);
// NO_EARLY_SUBSUMPTION:

		// If a square have a fixed position (x,y), allow all other 
        // squares to be positioned either left, right, top, or 
        // bottom of that square and reduce their domains accordingly.
        // The ckeck is performed by testing if the domain can be
        // reduced for each of positions relative to the fixed square.
        // If one staifactory direction is found the checks stop.
        // If no staifactory direction is found this is an invalid 
        // soulution.
        // We have subsumption if all x and y coordinates have been
        // assigned and there are no overlaps detetected.
        // Otherwise running this propagater again will result in the
        // same domain reductions (as no new values have been assigned) 
        // meaning we have reached a fix-point. 
        bool subsumed = true;
        for (int i = 0; i < (n - 1); i++) {
            if (x[i].assigned() && y[i].assigned()) 
                for (int j = i + 1; j < n; j++) {
                    if (nol(x, lq, -, w, j) &&
                        nol(y, lq, -, h, j) &&
                        nol(x, gq, +, w, i) &&
                        nol(y, gq, +, h, i)) return ES_FAILED;
                }
            else subsumed = false;
        }
        return subsumed ? home.ES_SUBSUMED(*this) : ES_FIX;
    }

    // Dispose propagator and return its size
    virtual size_t dispose(Space& home) {
        x.cancel(home, *this, PC_INT_BND);
        y.cancel(home, *this, PC_INT_BND);
        (void) Propagator::dispose(home);
        return sizeof(*this);
    }
};

/*
 * Post the constraint that the rectangles defined by the coordinates
 * x and y and width w and height h do not overlap.
 *
 * This is the function that you will call from your model. The best
 * is to paste the entire file into your model.
 */
void nooverlap2(Home home, 
               const IntVarArgs &x, const IntArgs &w,
               const IntVarArgs &y, const IntArgs &h)
{
    // Check whether the arguments make sense
    if ((x.size() != y.size()) || (x.size() != w.size()) ||
        (y.size() != h.size()))
        throw ArgumentSizeMismatch("nooverlap");
    
    // Never post a propagator in a failed space
    if (home.failed()) return;
    
    // Set up array of views for the coordinates
    ViewArray<IntView> vx(home,x);
    ViewArray<IntView> vy(home,y);
    
    // Set up arrays (allocated in home) for width and height and initialize
    int* wc = static_cast<Space&>(home).alloc<int>(x.size());
    int* hc = static_cast<Space&>(home).alloc<int>(y.size());
    for (int i = x.size(); i--; ) {
        wc[i] = w[i];
        hc[i] = h[i];
    }

    // If posting failed, fail space
    if (NoOverlap::post(home, vx, wc, vy, hc) != ES_OK)
        home.fail();
}

// Assignment 2 Squares-class for testing
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
        // Empty strip dominace
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

        const IntArgs a_sizes = IntArgs(ni, sizes);
        // Add custom nooverlap propagator
        nooverlap2(*this, x, a_sizes, y, a_sizes);
        
        // Row/column sum is less than s 
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

        // Branching
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
