#pragma once
#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>
//#include <math.h>
#include <chrono> 
#include <fstream>
#include <algorithm>
#include <sndfile.hh> // Wav in and out
#include "mkl_dfti.h" // Intel MKL
#include <tuple>
#include <boost/math/interpolators/cardinal_cubic_b_spline.hpp>
#include <filesystem> // to list all files in a directory

//#define DEBUG // for verbose

typedef unsigned long long pint;
typedef long long          inte;
typedef double             real;

typedef std::vector<pint> v_pint;
typedef std::vector<inte> v_inte;
typedef std::vector<real> v_real;

const real PI = 3.14159265358979323846;

extern "C" {
	void SURFIT(int* iopt, int* m, double* x, double* y, double* z, double* w, double* xb, double* xe, double* yb, double* ye, int* kx, int* ky,
		double* s, int* nxest, int* nyest, int* nmax, double* eps, int* nx, double* tx, int* ny, double* ty, double* c, double* fp,
		double* wrk1, int* lwrk1, double* wrk2, int* lwrk2, int* iwrk, int* kwrk, int* ier);
	void BISPEV(double* tx, int* nx, double* ty, int* ny, double* c, int* kx, int* ky, double* x, int* mx, double* y, int* my, double* z,
		double* wrk, int* lwrk, int* iwrk, int* kwrk, int* ier);
	void PARDER(double* tx, int* nx, double* ty, int* ny, double* c, int* kx, int* ky, int* nux, int* nuy, double* x, int* mx, double* y, int* my,
		double* z, double* wrk, int* lwrk, int* iwrk, int* kwrk, int* ier);
}

std::vector<double> Fit_Bspline_Surface(int m, double* x, double* y, double* z, int nx, int ny, int kx, int ky, double* tx, double* ty) {
	#ifdef DEBUG
		std::cout << "Fit_Bspline_Surface 01\n";
	#endif // DEBUG

	int nxest = nx; // max number of knots in the x axis
	int nyest = ny; // max number of knots in the y axis
	int nmax = std::max(nxest, nyest); // max number of knots

	int iopt = -1;                                     // Compute a smoothing spline
	 
	double* w = new double[m]; // Weight
	std::fill(w, w + m, 1.0);

	double smoothing = 0.0;

	int lc = (nxest - kx - 1) * (nyest - ky - 1);
	//std::vector<double> c(lc); // Coefficients
	double* c = new double[lc];

	double fp = 0.0; // Weighted sum of squared residuals

	// Allocate working memory required by surfit
	auto u = nxest - kx - 1;
	auto v = nyest - ky - 1;
	auto km = std::max(kx, ky) + 1;
	auto ne = std::max(nxest, nyest);
	auto bx = kx * v + ky + 1;
	auto by = ky * u + kx + 1;
	int b1, b2;
	if (bx <= by) {
		b1 = bx;
		b2 = b1 + v - ky;
	} else {
		b1 = by;
		b2 = b1 + u - kx;
	}

	int lwrk1 = u * v * (2 + b1 + b2) + 2 * (u + v + km * (m + ne) + ne - kx - ky) + b2 + 1;
	double* wrk1 = nullptr;
	try {
		//delete[] wrk1;
		wrk1 = new double[lwrk1];
	}
	catch (const std::exception e) {
		std::cout << "ERROR: Couldn't create array of double with size lwrk1=" << lwrk1 << " - ";
		std::cout << e.what() << "\n";
		exit(EXIT_FAILURE);
	}

	int lwrk2 = u * v * (b2 + 1) + b2;
	double* wrk2 = nullptr;
	try {
		//delete[] wrk2;
		wrk2 = new double[lwrk2];
	}
	catch (const std::exception e) {
		std::cout << "ERROR: Couldn't create array of double with size lwrk2=" << lwrk2 << " - ";
		std::cout << e.what() << "\n";
		exit(EXIT_FAILURE);
	}


	int kwrk = m + (nxest - 2 * kx - 1) * (nyest - 2 * ky - 1);	
	int* iwrk = nullptr;
	try {
		//delete[] iwrk;
		iwrk = new int[kwrk];
	}
	catch (const std::exception e) {
		std::cout << "ERROR: Couldn't create array of int with size kwrk=" << kwrk << " - ";
		std::cout << e.what() << "\n";
		exit(EXIT_FAILURE);
	}

	double eps = std::numeric_limits<double>::epsilon();

	int ier = 0;
	auto xb = 0.0;
	auto xe = 1.0;
	auto yb = 0.0;
	auto ye = 1.0;
	//std::cout << "yb=" << yb << " ye=" << ye << "\n";
	//std::cout << m << " " << nx << " " << ny << " " << nxest << " " << nyest << "\n";
	SURFIT(&iopt, &m, x, y, z, w, &xb, &xe, &yb, &ye, &kx, &ky, &smoothing, &nxest, &nyest, &nmax, &eps, &nx, tx, &ny, ty, c, &fp, wrk1, &lwrk1, wrk2, &lwrk2, iwrk, &kwrk, &ier);
	#ifdef DEBUG
		std::cout << "Fit_Bspline_Surface 02\n";
	#endif // DEBUG
	if (ier > 0) {
		if (ier >= 10) {
			std::stringstream s;
			s << "Error fitting B-Spline surface using surfit(): " << ier;
			std::cout << "Error fitting B-Spline surface using surfit(): " << ier << "\n";
			throw std::runtime_error(s.str());
		}
		else {
			std::cerr << "WARNING:  Non-fatal error while fitting B-Spline surface using surfit(): " << ier << std::endl;
		}
	}

	// De-allocate temporary memory
	delete[] w;
	delete[] wrk1;
	delete[] wrk2;
	delete[] iwrk;

	std::vector<double> cc(c, c + lc);
	return cc;
}

std::vector<double> Eval_Bspline_Surface(double *c, int nx, int ny, int kx, int ky, int mx, int my, double* tx, double* ty) {

	std::vector<double> xx(mx);
	for (size_t i = 0; i < mx; i++) {
		xx[i] = double(i) / double(mx);
		//std::cout << xx[i] << "\n";
	}

	std::vector<double> yy(my);
	for (size_t i = 0; i < my; i++) {
		yy[i] = double(i) / double(my);
	}

	std::vector<double> zz(mx * my);
	int lwrk = mx * (kx + 1) + my * (ky + 1);
	double* wrk = new double[lwrk];

	int kwrk = mx + my;
	int* iwrk = new int[kwrk];

	int ier = 0;
	BISPEV(tx, &nx, ty, &ny, c, &kx, &ky, &xx[0], &mx, &yy[0], &my, &zz[0], wrk, &lwrk, iwrk, &kwrk, &ier);
	if (ier > 0) {
		std::stringstream s;
		s << "Error evaluating B-Spline surface using bispev(): " << ier;
		std::cout << "Error evaluating B-Spline surface using surfit(): " << ier << "\n";

		throw std::runtime_error(s.str());
	}
	delete[] wrk;
	delete[] iwrk;
	return zz;
}

std::vector<double> Eval_Bspline_Surface(double* c, int nx, int ny, int kx, int ky, double x, int my, double* tx, double* ty) {

	std::vector<double> xx = { x };
	int mx = 1;

	std::vector<double> yy(my);
	for (size_t i = 0; i < my; i++) {
		yy[i] = double(i) / double(my);
	}

	std::vector<double> zz(mx * my);
	int lwrk = mx * (kx + 1) + my * (ky + 1);
	double* wrk = new double[lwrk];

	int kwrk = mx + my;
	int* iwrk = new int[kwrk];

	int ier = 0;
	BISPEV(tx, &nx, ty, &ny, c, &kx, &ky, &xx[0], &mx, &yy[0], &my, &zz[0], wrk, &lwrk, iwrk, &kwrk, &ier);
	if (ier > 0) {
		std::stringstream s;
		s << "Error evaluating B-Spline surface using bispev(): " << ier;
		std::cout << "Error evaluating B-Spline surface using surfit(): " << ier << "\n";

		throw std::runtime_error(s.str());
	}
	delete[] wrk;
	delete[] iwrk;
	return zz;
}

//double BSplineSurfaceeval(double x, double y)
//{
//	// Allocate working memory on the stack to keep this function thread-safe
//	double* wrk = (double*)alloca(sizeof(double) * lwrk);
//	std::fill(wrk, wrk + lwrk, 0.0);
//
//	double z = 0.0;
//	int m = 1; // Evaluate a single point
//	int kwrk = 2;
//	int iwrk[2];
//	std::fill(iwrk, iwrk + kwrk, 0);
//
//	// bispev clamps x and y to the available ranges.
//	int ier = 0;
//	BISPEV(tx, &nx, ty, &ny, c, &k, &k, &x, &m, &y, &m, &z, wrk, &lwrk, iwrk, &kwrk, &ier);
//	if (ier > 0) {
//		std::stringstream s;
//		s << "Error evaluating B-Spline surface using bispev() at point (" << x << ", " << y << "): " << ier;
//		throw std::runtime_error(s.str());
//	}
//
//	return z;
//}
template <typename T> 
void write_vector(const std::vector<T>& V, std::string path = "teste.csv") {
	std::ofstream out(path);
	for (pint i = 0; i < V.size() - 1; ++i) {

		out << V[i] << ",";
	}
	out << V.back();
	out.close();
	#ifdef DEBUG
		std::cout << "wrote " << path << "\n";
	#endif // DEBUG

}

static bool abs_compare(real a, real b) {
	return (std::abs(a) < std::abs(b));
}

class Chronograph {
private:
	std::chrono::time_point< std::chrono::steady_clock> start, end;
	double duration{ 0.0 };
public:
	Chronograph() {
		start = std::chrono::high_resolution_clock::now();
	}
	double stop(std::string message = "Time = ") {
		end = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		if (duration < 1000) {
			std::cout << message << duration << " milliseconds\n";
			return duration;
		}
		if (duration < 60000) {
			duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
			std::cout << message << duration << " seconds\n";
			return duration;
		}
		duration = std::chrono::duration_cast<std::chrono::minutes>(end - start).count();
		std::cout << message << duration << " minutes\n";
		return duration;
	}
};

class Wav {
public:
	pint fps;
	v_real W;

	Wav(v_real W_, pint fps_=44100) {
		fps = fps_;
		W = W_;
		//std::cout << "New wave with n=" << W.size() << " and fps=" << fps << " \n";
	}
	void write(std::string path = "test.wav") {
		if (W.size() == 0) {
			std::cout << "size = 0";
			return;
		}
		const char* fname = path.c_str();
		SF_INFO sfinfo;
		sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
		sfinfo.channels = 1;
		sfinfo.samplerate = fps;

		SNDFILE* outfile = sf_open(fname, SFM_WRITE, &sfinfo);
		sf_write_double(outfile, &W[0], W.size());
		sf_close(outfile);
		std::cout << "Wav file written at " << path << ", fps=" << fps << "\n";
	}
};

Wav read_wav(std::string path) {
	const char* fname = path.c_str();
	SF_INFO sfinfo;
	sfinfo.format = 0;
	SNDFILE* test = sf_open(fname, SFM_READ, &sfinfo);

	if (test == NULL) {
		std::cout << "Couldn't open file at:" << fname;
		std::cout << "\n" << sf_strerror(test);
		sf_close(test);
		exit(1);
	}
	sf_close(test);

	SndfileHandle file = SndfileHandle(fname);

	if (file.channels() != 1) {
		std::cout << "Unexpected number of channels:" << file.channels();
		exit(1);
	}

	const int fps = file.samplerate();
	const int n = file.frames();
	std::cout << "Successfully opened file at:" << path << "\n";

	v_real W(n);
	file.read(&W[0], n);
	return Wav(W, fps);
};

inte argabsmax(const v_real& V, const inte x0, const inte x1) {
	real max{ std::abs(V[x0]) };
	inte idx = x0;
	for (pint i = x0; i < x1; i++) {
		if (std::abs(V[i]) > max) {
			max = std::abs(V[i]);
			idx = i;
		}
	}
	return idx;
}

inte argmax(const v_real& V, const inte x0, const inte x1) {
	real max{ std::abs(V[x0]) };
	inte idx = x0;
	for (pint i = x0; i <= x1; i++) {
		if (V[i] > max) {
			max = V[i];
			idx = i;
		}
	}
	return idx;
}

inte argmin(const v_real& V, const inte x0, const inte x1) {
	real min{ std::abs(V[x0]) };
	inte idx = x0;
	for (pint i = x0; i <= x1; i++) {
		if (V[i] < min) {
			min = V[i];
			idx = i;
		}
	}
	return idx;
}

real error(const v_real& W1, const v_real& W2) {

	real err{ 0.0 };
	inte n = std::min(W1.size(), W2.size());
	for (pint i = 0; i < n; i++) {
		err += std::abs(W1[i] - W2[i]);
	}
	return err / n;
}

std::vector<float> rfft(const v_real& cin) {
	std::vector<float> in(cin.begin(), cin.end());
	//std::cout << "rfft";
	//auto tp = Chronograph();
	int n = (in.size() + 1) / 2;
	std::vector<std::complex<float>> out(in.size());

	DFTI_DESCRIPTOR_HANDLE descriptor;
	MKL_LONG status;
	// FFT 
	status = DftiCreateDescriptor(&descriptor, DFTI_SINGLE, DFTI_REAL, 1, in.size());//Specify size and precision
	status = DftiSetValue(descriptor, DFTI_PLACEMENT, DFTI_NOT_INPLACE);             //Out of place FFT
	status = DftiCommitDescriptor(descriptor);                                       //Finalize the descriptor
	status = DftiComputeForward(descriptor, in.data(), out.data());                  //Compute the Forward FFT
	status = DftiFreeDescriptor(&descriptor);                                        //Free the descriptor

	//for (size_t i = 0; i < n; i++) {
	//	out[i] = out[i] / double(n);
	//}

	float val = 0.0;
	float max_val = 0.0;
	float f = 0.0;
	float p = 0.0;
	float a = 0.0;
	for (size_t i = 0; i < n; ++i) {
		out[i] = out[i] / (float)n;
		val = std::pow(out[i].real(), 2.0) + std::pow(out[i].imag(), 2.0);
		if (val > max_val) {
			max_val = val;
			f = (float)i;
			p = std::arg(out[i]);
			a = std::abs(out[i]);
		}
	}

	//tp.stop("FFT Time: ");
	//std::cout << "f=" << f << " p=" << p << " a=" << a <<"\n";
	//std::cout << "------------";
	//auto ff = (double)f;
	std::vector<float> result { f,p,a };

	//std::cout << "f=" << result[0] << " p=" << result[1] << " a=" << result[2] << "\n";

	return result;
}

std::vector<float> rfft(v_real::const_iterator beg, v_real::const_iterator end) {
	std::vector<float> in(beg, end);
	//std::cout << "rfft";
	//auto tp = Chronograph();
	int n = (in.size() + 1) / 2;
	std::vector<std::complex<float>> out(in.size());

	DFTI_DESCRIPTOR_HANDLE descriptor;
	MKL_LONG status;
	// FFT 
	status = DftiCreateDescriptor(&descriptor, DFTI_SINGLE, DFTI_REAL, 1, in.size());//Specify size and precision
	status = DftiSetValue(descriptor, DFTI_PLACEMENT, DFTI_NOT_INPLACE);             //Out of place FFT
	status = DftiCommitDescriptor(descriptor);                                       //Finalize the descriptor
	status = DftiComputeForward(descriptor, in.data(), out.data());                  //Compute the Forward FFT
	status = DftiFreeDescriptor(&descriptor);                                        //Free the descriptor

	for (size_t i = 0; i < n; i++) {
		out[i] = { out[i].real() / float(in.size()), out[i].imag() / float(in.size()) };
	}

	float val = 0.0;
	float max_val = 0.0;
	float f = 0.0;
	float p = 0.0;
	float a = 0.0;
	for (size_t i = 0; i < n; ++i) {
		out[i] = out[i] / (float)n;
		val = std::pow(out[i].real(), 2.0) + std::pow(out[i].imag(), 2.0);
		if (val > max_val) {
			max_val = val;
			f = (float)i;
			p = std::arg(out[i]);
			//p = std::acos(std::cos(std::arg(out[i]) - PI / 2));
			a = std::abs(out[i]);
		}
	}

	//tp.stop("FFT Time: ");
	//std::cout << "f=" << f << " p=" << p << " a=" << a <<"\n";
	//std::cout << "------------";
	//auto ff = (double)f;
	std::vector<float> result{ f,p,a };

	//std::cout << "f=" << result[0] << " p=" << result[1] << " a=" << result[2] << "\n";

	return result;
}

template <typename I, typename R>
std::tuple<real, real> get_slope_and_intercept(I x0, I x1, R y0, R y1) {
	// y = a x + b
	// y1 = a x1 + b -> y1 - y0 = a (x1 - x0) -> a = (y1 - y0) / (x1 - x0)
	real a = (real(y1) - real(y0)) / (real(x1) - real(x0));
	real b = real(y1) - a * real(x1);
	return{ a,b };
}

template <typename T>
std::tuple<real, real> fit(const std::vector<T>& V, pint x0, pint x1) {
	real sum_x = { 0.0 };
	real sum_y = { 0.0 };
	real sum_xx = { 0.0 };
	real sum_xy = { 0.0 };
	for (size_t i = x0; i < x1; i++) {
		sum_x += real(i);
		sum_y += real(V[i]);
		sum_xx += real(i * i);
		sum_xy += real(i) * real(V[i]);
	}
	real n = real(x1 - x0);
	real x_mean = sum_x / n;
	real y_mean = sum_y / n;
	real a = (sum_xy - sum_x * y_mean) / (sum_xx - sum_x * x_mean);
	real b = y_mean - a * x_mean;
	return{ a,b };
}

template <typename T>
pint split(const std::vector<T>& V, pint x0, pint x1) {
	real sum_y = { 0.0 };
	for (size_t i = x0; i < x1; i++) {
		sum_y += std::abs(real(V[i]));
	}
	real partial_sum_y = { 0.0 };
	while (partial_sum_y < sum_y / 2) {
		partial_sum_y += std::abs(real(V[x0]));
		x0++;
	}
	return x0;
}

template <typename T>
real deviation(const std::vector<T>& V, pint x0, pint x1, real a, real b) {
	real d{ 0.0 };
	for (size_t i = x0; i < x1; i++) {
		d += pow((a * real(i) + b) - real(V[i]), 2.0);
	}
	return d / real(x1 - x0 - 1);
}

template <typename T>
v_pint segment(const std::vector<T>& V, pint iter_limit = 1000) {

	auto [a, b] = fit(V, 0, V.size());
	real d0 = deviation(V, 0, V.size(), a, b);
#ifdef DEBUG
	std::cout << "\nsegmenting: d0=" << d0 << "\n";
#endif // DEBUG

	std::vector<std::pair<pint, pint>> segments = { std::make_pair(0, V.size()) };
	std::vector<std::pair<pint, pint>> segments_fin = {};
	std::vector<std::pair<pint, pint>> segments_buf = {};

	pint ctr{ 0 };
	while (segments.size() > 0) {
		ctr++;
		for (size_t i = 0; i < segments.size(); i++) {
			pint x0 = segments[i].first;
			pint x1 = segments[i].second;
			pint x = split(V, x0, x1);

			std::tie(a, b) = fit(V, x0, x);
			real d = deviation(V, x0, x, a, b);

#ifdef DEBUG
			std::cout << "x0=" << x0 << " x=" << x << " d=" << d << "\n";
#endif // DEBUG

			if (d < 0.1 * d0 || x - x0 < 5) {
				segments_fin.push_back(std::make_pair(x0, x));
			}
			else {
				segments_buf.push_back(std::make_pair(x0, x));
			}

			std::tie(a, b) = fit(V, x, x1);
			d = deviation(V, x, x1, a, b);

#ifdef DEBUG
			std::cout << "x=" << x << " x1=" << x1 << " d=" << d << "\n";
#endif // DEBUG

			if (d < 0.1 * d0 || x1 - x < 5) {
				segments_fin.push_back(std::make_pair(x, x1));
			}
			else {
				segments_buf.push_back(std::make_pair(x, x1));
			}
		}
		segments = std::move(segments_buf);
	}

	v_pint X;
	for (auto p : segments_fin) {
		X.push_back(p.first);
		X.push_back(p.second);
	}
	std::sort(X.begin(), X.end());
	auto it = std::unique(X.begin(), X.end());
	X.resize(std::distance(X.begin(), it));

	//std::cout << "\n";// todo
	for (auto x : X) {
		std::cout << "x=" << x << "\n";
	}

	return X;
}


real orthogonal_distance(real x, real y, real a, real b) {
	return std::abs(b + a * x - y) / sqrt(1.0 + pow(a, 2.0));
}

typedef std::pair<std::tuple<real, real>, std::tuple<real, real>> pair_of_tuples;
template <typename T>
pair_of_tuples extend(const std::vector<T>& V) {
	auto [al, bl] = get_slope_and_intercept(0, 2, V[0], V[2]);
	real average_distance = orthogonal_distance(real(1), V[1], al, bl);

	real new_average_distance{ 0.0 };

	pint x{ 3 };
	while (x < V.size()) {
		auto [a, b] = get_slope_and_intercept(pint(0), x, V[0], V[x]);
		new_average_distance = 0.0;
		for (size_t i = 1; i < x; i++)	{
			new_average_distance += orthogonal_distance(real(x), V[x], a, b);
		}
		new_average_distance /= real(x - 1);
		if (new_average_distance < average_distance) {
			average_distance = new_average_distance;
			al = a;
			bl = b;
		}
		else {
			break;
		}
		x++;
	}


	pint n = V.size();
	auto [ar, br] = get_slope_and_intercept(n - 3, n - 1, V[n - 3], V[n - 1]);
	average_distance = orthogonal_distance(real(n-2), V[n-2], ar, br);
	x = n - 4;
	while (x > 0) {
		auto [a, b] = get_slope_and_intercept(x, n - 1, V[x], V[n - 1]);
		new_average_distance = 0.0;
		for (size_t i = n-1; i > x; i--) {
			new_average_distance += orthogonal_distance(real(x), V[x], a, b);
		}
		new_average_distance /= real(x - 1);
		if (new_average_distance < average_distance) {
			average_distance = new_average_distance;
			ar = a;
			br = b;
		}
		else {
			break;
		}
		x--;
	}
	return{ { al,bl }, { ar,br } };
}


template <typename T>
real extrapolate(const std::vector<T>& W) {
	real a, b;
	pint n =  W.size();
	real value{ 0.0 };
	real weight;
	real total_weight{ 0.0 };
	
	for (size_t i = 0; i < n-1; i++) {
		std::tie(a, b) = get_slope_and_intercept(i, i + 1, W[i], W[i + 1]);
		weight = pow(i + 1, 2);
		value += (a * n + b) * weight;
		total_weight += weight;
	}

	return value /= total_weight;
}

















