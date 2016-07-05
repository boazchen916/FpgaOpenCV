#ifndef NEURO
#define NEURO

#include<iostream>
#include<stdlib.h>
#include<cmath>

typedef float flt;

struct MLP
{
	MLP() {}
	MLP( int sz, int *lsz, flt ***w = nullptr );
	MLP( const MLP &mlp );
	~MLP();
	MLP &operator =( const MLP &mlp );
	
	void init( int sz, int *lsz, flt ***w = nullptr );
	void clear();
	
	// properties
	int *layersz;
	int layersN;
	
	// main memory
	flt ***weight; // [ layer, inputH, outputH ]
	
	// working variables
	flt **lreg; // [layer, inputH]
	
	// inside functions
	flt activationCutoff = -10.0;
	flt activation(flt x);
	flt activationDeriv(flt x); // производная нужна только при обучении
	
	// computing part
	void compLayer( float *data_in, float **weight_in, int N, int K, float *data_out );
	void predict( const flt *input, flt *output );
	
};








#endif //NEURO

