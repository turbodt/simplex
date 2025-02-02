#include "simplicialChainComplex.h"

int simplicialChainComplex::bubbleSort(int n, int * const v) const {

  if (n < 1)
    return 0;

  int sign = 1;
  
  for (int i = 0; i < n; i++)
    for (int j = i + 1; j < n ; j++)
      if ( v[i] > v[j] ) {
        int a = v[i];
        v[i] = v[j];
        v[j] = a;
        sign *= -1;
      }
  
  return sign;
}

void simplicialChainComplex::vcat(int n1, int n2, int m, int * const C, int const * const A, int const * const B) const {
  for (int j = m; j > 0; j--) {
    memcpy(&C[(n1+n2)*(j-1)],&A[n1*(j-1)], n1*sizeof(int));
    memcpy(&C[(n1+n2)*(j-1) + n1],&B[n2*(j-1)], n2*sizeof(int));
  }
}


void simplicialChainComplex::vsplit(int n, int m, int * A, int k, int const * const v, int * const B) const {

  int delay = 0;
  for (int j = 0 ; j < m ; j++) {
    int c = 0;
    for (int i = 0; i < n; i++) {
      while (c < k && v[c] < i)
        c++;
        
      if (c < k && i == v[c]) {
        A[j*n + i - delay] = A[j*n + i];
      } else {
        if (B != NULL)
          B[delay] = A[j*n + i];
        delay++;
      }
    }
  }
}





simplicialChainComplex& simplicialChainComplex::inflate(const simplicialPolyhedron& P) {
  this->n = P.dim()+1;
  this->P = (simplicialPolyhedron *) malloc(this->n*sizeof(simplicialPolyhedron));
  this->isHomologyCalculated = (bool *) malloc((this->n)*sizeof(bool));
  this->isCohomologyCalculated = (bool *) malloc((this->n)*sizeof(bool));
  this->homology = (sparseMatrix *) malloc((this->n)*sizeof(sparseMatrix));
  this->cohomology = (sparseMatrix *) malloc((this->n)*sizeof(sparseMatrix));
  this->d = (sparseMatrix *) malloc((this->n-1)*sizeof(sparseMatrix));
  this->d_ldu = (LDU_group *) malloc((this->n-1)*sizeof(LDU_group));
  this->orientation = (int *) malloc(P.length()*sizeof(int));
  for (int i = 0; i < P.length() ; i++)
    this->orientation[i] = 1;
  
  // skeleton

  this->P[this->n-1] = P;
  this->P[this->n-1].simplifySimplexes(this->orientation);
  
  for (int i = this->n-1; i > 0 ; i--)
    this->P[i - 1] = this->P[i].skeleton();
    
  // boundary operators

  for (int i = 0; i < this->n - 1; i++) {
    int numValues = (i+2)*this->P[i+1].length();
    int * values = (int *) malloc(numValues*sizeof(int));
    int * cols = (int *) malloc(numValues*sizeof(int));
    int * rows = (int *) malloc((this->P[i+1].length()+1)*sizeof(int));
    
    for (int j = 0; j < this->P[i+1].length(); j++) {
      
      int * signs = (int *) malloc((i+2)*sizeof(int));
      for (int k = 0; k < (i+2) ; k++)
        signs[k] = 1;
        
      simplicialPolyhedron S = this->P[i+1][j].boundary(signs);
      memcpy(&values[(i+2)*j],signs,(i+2)*sizeof(int));
      
      this->P[i].binarySearch(S,&cols[(i+2)*j]);
      for (int k = 0 ; k < S.length() ; k++)
        if (cols[(i+2)*j + k] < 0 ) {
          cerr << "Error!! facet not found at skeleton" << endl;
          cerr << "facet:" << endl;
          S[k].print(cerr);
          cerr << "of simplex:" << endl;
          this->P[i+1][j].print(cerr);
          cerr << "searched in:" << endl;
          this->P[i].print(cerr);
        }

      free(signs);
    }

    for (int j = 0; j <= this->P[i+1].length(); j++)
      rows[j] = (i+2)*j;

    this->d[i] = sparseMatrix (this->P[i+1].length(),this->P[i].length(),rows,cols,values);
    this->d[i] = this->d[i].transpose();

    //free
    free(rows);
    free(cols);
    free(values);
  }

  // boundary LDU

  for (int i = 0; i < this->n - 1; i++) {
    this->d[i].LDU_efficient(this->d_ldu[i].L,this->d_ldu[i].Dl,this->d_ldu[i].U,this->d_ldu[i].P,this->d_ldu[i].Q);
    this->d_ldu[i].Du = this->d_ldu[i].Dl; // tenen la mateixa forma, encara que no els mateixos valors
    // actualitzem els valors
    for (int j = 0; j < this->d_ldu[i].Du.length(); j++) {
      this->d_ldu[i].Du.getValues()[j] = Tools::gcd(this->d_ldu[i].U.numValuesInRow(j), &this->d_ldu[i].U.getValues()[this->d_ldu[i].U.getRows()[j]]);
      // ara els de U
      if (this->d_ldu[i].Du.getValues()[j] != 1 && this->d_ldu[i].Du.getValues()[j] != -1)
        for (int k = this->d_ldu[i].U.getRows()[j]; k < this->d_ldu[i].U.getRows()[j] + this->d_ldu[i].U.numValuesInRow(j); k++)
          this->d_ldu[i].U.getValues()[k] /= this->d_ldu[i].Du.getValues()[j];
    }


  }

  // homology / cohomology
  for (int i = 0; i < this->n ; i++) {
    this->isHomologyCalculated[i] = false;
    this->isCohomologyCalculated[i] = false;
  }

  return *this;
}

simplicialPolyhedron simplicialChainComplex::deflate(int * sign) const {
  if (sign != NULL) {
    for (int i = 0; i < this->P[this->n-1].length(); i++)
      sign[i] = this->orientation[i];
  }
  
  return this->P[n-1];
}

simplicialChainComplex::simplicialChainComplex() {
  this->n = 0;
  this->P = NULL;
  this->d = NULL;
  this->orientation = NULL;
}

simplicialChainComplex::~simplicialChainComplex() {
  free(this->P);
  free(this->d);
  free(this->orientation);
}

simplicialChainComplex::simplicialChainComplex(const simplicialPolyhedron& P) {
  this->inflate(P);
}

simplicialChainComplex::simplicialChainComplex(const simplicialChainComplex& CC) {
  *this = CC;
}

simplicialChainComplex& simplicialChainComplex::operator=(const simplicialChainComplex& CC) {
  this->n = CC.n;
  this->P = (simplicialPolyhedron *) malloc(this->n*sizeof(simplicialPolyhedron));
  this->d = (sparseMatrix *) malloc((this->n-1)*sizeof(sparseMatrix));
  this->orientation = (int *) malloc(CC.P[CC.n - 1].length()*sizeof(int));
  
  for (int i = 0; i < this->n ; i++)
    this->P[i] = simplicialPolyhedron(CC.P[i]);
    
  for (int i = 0; i < this->n -1 ; i++)
    this->d[i] = sparseMatrix(CC.d[i]);
  
  for (int i = 0; i < CC.P[CC.n -1].length(); i++)
    this->orientation[i] = CC.orientation[i];
  
  return *this;
}

simplicialChainComplex& simplicialChainComplex::read(istream& in) {

  in >> this->n;
  this->P = (simplicialPolyhedron *) malloc(this->n*sizeof(simplicialPolyhedron));
  this->d = (sparseMatrix *) malloc((this->n-1)*sizeof(sparseMatrix));
  
  for (int i = 0; i < this->n ; i++)
    this->P[i].read(in);

  for (int i = 0; i < this->n - 1  ; i++)
    this->d[i].read(in);
  
  int n;
  in >> n;
  for (int i = 0; i < n; i++)
    in >> this->orientation[i];

  return *this;
} 

const simplicialChainComplex& simplicialChainComplex::print(ostream & out) const {
  
  out << this->n << endl;
  
  for (int i = 0; i < this->n ; i++)
    this->P[i].print(out);
  
  for (int i = 0; i < this->n - 1; i++)
    this->d[i].print(out);
  
  out << this->P[this->n-1].length() << endl;
  for (int i = 0; i < this->P[this->n-1].length(); i++)
    out << " " << this->orientation[i];
  out << endl;

  
  return *this;
}




int simplicialChainComplex::dim() const {
  return this->n - 1;
}

int simplicialChainComplex::length(int i) const {
  return this->P[i].length();
}

int simplicialChainComplex::eulerCharacteristic() const {
  int s = 0, sign = 1;
  for (int i = 0; i < this->n; i++) {
    s += sign*(this->P[i].length());
    sign *= -1;
  }
    
  return s;
}

sparseMatrix simplicialChainComplex::fundamentalClass() const {
	int * values = (int *) malloc(this->P[this->n -1].length() * sizeof(int));
	int * rows = (int *) malloc(this->P[this->n -1].length() * sizeof(int));
	int * cols = (int *) malloc(2*sizeof(int));
	
  cols[0] = 0;
	for (int i = 0; i < this->P[this->n -1].length(); i++) {
		values[i] = this->orientation[i];
		rows[i] = i;
	}
  cols[1] = this->P[this->n -1].length();
	
	return sparseMatrix(1,this->P[n-1].length(),cols,rows,values);
	
}

simplicialPolyhedron& simplicialChainComplex::operator[](int i) {

  if (i < 0 || i >= n)
    cerr << "Value " << i << " is out of range 0 to max dim = " << this->n-1 << endl;

  return this->P[i];
}
  
const simplicialPolyhedron& simplicialChainComplex::operator[](int i) const {
  if (i < 0 || i >= n)
    cerr << "Value " << i << " is out of range 0 to max dim = " << this->n-1 << endl;
  return this->P[i];
}

sparseMatrix& simplicialChainComplex::boundaryOperator(int i) {
  if (i < 1 || i >= n)
    cerr << "Value " << i << " is out of range 1 to max dim = " << this->n-1 << endl;

  return this->d[i-1];
}

const sparseMatrix& simplicialChainComplex::boundaryOperator(int i) const {
  if (i < 1 || i >= n)
    cerr << "Value " << i << " is out of range 1 to max dim = " << this->n-1 << endl;

  return this->d[i-1];
}

sparseMatrix simplicialChainComplex::adjacencyMatrix(int i, int j) const {

  if (i > j)
    return this->adjacencyMatrix(j,i).transpose();

  if (i < 0 || i >= this->n)
    cerr << "Value (first) =" << i << " is out of range 0 to max dim = " << this->n-1 << endl;

  if (j < 0 || j >= this->n)
    cerr << "Value (second) =" << j << " is out of range 0 to max dim = " << this->n-1 << endl;
    
  if (i == j) {
    sparseMatrix M;
    M.eye(this->P[i].length());
  }
  
  sparseMatrix M(this->d[j-1]);
  for (int l = 0 ; l < M.length() ; l++)
    M.getValues()[l] = 1;
    
  if (i == j-1)
    return M; // we save one matrix multiplication
  
  M = this->adjacencyMatrix(i,j-1)*M;
 
  for (int l = 0 ; l < M.length() ; l++)
    M.getValues()[l] = 1;
  
  return M;

}

sparseMatrix simplicialChainComplex::boundary(int i,const sparseMatrix& M) const {
  if (i < 1 || i >= n)
    return sparseMatrix(0,0);

  sparseMatrix A;
  return A.multiplyByTransposed(M,this->d[i-1]);
}

simplicialPolyhedron simplicialChainComplex::support(int i, const sparseMatrix& M) const {
  if (i < 1 || i >= this->n)
    cerr << "Value " << i << " is out of range 0 to max dim = " << this->n-1 << endl;
    
  if (M.size(2) != this->P[i].length() || M.size(1) != 1)
    cerr << "Chain does not correspond to a " << i << "-chain" << endl;
    
  int * A = (int *) malloc((i+1)*M.length()*sizeof(int));
  
  int * values , *rows, *cols;
  int numCols = M.size(2), numRows = M.size(1), numValues = M.length();
  M.decompose(&values,&cols,&rows);
  
  for (int j = 0; j < numValues; j++)
    memcpy(&A[(i+1)*j],&(this->P[i].values()[(i+1)*cols[j]]),(i+1)*sizeof(int));
  
  return simplicialPolyhedron(i,numValues,A);
  
}

/*sparseMatrix simplicialChainComplex::cup(int k, const sparseMatrix& M, int l, const sparseMatrix& N) const {
  
  if ( k < 0 || l < 0 || k+l > this->n -1 )
    return M(0,0);
  
  sparseMatrix A = this->;

}
*/

sparseMatrix simplicialChainComplex::cup(int k1, const sparseMatrix & M1, int k2, const sparseMatrix & M2) const {
  
  int numRows = (M1.size(1) * M2.size(1) );
  int * rows = (int *) calloc((numRows +1) ,sizeof(int));

  if (k1 + k2 > this->dim() || k1 < 0 || k2 < 0) {
    sparseMatrix N(numRows,0,rows,NULL,NULL);
    free(rows);
    return N;
  }
  // allocate all memory
  int numCols = this->P[k1 + k2].length();
  int * cols = (int *) malloc(numRows *numCols *sizeof(int));
  int * values = (int *) malloc(numRows *numCols *sizeof(int));
  int * simplex = (int *) malloc((k1+k2+1)*sizeof(int));
  rows[0] = 0;
  for (int i = 0, lastRow = 0; i < M1.size(1); i++)
    for (int j = 0; j < M2.size(1) ; j++, lastRow++) {
      // prepare
      rows[lastRow + 1] = rows[lastRow];

      const int * local_cols1 = &M1.getCols()[M1.getRows()[i]];
      const int * local_values1 = &M1.getValues()[M1.getRows()[i]];
      const int * local_cols2 = &M2.getCols()[M2.getRows()[j]];
      const int * local_values2 = &M2.getValues()[M2.getRows()[j]];
      // calcule first bound of possible values
      for (int k = 0; k < M1.numValuesInRow(i); k++ )
        for (int l = 0; l < M2.numValuesInRow(j); l++ )
          // values last vertex of first simplex coincides with first vertex of the second simplex
          if (this->P[k1].values()[local_cols1[k]*(k1+1)+k1] == this->P[k2].values()[local_cols2[l]*(k2+1)]) {
            // simplex resulting of contatenate exists
/*for (int lcerr = 0; lcerr < k1+1; lcerr++ )
  cerr << " " << this->P[k1].values()[local_cols1[k]*(k1+1) + lcerr];
cerr << endl;
for (int lcerr = 0; lcerr < k2+1; lcerr++ )
  cerr << " " << this->P[k2].values()[local_cols2[l]*(k2+1) + lcerr];
cerr << endl;
*/
            memcpy(simplex,&this->P[k1].values()[local_cols1[k]*(k1+1)],(k1+1)*sizeof(int));
            memcpy(&simplex[k1],&this->P[k2].values()[local_cols2[l]*(k2+1)],(k2+1)*sizeof(int)); // note that first vertex is superposed
            simplicialPolyhedron S(k1 + k2,1,simplex);
//S.print(cerr);
            int index = -1;
            this->P[k1+k2].binarySearch(S,&index);
//cerr << index << endl;
            if (index >= 0) {
              // existeix
              cols[rows[lastRow + 1]] = index;
              values[rows[lastRow + 1]] = local_values1[i]*local_values2[j];
              rows[lastRow + 1]++;
            } else {
              // no existeix
            }
          }

    }

  free(simplex);

  sparseMatrix N(numRows,numCols,rows,cols,values);
  free(rows);
  free(cols);
  free(values);

  return N;
}

sparseMatrix simplicialChainComplex::flat(int k, const sparseMatrix & M) const {


  if (M.size(1) > 1) {

    //maybe not the fastest way
    //but it works

    sparseMatrix N[M.size(1)];
    for (int i = 0; i < M.size(1); i++) {
      N[i] = this->flat(k,M[i]);
    }

    int * rows = (int *) malloc((M.size(1)+1)*sizeof(int));
    rows[0] = 0;
    for (int i = 0; i < M.size(1); i++)
      rows[i+1] = rows[i] + N[i].length();
    
    int * cols = (int *) malloc(rows[M.size(1)]*sizeof(int));
    int * values = (int *) malloc(rows[M.size(1)]*sizeof(int));

    for (int i = 0; i < M.size(1); i++) {
      memcpy(&cols[rows[i]],N[i].getCols(), N[i].length()*sizeof(int));
      memcpy(&values[rows[i]],N[i].getValues(), N[i].length()*sizeof(int));
    }

    sparseMatrix R(M.size(1),N[0].size(2), rows, cols, values);
  
    //free

    free(rows);
    free(cols);
    free(values);
  
    return R;
  }

  int * A = (int *) malloc((k+1)*M.length()*sizeof(int));
  
  // copy k-simplexes to A
  for ( int i = 0; i < M.length(); i++)
    memcpy(&A[(k+1)*i],&this->P[k].values()[(k+1)*M.getCols()[i]],(k+1)*sizeof(int));

  simplicialPolyhedron Q(k,M.length(),A);
  free(A);
  // search them on maximal simplex
  
  int * start = (int *) malloc(Q.length()*sizeof(int));
  int * end = (int *) malloc(Q.length()*sizeof(int));
  
  this->P[this->dim()].subSearch(Q,start,end);
  
  // get complementary

  int l = 0;
  for (int i = 0; i < Q.length(); i++)
    l += end[i] - start[i];
  A = (int *) malloc((this->dim()-k+1)*l*sizeof(int));
  int * signs = (int *) malloc(l*sizeof(int));
  
  for (int i = 0, cnt = 0 ; i < Q.length(); i++)
    for (int j = start[i]; j < end[i]; j++, cnt++) {
      signs[cnt] = M.getValues()[i] * this->orientation[j];
      memcpy(&A[(this->dim()-k+1)*cnt], &this->P[this->dim()].values()[(this->dim()+1)*j + k], (this->dim()-k+1)*sizeof(int));
    }
  
  simplicialPolyhedron R(this->dim() - k, l, A);
  free(A);
  free(start);
  free(end);
  R.simplifySimplexes(signs);
  // search them on (dim - k) - simplexes
  
  start = (int *) malloc(R.length()*sizeof(int));
  end = (int *) malloc(R.length()*sizeof(int));
  
  this->P[this->dim() - k].subSearch(R,start,end);
  
  // put'em on a matrix

  l = 0;
  for (int i = 0; i < R.length(); i++)
    l += end[i] - start[i];
  
  int * rows = (int *) malloc(2*sizeof(int));
  rows[0] = 0;
  rows[1] = l;
  int * cols = (int *) malloc(l*sizeof(int));
  int * values = (int *) malloc(l*sizeof(int));

  for (int i = 0, cnt = 0 ; i < R.length(); i++)
    for (int j = start[i]; j < end[i]; j++, cnt++) {
      cols[cnt] = j;
      values[cnt] = signs[i];
    }
    
  sparseMatrix N(1,this->P[this->dim() - k].length(), rows, cols, values);
  N.removeZeros();
  
  //free

  free(start);
  free(end);
  free(signs);
  free(rows);
  free(cols);
  free(values);
  
  return N;

}

sparseMatrix simplicialChainComplex::getHomology(int i) const {

  if (i == this->dim()) {

    if ( !this->isHomologyCalculated[i] ) {
      this->homology[i] = (this->d_ldu[i-1].Q.transpose() * this->d_ldu[i-1].U.ker()).transpose();
      this->isHomologyCalculated[i] = true;

    }

    return this->homology[i];

  } else if (i == 0) {

    if ( !this->isHomologyCalculated[i] ) {
      sparseMatrix P1tP0KerL0t = this->d_ldu[0].P.transpose();
      sparseMatrix ImL1 = this->d_ldu[0].L;
      sparseMatrix X = this->d_ldu[0].P*ImL1.LComplementary(P1tP0KerL0t);
      
      this->homology[i] = X.transpose();
      this->isHomologyCalculated[i] = true;
    }


    return this->homology[i];

  } else if ( i < this->dim() && i > 0) {

    if ( !this->isHomologyCalculated[i] ) {

      sparseMatrix P1tP0KerL0t = this->d_ldu[i].P.transpose()*this->d_ldu[i-1].Q.transpose()*(this->d_ldu[i-1].U.ker());
      sparseMatrix ImL1 = this->d_ldu[i].L;
      sparseMatrix X = this->d_ldu[i].P*ImL1.LComplementary(P1tP0KerL0t);

      this->homology[i] = X.transpose();
      this->isHomologyCalculated[i] = true;

    }

    return this->homology[i];

  }

  return sparseMatrix();

}

sparseMatrix simplicialChainComplex::getCohomology(int i) const {

  if (i == 0) {

    if ( !this->isCohomologyCalculated[i] ) {
      this->cohomology[i] = (this->d_ldu[i].P * this->d_ldu[i].L.transpose().ker()).transpose();
      this->isCohomologyCalculated[i] = true;
    }

    return this->cohomology[i];

  } else if (i == this->dim()) {

    if ( !this->isCohomologyCalculated[i] ) {

      sparseMatrix P0tP1KerL1t = this->d_ldu[i-1].Q.transpose();
      sparseMatrix ImL0 = this->d_ldu[i-1].U.transpose();
      sparseMatrix X = this->d_ldu[i-1].Q.transpose()*ImL0.LComplementary(P0tP1KerL1t);

      this->cohomology[i] = X.transpose();
      this->isCohomologyCalculated[i] = true;
    }

    return this->cohomology[i];

  } else if ( i < this->dim() && i > 0) {
  
    if ( !this->isCohomologyCalculated[i] ) {

      sparseMatrix P0tP1KerL1t = this->d_ldu[i-1].Q*this->d_ldu[i].P*(this->d_ldu[i].L.transpose().ker());
      sparseMatrix ImL0 = this->d_ldu[i-1].U.transpose();
      sparseMatrix X = this->d_ldu[i-1].Q.transpose()*ImL0.LComplementary(P0tP1KerL1t);

      this->cohomology[i] = X.transpose();
      this->isCohomologyCalculated[i] = true;
    }

    return this->cohomology[i];

  }

  return sparseMatrix();

}

sparseMatrix simplicialChainComplex::getHomologyRepresentatives(int i, sparseMatrix M) const {

    sparseMatrix L,D,U,Q,P;
    this->getHomology(i).transpose().LDU_efficient(L,D,U,P,Q);
    if (i < this->dim()) {
      M = this->d_ldu[i].P * this->d_ldu[i].L.LComplementaryEach(this->d_ldu[i].P.transpose()*M.transpose());
    } else {
      // no boundaries to remove
      M = M.transpose();
    }

// D and U should be diagonal and self inverses
    sparseMatrix X = Q.transpose()*U*D*(L.LXeqY(P.transpose()*M));
    return X;

}

sparseMatrix simplicialChainComplex::getCohomologyRepresentatives(int i, sparseMatrix M) const {

    sparseMatrix L,D,U,Q,P;
    this->getCohomology(i).transpose().LDU_efficient(L,D,U,P,Q);

    if (i > 0) {
//this->d_ldu[i-1].U.transpose().print_octave(cerr);
      M = this->d_ldu[i-1].Q.transpose() * this->d_ldu[i-1].U.transpose().LComplementaryEach(this->d_ldu[i-1].Q*M.transpose());
    } else {
      // no coboundaries to remove
      M = M.transpose();
    }
// D and U should be diagonal and invertibles and self inverses
    sparseMatrix X = Q.transpose()*U*D*L.LXeqY(P.transpose()*M);
    return X;

}

