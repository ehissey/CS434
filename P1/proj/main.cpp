#include "v3.h"
#include "m33.h"

#include <iostream>

using namespace std;

void main() {

  V3 v0(1.0f, 1.0f, 2.0f);
  V3 v1(2.0f, 2.0f, 1.0f);
  V3 v2(6.0f, -1.0f, 10.0f);

  M33 m;

  cerr << "row 0: " << m[0] << endl;
  cerr << "row 1: " << m[1] << endl;
  cerr << "row 2: " << m[2] << endl;


}