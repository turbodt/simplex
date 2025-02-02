/*
 *  Class: simplicialPolyhedron
 *  Author: Daniel Torres
 *  Filename: simplicialPolyhedron.h
 * 
 * 
 * 
 *  Notes: 
 * 
 *
 */

#ifndef H_SIMPLICIALPOLYHEDRON
#define H_SIMPLICIALPOLYHEDRON

#include <iostream>
#include <cstdlib> 
#include <fstream>
#include <string.h>
#include "tools.h"
using namespace std;

class simplicialPolyhedron {

  private:
  
  int * A;
  int n;
  int m;

  private:

  int vectorMax(int n, int const * const A) const;
  int bubbleSort(int n, int * const v) const;
  bool search(int n1, int m, int * A, int n2, int const * const v, int & start, int & end) const;
  void vcat(int n1, int n2, int m, int * const C, int const * const A, int const * const B) const;
  void vsplit(int n, int m, int * A, int k, int const * const v, int * const B = NULL) const;
  long long int choose(int n, int m) const;
  
  public:
  
  simplicialPolyhedron();
  simplicialPolyhedron(const simplicialPolyhedron& A);
  simplicialPolyhedron(int n, int m, int const * const A);
  ~simplicialPolyhedron();
  simplicialPolyhedron& operator=(const simplicialPolyhedron& A);
  
  int length() const;
  int dim() const;
  const int * values() const;
  simplicialPolyhedron& read(istream & in);
  const simplicialPolyhedron& print(ostream & out) const;
  
  simplicialPolyhedron& orientSimplexes(int * signs);
  simplicialPolyhedron& sortSimplexes(int * signs = NULL);
  simplicialPolyhedron& simplifySimplexes(int * coeffs = NULL);
  simplicialPolyhedron boundary(int * coeffs2 = NULL, int * coeffs = NULL) const;
  simplicialPolyhedron skeleton() const;
  
  simplicialPolyhedron operator[](int k) const;
  const simplicialPolyhedron& binarySearch(const simplicialPolyhedron& P, int * const v, int * a = NULL, int * b = NULL) const;
  void subSearch(const simplicialPolyhedron & P, int * start, int * end) const;
  simplicialPolyhedron& remove(int n, int * simplexes);
  simplicialPolyhedron& remove(int i);
  simplicialPolyhedron& times(const simplicialPolyhedron& P, const simplicialPolyhedron& Q, int * signs = NULL, int *A = NULL , int level = -1, int n = -1, int m = -1, bool * path = NULL, int M = -1, int N = -1,int C = -1, int * cnt = NULL);

  simplicialPolyhedron operator*(const simplicialPolyhedron & P) const;
  simplicialPolyhedron& cone();
  simplicialPolyhedron& suspension();
  
  simplicialPolyhedron& operator<<(const simplicialPolyhedron& P);

  
};

#endif
	
