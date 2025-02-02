#include <iostream>
#include <ctime>
#include "simplicialChainComplex.h"
using namespace std;

float timeMark(clock_t & c) {
  float t = 1.0 * (clock() - c) / CLOCKS_PER_SEC;
  c = clock();
  return t;
}


int main(int argc, char *argv[]) {

  clock_t c_start = clock();

  simplicialPolyhedron P,Q;
  P.read(cin);
cerr << "Read polyhedron: " << timeMark(c_start) << "s" << endl;
  simplicialChainComplex S(P);
cerr << "Read and inflate simplicial Chain: " << timeMark(c_start) << "s" << endl;
  cout << "# SIMPLICIAL CHAIN COMPLEX" << endl;
  S.print(cout);
  
  cout << "# EULER CHARACTERISTIC" << endl;
  cout << S.eulerCharacteristic() << endl;


cout << endl << endl;
cout << "########################" << endl;
cout << "## HOMOLOGY / COHOMOLOGY" << endl;
cout << "########################" << endl;
cout << endl << endl;


  sparseMatrix  H[S.dim()+1];

  for (int i = S.dim(); i >= 0 ; i--) {
    H[i] = S.getHomology(i);
cerr << "Find homology classes of grade " << i << ": " << timeMark(c_start) << "s" << endl;
    cout << "# HOMOLOGY CLASSES OF GRADE " << i << "(" << H[i].size(1) << ")" << endl;
    if (H[i].size(1) == 1 ) {
      cout << "A" << i << "_1" << endl;
      H[i].print(cout);
    } else if (H[i].size(1) > 1) {
      for (int j = 0; j < H[i].size(1); j++) {
        cout << "A" << i << "_" << j+1 << endl;
        H[i][j].print(cout);
      }
    }
  }

  sparseMatrix cH[S.dim()+1];

  for (int i = S.dim(); i >= 0 ; i--) {
    cH[i] = S.getCohomology(i);
cerr << "Find cohomology classes of grade " << i << ": " << timeMark(c_start) << "s" << endl;
    cout << "# COHOMOLOGY CLASSES OF GRADE " << i << "(" << cH[i].size(1) << ")"  << endl;
    if (cH[i].size(1) == 1 ) {
      cout << "B" << i << "_1" << endl;
      cH[i].print(cout);
    } else if (cH[i].size(1) > 1) {
      for (int j = 0; j < cH[i].size(1); j++) {
        cout << "B" << i << "_" << j+1 << endl;
        cH[i][j].print(cout);
      }
    }
  }


cout << endl << endl;
cout << "########################" << endl;
cout << "## POINCARE DUALITY" << endl;
cout << "########################" << endl;
cout << endl << endl;

  for (int i = S.dim(); i >= 0 ; i--) {
    cout << "# PD OF COHOMOLOGY CLASSES (IN HOMOLOGY/COHOMOLOGY BASES ABOVE) OF GRADE " << i << endl;
    sparseMatrix PD_C = S.flat(i,cH[i]);
cerr << "Calcule Poincare dual of classes of grade " << i << ": " << timeMark(c_start) << "s" << endl;
    /*
    sparseMatrix L,D,U,Q,P;
    H[S.dim()-i].transpose().LDU_efficient(L,D,U,P,Q);
    if (i > 0) {
      PD_C = S.d_ldu[S.dim()-i].P * S.d_ldu[S.dim()-i].L.LComplementary(S.d_ldu[S.dim()-i].P.transpose()*PD_C.transpose());
    } else {
      // no boundaries to remove
      PD_C = PD_C.transpose();
    }

    sparseMatrix X = L.LXeqY(P.transpose()*PD_C);
    X.print_octave(cout);
    */
//sizePD_C.print(cerr); 
    S.getHomologyRepresentatives(S.dim()-i,PD_C).print_octave(cout);
cerr << "Calcule representatives: " << timeMark(c_start) << "s" << endl;
  }

cout << endl << endl;
cout << "########################" << endl;
cout << "## CUP PRODUCT" << endl;
cout << "########################" << endl;
cout << endl << endl;

  for (int i = 0 ; i <= S.dim(); i++)
    for (int j = 0; j <= S.dim(); j++) {
      if ( i + j <= S.dim() ) {
        cout << "# CUP PRODUCT OF COHOMOLOGY CLASSES OF GRADE " << i << " WITH ONES OF GRADE " << j << " (IN BASE {B" << i+j << "_i} )" << endl;
        sparseMatrix R = S.cup(i,cH[i],j,cH[j]);
cerr << "Calcule cup product of classes of grade " << i << " with classes of grade " << j <<": " << timeMark(c_start) << "s" << endl;
        sparseMatrix T = S.getCohomologyRepresentatives(i+j,R).transpose();
cerr << "Calcule representatives: " << timeMark(c_start) << "s" << endl;

        if (T.size(1) == 1) {
          cout << "B" << i << "_1" << " \\cup " << "B" << j << "_1 = " ;
          T.print_octave(cout);
          //R.print(cout);
        } else if (T.size(1) > 1)
          for (int k = 0 ; k < T.size(1); k++) {
            cout << "B" << i << "_" << (int) (k/cH[j].size(1)) + 1 << " \\cup " << "B" << j << "_" << (k%cH[j].size(1)) + 1 << " = ";
            T[k].print_octave(cout);
            //R[k].print(cout);
        }

      }

    }

  //delete[] H;
  //delete[] cH;

  return 0;
}
