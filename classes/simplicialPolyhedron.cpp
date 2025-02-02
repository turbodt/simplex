#include "simplicialPolyhedron.h"

int simplicialPolyhedron::vectorMax(int n, int const * const A) const {
  if (n <= 0)
    return -1;
  int M = A[0];
  for (int i = 1; i < n; i++)
    if (M < A[i])
      M = A[i];
  return M;
}


int simplicialPolyhedron::bubbleSort(int n, int * const v) const {

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

bool simplicialPolyhedron::search(int n1, int m, int * A, int n2, int const * const v, int & start, int & end) const {
  
  if (n2 > n1)
    return false;
  
  if ( end <= 0)
    end = m;
  
  if ( end == start)
    return false;
    
  int c = (start + end)/2;
  
  if (!Tools::leq(n2,&A[n1*c],v)) {
    end = c;
    return this->search(n1,m,A,n2,v,start,end);
  } else if (!Tools::leq(n2,v,&A[n1*c])) {
    start = c + 1;
    return this->search(n1,m,A,n2,v,start,end);
  } else {
    // now we have A[n1*c] == v
    if (!Tools::leq(n2,v,&A[n1*start]) ){
      start++;
      this->search(n1,m,A,n2,v,start,c); // find start of the block
    }

    if (!Tools::leq(n2,&A[n1*(end-1)],v) ) {
      end--;
      this->search(n1,m,A,n2,v,c,end); // find end of the block
    }
    
    return true;

  }

  return false;

}

void simplicialPolyhedron::vcat(int n1, int n2, int m, int * const C, int const * const A, int const * const B) const {
  for (int j = m; j > 0; j--) {
    memcpy(&C[(n1+n2)*(j-1)],&A[n1*(j-1)], n1*sizeof(int));
    memcpy(&C[(n1+n2)*(j-1) + n1],&B[n2*(j-1)], n2*sizeof(int));
  }
}


void simplicialPolyhedron::vsplit(int n, int m, int * A, int k, int const * const v, int * const B) const {

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

long long int simplicialPolyhedron::choose(int n, int m) const {
  
  int k = max(n,m);
  n = min(n,m);  
  m = k;
  n = min(n,m-n);
  long long int res = 1;

  for (int i = 1; i <= n; i++)
    res = res*(m-n+i)/i;
  
  return res;
  
}




simplicialPolyhedron::simplicialPolyhedron() {
  this->n = 0;
  this->m = 0;
  this->A = NULL;
}

simplicialPolyhedron::simplicialPolyhedron(const simplicialPolyhedron& P) {
  this->n = P.dim() + 1;
  this->m = P.length();
  this->A = (int *) malloc(this->n*this->m*sizeof(int));
  memcpy(this->A,P.values(),this->n*this->m*sizeof(int));
}

simplicialPolyhedron::simplicialPolyhedron(int n, int m, int const * const A) {
  this->n = n + 1;
  this->m = m;
  this->A = (int *) malloc((n+1)*m*sizeof(int));
  memcpy(this->A,A,(n+1)*m*sizeof(int));
}

simplicialPolyhedron::~simplicialPolyhedron() {
  free(this->A);
}


simplicialPolyhedron& simplicialPolyhedron::operator=(const simplicialPolyhedron& P) {
  this->n = P.dim() + 1;
  this->m = P.length();
  this->A = (int *) realloc(this->A,this->n*this->m*sizeof(int));
  memcpy(this->A,P.values(),this->n*this->m*sizeof(int));
  return *this;
}

int simplicialPolyhedron::dim() const {
  return this->n - 1;
}

int simplicialPolyhedron::length() const {
  return this->m;
}

const int * simplicialPolyhedron::values() const {
  return this->A;
};

simplicialPolyhedron& simplicialPolyhedron::read(istream & in) {
  int n, m;
  in >> n >> m;
  n--;
  int * A = (int *) malloc((n+1)*m*sizeof(int));

  
  for (int i = 0; i < n+1; i++)
    for (int j = 0; j < m; j++)
      in >> A[ (n+1)*j + i];
/*
  for (int i = 0; i < m; i++)
    for (int j = 0; j < n+1; j++)
      in >> A[ (n+1)*i + j];
*/
  this->n = n + 1;
  this->m = m;
  this->A = (int *) malloc((n+1)*m*sizeof(int));
  memcpy(this->A,A,(n+1)*m*sizeof(int));

  return *this;

}

const simplicialPolyhedron& simplicialPolyhedron::print(ostream & out) const {
  
  out << this->n << " " << this->m << endl;

  for (int i = 0 ; i < this->n; i++) {
    for (int j = 0; j < this->m; j++)
      out << " " << this->A[j*this->n + i];
    out << endl;
  }
/*
  for (int i = 0 ; i < this->m; i++) {
    for (int j = 0; j < this->n; j++)
      out << " " << this->A[i*this->n + j];
    out << endl;
  } */
  
  return *this;
}



simplicialPolyhedron& simplicialPolyhedron::orientSimplexes(int * signs) {

  for (int i = 0; i < this->length(); i++) {
    if (signs[i] < 0) {
      // swap 
      int a = this->A[i*(this->dim()+1) + this->dim() - 1];
      this->A[i*(this->dim()+1) + this->dim() - 1] = A[i*(this->dim()+1) + this->dim()];
      this->A[i*(this->dim()+1) + this->dim() ] = a;
    }
  }

  return *this;
}

simplicialPolyhedron& simplicialPolyhedron::sortSimplexes(int * signs) {
  if (signs != NULL) 
    for (int i = 0; i < m ; i++)
      signs[i] *= this->bubbleSort(this->n,&(this->A[n*i]));
  else
    for (int i = 0; i < m ; i++)
      this->bubbleSort(this->n,&(this->A[n*i]));
      
  return *this;
}


simplicialPolyhedron& simplicialPolyhedron::simplifySimplexes(int * coeffs) {

  if ( this->n <= 0)
    return *this;

  if (coeffs == NULL) {
    coeffs = (int *) malloc(this->m*sizeof(int));
    for (int i = 0; i < this->m ; i++)
      coeffs[i] = 1;
  }

  int * p = (int *) malloc(this->m*sizeof(int));
  for (int i = 0; i < this->m; i++)
    p[i] = i;

  int * Ap = (int *) malloc((this->n+1)*this->m*sizeof(int));
  
  // sort simplexes  
  this->sortSimplexes(coeffs);
  // sort among them and obtain
  // the corresponding permutation
  this->vcat(this->n,1,this->m,Ap,this->A,p);
  Tools::mergeSortBlocks(this->n+1,this->m,Ap);

  int * v = (int *) malloc(this->n*sizeof(int));
  for (int i = 0; i < this->n; i++)
    v[i] = i;
  
  this->vsplit(this->n+1,this->m,Ap,this->n,v,p);
    
  // count number of different simplexes
  int m2 = 0; // number of DIFERENT simplexes
  for (int i = 0; i < this->m; i++)
    if ( i == 0 || !Tools::eq(this->n,&Ap[this->n*(i-1)],&Ap[this->n*i]) )
      m2++;
  
  // group by

  int * c = (int *) malloc(this->m*sizeof(int));
  memcpy(c,coeffs,this->m*sizeof(int));
  
  this->A = (int *) realloc(this->A,this->n*m2*sizeof(int));
  if (m2 < this->m)
    coeffs = (int *) realloc(coeffs,m2*sizeof(int));
  int cnt = -1;
  for (int i = 0; i < this->m; i++)
    if ( i == 0 || !Tools::eq(n,&Ap[this->n*(i-1)],&Ap[this->n*i]) ) {
      cnt++;
      memcpy(&A[n*cnt],&Ap[this->n*i],this->n*sizeof(int));
      memcpy(&coeffs[1*cnt],&c[p[i]],1*sizeof(int));
    } else {
      coeffs[1*cnt] += c[p[i]];
    } 
  // delete 0's

  cnt = 0;
  
  for (int i = 0; i < m2; i++)
    if ( coeffs[i] != 0 ) {
      if ( cnt != i) {
        memcpy(&A[this->n*cnt],&A[this->n*i],this->n*sizeof(int));
        memcpy(&coeffs[1*cnt],&coeffs[1*i],1*sizeof(int));
      }
      cnt++;
    }
        
  int m3 = cnt;
  this->A = (int *) realloc(this->A,this->n*m3*sizeof(int));
  if (m3 < m2)
    coeffs = (int *) realloc(coeffs,1*m3*sizeof(int));

  this->m = m3;

  //free
  free(Ap);
  free(c);
  free(p);
  free(v);

  return *this;
  
}


simplicialPolyhedron simplicialPolyhedron::boundary(int * coeffs2, int * coeffs) const{

  if (this->n == 1) {
    return simplicialPolyhedron();
  }

  if (coeffs == NULL) {
    // sign of the original simplexes (by default 1)
    coeffs = (int *) malloc(this->m*sizeof(int));
    for (int i = 0; i < this->m ; i++)
      coeffs[i] = 1;
  }
  
  if (coeffs2 == NULL) {
    // sign of the boundary resultant simplexes
    coeffs2 = (int *) malloc(this->n*this->m*sizeof(int));
    for (int i = 0; i < this->m*this->n ; i++)
      coeffs2[i] = 1;
  }
    
  simplicialPolyhedron res((this->n)-1,this->m,this->A);
  res.simplifySimplexes(coeffs);
  int n = res.dim()+1, m = res.length();
  const int * A = res.values();  
  
  int * B = (int *) malloc((n-1)*n*m*sizeof(int));
  int * v = (int *) malloc(n*sizeof(int));
  
  for (int i = 0; i < n - 1; i++)
    v[i] = i + 1;
  
  int sign = 1;
  
  for (int i = 0; i < n ; i++) {
    int * A2 = (int *) malloc(n*m*sizeof(int));
    memcpy(A2,A,n*m*sizeof(int));
    this->vsplit(n,m,A2,n-1,v,NULL);
    memcpy(&B[(n-1)*m*i],A2,(n-1)*m*sizeof(int));
    for (int j = 0; j < m; j++)
      coeffs2[m*i + j] = coeffs[j]*sign;
    if (i < n-1 )
      v[i]--;
    sign *= -1;
    free(A2);
  }

  res = simplicialPolyhedron((n-1)-1,n*m,B);
  res.simplifySimplexes(coeffs2);
  free(B);
  free(v);
  return res;

}

simplicialPolyhedron simplicialPolyhedron::skeleton() const{

  if (this->n == 1) {
    return simplicialPolyhedron();
  }
  
  simplicialPolyhedron res((this->n)-1,this->m,A);
  
  res.simplifySimplexes();
  int n = res.dim()+1, m = res.length();
  const int * A = res.values();
  
  
  int * B = (int *) malloc((n-1)*n*m*sizeof(int));
  
  int * v = (int *) malloc(n*sizeof(int));
  for (int i = 0; i < n - 1; i++)
    v[i] = i + 1;
  
  for (int i = 0; i < n ; i++) {
    int * A2 = (int *) malloc(n*m*sizeof(int));
    memcpy(A2,A,n*m*sizeof(int));
    this->vsplit(n,m,A2,n-1,v,NULL);
    memcpy(&B[(n-1)*m*i],A2,(n-1)*m*sizeof(int));
    if (i < n-1 )
      v[i]--;
    free(A2);
  }

  res = simplicialPolyhedron((n-1)-1,n*m,B);
  res.sortSimplexes(); // ordenem *sense coefficients* per evitar cancelacions
  res.simplifySimplexes();  // eliminem duplicats

  //free
  free(B);
  free(v);
  
  return res;

}



simplicialPolyhedron simplicialPolyhedron::operator[](int k) const {
  return simplicialPolyhedron(this->n-1,1,&(this->A[k*n]));
}

const simplicialPolyhedron& simplicialPolyhedron::binarySearch(const simplicialPolyhedron& P, int * const v, int * a, int * b) const {

  if (this->n == 0 || this->dim() != P.dim() )
    return *this;
  
  if ( a == NULL || b == NULL) {
    a = (int *) malloc(P.length()*sizeof(int));
    for (int i = 0; i < P.length(); i++)
      a[i] = 0;
    b = (int *) malloc(P.length()*sizeof(int));
    for (int i = 0; i < P.length(); i++)
      b[i] = this->m;
    this->binarySearch(P,v,a,b);
    //free
    free(a);
    free(b);
    return *this;
  }
  
  bool finish = true;
  for (int i = 0; i < P.length() && finish ; i++)
    finish = finish && a[i] >= b[i];
  if (finish)
    return *this;
  
  const int * A = P.values();
  int * c = (int *) malloc(P.length()*sizeof(int));
  for (int i = 0; i < P.length(); i++)
    c[i] = (a[i]+b[i])/2;

  for (int i = 0; i < P.length(); i++) {
	  const int * v1 = &(this->A[this->n*c[i]]);
	  const int * v2 = &(A[this->n*i]);
    if (a[i] >= b[i]) {
      // vam trobar la solucio en una iteracio
      // anterior. No fem res
    } else if ( Tools::eq(this->n,v1,v2) ) {
  	  // solucio trobada
  	  v[i] = c[i];
  	  a[i] = c[i];
  	  b[i] = c[i];
  	} else if (a[i] == c[i]) {
  	  // solucio no existent
  	  v[i] = -1;
  	  a[i] = c[i];
  	  b[i] = c[i];	  
  	} else if ( Tools::leq(this->n,v1,v2) ) {
  	  //ens restringim a la dreta
  	  a[i] = c[i];
  	} else {
  	  //ens restringim a l'esquerra
  	  b[i] = c[i];
  	}
  
  }

  //free
  free(c);

  return this->binarySearch(P,v,a,b);
}

void simplicialPolyhedron::subSearch(const simplicialPolyhedron & P, int * start, int * end) const {

  int s = 0;

  for (int i = 0; i < P.length(); i++) {
    int e = this->m;
    this->search(this->n,this->m,this->A,P.dim()+1,&P.values()[(P.dim()+1)*i],s,e);
    start[i] = s;
    end[i] = e;
    s = e;
  }

}

simplicialPolyhedron& simplicialPolyhedron::remove(int n, int * I) {
  
  this->bubbleSort(n,I);
  
  int numRemoved = 0;
  
  for (int i = 0, k = 0; i < this->m && k < n; )
    if (I[k] == i) {
      for (int j = this->n*(i+1); j < this->n*this->m; j++)
        this->A[j-this->n] = this->A[j];
      numRemoved++;
      k++;
      i++;
    } else if ( I[k] > i) {
      i++;
    } else if (I[k] < i) {
      k++;
    }
    
    this->A = (int *) realloc(this->A,this->n*(this->m - numRemoved)*sizeof(int));
    this->m -= numRemoved;
  
  return *this;
}

simplicialPolyhedron& simplicialPolyhedron::remove(int i) {
  return this->remove(1,&i);
}

simplicialPolyhedron& simplicialPolyhedron::times(const simplicialPolyhedron& P, const simplicialPolyhedron& Q, int * signs, int * A, int level, int n, int m, bool * path, int M, int N,int C, int  * cnt) {

  if (level < 0) {
    // preparing for recursion
    
    int C = (int) this->choose(P.dim(),Q.dim()+P.dim());
    A = (int *) malloc((P.dim()+Q.dim()+1)*P.length()*Q.length()*C*sizeof(int));
    bool * path = (bool *) malloc((P.dim()+Q.dim()+1)*sizeof(bool));
    M =  this->vectorMax((P.dim()+1)*P.length(),P.A)+1;
    N =  this->vectorMax((Q.dim()+1)*Q.length(),Q.A)+1;
    int cnt_value = 0;
    cnt = &cnt_value;
    // signs
    simplicialPolyhedron P2 = P, Q2 = Q;
    int * P_signs = (int *) malloc(P.length()*sizeof(int));
    int * Q_signs = (int *) malloc(Q.length()*sizeof(int));
    for (int i = 0; i < P.length(); i++)
      P_signs[i] = 1;
    for (int i = 0; i < Q.length(); i++)
      Q_signs[i] = 1;
    P2.sortSimplexes(P_signs);
    Q2.sortSimplexes(Q_signs);
    // start recursion
    this->times(P2,Q2,signs, A,0,P.dim(),Q.dim(),path,M, N,C,cnt);
    // end recursion
    // arrange signs
    for (int i = 0; i < C; i++)
      for (int j = 0; j < P.length(); j++)
        for (int k = 0; k < Q.length(); k++)
          signs[i*P.length()*Q.length() + j*Q.length() + k] *= P_signs[j]*Q_signs[k];
    // finish
    *this = simplicialPolyhedron(P.dim()+Q.dim(),P.length()*Q.length()*C,A);
    return *this;
  }

  if (n == 0 && m == 0) {
    // we have a path, so we apply algorimth
    
    // calculate sign, if it procceds
    if (signs != NULL) {
      int sign_index = (*cnt)*(P.length()*Q.length());
      int sign = 1;
      int true_cnt = 0;
      for (int l = 0 ; l <P.dim()+Q.dim() ; l++)
        if (path[l]) {
          if ( true_cnt%2 == 1)
            sign *= -1;
            
        } else
          true_cnt++;
      
      for (int i = 0; i < P.length()*Q.length(); i++)
        signs[sign_index + i] = sign;
    }

    // calcule simplexes
    int A_index = (*cnt)*(P.length()*Q.length())*(P.dim()+Q.dim()+1);    
    
    for (int i = 0; i < P.length(); i++)
      for (int j = 0; j < Q.length(); j++) {

        int position = (i*Q.length() + j)*(P.dim()+Q.dim()+1);

        A[A_index + position] = Q.A[j*(Q.dim()+1)]*M + P.A[i*(P.dim()+1)];

        for (int r = 0, c = 0; r+c < P.dim()+Q.dim();) {
          if ( path[r+c] ) {
            r++;
          } else {
            c++;
          }
          A[A_index + position + r + c] = Q.A[j*(Q.dim()+1)+c]*M + P.A[i*(P.dim()+1)+r];
        }
      }

    (*cnt)++;

  }
  
  if (n > 0) {
    // building path, we continue
    path[level] = true;
    this->times(P,Q,signs,A,level+1,n-1,m,path,M,N,C,cnt);
  }

  if (m > 0) {
    // building path, we continue
    path[level] = false;
    this->times(P,Q,signs,A,level+1,n,m-1,path,M,N,C,cnt);
  }
  
  return *this;
}

simplicialPolyhedron simplicialPolyhedron::operator*(const simplicialPolyhedron & P) const {
  simplicialPolyhedron Q;
  int * signs = (int *) malloc (this->choose(this->dim(),this->dim()+P.dim())*this->length()*P.length()*sizeof(int));
  Q.times(*this,P,signs);
  Q.orientSimplexes(signs);
  return Q;
}

simplicialPolyhedron& simplicialPolyhedron::cone() {
  
  int * A = (int *) malloc((this->dim()+2)*this->length()*sizeof(int));
  int * B = (int *) malloc(this->length()*sizeof(int));
  int p = this->vectorMax((this->dim()+1)*this->length(),this->A)+1;
  for (int i = 0; i < this->length(); i++)
    B[i] = p;
  
  this->vcat(this->dim()+1,1,this->length(),A,this->A,B);
  *this = simplicialPolyhedron(this->dim()+1,this->length(),A);
  
  return *this;
}
simplicialPolyhedron& simplicialPolyhedron::suspension() {

  int * A = (int *) malloc((this->dim()+2)*2*this->length()*sizeof(int));
  int * B = (int *) malloc(this->length()*sizeof(int));
  
  int p = this->vectorMax((this->dim()+1)*this->length(),this->A)+1;
  for (int i = 0; i < this->length(); i++)
    B[i] = p;
  this->vcat(this->dim()+1,1,this->length(),A,this->A,B);

  int q = p+1;
  for (int i = 0; i < this->length(); i++)
    B[i] = q;
  this->vcat(this->dim()+1,1,this->length(),&A[(this->dim()+2)*this->length()],this->A,B);

  for (int i = 0; i < this->length(); i++) {
    // swap 
    int a = A[(this->dim()+2)*this->length() + i*(this->dim()+2) + this->dim() + 1];
    A[(this->dim()+2)*this->length() + i*(this->dim()+2) + this->dim() + 1] = A[(this->dim()+2)*this->length() + i*(this->dim()+2) + this->dim()];
    A[(this->dim()+2)*this->length() + i*(this->dim()+2) + this->dim() ] = a;
  }
  
  *this = simplicialPolyhedron(this->dim()+1,2*this->length(),A);
  
  return *this;
}


simplicialPolyhedron& simplicialPolyhedron::operator <<(const simplicialPolyhedron& P) {
  
  if (this->dim() != P.dim())
    return *this;
  
  int M = this->vectorMax((this->dim()+1)*this->length(),this->A)+1;
  // obtaning last simplex of this
  simplicialPolyhedron B = this->operator[](this->length()-1);
  int a = B.A[this->dim() - 1];
  B.A[this->dim() - 1] = B.A[this->dim()];
  B.A[this->dim()] = a;
  // reallocating
  this->A = (int *) realloc(this->A, (this->dim()+1)*(this->length() + P.length() -2)*sizeof(int));
  
  // setting values
  for (int i = P.dim()+1; i < (P.dim()+1)*P.length(); i++) {
    int j = 0;
    while (j < P.dim()+1 && P.A[j] != P.A[i])
      j++;
    if ( j < P.dim()+1)
      this->A[(this->dim() + 1)*(this->length() - 2) + i] = B.A[j];
    else
      this->A[(this->dim() + 1)*(this->length() - 2) + i] = P.A[i] + M;
  }
  
  // upload variables
  this->m += P.length()-2; 
  
  return *this;
}
