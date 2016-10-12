//
// Created by hokop on 8/2/16.
//

#include <cmath>
#include "Convolution.hpp"

Convolution::Convolution(int sK, int sS, string sName) : K(sK), S(sS), Layer(sName) {}

Convolution &Convolution::operator=(Convolution const &convolution) {
	Layer::operator=(convolution);
	K = convolution.K;
	N = convolution.N;
	weight = convolution.weight;
	grad = convolution.grad;
	bias = convolution.bias;
	biasGrad = convolution.biasGrad;
	preaxon = convolution.preaxon;
	K = convolution.K;
	S = convolution.S;
	N = convolution.N;
	M = convolution.M;
	os = convolution.os;
	is = convolution.is;
	return *this;
}

void Convolution::setData(Data sDendrite, Data sAxon, Data sEDendrite, Data sEAxon) {
	Layer::setData(sDendrite, sAxon, sEDendrite, sEAxon);
	preaxon = axon;
	bias = axon;
	biasGrad = axon;
	
	preaxon.initMem();
	bias.initMem();
	biasGrad.initMem();
	
	for(int i = 0; i < bias.N; ++i)
		for (int j = 0; j < bias.M; ++j)
			for (int k = 0; k < bias.M; ++k)
				bias.at(i, j, k) = ( 2 * ( (flt) rand() / (flt) RAND_MAX ) - 1) / (flt) sqrt(K * K * M + 1);
	
	N = dendrite.N;
	M = axon.N;
	
	is = dendrite.M;
	os = axon.M;
	
	weight = (flt****) malloc( sizeof(flt***) * N );
	grad = (flt****) malloc( sizeof(flt***) * N );
	for(int i = 0; i < N; ++i){
		weight[i] = (flt***) malloc( sizeof(flt**) * M);
		grad[i] = (flt***) malloc( sizeof(flt**) * M);
		for(int j = 0; j < M; ++j){
			weight[i][j] = (flt**) malloc( sizeof(flt*) * K );
			grad[i][j] = (flt**) malloc( sizeof(flt*) * K );
			for (int k = 0; k < K; ++k) {
				weight[i][j][k] = (flt*) malloc( sizeof(flt) * K );
				grad[i][j][k] = (flt*) malloc( sizeof(flt) * K );
				for (int l = 0; l < K; ++l) {
					weight[i][j][k][l] = ( 2 * ( (flt) rand() / (flt) RAND_MAX ) - 1) / (flt) sqrt(K * K * M + 1); // initial weights
				}
			}
		}
	}
}

bool Convolution::check() {
	if(os != axon.M || is != dendrite.M)
		throw "size error";
	if(N != axon.N || M != dendrite.N)
		throw "formats error";
	if(is != (os - (K - S)) / S || (os - (K - S)) % S != 0)
		throw "sizes of maps do not match";
	return true;
}

vector<flt*> Convolution::stretchArray(flt ****a) {
	vector<flt*> v;
	for(int i = 0; i < N; ++i)
		for (int j = 0; j < M; ++j)
			for (int k = 0; k < K; ++k)
				for (int l = 0; l < K; ++l)
					v.push_back(&a[i][j][k][l]);
	return v;
}
vector<flt*> stretchData(Data data){
	vector<flt*> v;
	for (int i = 0; i < data.N; ++i)
		for (int j = 0; j < data.M; ++j)
			for (int k = 0; k < data.M; ++k)
				v.push_back(&data.at(i, j, k));
	return v;
}

vector<flt*> Convolution::getWeights() {
	vector<flt*> a = stretchArray(weight);
	vector<flt*> b = stretchData(bias);
	a.insert(a.end(), b.begin(), b.end());
	return a;
}

vector<flt*> Convolution::getGrads() {
	vector<flt*> a = stretchArray(grad);
	vector<flt*> b = stretchData(biasGrad);
	a.insert(a.end(), b.begin(), b.end());
	return a;
}

flt f(flt x){
	if( x < -10.f )
		return -1.f;
	flt ex = exp(-x);
	return (1.0f - ex) / (1.0f + ex);
}

flt df(flt x){
	if( x < -10.f )
		return 0.f;
	flt ex = exp(-x);
	flt s = 1.f + ex;
	return 2.f * ex / (s*s);
}

void Convolution::compute() {
	for(int row = 0; row < os; ++row)
		for (int col = 0; col < os; ++col)
			for (int neuron = 0; neuron < M; ++neuron)
				preaxon.at(neuron, row, col) = bias.at(neuron, row, col);
	// these cycles can be merged
	for(int row = 0; row < os; ++row)
		for (int col = 0; col < os; ++col) {
			int rS = row * S;
			int cS = col * S;
			for (int map = 0; map < N; ++map)
				for (int neuron = 0; neuron < M; ++neuron)
					for (int i = 0; i < K; ++i)
						for (int j = 0; j < K; ++j)
							preaxon.at(neuron, row, col) +=
									weight[map][neuron][i][j]
									* dendrite.at(map, rS + i, cS + j);
		}
	for(int row = 0; row < os; ++row)
		for (int col = 0; col < os; ++col)
			for (int neuron = 0; neuron < M; ++neuron)
				axon.at(neuron, row, col) = f(preaxon.at(neuron, row, col));
}

void Convolution::proceedError() {
	for(int row = 0; row < os; ++row)
		for (int col = 0; col < os; ++col) {
			int rS = row * S;
			int cS = col * S;
			for (int map = 0; map < N; ++map) {
				for (int neuron = 0; neuron < M; ++neuron) {
					flt dfS = df(preaxon.at(neuron, row, col));
					flt eA = errAxon.at(neuron, row, col);
					biasGrad.at(neuron, row, col) = eA * dfS;
					for (int i = 0; i < K; ++i)
						for (int j = 0; j < K; ++j) {
							grad[map][neuron][i][j] +=
									eA * dfS * dendrite.at(map, rS + i, cS + j);
							errDend.at(map, rS + i, cS + j) +=
									eA * dfS * weight[map][neuron][i][j];
						}
				}
			}
		}
}