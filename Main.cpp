#include "Header.h"
#include <filesystem>
#include <boost/math/interpolators/cubic_hermite.hpp>
#include <unordered_map>

//#include "BSplineSurface.h"
//#include <random>
//#include <cmath>
//#include <cstdlib>
//#include <boost/math/interpolators/cubic_hermite.hpp>
//#include <Mathematics/BSplineSurfaceFit.h>

//#define TIME

const std::string HCEXT = ".hc";
const std::string PCEXT = ".pc";
std::string APPEND = "_reconstructed";

pint get_mode(v_pint& T) {
	std::sort(T.begin(), T.end());
	pint curr_value{ T[0] };
	pint curr_count = 0;
	pint max_count = 0;
	pint mode = 0;

	for (auto period: T){
		if (period == curr_value) {
			curr_count++;
		} else {
			if (curr_count > max_count) {
				max_count = curr_count;
				mode = curr_value;
			}
			curr_value = period;
			curr_count = 1;
		}
	}
	if (curr_count > max_count) {
		max_count = curr_count;
		mode = curr_value;
	}
	#ifdef DEBUG
	std::cout << "mode count=" << max_count << " of " << T.size() << "\n";
	#endif // DEBUG

	return mode;
}

template <typename T>
std::vector<T> flatten(const std::vector<std::vector<T>>& V) {
	std::vector<T> Vf;
	for (auto subV : V) {
		for (auto item : subV) {
			Vf.push_back(item);
		}
	}
	return Vf;
}

v_real reconstruct_fd(const v_pint& Xpcs, const v_real& Waveform, const  v_real& Envelope, pint n) {

	auto e = extend(Envelope);
	auto [a_el, b_el] = e.first;
	auto [a_er, b_er] = e.second;

	auto g = extend(Xpcs);
	auto [a_xl, b_xl] = g.first;
	auto [a_xr, b_xr] = g.second;

	v_inte Xpcs_r(Xpcs.begin(), Xpcs.end());
	v_real Enve_r(Envelope.begin(), Envelope.end());
	pint samples = 3;

	inte i{ -1 };
	while (Xpcs_r[0] > 0) { // left
		Xpcs_r.insert(Xpcs_r.begin(), a_xl * real(i) + b_xl);
		Enve_r.insert(Enve_r.begin(), std::max(0.0, a_el * real(i) + b_el));
		i--;
	}

	i = 0;
	while (Xpcs_r.back() < n) { // right
		Xpcs_r.push_back(a_xr * real(Xpcs.size() + i) + b_xr);
		Enve_r.push_back(std::max(0.0, a_er * real(Envelope.size() + i) + b_er));
		i++;
	}

	//v_real X(Xpcs_r.size() - 1, 0.0);
	//v_real D(Xpcs_r.size() - 1, 0.0);
	//X[0] = Xpcs_r[0];// real(Xpcs_r[1] + Xpcs_r[0]) / 2.0;
	//X[X.size() - 1] = Xpcs_r[Xpcs_r.size() - 1];// real(Xpcs_r[Xpcs_r.size() - 1] + Xpcs_r[Xpcs_r.size() - 2]) / 2.0;
	//D[0] = Enve_r[0];

	//for (size_t i = 1; i < X.size() - 1; i++) {
	//	X[i] = real(Xpcs_r[i + 1] + Xpcs_r[i]) / 2.0;
	//	D[i - 1] = (Enve_r[i] + Enve_r[i - 1]) / real(X[i] + X[i - 1]);
	//}
	//auto ex = boost::math::interpolators::cubic_hermite(std::move(X), std::move(Enve_r), std::move(D));


	pint nn = Xpcs_r.back() - Xpcs_r[0];
	v_real W(nn, 0.0);
	//inte x0{ 0 };
	//inte x1{ 0 };

	real step = 1.0 / (real)Waveform.size();
	boost::math::interpolators::cardinal_cubic_b_spline<real> spline(Waveform.begin(), Waveform.end(), 0.0, step);

	real half_step{ 0.0 }; // todo refine half step
	if (Waveform[0] < 0) {
		half_step = -0.5;
	} else {
		half_step = 0.5;
	}

	//std::unordered_map<pint, v_real> waveforms;
	//for (pint i = 0; i < Xpcs_r.size() - 1; i++) {
	//	inte x0 = Xpcs_r[i];
	//	inte x1 = Xpcs_r[i + 1];
	//	pint p = x1 - x0;
	//	if (waveforms.count(p) > 0) { // map contains waveform with period p
	//		for (inte j = x0; j < x1; j++) {
	//			W[j - Xpcs_r[0]] = waveforms[p][j - x0] * Enve_r[i];// ex(j);
	//		}
	//	}
	//	else {
	//		step = 1.0 / real(x1 - x0);
	//		//v_real wav_p(p);
	//		waveforms.insert(std::make_pair(p, v_real(p)));
	//		for (inte j = x0; j < x1; j++) {
	//			waveforms[p][j - x0] = spline((j - x0 + half_step) * step);
	//			W[j - Xpcs_r[0]] = waveforms[p][j - x0] * Enve_r[i];// ex(j);
	//		}
	//		
	//	}
	//}

	for (pint i = 0; i < Xpcs_r.size() - 1; i++) {
		inte x0 = Xpcs_r[i];
		inte x1 = Xpcs_r[i + 1];
		pint p = x1 - x0;
		step = 1.0 / real(x1 - x0);
		for (inte j = x0; j < x1; j++) {
			W[j - Xpcs_r[0]] = spline((j - x0 + half_step) * step) * Enve_r[i];// ex(j);
		}		
	}

	//v_real V(n, 0.0);
	//for (size_t i = 0; i < n; i++)	{ // todo
	//	V[i] = (W[i - Xpcs_r[0] - 1] + 2 * W[i - Xpcs_r[0]] + 3 * W[i - Xpcs_r[0] + 1]) / 6.0;
	//}


	v_real V(W.begin() - Xpcs_r[0], W.begin() - Xpcs_r[0] + n);

	#ifdef DEBUG
		//write_vector(W, "original_recreated_signal.csv");
		write_vector(Envelope, "Envelope_before_extrapolation.csv");
		write_vector(Xpcs, "Xpcs_before_extrapolation.csv");
		write_vector(Xpcs_r, "Xpcs_after_extrapolation.csv");
		//write_vector(Enve_r, "Envelope_after_extrapolation.csv");
		write_vector(V, "cropped_recreated_signal.csv");
	#endif

	return V;
}

std::tuple<v_real, v_real> average_waveform_and_envelope(const v_pint& Xp, const v_real& W) {
	v_pint T(Xp.size() - 1);
	for (pint i = 0; i < Xp.size() - 1; ++i) {
		T[i] = Xp[i + 1] - Xp[i];
	}

	pint mode = get_mode(T);

	inte x0{ 0 };
	inte x1{ 0 };
	real step{ 1.0 / real(mode) };
	v_real Waveform(mode, 0.0);
	v_real Envelope;
	//std::fill(Waveform.begin(), Waveform.end(), 0.0);
	real amp;
	//#pragma loop(hint_parallel(4))
	for (pint i = 0; i < Xp.size() - 1; i++) {
		x0 = Xp[i];
		x1 = Xp[i + 1];
		amp = std::abs(*std::max_element(W.begin() + x0, W.begin() + x1, abs_compare));

		#ifdef DEBUG
			if (amp >= 1.0)	{
				std::cout << "Envelope >= 1.0\n";
			}
		#endif // v

		Envelope.push_back(amp);
		if (x1 - x0 > 5) {
			boost::math::interpolators::cardinal_cubic_b_spline<real> spline(W.begin() + x0, W.begin() + x1, 0, 1.0 / real(x1 - x0));
			for (pint j = 0; j < mode; j++) {
				Waveform[j] += spline(j * step);
			}
		}
	}
	amp = std::abs(*std::max_element(Waveform.begin(), Waveform.end(), abs_compare));
	for (pint i = 0; i < mode; i++) {
		Waveform[i] = Waveform[i] / amp;
		#ifdef DEBUG
			if (abs(Waveform[i]) > 1.0) {
				std::cout << "|Waveform| > 1.0 at i="<<i<< "(" << Waveform[i] <<")\n";
			}
		#endif // v
	}

	return {Waveform, Envelope};
}

v_pint compress_fd(const v_real& W) {

	auto fpa = rfft(W);

	pint T{ 0 };
	if (fpa[0] > 0) {
		T = round((float)W.size() / (float)fpa[0]);
	}
	else {
		std::cout << "Warning: this signal doesn't seem periodic!"; // text
		T = 0.1 * W.size();
	}

	v_real conv(6 * T, 0.0);
	real conv_val{ 0.0 };
	real best_val{ 0.0 };
	real best_offset{ 0.0 };
	for (size_t offset = T / 2; offset < 6 * T; offset++) {
		conv_val = 0.0;
		for (size_t i = 0; i < W.size() - offset; i++) {
			conv_val += W[i] * W[i + offset];
		}
		conv[offset] = conv_val;
		if (conv_val > best_val) {
			best_val = conv_val;
			best_offset = offset;
		}
	}

	T = best_offset;

	pint x0{ 0 };
	while (round(W[x0] * 254) <= 0.0 && x0 < W.size()) {
		x0++;
	}

	pint n{ W.size() - 1 };
	while (round(W[n] * 254) <= 0.0 && n > 0) {
		n--;
	}
		
	v_pint Xpcs;
	#ifdef DEBUG
		v_real phases;
		v_real amplitudes;
	#endif // v

	real p{ 0.0 };
	float best_p{ 0.0 };
	pint best_x{ 0 };
	bool inside_peak = false;
	while (x0 + T <= n) {

		fpa = rfft(W.begin() + x0, W.begin() + x0 + T);

		//adjust = 1;
		//while (fpa[0] > 1) {
		//	fpa = rfft(W.begin() + x0, W.begin() + x0 + T - adjust);
		//	adjust++;
		//}
		//adjust = 1;
		//while (fpa[0] <= 0 && x0 + T + adjust < W.size()) {
		//	fpa = rfft(W.begin() + x0, W.begin() + x0 + T + adjust);
		//	adjust++;
		//}

		#ifdef DEBUG	
			phases.push_back(fpa[1]);
			amplitudes.push_back(fpa[2]);
		#endif // v

		if (std::pow(fpa[1], 3.0) > 25.0) {
			if (fpa[1] > best_p){
				best_p = fpa[1];
				best_x = x0;
			}
			inside_peak = true;
		} else {
			if (inside_peak) {
				Xpcs.push_back(best_x);
				best_p = 0.0;
				x0 = best_x + round(0.8 * (float)T);
			}
			inside_peak = false;
		}
		x0++;
	}

	#ifdef DEBUG
		write_vector(conv, "conv.csv");
		write_vector(phases, "phases.csv");
		write_vector(amplitudes, "amplitudes.csv");
	#endif // v

	return Xpcs;
}

template <typename T> 
int sign(T val) {
	return (T(0) < val) - (val < T(0));
}

void adjust_xpcs(v_pint& Xpcs, const v_real& W) {
	std::vector<v_pint> D(Xpcs.size() - 1);
	real avg_D{ 0.0 };
	real avg_T{ 0.0 };
	for (size_t i = 0; i < Xpcs.size() - 1; i++) {
		pint x0 = Xpcs[i];
		pint x1 = Xpcs[i + 1];
		avg_T += (x1 - x0);
		pint best_x{ 0 };
		real min_distance{ 10.0 }; // arbitrary value > 2.0
		for (size_t x = x0; x < x1; x++) {
			real distance = std::abs(W[x]) + std::abs(W[x + 1]);
			if (distance < min_distance) {
				min_distance = distance;
				best_x = x;
			}

			if (sign(W[x]) != sign(W[x + 1])) {
				real d = x - x0;
				avg_D += d;
				D[i].push_back(d);
			}
		}

		if (D[i].size() <= 0) {
			real d = best_x - x0;
			avg_D += d;
			D[i].push_back(d);
		}
	}	
	auto D_flat = flatten(D);
	avg_D /= real(D_flat.size());
	avg_T /= real(Xpcs.size() - 1);
	pint mode_D = get_mode(D_flat);

	v_pint adjusted_Xpcs(Xpcs.size() - 1);
	real average_ratio{ avg_D / avg_T };
	real shift{ 0.0 };
	for (size_t i = 0; i < Xpcs.size() - 1; i++) { // todo
		pint x0 = Xpcs[i] + round(shift);
		pint x1 = Xpcs[i + 1] + round(shift);
		real t = x1 - x0;
		real best_error{ 2.0 }; // todo
		pint best_x0{ 0 };
		for (auto d : D[i]) {
			inte dist = round(real(d) - shift);
			real ratio = real(dist) / t;
			real error = std::abs(average_ratio - ratio);
			if (error <= best_error) {
				best_error = error;
				best_x0 = x0 + dist;
			}
		}
		adjusted_Xpcs[i] = best_x0;
		shift += real(best_x0) - (real(x0) + t * average_ratio);
	}

	std::sort(adjusted_Xpcs.begin(), adjusted_Xpcs.end());
	auto it = std::unique(adjusted_Xpcs.begin(), adjusted_Xpcs.end());
	adjusted_Xpcs.resize(std::distance(adjusted_Xpcs.begin(), it));

	while (adjusted_Xpcs.back() > W.size())	{
		adjusted_Xpcs.pop_back();
	}

	#ifdef DEBUG
		write_vector(Xpcs, "Xpcs_before_adjustment.csv");
	#endif // DEBUG

	Xpcs = std::move(adjusted_Xpcs);

	#ifdef DEBUG
		write_vector(Xpcs, "Xpcs_after_adjustment.csv");
		std::cout << "avg=" << avg_D << " mode=" << mode_D << " average period=" << avg_T << "\n";
	#endif // DEBUG
}

void write_bin(std::string path, pint n, pint fps, const v_pint& Xpcs, const v_real& Waveform, const v_real& Envelope) {

	std::ofstream data_file;      // pay attention here! ofstream

	std::vector<unsigned short> T(Xpcs.size());
	T[0] = Xpcs[0];
	for (size_t i = 1; i < Xpcs.size(); i++) {
		T[i] = Xpcs[i] - Xpcs[i - 1];
		#ifdef DEBUG
			if (T[i] > std::numeric_limits<unsigned short>::max()) {
				std::cout << "Loss of data when writing to binary format:" << T[i] << " > " << std::numeric_limits<unsigned short>::max() <<"\n";
				//max_t = T_pint[i];
			} 
			else if (T[i] < 0) {
				std::cout << "Loss of data when writing to binary format:" << T[i] << " < 0 \n" ;
				//min_t = T_pint[i];
			}	
		#endif // DEBUG
	}

	v_pint pint_data = { n, fps, Envelope.size(), Waveform.size() }; // header

	std::vector<char> wv(Waveform.size());
	for (size_t i = 0; i < Waveform.size(); i++) {
		wv[i] = round(Waveform[i] * 126.0);
	}

	std::vector<unsigned char> en(Envelope.size());
	for (size_t i = 0; i < Envelope.size(); i++) {
		en[i] = round(Envelope[i] * 254.0);
	}

	data_file.open(path, std::ios::out | std::ios::binary | std::fstream::trunc);
	data_file.write((char*)&pint_data[0], pint_data.size() * sizeof(pint));
	data_file.write((char*)&T[0], T.size() * sizeof(unsigned short));
	data_file.write((char*)&wv[0], wv.size() * sizeof(char));
	data_file.write((char*)&en[0], en.size() * sizeof(unsigned char));
	data_file.close();

	#ifdef DEBUG
		//std::cout << "Written min t:" << min_t << " Written max t:" << max_t << "\n";
		write_vector(T, "written_Periods.csv");
		write_vector(Waveform, "written_Waveform.csv");
		write_vector(Envelope, "written_Envelope.csv");
		write_vector(Xpcs, "written_XPC.csv");
	#endif // v
}

Wav read_bin(std::string path) {
	std::ifstream  data_file;
	data_file.open(path, std::ios::in | std::ios::binary);

	pint* header = new pint[4];
	data_file.read(reinterpret_cast<char*>(&header[0]), 4 * sizeof(pint));

	std::cout << "Decompressing "<<path << " with n=" << header[0] << " and fps=" << header[1] << "\n"; // text

	unsigned short* Xpcs_buffer = new unsigned short[header[2] + 1];
	data_file.read(reinterpret_cast<char*>(&Xpcs_buffer[0]), (header[2] + 1) * sizeof(unsigned short));

	v_pint Xpcs(header[2] + 1);
	//v_pint T(Xpcs_buffer, Xpcs_buffer + Xpcs.size());
	Xpcs[0] = Xpcs_buffer[0];
	for (size_t i = 1; i < Xpcs.size(); i++) {
		Xpcs[i] = Xpcs[i - 1] + Xpcs_buffer[i];
	}
	delete[] Xpcs_buffer;

	char* waveform_buffer = new char[header[3]];
	data_file.read((char*)&waveform_buffer[0], (header[3]) * sizeof(char));
	v_real Waveform(header[3]);
	for (size_t i = 0; i < header[3]; i++) {
		Waveform[i] = real(waveform_buffer[i]) / 126.0;
	}
	delete[] waveform_buffer;

	unsigned char* envelope_buffer = new unsigned char[header[2]];
	data_file.read(reinterpret_cast<char*>(&envelope_buffer[0]), header[2] * sizeof(unsigned char));
	v_real Envelope(header[2]);
	for (size_t i = 0; i < header[2]; i++) {
		Envelope[i] = real(envelope_buffer[i]) / 254.0;
	}
	delete[] envelope_buffer;

	data_file.close();

	#ifdef DEBUG
		//std::cout << "Read min t:" << min_t << " Read max t:" << max_t << "\n";
		//write_vector(Xpcs_buffer, "read_periods.csv");
		write_vector(Xpcs, "read_XPC.csv");
		write_vector(Waveform, "read_Waveform.csv");
		write_vector(Envelope, "read_Envelope.csv");
	#endif // v

	// header[0] = n, header[1] = fps
	return Wav(reconstruct_fd(Xpcs, Waveform, Envelope, header[0]), header[1]);
}

v_real get_residue(const v_real& W0, const v_real& W1) {
	v_real R(W0.size(), 0.0);
	for (pint i = 0; i < R.size(); i++)	{
		R[i] = W0[i] - W1[i];
	}
	return R;
}

inline bool ends_with(std::string const& value, std::string const& ending) {
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void compress(std::string inpath, std::string outpath, double q, double qxy = 0.1, int kx=3, int ky=3) {

	Wav WV = read_wav(inpath);
	v_pint Xpcs = compress_fd(WV.W);
	adjust_xpcs(Xpcs, WV.W);

	pint n_inner = Xpcs.back() - Xpcs[0];
	std::vector<double> X;
	std::vector<double> Y;
	std::vector<double> Z;

	int p = Xpcs.size() - 1;
	int m = 0;
	std::vector<unsigned short> T(Xpcs.size());
	T[0] = Xpcs[0];
	for (size_t i = 0; i < p; i++) {
		auto t = Xpcs[i + 1] - Xpcs[i];
		T[i + 1] = t;
		if (t > m) {
			m = t;
		}
		double xx = double(i + 1) / double(p);
		for (size_t j = Xpcs[i]; j <= Xpcs[i + 1]; j++) {
			X.push_back(xx);
			Y.push_back(double(j - Xpcs[i]) / double(t));
			Z.push_back(WV.W[j]);
		}
	}

	int n = WV.W.size();
	int nxmax = (kx * ky + kx + ky + n - p + 1) / (ky + 1);

	int nx = 2 * kx + 2 + round(qxy * double(nxmax - (2 * kx + 2))); // number of knots in the x axis
	int ny = (kx * ky + kx - ky * nx + ky - n + p - nx + 1) / (kx - nx + 1);

	nx = 2 * kx + 2 + q * (std::min(nx, p) - (2 * kx + 2));
	ny = 2 * ky + 2 + q * (std::min(ny, m) - (2 * ky + 2));

	std::cout << "n=" << n << " rate=" << double(WV.W.size()) / double(p + (nx - kx - 1) * (ny - ky - 1)) << " x0=" << Xpcs[0] << " x1=" << Xpcs.back() << " p=" << p << " m=" << m << " nx=" << nx << " ny=" << ny << "\n";

	//return 0;

	int nmax = std::max(nx, ny);
	std::vector<double> tx(nmax, 0.0);                        // X knots
	std::vector<double> ty(nmax, 0.0);                         // Y knots


	for (size_t i = nx - kx - 1; i < nx; i++) {
		tx[i] = 1.0;
	}
	for (size_t i = ny - ky - 1; i < ny; i++) {
		ty[i] = 1.0;
	}

	double dx = 1.0 / double(nx - 2 * kx);
	for (size_t i = kx + 1; i < nx - kx - 1; i++) {
		tx[i] = double(i - kx) * dx;
		//std::cout << tx[i] << "\n";
	}
	//std::cout  << "\n";

	double dy = 1.0 / double(ny - 2 * ky);
	for (size_t i = ky; i < ny - ky - 1; i++) {
		ty[i] = double(i - ky) * dy;
	}

	//std::vector<double> Z(WV.W.begin() + Xpcs[0], WV.W.begin() + Xpcs.back());

	write_vector(X, "X.csv");
	write_vector(Y, "Y.csv");
	write_vector(Z, "Z.csv");
	write_vector(tx, "Tx_b.csv");
	write_vector(ty, "Ty_b.csv");
	std::vector<double> C = Fit_Bspline_Surface(X.size(), &X[0], &Y[0], &Z[0], nx, ny, kx, ky, &tx[0], &ty[0]);

	std::vector<float> CC(C.begin(), C.end());
	write_vector(C, "C.csv");
	tx.resize(nx);
	ty.resize(ny);
	write_vector(tx, "Tx.csv");
	write_vector(ty, "Ty.csv");

	std::vector<int> P = { p, nx, ny, int(WV.fps) };
	std::ofstream data_file;      // pay attention here! ofstream
	data_file.open(outpath, std::ios::out | std::ios::binary | std::fstream::trunc);
	data_file.write((char*)&P[0], P.size() * sizeof(int));
	data_file.write((char*)&T[0], T.size() * sizeof(unsigned short));
	data_file.write((char*)&CC[0], CC.size() * sizeof(float));
	data_file.close();
}

void decompress(std::string inpath, std::string outpath, int kx = 3, int ky = 3) {

	std::ifstream  data_file;
	data_file.open(inpath, std::ios::in | std::ios::binary);

	std::vector<int> H(4);
	data_file.read(reinterpret_cast<char*>(&H[0]),  4 * sizeof(int));
	int p = H[0];
	int nx = H[1];
	int ny = H[2];
	int fps = H[3];

	std::vector<unsigned short> T(p + 1);
	data_file.read(reinterpret_cast<char*>(&T[0]), (p + 1) * sizeof(unsigned short));

	int lc = (nx - kx - 1) * (ny - ky - 1);
	std::vector<float> C_buffer(lc);
	data_file.read(reinterpret_cast<char*>(&C_buffer[0]), lc * sizeof(float));

	std::vector<double> C(C_buffer.begin(), C_buffer.end());

	std::cout << "p=" << p << " nx=" << nx << " ny=" << ny << " fps=" << fps << "\n";

	std::vector<double> tx(nx, 0.0);                        // X knots
	std::vector<double> ty(ny, 0.0);                         // Y knots

	for (size_t i = nx - kx - 1; i < nx; i++) {
		tx[i] = 1.0;
	}
	for (size_t i = ny - ky - 1; i < ny; i++) {
		ty[i] = 1.0;
	}

	double dx = 1.0 / double(nx - 2 * kx);
	for (size_t i = kx + 1; i < nx - kx - 1; i++) {
		tx[i] = double(i - kx) * dx;
		//std::cout << tx[i] << "\n";
	}
	//std::cout  << "\n";

	double dy = 1.0 / double(ny - 2 * ky);
	for (size_t i = ky; i < ny - ky - 1; i++) {
		ty[i] = double(i - ky) * dy;
	}

	std::vector<double> Z_eval;
	for (size_t i = 0; i < p; i++) {
		int t = T[i];
		double x = double(i + 1) / double(p);
		std::vector<double> z = Eval_Bspline_Surface(&C[0], nx, ny, kx, ky, x, t, &tx[0], &ty[0]);
		Z_eval.insert(Z_eval.end(), z.begin(), z.end());
	}
	Wav Rec(Z_eval,fps);
	Rec.write(outpath);
}


void print_help() {
	std::cout
		<< "char: " << sizeof(char) << " byte, " << (int)std::numeric_limits<char>::min() << " to " << (int)std::numeric_limits<char>::max() << "\n"
		<< "unsigned char: " << sizeof(unsigned char) << " byte, " << (int)std::numeric_limits<unsigned char>::min() << " to " << (int)std::numeric_limits<unsigned char>::max() << "\n"
		<< "short: " << sizeof(short) << " bytes, " << std::numeric_limits<short>::min() << " to " << std::numeric_limits<short>::max() << "\n"
		<< "unsigned short: " << sizeof(unsigned short) << " bytes, " << std::numeric_limits<unsigned short>::min() << " to " << std::numeric_limits<unsigned short>::max() << "\n"
		<< "Size of int: " << sizeof(int) << " bytes\n"
		<< "Size of float: " << sizeof(float) << " bytes\n"
		<< "Size of double: " << sizeof(double) << " bytes\n\n"
		<< "For more info about the author: carlos-tarjano.web.app\n"
		<< "Mono WAV files are the only format supported currently\n\n"
		<< "Usage: \n"
		<< " -[a string] [-csv] [path/to/file_1.wav]...[path/to/file_n.wav]  [path/to/file_1" << HCEXT << "]...[path/to/file_m" << HCEXT << "]\n"
		<< " -a or --append: (default \"" << APPEND << "\") string to be appended to each reconstructed file name\n"
		<< " -q or --quality: 0.0 <= q <= 1.0. If supplied, " << PCEXT << " mode is used, with quality q\n"
		<< " -csv: flag to turn on saveing csv files with the beginning of each pseudo cycle, envelope and average waveform\n"
		<< " If no path is given, the root folder will be scanned for .wav and " << HCEXT << " files, and those will be processed accordingly\n";


}

//int main(int argc, char** argv) {
//
//
//}

int main(int argc, char** argv) {
	bool save_csv = false;
	bool pc_mode = false;
	std::vector<std::string> wav_paths;
	std::vector<std::string> hc_paths;
	std::vector<std::string> pc_paths;
	double Q = 0.3;
	double QXY = 0.1;
	
	for (int i = 1; i < argc; ++i) { // parsing args
		if (std::strcmp(argv[i], "-h") == 0 || std::strcmp(argv[i], "--help") == 0) {
			print_help();
			return 0;
		}
		else if (std::strcmp(argv[i], "-a") == 0 || std::strcmp(argv[i], "--append") == 0) {
			APPEND = argv[i + 1];
			std::cout << "Append \"" << APPEND << "\" to the name of reconstructed files\n";
			i++;
		}
		else if (std::strcmp(argv[i], "-q") == 0 || std::strcmp(argv[i], "--quality") == 0) {
			Q = atof(argv[i + 1]);
			pc_mode = true;
			std::cout << PCEXT << " mode, with quality=" << Q << "\n";
			i++;
		}
		else if (std::strcmp(argv[i], "-qxy") == 0) {
			QXY = atof(argv[i + 1]);
			pc_mode = true;
			std::cout << PCEXT << " mode, with x vs y ratio=" << QXY << "\n";
			i++;
		}
		else if (std::strcmp(argv[i], "-csv") == 0) {
			save_csv = true;
			std::cout << "Info will be saved as csv\n";
		}
		else if (ends_with(argv[i], ".wav")) {
			wav_paths.push_back(argv[i]);
		}
		else if (ends_with(argv[i], HCEXT)) {
			hc_paths.push_back(argv[i]);
		}
		else if (ends_with(argv[i], PCEXT)) {
			pc_paths.push_back(argv[i]);
		}
	}

	if (wav_paths.empty() && hc_paths.empty() && pc_paths.empty()) { // no files found in args, searching root
		for (const auto& entry : std::filesystem::directory_iterator("./")) {
			std::string path = { entry.path().u8string() };
			if (ends_with(path, ".wav")) {
				wav_paths.push_back(path);
			}
			else if (ends_with(path, HCEXT)) {
				hc_paths.push_back(path);
			}
			else if (ends_with(path, PCEXT)) {
				pc_paths.push_back(path);
			}
		}
	}

	if (pc_mode) {
		for (auto path : wav_paths) { // COMPRESSING PC
			std::string inpath = path;
			path.replace(path.end() - 4, path.end(), PCEXT);
			compress(inpath, path, Q, QXY);
		}

	} else {
		for (auto path : wav_paths) { // COMPRESSING HC
			std::cout << "\nCompressing " << path << "\n";
			Wav WV = read_wav(path);

			v_pint Xpcs = compress_fd(WV.W);

			adjust_xpcs(Xpcs, WV.W);

			auto [Waveform, Envelope] = average_waveform_and_envelope(Xpcs, WV.W);

			#ifdef DEBUG
				write_vector(Waveform, "waveform.csv");
			#endif // DEBUG

			path.replace(path.end() - 4, path.end(), HCEXT);

			write_bin(path, WV.W.size(), WV.fps, Xpcs, Waveform, Envelope);

			if (save_csv) {
				path.replace(path.end() - 4, path.end(), ".csv");
				write_vector(Xpcs, path);
			}

			#ifdef DEBUG
				Wav Rec(reconstruct_fd(Xpcs, Waveform, Envelope, WV.W.size()), WV.fps);
				Rec.write(path.replace(path.end() - 4, path.end(), "_rec.wav"));
				Wav Res(get_residue(WV.W, Rec.W), WV.fps);
				Res.write(path.replace(path.end() - 8, path.end(), "_res.wav"));
			#endif // v
		}
	}

	for (auto path : hc_paths) { // DECOMPRESSING HC
		//std::cout << "\nDecompressing " << path << "\n";
		#ifdef TIME
			auto time = Chronograph();
		#endif
		auto WW = read_bin(path);
		#ifdef TIME
			time.stop("Read & decompressed " + path + " in ");
			auto time3 = Chronograph();
		#endif
		path.replace(path.end() - HCEXT.size(), path.end(), APPEND + ".wav");
		WW.write(path);
		#ifdef TIME
			time3.stop("Wrote " + path + " in ");
		#endif
	}

	for (auto path : pc_paths) { // DECOMPRESSING PC
		std::string inpath = path;
		path.replace(path.end() - HCEXT.size(), path.end(), APPEND + ".wav");
		decompress(inpath, path);
	}

	return 0;
}

//v_real center(v_real::const_iterator beg, v_real::const_iterator end) {
//	v_real W(beg, end);
//	float average{ 0.0 };
//	for (size_t i = 0; i < W.size(); i++) {
//		average += W[i];
//	}
//	average /= (float)W.size();
//	for (size_t i = 0; i < W.size(); i++) {
//		W[i] -= average;
//	}
//	return W;
//}

//v_real reconstruct_fd(const v_pint& Xpcs, const v_real& Waveform, const  v_real& Envelope, pint n) {
//
//	v_inte Xpcs_r(Xpcs.begin(), Xpcs.end());
//	v_real Enve_r(Envelope.begin(), Envelope.end());
//	pint samples = 3;
//
//	while (Xpcs_r[0] > 0) { // left
//		v_inte x(Xpcs_r.begin(), Xpcs_r.begin() + samples);
//		std::reverse(x.begin(), x.end());
//		Xpcs_r.insert(Xpcs_r.begin(), (inte)extrapolate(x));
//
//		v_real e(Enve_r.begin(), Enve_r.begin() + samples);
//		std::reverse(e.begin(), e.end());
//		Enve_r.insert(Enve_r.begin(), std::max(0.0, extrapolate(e)));
//	}
//
//	while (Xpcs_r.back() < n) { // right
//		v_inte x(Xpcs_r.end() - samples, Xpcs_r.end());
//		Xpcs_r.push_back((inte)extrapolate(x));
//
//		v_real e(Enve_r.end() - samples, Enve_r.end());
//		Enve_r.push_back(std::max(0.0, extrapolate(e)));
//	}
//
//	pint nn = Xpcs_r.back() - Xpcs_r[0];
//	v_real W(nn, 0.0);
//	inte x0{ 0 };
//	inte x1{ 0 };
//	real step;
//	boost::math::interpolators::cardinal_cubic_b_spline<real> spline(Waveform.begin(), Waveform.end(), 0.0, 1.0 / (real)Waveform.size());
//
//	for (pint i = 0; i < Xpcs_r.size() - 1; i++) {
//		x0 = Xpcs_r[i];
//		x1 = Xpcs_r[i + 1];
//		step = 1.0 / real(x1 - x0);
//		for (inte j = x0; j < x1; j++) {
//			W[j - Xpcs_r[0]] = spline((j - x0) * step) * Enve_r[i];
//		}
//	}
//
//	v_real V(W.begin() - Xpcs_r[0], W.begin() - Xpcs_r[0] + n);
//
//	#ifdef v
//		write_vector(W, "W.csv");
//		write_vector(Envelope, "Envelope_0.csv");
//		write_vector(Xpcs_r, "Xpcs_r.csv");
//		write_vector(Enve_r, "Envelope_r.csv");
//		write_vector(V, "V.csv");
//	#endif // v
//
//	return V;
//}

//void adjust_xpcs(v_pint& Xpcs, const v_real& W) {
//	std::vector<v_pint> L(Xpcs.size() - 1);
//	std::vector<v_pint> R(Xpcs.size() - 1);
//	pint x0{ 0 };
//	pint x1{ 0 };
//	real avg_L{ 0.0 };
//	real avg_R{ 0.0 };
//	real l, r;
//	for (size_t i = 0; i < Xpcs.size() - 1; i++) {
//		x0 = Xpcs[i];
//		x1 = Xpcs[i + 1];
//		real distance;
//		real min_distance{ 10.0 }; // arbitrary value > 2.0
//		for (size_t x = x0; x < x1; x++) {
//			distance = std::abs(W[x0]) + x0;
//
//			if (sign(W[x]) != sign(W[x + 1])) {
//				l = x - x0;
//				r = x1 - x;
//				avg_L += l;
//				avg_R += r;
//				L[i].push_back(l);
//				R[i].push_back(r);
//				//continue;
//			}
//
//		}
//	}
//	avg_L /= real(L.size());
//	avg_R /= real(R.size());
//	std::cout << "avg=" << std::min(avg_L, avg_L) << "\n";
//	auto Lf = flatten(L);
//	pint mode_L = get_mode(Lf);
//	auto Rf = flatten(L);
//	pint mode_R = get_mode(Rf);
//
//	if (mode_L <= Xpcs[0]) {
//		for (size_t i = 0; i < Xpcs.size(); i++) {
//			Xpcs[i] -= mode_L;
//		}
//#ifdef DEBUG
//		std::cout << "Shifted Xpcs to the left by " << mode_L << "\n";
//#endif // DEBUG
//
//	}
//	else if (mode_R + Xpcs.back() < W.size()) {
//		for (size_t i = 0; i < Xpcs.size(); i++) {
//			Xpcs[i] += mode_R;
//		}
//#ifdef DEBUG
//		std::cout << "Shifted Xpcs to the right by " << mode_R << "\n";
//#endif // DEBUG
//	}
//	else if (mode_L < mode_R) {
//		Xpcs.erase(Xpcs.begin());
//		for (size_t i = 0; i < Xpcs.size(); i++) {
//			Xpcs[i] -= mode_L;
//		}
//#ifdef DEBUG
//		std::cout << "Shifted Xpcs to the left by " << mode_R << " (removed previous Xpcs[0])\n";
//#endif // DEBUG
//	}
//	else {
//		Xpcs.pop_back();
//		for (size_t i = 0; i < Xpcs.size(); i++) {
//			Xpcs[i] += mode_R;
//		}
//#ifdef DEBUG
//		std::cout << "Shifted Xpcs to the right by " << mode_R << " (removed previous Xpcs[0])\n";
//#endif // DEBUG
//	}
//}
